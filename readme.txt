https://os.phil-opp.com/first-edition/

1. compile:
nasm -f elf64 ./src/arch/x86-64/multiboot_header.asm
nasm -f elf64 ./src/arch/x86-64/boot.asm
nasm -f elf64 ./src/arch/x86-64/long_mode_main.asm
gcc -c ./src/arch/x86-64/c/c_main.c -o ./src/arch/x86-64/c/c_main.o
ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/c/c_main.o

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
qemu-system-x86_64 -cdrom ./build/os.iso -vga std
wiki.gentoo.org/wiki/QEMU/Options

Usfull commands:
ndisasm -b 32 ./src/arch/x86-64/boot
hexdump -x ./src/arch/x86-64/multiboot_header


Extra reading:
1. http://pages.cs.wisc.edu/%7Eremzi/OSTEP/
2. http://littleosbook.github.io/


