
1. compile:
nasm -f elf64 ./src/arch/x86-64/multiboot_header.asm
nasm -f elf64 ./src/arch/x86-64/boot.asm
nasm -f elf64 ./src/arch/x86-64/long_mode_main.asm
nasm -f elf64 ./src/arch/x86-64/isr.asm

gcc -c ./src/arch/x86-64/c/c_main.c -o ./src/arch/x86-64/c/c_main.o
gcc -c ./src/arch/x86-64/c/net/rtl8139.c -o ./src/arch/x86-64/c/net/rtl8139.o
gcc -c ./src/arch/x86-64/c/net/pci.c -o ./src/arch/x86-64/c/net/pci.o
gcc -c ./src/arch/x86-64/c/core/port.c -o ./src/arch/x86-64/c/core/port.o
gcc -c ./src/arch/x86-64/c/uefi_framebuffer.c -o ./src/arch/x86-64/c/uefi_framebuffer.o

gcc -c ./src/arch/x86-64/c/interrupts/idt.c -o ./src/arch/x86-64/c/interrupts/idt.o
gcc -c ./src/arch/x86-64/c/interrupts/isr.c -o ./src/arch/x86-64/c/interrupts/isr.o
gcc -c ./src/arch/x86-64/c/interrupts/interrupts.c -o ./src/arch/x86-64/c/interrupts/interrupts.o

gcc -c ./src/arch/x86-64/c/interrupts/keyboard/keyboard.c -o ./src/arch/x86-64/c/interrupts/keyboard/keyboard.o

ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/c/c_main.o
ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/isr.o ./src/arch/x86-64/c/c_main.o ./src/arch/x86-64/c/core/port.o ./src/arch/x86-64/c/net/rtl8139.o ./src/arch/x86-64/c/net/pci.o
ld -n -o ./isofiles/boot/kernel.bin -T ./src/arch/x86-64/linker.ld ./src/arch/x86-64/multiboot_header.o ./src/arch/x86-64/boot.o ./src/arch/x86-64/long_mode_main.o ./src/arch/x86-64/isr.o ./src/arch/x86-64/c/c_main.o ./src/arch/x86-64/c/interrupts/idt.o ./src/arch/x86-64/c/interrupts/isr.o ./src/arch/x86-64/c/interrupts/interrupts.o ./src/arch/x86-64/c/core/port.o ./src/arch/x86-64/c/net/rtl8139.o ./src/arch/x86-64/c/net/pci.o ./src/arch/x86-64/c/interrupts/keyboard/keyboard.o ./src/arch/x86-64/c/uefi_framebuffer.o

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

    -------------------------------------------------------------
    Steps to Create a UEFI-Compatible ISO

    Prepare the File Structure:
        Create a directory (let’s call it isofiles) to hold the files you want to include in your ISO.
        Within isofiles, create the directory structure for UEFI. At a minimum, you need a directory named boot and within it, a directory named grub.

    Create the GRUB Configuration File:
        Inside isofiles/boot/grub, create a grub.cfg file. This file contains the GRUB menu configuration.

    Add Your Kernel and Initramfs (if creating a bootable Linux ISO):
        Place your Linux kernel and initramfs files in the appropriate directories within isofiles.

    Create an EFI Boot Image:
        Create a sub-directory inside isofiles/boot named efi.
        Within this, create the directories efi/boot.
        Copy the GRUB UEFI bootloader into this directory. The bootloader is typically named grubx64.efi for 64-bit systems. You can find it in your system’s GRUB installation directory or in /boot/efi/EFI/ubuntu (or a similar path, depending on your distribution).
        Rename grubx64.efi to bootx64.efi and place it in isofiles/boot/efi/boot.

    Use grub-mkrescue to Create the ISO:
        Run the grub-mkrescue command to create the ISO. This tool will automatically create a UEFI-compatible ISO if it detects the presence of the bootx64.efi file.

    Rufus:
    Partitioning: GPT FileSystem:FAT32   
    When run on Dell" se;ect boot path: /boot/efi/boot/bootx64.efi 
    Then during run F12-> select run configuration.
    grub> set root=(hd0,gpt1)
    grub> configfile /boot/efi/boot/grub.cnf 

3. Run iso in simulator:
qemu-system-x86_64 -cdrom ./build/os.iso -vga std  -m 1G -net nic,model=rtl8139

a)connect to host with NAT:
qemu-system-x86_64 -cdrom ./build/os.iso -vga std  -m 1G -netdev user,id=mynet0 -device rtl8139,netdev=mynet0

This command does the following:
    -netdev user,id=mynet0: Creates a new network backend of type user and gives it an ID of mynet0. The user type network backend is a built-in QEMU network stack that requires no administrator privileges and provides a simple network connectivity to your virtual machine.
    -device rtl8139,netdev=mynet0: Creates a virtual network interface card in the VM with the rtl8139 model, and connects it to the mynet0 backend.

