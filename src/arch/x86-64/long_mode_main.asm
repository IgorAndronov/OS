global long_mode_start
extern print
extern println
extern print_int
extern print_graphics 
; extern rust_main
extern isr_init
extern keyboard_init
extern network_init
extern var_multiboot_info
extern p2_table_0
extern p2_table_1

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

; //////////////testing section//////////////////////////////

    ; call the rust main
    ; mov edi, [var_multiboot_info]  
    ; call rust_main       ; edi should contain address of the variable to pass.

    ;call c function print
    ;according to ABI the first 6 integers or pointers are sent in registers only then in stack
    mov rdi, var_a
    mov rsi, var_a.leng
    ; call print 

    mov rdi, long_mode_start
    call print_int
    mov rdi, 0
    mov rsi, 0
    call println


    mov rdi, [p2_table_0]
    call print_int
    mov rdi, 0
    mov rsi, 0
    call println

    mov rdi, [p2_table_0+8]
    call print_int
    mov rdi, 0
    mov rsi, 0
    call println

    mov rdi, [p2_table_0+511*8]
    call print_int
    mov rdi, 0
    mov rsi, 0
    call println

    mov rdi, [p2_table_1]
    call print_int
    mov rdi, 0
    mov rsi, 0
    call println

; ////////////////////////////////////////////

    call isr_init
    call keyboard_init
    call network_init ;rtl8139.c
  
   ;call c function print_graphics
   ; call print_graphics 

    ; direct print `OKAY` to screen
    ;mov rax, 0x2f592f412f4b2f4f
    ;mov qword [0xb8100], rax
    hlt

section .data
align 4096
var_a:
    db 'Hello World! Flexibility works.'
.leng: equ $ - var_a    
     