# compile assembly files
nasm -f elf64 ./src/arch/x86-64/multiboot_header.asm
nasm -f elf64 ./src/arch/x86-64/boot.asm
ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o

grub-mkrescue -o ./build/os.iso isofiles --xorriso="$HOME"/xorriso/xorriso-1.4.6/xorriso/xorriso
