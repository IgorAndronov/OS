
1. compile:
nasm -f elf64 ./src/arch/x86-64/multiboot_header.asm
nasm -f elf64 ./src/arch/x86-64/boot.asm
nasm -f elf64 ./src/arch/x86-64/long_mode_main.asm
nasm -f elf64 ./src/arch/x86-64/isr.asm

gcc -c ./src/arch/x86-64/c/c_main.c -o ./src/arch/x86-64/c/c_main.o
gcc -c ./src/arch/x86-64/c/net/rtl8139.c -o ./src/arch/x86-64/c/net/rtl8139.o
gcc -c ./src/arch/x86-64/c/net/pci.c -o ./src/arch/x86-64/c/net/pci.o
gcc -c ./src/arch/x86-64/c/core/port.c -o ./src/arch/x86-64/c/core/port.o

gcc -c ./src/arch/x86-64/c/interrupts/idt.c -o ./src/arch/x86-64/c/interrupts/idt.o
gcc -c ./src/arch/x86-64/c/interrupts/isr.c -o ./src/arch/x86-64/c/interrupts/isr.o
gcc -c ./src/arch/x86-64/c/interrupts/interrupts.c -o ./src/arch/x86-64/c/interrupts/interrupts.o

gcc -c ./src/arch/x86-64/c/interrupts/keyboard/keyboard.c -o ./src/arch/x86-64/c/interrupts/keyboard/keyboard.o

ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/c/c_main.o
ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/isr.o ./src/arch/x86-64/c/c_main.o ./src/arch/x86-64/c/core/port.o ./src/arch/x86-64/c/net/rtl8139.o ./src/arch/x86-64/c/net/pci.o
ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/isr.o ./src/arch/x86-64/c/c_main.o ./src/arch/x86-64/c/interrupts/idt.o ./src/arch/x86-64/c/interrupts/isr.o ./src/arch/x86-64/c/interrupts/interrupts.o ./src/arch/x86-64/c/core/port.o ./src/arch/x86-64/c/net/rtl8139.o ./src/arch/x86-64/c/net/pci.o ./src/arch/x86-64/c/interrupts/keyboard/keyboard.o 

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
qemu-system-x86_64 -cdrom ./build/os.iso -vga std  -m 1G -net nic,model=rtl8139

