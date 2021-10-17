#include "c_main.h"
#include <stdint.h>


void print (char* char_ptr, int len){
    color_t colors;
    colors = (color_t) {0,1,2,4,15};

    char* buffer_ptr = (char*) 0xb8000;
    for(int i =0; i<len; i++){
        *(buffer_ptr+i*2) = *(char_ptr+i);
        *(buffer_ptr+i*2+1) = colors.green;
    }
  
    return;
}

void WRITE_PORT_USHORT(uint16_t index_port, uint16_t index){
    __asm__ __volatile__(
        "out %0, %1 \n\t" : : "a"(index), "dN"(index_port)
    );
}

void BlBochsVbeWrite (uint16_t index, uint16_t value) {

   WRITE_PORT_USHORT(VBE_DISPI_IOPORT_INDEX, index);
   WRITE_PORT_USHORT(VBE_DISPI_IOPORT_DATA, value);
}

void BlBochsSetMode (uint16_t xres, uint16_t yres, uint16_t bpp) {

   BlBochsVbeWrite (VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
   BlBochsVbeWrite (VBE_DISPI_INDEX_XRES, xres);
   BlBochsVbeWrite (VBE_DISPI_INDEX_YRES, yres);
   BlBochsVbeWrite (VBE_DISPI_INDEX_BPP, bpp);
   BlBochsVbeWrite (VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);   
}

void print_graphics(){
    BlBochsSetMode(800, 600, 32);
    uint32_t* buffer_ptr = (uint32_t*) 0xE0000000;
    const uint32_t red = 0x00FF00;

    for(int i =0; i<800*600; i++){
        *(buffer_ptr+i) = red;
        
    }

}