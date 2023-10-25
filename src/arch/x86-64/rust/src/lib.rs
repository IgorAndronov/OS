
#![feature(lang_items)]
#![feature(ptr_internals)]
#![no_std]

use core::panic::PanicInfo;
extern crate multiboot2;
extern crate volatile;
extern crate spin;
extern crate lazy_static;

#[macro_use]
mod vga_buffer;

mod memory;

#[no_mangle]
pub extern fn rust_main(multiboot_information_address: usize) {

    use memory::FrameAllocator;

    vga_buffer::clear_screen();
    println!("Hello World{}", "!");

    let boot_info = unsafe{ multiboot2::load(multiboot_information_address) };
    let memory_map_tag = boot_info.memory_map_tag()
        .expect("Memory map tag required");

    println!("memory areas:");
    for area in memory_map_tag.memory_areas() {
        println!("    start: 0x{:x}, length: 0x{:x}",
            area.base_addr, area.length);
    }

    let elf_sections_tag = boot_info.elf_sections_tag()
        .expect("Elf-sections tag required");

    println!("kernel sections:");
    for section in elf_sections_tag.sections() {
        println!("    addr: 0x{:x}, size: 0x{:x}, flags: 0x{:x}",
            section.addr, section.size, section.flags);
    }

    let kernel_start = elf_sections_tag.sections().map(|s| s.addr)
        .min().unwrap();
    let kernel_end = elf_sections_tag.sections().map(|s| s.addr + s.size)
        .max().unwrap();
    let multiboot_start = multiboot_information_address;
    let multiboot_end = multiboot_start + (boot_info.total_size as usize);

    
    println!("  kernel_start  addr: 0x{:x}, kernel_end addr: 0x{:x}",
    kernel_start, kernel_end);

    println!("  multiboot_info_start  addr: 0x{:x}, multiboot_info_end addr: 0x{:x}",
    multiboot_start, multiboot_end);


    let mut frame_allocator = memory::AreaFrameAllocator::new(
        kernel_start as usize, kernel_end as usize, multiboot_start,
        multiboot_end, memory_map_tag.memory_areas());

    for i in 0.. {
        if let None = frame_allocator.allocate_frame() {
            println!("allocated {} frames", i);
            break;
        }
    }

    loop{}

}

   

#[lang = "eh_personality"] #[no_mangle] 
pub extern fn eh_personality() {
}

/// This function is called on panic.
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    println!("\n\nPANIC:");
    loop {}
}