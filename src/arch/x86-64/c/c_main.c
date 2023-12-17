#include "c_main.h"
#include "font.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDRESS 0xB8000
#define BUFER_SIZE (VGA_WIDTH * VGA_HEIGHT * 2 * 10)  //10 pages

//framebuffer provided by grub during bootstrap
static uint32_t* framebuffer_ptr_g = NULL;
static uint32_t framebuffer_pitch_g;
static int is_uefi_bios_g = 0;

int a = 5;

static char *video_buffer_ptr = (char *)VGA_ADDRESS;
static char output_buffer[BUFER_SIZE] = {0};
static char *buffer_ptr = output_buffer;
static char *start_page_ptr = output_buffer;
static char *start_page_scroll_ptr = output_buffer;

void init_framebuffer(uint32_t* framebuffer_ptr, uint32_t framebuffer_pitch, int is_uefi_bios){
    framebuffer_ptr_g=framebuffer_ptr;
    framebuffer_pitch_g =framebuffer_pitch;
    is_uefi_bios_g=is_uefi_bios;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    if(n<=0 ){
        return dest;
    }
  
    // Cast src and dest to character pointers
    char *d = dest;
    const char *s = src;

    // Copy bytes one at a time
    while (n--)
    {
        *d++ = *s++;
    }

    // Return the original destination pointer
    return dest;
}

static inline uint64_t rdtsc(){
    unsigned int lo, hi;
    __asm__ __volatile__ (
      "rdtsc" : "=a" (lo), "=d" (hi)
    );
    return ((uint64_t)hi << 32) | lo;
}

void delay(uint64_t ticks) {
    uint64_t start = rdtsc();
    while (rdtsc() - start < ticks);
}

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