If you want the VM to have access to the internet or to be accessible from the host, you'll usually use the user type network, which is suitable for most needs and creates a kind of NAT-ed connection. For more advanced networking setups, such as bridging or tap devices, additional configuration on the host system is required.

b - default)connect to host with bridge network:

sudo qemu-system-x86_64 -cdrom ./build/os.iso -vga std  -m 1G -netdev tap,id=net0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=net0,mac=52:55:00:d1:55:01 -no-reboot

!!! then on host mashine: 
  sudo ip link set tap0 up  (to run tap0 interface)
  ip link show (to check status)

c)sudo qemu-system-x86_64 -cdrom ./build/os.iso -vga std  -m 1G -netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
-device rtl8139,netdev=net0,mac=52:55:00:d1:55:01 -no-reboot \
-drive if=pflash,format=raw,readonly,file=/usr/share/OVMF/OVMF_CODE.fd \
-drive if=pflash,format=raw,file=/usr/share/OVMF/OVMF_VARS.fd 

Testing:
use python program for package send. Details in py/readme.txt

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
info:
1) 
    a)   If your system is booting in UEFI mode, GRUB typically starts your kernel in long mode (64-bit mode) for x86_64 architectures. In this mode, the CPU operates in 64-bit mode with paging enabled.
    b)   If your system is booting in legacy BIOS mode, GRUB typically starts your kernel in protected mode (32-bit mode). This is the mode used by most older systems and some modern systems with CSM (Compatibility Support Module) enabled.
    c) A specific tag (EFI Boot Services Tag) in the Multiboot2 information structure indicates that the system has booted in UEFI mode. If this tag is present, you can be confident that the system is in UEFI mode and that your kernel has been started in long mode.

2) rtl8139_receive():
In the case of the RTL8139 and many other network interface cards, the status of the receive buffer is managed through a ring buffer mechanism and not through an "empty" flag in the command register. The ring buffer has a read pointer (which the CPU updates after processing packets) and a write pointer (which the NIC updates as it writes new data). If both pointers are at the same location, the buffer can be considered empty, meaning there is no new data to read.

The command register is typically used to control the operation of the NIC, like enabling or disabling the receiver, transmitter, and other operational states.

When your receive handler processes a packet, it's responsible for updating the Current Address of Packet Read (CAPR) register to indicate to the NIC that the data has been consumed. The NIC will then know it can overwrite the old data with new incoming packets.

Here is a conceptual flow of how this is typically handled:

    NIC receives a packet and writes it to the receive buffer, updates the write pointer.
    NIC raises an interrupt indicating that a packet has been received.
    CPU handler for the interrupt reads the packet from the receive buffer.
    After processing the packet, the CPU handler updates the CAPR to point past the end of the last packet read.
    NIC checks the CAPR and if it matches the write pointer (or in some implementations, if the next packet header is not yet written by the NIC), it knows that the CPU has read all the data, and the buffer is effectively "empty" from the perspective of the CPU.

In summary, the "buffer empty" condition is implicitly managed through the ring buffer's read and write pointers, not through a specific "empty" flag in the command register. The NIC knows to stop writing to the buffer when it reaches the read pointer, which prevents overwriting unread data.

Here's a breakdown of a standard Ethernet II frame structure:
    Preamble (7 bytes) and Start of Frame Delimiter (SFD, 1 byte): These are used for synchronization and signaling the start of the frame, respectively.
    Destination MAC Address (6 bytes): The MAC address of the destination device.
    Source MAC Address (6 bytes): The MAC address of the source device.
    EtherType/Length (2 bytes): This field serves two purposes. If its value is greater than or equal to 1536 (0x0600), it indicates the EtherType, which identifies the protocol encapsulated in the payload of the frame (like IPv4, IPv6, ARP, etc.). If its value is less than 1536, it indicates the size of the payload in bytes.
    Payload (46 to 1500 bytes): The data carried by the frame. Its size can be as small as 46 bytes and as large as 1500 bytes. If the data to be transmitted is less than 46 bytes, padding bytes are added to meet the minimum size requirement. This is where the length value (if used instead of EtherType) indicates the actual size of the payload.
    Frame Check Sequence (FCS, 4 bytes): A 32-bit CRC used for error-checking the contents of the frame.

Note: Frame Check Sequence is not seen in wireshark.
      Buffer Handling by NIC: When a network interface card (NIC) writes a packet into the RX buffer, it might include additional information like its own headers, status information, or even padding for alignment purposes. This can make the packet appear larger when read directly from the RX buffer.
      Frame Check Sequence put at the end of packet in buffer. So packet_in_buffer = realPacket+padding_zeroes+FCS(4bytes)