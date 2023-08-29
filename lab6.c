#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void itoaSize(unsigned int size, unsigned char *result) // побитовый сдвиг. перевод из числа в массив символов
{
    for (int i = 0; i < 4; i++)
    {
        result[i] = (size << (i * 8)) >> 24;
    }
}
int atoiSize(unsigned char *size) // побитовый сдвиг. перевод из массива символов в число
{
    int result = 0;
    for (int i = 0; i < 4; i++)
    {
        result |= (size[i] << 8 * (3 - i));
    }
    return result;
}
int myCreate(FILE *fout, char *file)
{
    FILE *fin = fopen(file, "rb");
    if (fin == NULL)
    {
        fprintf(stderr, "%s", "ERROR: the file does not exist");
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    unsigned int sizeFile = ftell(fin);       // узнаем размер архивируемого файла
    unsigned int sizeNameFile = strlen(file); //длина названия файла
    unsigned char size[4];
    itoaSize(sizeFile, size);                       //
    fwrite(size, sizeof(char), 4, fout);            //
    itoaSize(sizeNameFile, size);                   // записываем в архив размер файла, размер названия и название архивируемого файла
    fwrite(size, sizeof(char), 4, fout);            //
    fwrite(file, sizeof(char), sizeNameFile, fout); //
    fseek(fin, 0, SEEK_SET);
    char buf[4096];
    for (int i = 0; i < (sizeFile / 4096); i++)       //
    {                                                 //
        fread(buf, sizeof(char), 4096, fin);          //
        fwrite(buf, sizeof(char), 4096, fout);        // перезаписываем данные файла в архив
    }                                                 //
    fread(buf, sizeof(char), sizeFile % 4096, fin);   //
    fwrite(buf, sizeof(char), sizeFile % 4096, fout); //
    fclose(fin);
    // remove(file);
    return 0;
}
int myList(char *arcFile)
{
    FILE *fout = fopen(arcFile, "rb");
    if (fout == NULL)
    {
        fprintf(stderr, "%s", "ERROR: the file does not exist");
        return 1;
    }
    fseek(fout, 0, SEEK_END);
    unsigned int arcSize = ftell(fout); // узнаем размер архива
    if (arcSize == 0)
    {
        printf("the archive is empty");
    }
    fseek(fout, 0, SEEK_SET);
    while (ftell(fout) < arcSize)
    {
        unsigned char size[4];
        fread(size, sizeof(char), 4, fout);
        unsigned int sizeFile = atoiSize(size);
        fread(size, sizeof(char), 4, fout);
        unsigned int sizeNameFile = atoiSize(size);
        for (int i = 0; i < sizeNameFile; i++) //печатаем названия хранимых архивом файлов
        {
            char symbol = fgetc(fout);
            printf("%c", symbol);
        }
        printf("\n");
        fseek(fout, sizeFile, SEEK_CUR);
    }
    return 0;
}
int myExtract(char *arcFile)
{
    FILE *fin = fopen(arcFile, "rb");
    if (fin == NULL)
    {
        fprintf(stderr, "%s", "ERROR: the file does not exist");
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    unsigned int arcSize = ftell(fin); // узнаем размер архива
    fseek(fin, 0, SEEK_SET);
    while (ftell(fin) < arcSize)
    {
        unsigned char size[4];
        fread(size, sizeof(char), 4, fin);
        unsigned int sizeFile = atoiSize(size);
        fread(size, sizeof(char), 4, fin);
        unsigned int sizeNameFile = atoiSize(size);
        char filename[sizeNameFile + 1];
        for (int i = 0; i < sizeNameFile; i++) //считываем название файла
        {
            char symbol = fgetc(fin);
            filename[i] = symbol;
        }
        filename[sizeNameFile] = '\0';
        char *file = filename;
        FILE *fout = fopen(filename, "wb");
        char buf[4096];
        for (int i = 0; i < (sizeFile / 4096); i++)       //
        {                                                 //
            fread(buf, sizeof(char), 4096, fin);          //
            fwrite(buf, sizeof(char), 4096, fout);        // перезаписываем данные файла
        }                                                 //
        fread(buf, sizeof(char), sizeFile % 4096, fin);   //
        fwrite(buf, sizeof(char), sizeFile % 4096, fout); //
        fclose(fout);
    }
    fclose(fin);
    // remove(arcFile);
    return 0;
}
int myExtractOneFile(char *arcFile, char *name)
{
    FILE *fin = fopen(arcFile, "rb");
    if (fin == NULL)
    {
        fprintf(stderr, "%s", "ERROR: the file does not exist");
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    unsigned int arcSize = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    while (ftell(fin) < arcSize)
    {
        unsigned char size[4];
        fread(size, sizeof(char), 4, fin);
        unsigned int sizeFile = atoiSize(size);
        fread(size, sizeof(char), 4, fin);
        unsigned int sizeNameFile = atoiSize(size);
        char filename[sizeNameFile + 1];
        for (int i = 0; i < sizeNameFile; i++)
        {
            char symbol = fgetc(fin);
            filename[i] = symbol;
        }
        filename[sizeNameFile] = '\0';
        char *file = filename;
        FILE *fout;
        if (strcmp(name, file) == 0)
        {
            fout = fopen(filename, "wb");
        }
        char buf[4096];
        for (int i = 0; i < (sizeFile / 4096); i++)
        {
            fread(buf, sizeof(char), 4096, fin);
            if (strcmp(name, file) == 0)
            {
                fwrite(buf, sizeof(char), 4096, fout);
            }
        }
        fread(buf, sizeof(char), sizeFile % 4096, fin);
        if (strcmp(name, file) == 0)
        {
            fwrite(buf, sizeof(char), sizeFile % 4096, fout);
            fclose(fout);
        }
    }
    fclose(fin);
    // remove(arcFile);
    return 0;
}
int main(int argc, char **argv)
{
    char *arcFile;
    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "--file") == 0)
        {
            i++;
            arcFile = argv[i];
            i++;
        }
        else if (strcmp(argv[i], "--create") == 0)
        {
            i++;
            FILE *fout = fopen(arcFile, "ab");
            if (fout == NULL)
            {
                fprintf(stderr, "%s", "ERROR: the file does not exist");
                return 1;
            }
            while (i < argc)
            {
                char *file = argv[i];
                myCreate(fout, file);
                i++;
            }
            fclose(fout);
            break;
        }
        else if (strcmp(argv[i], "--extract") == 0)
        {
            if (argc == 4)
            {
                myExtract(arcFile);
            }
            else
            {
                myExtractOneFile(arcFile, argv[i + 1]);
            }
            break;
        }
        else if (strcmp(argv[i], "--list") == 0)
        {
            myList(arcFile);
            break;
        }
    }
    return 0;
}