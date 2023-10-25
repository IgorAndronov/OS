https://os.phil-opp.com/first-edition/

1. compile:
nasm -f elf64 ./src/arch/x86-64/multiboot_header.asm
nasm -f elf64 ./src/arch/x86-64/boot.asm
nasm -f elf64 ./src/arch/x86-64/long_mode_main.asm
gcc -c ./src/arch/x86-64/c/c_main.c -o ./src/arch/x86-64/c/c_main.o
ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/c/c_main.o
ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/c/c_main.o ./src/arch/x86-64/rust/target/x86_64-blog_os/debug/libblog_os.a

2. Make ISO:
    a. install xorriso
    mkdir "$HOME"/xorriso
    cd "$HOME"/xorriso
    wget https://ftp.gnu.org/gnu/xorriso/xorriso-1.4.6.tar.gz

    tar xzf xorriso-1.4.6.tar.gz
    cd xorriso-1.4.6
    ./configure --prefix=/usr
    make

    b. use GRUB bootloader to create iso file
    grub-mkrescue -o ./build/os.iso isofiles --xorriso="$HOME"/xorriso/xorriso-1.4.6/xorriso/xorriso

3. Run iso in simulator:
qemu-system-x86_64 -cdrom ./build/os.iso -vga std  -m 1G
wiki.gentoo.org/wiki/QEMU/Options

Usfull commands:
ndisasm -b 32 ./src/arch/x86-64/boot.o
hexdump -x ./src/arch/x86-64/multiboot_header.o
#inspect object file
readelf -hSl ./src/arch/x86-64/boot.o
readelf -sS ./src/arch/x86-64/boot.o //symtable info
readelf -r ./src/arch/x86-64/boot.o //relocation entries
readelf -s ./isofiles/boot/kernel.bin
readelf -hSl ./isofiles/boot/kernel.bin
readelf -x .strtab ./src/arch/x86-64/boot.o //dump of the strtab section
#dissassemble object file
objdump -M i386,intel -D ./src/arch/x86-64/boot.o  (i386 for 32 bit code dissassemble)
objdump -M intel -dwarf ./isofiles/boot/kernel.bin


Extra reading:
1. http://pages.cs.wisc.edu/%7Eremzi/OSTEP/
2. http://littleosbook.github.io/


Explanations:

1). when offsets are calculated like "stack_top" in
mov esp, stack_top
stack_top has offset relative to start of current section (.bss in this case)
2). Then when linkage happens this offset is recalculated according to sections layout and initial offset that defined in linker.ld
in our case it is 1M.
objdump -h ./isofiles/boot/kernel.bin
Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .boot         00000018  0000000000100000  0000000000100000  00001000  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .rodata       0000001a  0000000000100018  0000000000100018  00001018  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  2 .eh_frame     000000b8  0000000000100038  0000000000100038  00001038  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .text         0000032c  00000000001000f0  00000000001000f0  000010f0  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  4 .data         0000001f  0000000000101000  0000000000101000  00002000  2**12
                  CONTENTS, ALLOC, LOAD, DATA
  5 .bss          0000a000  0000000000102000  0000000000102000  0000201f  2**12
                  ALLOC
  6 .comment      00000029  0000000000000000  0000000000000000  0000201f  2**0
                  CONTENTS, READONLY

So "stack_top" in resulting code will be replaced with 100000h + offset of the end of bss resulting to 10C000h (objdump -D ./isofiles/boot/kernel.bin)

As we have identity mapping of first 1GB and bootloader loads program in memory to the same addresses as defined in sections layout 
all adresses that are calculated as offsets correspond to real phiscal addresses

Notes:
1. kernel.bin format
  The kernel.bin file produced by the ld linker command you provided will be in the ELF (Executable and Linkable Format) format. Here's why:
  GNU Linker (ld) Default: By default, when you use the GNU linker on Linux systems and most other UNIX-like systems, the output format is ELF, which is a common file format for executables, object code, shared libraries, and even core dumps on these platforms.
  file ./isofiles/boot/kernel.bin
2. readelf Ndx column:
    UND: Stands for "undefined". If a symbol's section index is "UND", it means that symbol is not defined in the current ELF file and must be resolved at runtime or link time from another object or shared library.
    ABS: Means that the symbol is absolute and not associated with any section. For such symbols, the address is an absolute value and won't be adjusted by runtime relocations.
    COM: Stands for "common". This is used for uninitialized global variables that can be merged with other symbols of the same name in different files.
    Example ABS defines constant:
    section .text
      ...
      jmp gdt64.code:long_mode_start
      ...
    section .rodata
    gdt64:
      dq 0 ; zero entry
    .code: equ $ - gdt64 ;
      dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment

   gdt64.code has type ABS. So it means that section .rodata can be placed in any place after link. And as result gdt64 reference can have any address_value but reference
   gdt64.code is always resolved to 8 (dq defins 8 bytes).

    readelf -sS ./isofiles/boot/kernel.bin
    29: 00000000001016e0     0 NOTYPE  LOCAL  DEFAULT    3 gdt64
    30: 0000000000000008     0 NOTYPE  LOCAL  DEFAULT  ABS gdt64.code  

   But it only reference to how symbol gdt64.code is resolved. The content of rodata gdt64 that is "dq (1<<43) | (1<<44) | (1<<47) | (1<<53)"
   will be relocated with .rodata segment.
   So the code jmp gdt64.code:long_mode_start is resolved to jmp 0000000000000008:long_mode_start
   0000000000000008 is offset in gdt that contains second record with value "dq (1<<43) | (1<<44) | (1<<47) | (1<<53)"

   