#include "c_main.h"
#include <stdint.h>

int a = 5;

char *buffer_ptr = (char *)0xb8000;

int num_digits(int val)
{
    int n = val;   // variable declaration
    int count = 0; // variable declaration

    if (n == 0)
    {
        return 1;
    }

    while (n != 0)
    {
        n = n / 10;
        count++;
    }

    return count;
}

void print(char *char_ptr, int len)
{
    color_t colors;
    colors = (color_t){0, 1, 2, 4, 15};

    for (int i = 0; i < len; i++)
    {
        *(buffer_ptr) = *(char_ptr + i);
        *(buffer_ptr + 1) = colors.green;
        if (buffer_ptr + 2 > (char *)0xb8F00)
        {
            buffer_ptr = (char *)0xb8000;
        }
        else
        {
            buffer_ptr = buffer_ptr + 2;
        }
    }

    return;
}

void println(char *char_ptr, int len)
{
    print(char_ptr, len);

    buffer_ptr = (char*) (((((long)buffer_ptr + 80*2) - 0xb8000)/160)*2*80 + 0xb8000);
    if (buffer_ptr > (char *)0xb8F00)
    {
        buffer_ptr = (char *)0xb8000;
    }

    return;
}

void print_int(int intval)
{
    int val = intval;
    int n = num_digits(intval);
    char buf[n];
    for (int i = n; i > 0; i--)
    {
        char a = val % 10 + '0';
        buf[i - 1] = a;

        val /= 10;
    }
    print((char *)&buf, n);
}

void WRITE_PORT_USHORT(uint16_t index_port, uint16_t index)
{
    __asm__ __volatile__(
        "out %0, %1 \n\t"
        :
        : "a"(index), "dN"(index_port));
}

void BlBochsVbeWrite(uint16_t index, uint16_t value)
{

    WRITE_PORT_USHORT(VBE_DISPI_IOPORT_INDEX, index);
    WRITE_PORT_USHORT(VBE_DISPI_IOPORT_DATA, value);
}

void BlBochsSetMode(uint16_t xres, uint16_t yres, uint16_t bpp)
{

    BlBochsVbeWrite(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    BlBochsVbeWrite(VBE_DISPI_INDEX_XRES, xres);
    BlBochsVbeWrite(VBE_DISPI_INDEX_YRES, yres);
    BlBochsVbeWrite(VBE_DISPI_INDEX_BPP, bpp);
    BlBochsVbeWrite(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
}

void print_graphics()
{
    BlBochsSetMode(800, 600, 32);
    uint32_t *buffer_ptr = (uint32_t *)0xE0000000;
    const uint32_t red = 0x00FF00;

    for (int i = 0; i < 800 * 600; i++)
    {
        *(buffer_ptr + i) = red;
    }
}
