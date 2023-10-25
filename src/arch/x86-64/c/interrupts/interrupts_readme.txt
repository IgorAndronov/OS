File structure:
1). isr.asm:
    Creates set of methods like:
    irq1, irq2 ...
    ex1, ex2 ...
    
    irq1:
    cli
    push dword 0 ;error_code
    push dword (32 + %1) ;id
    jmp irq_common_stub ;after jmp to irq_common_stub, save_registers are called as result we push in stack struct isr_stack (defined in interrupt.h)
                        ;to be more presize from r15 to rax + id +error_code from two push instructions before jmp

    irq_common_stub:
        save_registers
        extern isr_irq_handler ;defined in src/arch/x86_64/c/interrupts/interrupts.c
        call isr_irq_handler
        restore_registers
        add rsp, 16
        sti
        iretq 

   interrupts.c:
   void isr_irq_handler(isr_stack_t stack){
    isr_handler_t handler = isr_get_handler(stack.id);
    handler(&stack);
    ...
   }     

when isr_irq_handler is called it recive isr_stack_t that contains error_code, id of the method, saved registers;
then isr_get_handler(stack.id) retrive specific handler.

isr.c:
static isr_handler_t handlers[MAX_HANDLERS] = { 0 };

isr_handler_t isr_get_handler(uint32_t id)
{
  return handlers[id];
}

Handlers registration:
isr.c:
void isr_register_handler(uint32_t id, isr_handler_t handler)
{
  handlers[id] = handler;
}

keyboard.c:
#define IRQ1  33

void keyboard_init()
{
  isr_register_handler(IRQ1, keyboard_callback);
}

static void keyboard_callback(isr_stack_t* stack){
    ...
}

keyboard_init is called from long_mode_main.asm

So the remain part is to register these  irq1, irq2 ... methods in IDT:

isr.c:
void isr_init()
{
  arch_isr_init();
  ...
}

interrupts.c:
void arch_isr_init()
{
    ;pic initialisation process
    ...
    idt_register_interrupt(IRQ0, (uint64_t)irq0); here adresses of irq0,1,2... are sent as parameter
    ...
    idt_load(); loads idt address to register
}

idt.c:
void idt_register_interrupt(uint16_t n, uint64_t handler)
{
  idt_register_gate(n, handler, INTERRUPT_GATE, 0);
}

static idt_gate_t idt[IDT_ENTRIES] = { 0 };

void idt_register_gate(uint16_t n, uint64_t handler, uint8_t type, uint8_t dpl)
{
  idt[n].ptr_low = (uint16_t)handler;
  idt[n].ptr_mid = (uint16_t)(handler >> 16);
  idt[n].ptr_high = (uint32_t)(handler >> 32);
  idt[n].selector = KERNEL_BASE_SELECTOR;
  idt[n].ist = 0;
  idt[n].type = type;
  idt[n].s = 0;
  idt[n].dpl = dpl;
  idt[n].present = 1;
}

long_mode_main.asm:
extern isr_init
extern keyboard_init

Resume: all methods like irq0,1,2... differ by index they send as parameter when calling isr_irq_handler.
isr_irq_handler retrives handler by inde from internal array of handlers

