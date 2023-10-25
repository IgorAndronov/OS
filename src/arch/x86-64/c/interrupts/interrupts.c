
#include "idt.h"
#include "interrupt.h"
#include "isr.h"
#include "../core/port.h"
#include <inttypes.h>


void breakpoint_handler(isr_stack_t* stack);
void page_fault_handler(isr_stack_t* stack);

//below dummy declaration is needed as c compiler adds some stack protection to resulting code and as result it cannot be linked due to missing below function.


void arch_isr_init()
{
  // start initialization
  port_byte_out(PIC1, 0x11);
  port_byte_out(PIC2, 0x11);

  // set IRQ base numbers for each PIC
  port_byte_out(PIC1_DATA, IRQ_BASE); //Base address is the vector number for ISR0
  port_byte_out(PIC2_DATA, IRQ_BASE + 8);

  // use IRQ number 2 to relay IRQs from the secondary PIC
  port_byte_out(PIC1_DATA, 0x04);
  port_byte_out(PIC2_DATA, 0x02);

  // finish initialization
  port_byte_out(PIC1_DATA, 0x01);
  port_byte_out(PIC2_DATA, 0x01);

  // mask all IRQs
  port_byte_out(PIC1_DATA, 0x00);
  port_byte_out(PIC2_DATA, 0x00);

  // Exceptions
  idt_register_interrupt(0, (uint64_t)exc0);
  idt_register_interrupt(1, (uint64_t)exc1);
  idt_register_interrupt(2, (uint64_t)exc2);
  idt_register_interrupt(3, (uint64_t)exc3);
  idt_register_interrupt(4, (uint64_t)exc4);
  idt_register_interrupt(5, (uint64_t)exc5);
  idt_register_interrupt(6, (uint64_t)exc6);
  idt_register_interrupt(7, (uint64_t)exc7);
  idt_register_interrupt(8, (uint64_t)exc8);
  idt_register_interrupt(9, (uint64_t)exc9);
  idt_register_interrupt(10, (uint64_t)exc10);
  idt_register_interrupt(11, (uint64_t)exc11);
  idt_register_interrupt(12, (uint64_t)exc12);
  idt_register_interrupt(13, (uint64_t)exc13);
  idt_register_interrupt(14, (uint64_t)exc14);
  idt_register_interrupt(15, (uint64_t)exc15);
  idt_register_interrupt(16, (uint64_t)exc16);
  idt_register_interrupt(17, (uint64_t)exc17);
  idt_register_interrupt(18, (uint64_t)exc18);
  idt_register_interrupt(19, (uint64_t)exc19);
  idt_register_interrupt(20, (uint64_t)exc20);
  idt_register_interrupt(21, (uint64_t)exc21);
  idt_register_interrupt(22, (uint64_t)exc22);
  idt_register_interrupt(23, (uint64_t)exc23);
  idt_register_interrupt(24, (uint64_t)exc24);
  idt_register_interrupt(25, (uint64_t)exc25);
  idt_register_interrupt(26, (uint64_t)exc26);
  idt_register_interrupt(27, (uint64_t)exc27);
  idt_register_interrupt(28, (uint64_t)exc28);
  idt_register_interrupt(29, (uint64_t)exc29);
  idt_register_interrupt(30, (uint64_t)exc30);
  idt_register_interrupt(31, (uint64_t)exc31);

  // Hardware interrupts
  idt_register_interrupt(IRQ0, (uint64_t)irq0);
  idt_register_interrupt(IRQ1, (uint64_t)irq1);
  idt_register_interrupt(IRQ2, (uint64_t)irq2);
  idt_register_interrupt(IRQ3, (uint64_t)irq3);
  idt_register_interrupt(IRQ4, (uint64_t)irq4);
  idt_register_interrupt(IRQ5, (uint64_t)irq5);
  idt_register_interrupt(IRQ6, (uint64_t)irq6);
  idt_register_interrupt(IRQ7, (uint64_t)irq7);
  idt_register_interrupt(IRQ8, (uint64_t)irq8);
  idt_register_interrupt(IRQ9, (uint64_t)irq9);
  idt_register_interrupt(IRQ10, (uint64_t)irq10);
  idt_register_interrupt(IRQ11, (uint64_t)irq11);
  idt_register_interrupt(IRQ12, (uint64_t)irq12);

  // Specific handlers for exceptions.
  isr_register_handler(EXCEPTION_BP, breakpoint_handler);
  isr_register_handler(EXCEPTION_PF, page_fault_handler);

  idt_load();
}

void arch_isr_enable_interrupts()
{
  __asm__("sti");
}

void arch_isr_disable_interrupts()
{
  __asm__("cli");
}

void isr_int_handler(isr_stack_t stack)
{
  isr_handler_t handler = isr_get_handler(stack.id);

  if (handler != 0) {
    handler(&stack);
    return;
  }

}

void isr_irq_handler(isr_stack_t stack)
{
  isr_handler_t handler = isr_get_handler(stack.id);

  if (handler != 0) {
    handler(&stack);
  }

  if (stack.id >= 40) {
    port_byte_out(PIC2, PIC_EOI);
  }

  port_byte_out(PIC1, PIC_EOI);
}

void breakpoint_handler(isr_stack_t* stack)
{
  
}

void page_fault_handler(isr_stack_t* stack)
{
 
}
