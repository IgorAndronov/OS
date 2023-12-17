#include "uefi_framebuffer.h"
#include <stdint.h>
#include "./c_main.h"

#define MULTIBOOT2_TAG_TYPE_EFI_BOOT_SERVICES_AVAILABLE 0x14
#define MULTIBOOT2_TAG_TYPE_EFI_32_SYSTEM_TABLE  9
#define MULTIBOOT2_TAG_TYPE_EFI_64_SYSTEM_TABLE 10
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER 8

extern char* var_multiboot_info;

static multiboot_info_t* mb_info;


// Main kernel entry point
void multiboot_info() {
  print("framebuffer ptr:",-1);
  print_hex((char*)mb_info, 4);
  println("",-1);

char* framebuffer_addr;
uint32_t framebuffer_pitch;
uint8_t  framebuffer_type;

  multiboot_tag_t* tag = (multiboot_tag_t*)((uint8_t*)mb_info + 8);
    while (tag->type != 0) {
      // println("multiboot2 tag: ",-1);
      //       print_int(tag->type);
      //       println("",-1);
        if (tag->type == MULTIBOOT2_TAG_TYPE_EFI_32_SYSTEM_TABLE ||
            tag->type == MULTIBOOT2_TAG_TYPE_EFI_64_SYSTEM_TABLE) {
            // System booted in UEFI mode
            println("Boot mode:",-1);
            print_int(tag->type);
            println("",-1);

        }
        if (tag->type == 20) {
            // UEFI boot services are still available
              println("UEFI boot services are still available",-1);

        }
        if (tag->type == MULTIBOOT2_TAG_TYPE_FRAMEBUFFER) {
            multiboot_tag_framebuffer_t *fb_tag = (multiboot_tag_framebuffer_t *)tag;
            framebuffer_addr = (char*)fb_tag->framebuffer_addr;
            framebuffer_pitch = fb_tag->framebuffer_pitch;
            framebuffer_type = fb_tag->framebuffer_type;
              println("framebuffer_type", -1);
              print_hex((char*)&(framebuffer_type), 1);
              println("",-1);
              print("framebuffer_addr:",-1);
              print_hex(framebuffer_addr, 8);
              println("",-1);
            int is_grafic_mode = 1; //yes
            if(framebuffer_type == 2){
              is_grafic_mode=0;
            }
            init_framebuffer((uint32_t*)framebuffer_addr, framebuffer_pitch, is_grafic_mode);
            println("init_framebuffer finished", -1);
            break;
        }
        // Move to the next tag
        tag = (multiboot_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }

    //test output
    // if (framebuffer_addr != 0 && framebuffer_type != 2) {
    //     for(int i = 0x20; i < 0x7F; i++){
    //       draw_character((uint32_t*)framebuffer_addr, framebuffer_pitch, (CHAR_WIDTH+2)*i, 10, COLOR_WHITE, i);
    //     }
    // }

    delay(100000); 
}


void init_multibootinfo(void * ptr){
  mb_info=(multiboot_info_t*)ptr;

  print("var_multiboot_info:",-1);
  print_hex((char*)var_multiboot_info, 4);
  println("",-1);

  multiboot_info();
  delay(100000000000);
}