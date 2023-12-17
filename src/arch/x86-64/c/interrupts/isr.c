#include "isr.h"
#include "../c_main.h"


static isr_handler_t handlers[MAX_HANDLERS] = { 0 };

void isr_init()
{
  println("call isr_init",-1);
  
  arch_isr_init();

  isr_enable_interrupts();
}

void isr_enable_interrupts()
{
  arch_isr_enable_interrupts();
}

void isr_disable_interrupts()
{
  arch_isr_disable_interrupts();
}

void isr_register_handler(uint32_t id, isr_handler_t handler)
{
  handlers[id] = handler;
}

isr_handler_t isr_get_handler(uint32_t id)
{
  return handlers[id];
}
