#include <stdio.h>
#include <string.h>

unsigned long long UINT_MAX = 4294967296;
typedef struct
{
    unsigned int list[32];
} uint1024_t;

uint1024_t uint1024_t_from_uint(unsigned int x)
{
    uint1024_t result;
    for (int i = 0; i < 31; i++)
    {
        result.list[i] = 0;
    }
    result.list[31] = x;
    return result;
}
uint1024_t add_op(uint1024_t x, uint1024_t y)
{
    uint1024_t result = uint1024_t_from_uint(0);
    for (int i = 31; i > 0; i--)
    {
        result.list[i] += x.list[i] + y.list[i];
        if ((result.list[i] < x.list[i]) && (result.list[i] < y.list[i]))
        {
            result.list[i - 1]++;
        }
    }
    result.list[0] += x.list[0] + y.list[0];
    return result;
}
uint1024_t subtr_op(uint1024_t x, uint1024_t y)
{
    uint1024_t result = uint1024_t_from_uint(0);
    for (int i = 31; i > 0; i--)
    {
        result.list[i] += x.list[i] - y.list[i];
        if (result.list[i] > x.list[i])
        {
            result.list[i - 1]--;
        }
    }
    result.list[0] += x.list[0] - y.list[0];
    return result;
}
uint1024_t mult_at_uint(uint1024_t x, unsigned y)
{
    uint1024_t result = uint1024_t_from_uint(0);
    for (int i = 31; i >= 0; i--)
    {
        unsigned long long xlong = x.list[i];
        unsigned long long ylong = y;
        unsigned long long multiply = xlong * ylong;
        result.list[i] += multiply % UINT_MAX;
        if (i != 0)
        {
            result.list[i - 1] += multiply / UINT_MAX;
        }
    }
    return result;
}
uint1024_t div_at_uint(uint1024_t x, unsigned y)
{
    uint1024_t result = uint1024_t_from_uint(0);
    unsigned long long remainder = 0;
    for (int i = 0; i < 32; i++)
    {
        result.list[i] = (remainder * UINT_MAX + x.list[i]) / y;
        remainder = (remainder * UINT_MAX + x.list[i]) % y;
    }
    return result;
}
unsigned mod_at_uint(uint1024_t x, unsigned y)
{
    uint1024_t result = uint1024_t_from_uint(0);
    unsigned long long remainder = 0;
    for (int i = 0; i < 32; i++)
    {
        result.list[i] = (remainder * UINT_MAX + x.list[i]) / y;
        remainder = (remainder * UINT_MAX + x.list[i]) % y;
    }
    return remainder;
}
uint1024_t mult_op(uint1024_t x, uint1024_t y)
{
    uint1024_t result = uint1024_t_from_uint(0);
    for (int i = 31; i >= 0; i--)
    {
        if (y.list[i] != 0)
        {
            uint1024_t temp = mult_at_uint(x, y.list[i]);
            int k = 31 - i;
            for (int j = k; j < 32; j++)
            {
                temp.list[j - k] = temp.list[j];
            }
            for (int j = 32 - k; j < 32; j++)
            {
                temp.list[j] = 0;
            }
            result = add_op(result, temp);
        }
    }
    return result;
}
void printf_value(uint1024_t x)
{
    char buf[1024] = {0};
    for (int i = 0; i < 1024; i++)
    {
        unsigned mod = mod_at_uint(x, 10);
        buf[1023 - i] = mod + '0';
        x = div_at_uint(x, 10);
    }
    unsigned start = 0;
    while (buf[start] == '0')
    {
        start++;
    }
    if (start == 1024)
    {
        printf("0");
    }
    for (int i = start; i < 1024; i++)
    {
        printf("%c", buf[i]);
    }
}
void scanf_value(uint1024_t *x)
{
    char buf[1024];
    scanf("%s", buf);
    *x = uint1024_t_from_uint(0);

    for (int i = 0; i < 1024 && buf[i] != '\0'; i++)
    {
        *x = mult_at_uint(*x, 10);
        uint1024_t temp = uint1024_t_from_uint(buf[i] - '0');
        *x = add_op(*x, temp);
    }
}
int main()
{
    uint1024_t a, b;
    printf("first number: ");
    scanf_value(&a);
    printf("second number: ");
    scanf_value(&b);
    printf("\nsum is: ");
    printf_value(add_op(a, b));
    printf("\nsub is: ");
    printf_value(subtr_op(a, b));
    printf("\nmul is: ");
    printf_value(mult_op(a, b));
    return 0;
}