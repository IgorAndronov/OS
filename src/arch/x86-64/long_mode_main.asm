global long_mode_start
extern print
extern print_graphics 

section .text
bits 64
long_mode_start:
    ; load 0 into all data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;call c function print
    ;according to ABI the first 6 integers or pointers are sent in registers only then in stack
    mov rdi, var_a
    mov rsi, var_a.leng
   ; call print 

   ;call c function print_graphics
    call print_graphics 

    ; direct print `OKAY` to screen
    mov rax, 0x2f592f412f4b2f4f
    mov qword [0xb8000], rax
    hlt

section .data
align 4096
var_a:
    db 'Hello World! Flexibility works.'
.leng: equ $ - var_a    
     