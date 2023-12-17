#include <stdint.h>

#ifndef C_UEFI_H
#define C_UEFI_H

typedef struct {
    uint32_t total_size;
    uint32_t reserved;
    // Followed by one or more multiboot_tag structures
} multiboot_info_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    // Tag-specific data follows here
} multiboot_tag_t;

typedef struct {
    uint32_t type;                 // Should be MULTIBOOT_TAG_TYPE_FRAMEBUFFER
    uint32_t size;                 // Size of the tag
    uint64_t framebuffer_addr;     // Physical address of the framebuffer
    uint32_t framebuffer_pitch;    // The number of bytes per screen line
    uint32_t framebuffer_width;    // Width of the framebuffer in pixels
    uint32_t framebuffer_height;   // Height of the framebuffer in pixels
    uint8_t  framebuffer_bpp;      // Bits per pixel
    uint8_t  framebuffer_type;     // Framebuffer type (0 = indexed, 1 = RGB, 2 = EGA text)
    uint16_t reserved;
    union {
        struct {
            uint32_t framebuffer_palette_num_colors;
            struct {
                uint8_t red;
                uint8_t green;
                uint8_t blue;
            } framebuffer_palette[];
        } palette;
        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        } rgb;
    } framebuffer_info;
} multiboot_tag_framebuffer_t;

void init_multibootinfo(void * ptr);


#endif