int num_digits_t16(uint16_t val)
{
    uint16_t n = val;   // variable declaration
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

void clear_screen()
{
    if(is_uefi_bios_g==0){
        unsigned char *video_buffer = video_buffer_ptr;
        // Iterate over each cell in the video buffer.
        for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        {
            // Set the character to space (ASCII code 32).
            *video_buffer++ = 32;
            // Set the attribute byte to light grey on black (0x07).
            *video_buffer++ = 0x07;
        }
    }
    
}


void write_to_vga(const char* data, size_t size) {
    volatile char* vga = (volatile char*)VGA_ADDRESS;
    for (size_t i = 0; i < size && i < VGA_WIDTH * VGA_HEIGHT * 2; ++i) {
        vga[i] = data[i];
    }
}

void write_to_frame_buffer(const char* data, size_t size){
    int write_pos = 1;

    int current_y = 10;
    for (size_t i = 0; i < size && i < VGA_WIDTH * VGA_HEIGHT * 2; ++i) {
        //size contains amount of bytes. Each char is 2 bytes: code+color
        if(i%2 == 0){
        char char_code = data[i];
        
        if(i%(VGA_WIDTH*2)==0){
            current_y += CHAR_HEIGHT+5;
            write_pos = 1;
        }
        if(char_code!=0){
            draw_character(framebuffer_ptr_g, framebuffer_pitch_g, (CHAR_WIDTH+2)*write_pos, current_y, COLOR_WHITE, (int)char_code);
        }
        
        write_pos++;
        }
        
    }
    
}

void write_to_display(const char* data, size_t size){
    if(is_uefi_bios_g!=0){
        write_to_frame_buffer(data, size);

    }else{
        write_to_vga(data, size);
    }

}

void display()
{
    int length;
    if (buffer_ptr == output_buffer)
    {
        start_page_ptr = output_buffer;
        length = VGA_WIDTH * VGA_HEIGHT * 2;
    }
    else if ((buffer_ptr - 2 - VGA_WIDTH * VGA_HEIGHT * 2) < output_buffer)
    {
        start_page_ptr = output_buffer;
        length = buffer_ptr - 2 - output_buffer;
    }
    else
    {
        start_page_ptr = buffer_ptr + (VGA_WIDTH * 2 - (buffer_ptr - output_buffer) % (VGA_WIDTH * 2)) - VGA_WIDTH * VGA_HEIGHT * 2;
        length = buffer_ptr - 2 - start_page_ptr;
    }
    delay(100000000);

    clear_screen();
    start_page_scroll_ptr = start_page_ptr;
    write_to_display(start_page_ptr, length);

}

void scrollUp()
{
    start_page_scroll_ptr = start_page_scroll_ptr - VGA_WIDTH * 2;
    if (start_page_scroll_ptr < output_buffer)
    {
        start_page_scroll_ptr = output_buffer;
    }

    int length = buffer_ptr - 2 - start_page_scroll_ptr;

    if ((buffer_ptr - 2 - start_page_scroll_ptr) > VGA_WIDTH * VGA_HEIGHT * 2)
    {
        length = VGA_WIDTH * VGA_HEIGHT * 2;
    }

    clear_screen();
    memcpy(video_buffer_ptr, start_page_scroll_ptr, length);
}

void scrollDown()
{
    start_page_scroll_ptr = start_page_scroll_ptr + VGA_WIDTH * 2;
    if (start_page_scroll_ptr > start_page_ptr)
    {
        start_page_scroll_ptr = start_page_ptr;
    }

    int length = buffer_ptr - 2 - start_page_scroll_ptr;

    if ((buffer_ptr - 2 - start_page_scroll_ptr) > VGA_WIDTH * VGA_HEIGHT * 2)
    {
        length = VGA_WIDTH * VGA_HEIGHT * 2;
    }

    clear_screen();
    memcpy(video_buffer_ptr, start_page_scroll_ptr, length);
}

int num_digits_hex(int val)
{
    int n = val;   // variable declaration
    int count = 0; // variable declaration

    if (n == 0)
    {
        return 1;
    }

    while (n != 0)
    {
        n = n / 16;
        count++;
    }

    return count;
}

int num_digits_hex_t16(uint16_t val)
{
    uint16_t n = val;   // variable declaration
    int count = 0; // variable declaration

    if (n == 0)
    {
        return 1;
    }

    while (n != 0)
    {
        n = n / 16;
        count++;
    }

    return count;
}


void print(char *char_ptr, int len)
{
    color_t colors;
    colors = (color_t){0, 1, 2, 4, 15};

    int length = 0;
    while (char_ptr[length] != '\0')
    {
        length++;
    }

    for (int i = 0; i < length; i++)
    {
        *(buffer_ptr) = *(char_ptr + i);
        *(buffer_ptr + 1) = colors.green;
        if (buffer_ptr + 2 > output_buffer + BUFER_SIZE - 1)
        {
            buffer_ptr = output_buffer;
        }
        else
        {
            buffer_ptr = buffer_ptr + 2;
        }
    }

    display();

    return;
}

void charToHex(unsigned char c, char *hex)
{
    const char *hexDigits = "0123456789ABCDEF"; // Hexadecimal digits

    hex[0] = hexDigits[(c >> 4) & 0x0F]; // Extract the high nibble (4 bits) and find the hexadecimal representation
    hex[1] = hexDigits[c & 0x0F];        // Extract the low nibble and find the hexadecimal representation
    hex[3] = ' ';
    hex[4] = '\0'; // Null-terminate the string
}

void print_hex(char *char_ptr, int len)
{
    color_t colors;
    colors = (color_t){0, 1, 2, 4, 15};

    for (int i = 0; i < len; i++)
    {
        char hex[4];
        charToHex(*(char_ptr + i), hex);
        print(hex, -1);
    }

    return;
}

void println(char *char_ptr, int len)
{
    print(char_ptr, len);

    buffer_ptr = (char *)(((((long)buffer_ptr + VGA_WIDTH * 2) - (long)output_buffer) / (VGA_WIDTH * 2)) * VGA_WIDTH * 2 + (long)output_buffer);
    if (buffer_ptr > output_buffer + BUFER_SIZE - 1)
    {
        buffer_ptr = output_buffer;
    }

    display();

    return;
}

void println_hex(char *char_ptr, int len)
{
    print_hex(char_ptr, len);
    println("", -1);

    return;
}

void print_int(int intval)
{
    int val = intval;
    int n = num_digits(intval);
    char buf[n + 1]; // 1 extra char for 0 termination of string
    for (int i = n; i > 0; i--)
    {
        char a = val % 10 + '0'; // '0' converted to int 48
        buf[i - 1] = a;

        val /= 10;
    }
    buf[n] = '\0';
    print((char *)&buf, n); // print in dec
}

void print_int_t16(uint16_t intval)
{
    uint16_t val = intval;
    int n = num_digits_t16(intval);
    char buf[n + 1]; // 1 extra char for 0 termination of string
    for (int i = n; i > 0; i--)
    {
        char a = val % 10 + '0'; // '0' converted to int 48
        buf[i - 1] = a;

        val /= 10;
    }
    buf[n] = '\0';
    print((char *)&buf, n); // print in dec
}

void print_int_hex(int intval)
{
    int val = intval;
    int n = num_digits_hex(intval);
    char buf[n + 1]; // 1 extra char for 0 termination of string
    for (int i = n; i > 0; i--)
    {
        char a;
        if ((val % 16) < 10)
        {
            a = (val % 16) + '0';
        }
        else
        {
            a = (val % 16 - 10) + 'A';
        }

        buf[i - 1] = a;

        val /= 16;
    }
    buf[n] = '\0';

    print((char *)&buf, n); // print in hex
}

void print_int_hex_t16(uint16_t intval)
{
    uint16_t val = intval;
    int n = num_digits_hex_t16(intval);
    char buf[n + 1]; // 1 extra char for 0 termination of string
    for (int i = n; i > 0; i--)
    {
        char a;
        if ((val % 16) < 10)
        {
            a = (val % 16) + '0';
        }
        else
        {
            a = (val % 16 - 10) + 'A';
        }

        buf[i - 1] = a;

        val /= 16;
    }
    buf[n] = '\0';

    print((char *)&buf, n); // print in hex
}


void print_bits(unsigned int num)
{
    unsigned int size = sizeof(unsigned int);
    unsigned int maxPow = 1 << (size * 8 - 1);
    for (int i = 0; i < size * 8; ++i)
    {
        // Print each bit starting from the leftmost bit
        print_int(num & maxPow ? 1 : 0);
        num = num << 1; // Shift left by one
    }
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

void draw_pixel(int x, int y, uint32_t color, uint32_t* framebuffer, uint32_t framebuffer_pitch) {
    uint32_t* pixel = (uint32_t*)((uintptr_t)framebuffer + y * framebuffer_pitch + x * 4);
    *pixel = color;
}

void draw_character(uint32_t* framebuffer, uint32_t framebuffer_pitch, int start_x, int start_y, uint32_t color, int char_code) {
    if(framebuffer==NULL){
       return;     
    }

    char (*letter)[CHAR_HEIGHT] = &font8x8_basic[char_code];
    for (int y = 0; y < CHAR_HEIGHT; y++) {
        for (int x = 0; x < CHAR_WIDTH; x++) {
            if ((*letter)[y] & (1<<x)) {
                draw_pixel(start_x + x, start_y + y, color, framebuffer, framebuffer_pitch);
            }
        }
    }
}

