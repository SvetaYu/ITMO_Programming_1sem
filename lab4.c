#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef union TAG
{
    char buf[10];
    struct tag
    {
        unsigned char signature[3];
        unsigned char version[2];
        unsigned char flag[1];
        unsigned char size[4];
    };
} TAG;
typedef union FRAME
{
    char buf[10];
    struct frame
    {
        unsigned char id[4];
        unsigned char size[4];
        unsigned char flag[2];
    };
} FRAME;

unsigned int SIZE(unsigned char size[4])
{
    unsigned int result = 0;
    for (int i = 0; i < 4; i++)
    {
        result |= (size[3 - i] << (7 * i));
    }
    return result;
}
void SIZEBACK(unsigned int size, unsigned char *result)
{
    for (int i = 1; i < 4; i++)
    {
        result[i] = (size << (7 * i + 4)) >> 25;
    }
    result[0] = size << 4 >> 25;
}
int show(char *file)
{
    FILE *fin;
    fin = fopen(file, "rb");
    if (fin == NULL)
    {
        fprintf(stderr, "% s", "ERROR: the file does not exist");
        return 1;
    }
    TAG TAG;
    fread(TAG.buf, sizeof(char), 10, fin);
    printf("%.3sv2.%d\n", TAG.signature, TAG.version[0]);
    while (ftell(fin) <= SIZE(TAG.size))
    {
        FRAME FRAME;
        fread(FRAME.buf, sizeof(char), 10, fin);
        if (FRAME.id[0] == 0)
        {
            break;
        }
        printf("%4s: ", FRAME.id);
        unsigned char value[SIZE(FRAME.size)];
        fread(value, sizeof(char), SIZE(FRAME.size), fin);
        for (int i = 0; i < SIZE(FRAME.size); i++)
        {
            printf("%c", value[i]);
        }
        printf("\n");
    }
    fclose(fin);
    return 0;
}
int get(char *file, unsigned char *frameID)
{
    FILE *fin;
    fin = fopen(file, "rb");
    if (fin == NULL)
    {
        fprintf(stderr, "% s", "ERROR: the file does not exist");
        return 1;
    }
    int check = 0;
    TAG TAG;
    fread(TAG.buf, sizeof(char), 10, fin);
    while (ftell(fin) <= SIZE(TAG.size))
    {
        FRAME FRAME;
        fread(FRAME.buf, sizeof(char), 10, fin);
        if (FRAME.id[0] == 0)
        {
            break;
        }
        unsigned char value[SIZE(FRAME.size)];
        fread(value, sizeof(char), SIZE(FRAME.size), fin);
        if (strcmp(frameID, FRAME.id) == 0)
        {
            printf("%4s: ", FRAME.id);
            for (int i = 0; i < SIZE(FRAME.size); i++)
            {
                printf("%c", value[i]);
            }
            printf("\n");
            check = 1;
            // break;
        }
    }
    if (check == 0)
    {
        printf("frame not found");
        printf("\n");
    }
    fclose(fin);
    return 0;
}
int set(char *file, unsigned char *frameID, unsigned char *newvalue)
{
    TAG TAG;
    FRAME FRAME;
    FILE *fin;
    fin = fopen(file, "rb");
    if (fin == NULL)
    {
        fprintf(stderr, "% s", "ERROR: the file does not exist");
        return 1;
    }
    fseek(fin, 0, SEEK_SET);
    fread(TAG.buf, sizeof(char), 10, fin);
    unsigned int sizetag = SIZE(TAG.size);
    unsigned int position = -1;
    unsigned int sizeframe = 0;
    while (ftell(fin) <= SIZE(TAG.size))
    {

        fread(FRAME.buf, sizeof(char), 10, fin);
        if (FRAME.id[0] == 0)
        {
            break;
        }
        unsigned char value[SIZE(FRAME.size)];
        fread(value, sizeof(char), SIZE(FRAME.size), fin);
        if (memcmp(frameID, FRAME.id, 4) == 0)
        {
            position = ftell(fin) - 10 - SIZE(FRAME.size);
            sizeframe = SIZE(FRAME.size);
            sizetag = sizetag - sizeframe + (unsigned int)strlen(newvalue) + 1;
            break;
        }

        position = ftell(fin);
    }
    if (memcmp(frameID, FRAME.id, 4) == 1)
    {
        for (int i = 0; i < 4; i++)
        {
            FRAME.id[i] = frameID[i];
        }
        FRAME.flag[0] = 0;
        FRAME.flag[1] = 0;
        sizetag = sizetag + (unsigned int)strlen(newvalue) + 1;
    }
    SIZEBACK((unsigned int)strlen(newvalue) + 1, FRAME.size);
    SIZEBACK(sizetag, TAG.size);
    fseek(fin, 0, SEEK_END);
    long int end = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    FILE *fin2;
    fin2 = fopen("tempfile.mp3", "wb");
    while (ftell(fin2) <= end)
    {
        char symbol = fgetc(fin);
        fputc(symbol, fin2);
    }
    fclose(fin);
    fclose(fin2);
    fin = fopen(file, "wb");
    fin2 = fopen("tempfile.mp3", "rb");
    fwrite(TAG.buf, sizeof(char), 10, fin);
    fseek(fin2, 10, SEEK_CUR);
    while (ftell(fin2) < position)
    {
        char symbol = fgetc(fin2);
        fputc(symbol, fin);
    }
    fwrite(FRAME.buf, sizeof(char), 10, fin);
    fseek(fin2, 10, SEEK_CUR);
    char symbol = fgetc(fin2);
    fputc(symbol, fin);
    fwrite(newvalue, sizeof(char), strlen(newvalue), fin);
    fseek(fin2, sizeframe - 1, SEEK_CUR);
    end = end - sizeframe + (unsigned int)strlen(newvalue);
    while (ftell(fin) <= end)
    {
        char symbol = fgetc(fin2);
        fputc(symbol, fin);
    }
    fclose(fin);
    fclose(fin2);
    remove("tempfile.mp3");
    return 0;
}

int main(int argc, char **argv)
{
    if ((argc != 3) && (argc != 4))
    {
        fprintf(stderr, "% s", "ERROR: invalid number of arguments");
        return 1;
    }
    char *file;
    file = strpbrk(argv[1], "=") + 1;
    char *showstr = "show";
    char *getstr = "get";
    char *setstr = "set";
    if (strstr(argv[2], showstr) != NULL)
    {
        show(file);
    }
    else if (strstr(argv[2], getstr) != NULL)
    {
        if (strlen(argv[2]) == 10)
        {
            getstr = strpbrk(argv[2], "=") + 1;
            get(file, getstr);
        }
    }
    else if (strstr(argv[2], setstr) != NULL)
    {
        char *value;
        value = strpbrk(argv[3], "=") + 1;
        setstr = strpbrk(argv[2], "=") + 1;
        set(file, setstr, value);
    }
    else
    {
        fprintf(stderr, "% s", "ERROR: invalid argument");
        return 1;
    }
    return 0;
}