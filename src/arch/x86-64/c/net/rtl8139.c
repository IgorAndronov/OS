#include "pci.h"
#include "../core/port.h"
#include "rtl8139.h"
#include "../c_main.h"
#include "../interrupts/interrupt.h"
#include "../interrupts/isr.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static uint32_t base_port_ioaddr = 0;

// uint8_t* rtl8139_get_mac_address();
// void rtl8139_transmit(uint8_t* data, uint32_t len);
// void rtl8139_receive();

static uint8_t tx_buffer[TX_BUFFER_SIZE] = {0};
static uint8_t rx_buffer[RX_BUFFER_SIZE] = {0};

static uint64_t tx_buffer_addr = 0;
static uint32_t rx_index = 0;

static uint8_t transmit_start_registers[4] = { 0x20, 0x24, 0x28, 0x2C };
static uint8_t transmit_command_registers[4] = { 0x10, 0x14, 0x18, 0x1C };
static uint8_t current_transmit_pair = 0;

static uint8_t mac_address[6] = { 0 };


// static net_driver_t driver = {
//   .type = 1, // ARP_HTYPE_ETHERNET
//   .name = "RealTek RTL8139",
//   .get_mac_address = rtl8139_get_mac_address,
//   .transmit = rtl8139_transmit,
//   .interface = NULL, // will be set in `net_interface_init()`.
// };


static void net_callback(isr_stack_t* stack)
{
  println("net",3);
 
}


bool rtl_net_card_init(pci_device_t device)
{
  uint32_t ret = pci_read(device, PCI_BASE_ADDR_0_FIELD_OFFSET, 4);
  println("BAR0:", 5);
  print_int(ret);
  println("",0);
  base_port_ioaddr = ret & (~0x3);  //ignoring two last digits of ret gives base port address
  println("base port address", 17);
  print_int(base_port_ioaddr);
 

  // Enable PCI Bus Mastering (https://wiki.osdev.org/RTL8139)
  uint16_t pci_command_reg = pci_read(device, PCI_COMMAND_FIELD_OFFSET, 2);
  if (!(pci_command_reg & (1 << 2))) {
    pci_command_reg |= (1 << 2);
    pci_write(device, PCI_COMMAND_FIELD_OFFSET, pci_command_reg);
  }

  // Set the LWAKE + LWPTN to active high in order to power on the device. (Turning on the RTL_NIC)
  port_byte_out(base_port_ioaddr + RTL8139_REGISTER_CONFIG_1, 0x0);

  // Software reset to clear the RX and TX buffers and set everything back to
  // defaults. (0x10 = reset)
  port_byte_out(base_port_ioaddr + RTL8139_REGISTER_COMMAND, 0x10);
  while ((port_byte_in(base_port_ioaddr + RTL8139_REGISTER_COMMAND) & 0x10) != 0) {
    ;
  }

  // Initialize transmit buffer.
  tx_buffer_addr = &tx_buffer;

  // Initialize receive buffer.
  port_dword_out(base_port_ioaddr + RTL8139_REGISTER_RBSTART,
                 rx_buffer);

  // Set IMR + ISR.
  // '0x0005' sets the TOK and ROK bits high.
  port_word_out(base_port_ioaddr + RTL8139_REGISTER_IMR, 0x0005);

  // Configure receive buffer (RCR).
  // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
  port_dword_out(base_port_ioaddr + 0x44, 0xf | (1 << 7));

  // Accept and transmit packets.
  port_byte_out(base_port_ioaddr + RTL8139_REGISTER_COMMAND, 0x0C);

  // Register an interrupt handler.
  uint8_t irq_num = pci_read(device, PCI_INTERRUPT_LINE, 1);
  isr_register_handler(32 + irq_num, net_callback);
//   NET_DEBUG("registered interrupt handler: %d", 32 + irq_num);

// Read MAC address.
  uint32_t mac_part1 = port_dword_in(base_port_ioaddr + RTL8139_REGISTER_MAC1);
  uint16_t mac_part2 = port_word_in(base_port_ioaddr + RTL8139_REGISTER_MAC2);

  mac_address[0] = mac_part1 >> 0;
  mac_address[1] = mac_part1 >> 8;
  mac_address[2] = mac_part1 >> 16;
  mac_address[3] = mac_part1 >> 24;
  mac_address[4] = mac_part2 >> 0;
  mac_address[5] = mac_part2 >> 8;

  println("",1);
  println("MAC:", 4);
  print_int(mac_address[0]);
  print(":",1);
  print_int(mac_address[1]);
  print(":",1);
  print_int(mac_address[2]);
  print(":",1);
  print_int(mac_address[3]);
  print(":",1);
  print_int(mac_address[4]);
  print(":",1);
  print_int(mac_address[5]);
  println("",1);


  return true;
}



int network_init(){
   pci_device_t device = pci_get_device(NETCARD_VENDOR_ID, NETCARD_DEVICE_ID); 
   println("bus_number/device_number/function_number:", 41);
   print_int(device.bus_number);
   print("/",1);
   print_int(device.device_number);
   print("/",1);
   print_int(device.function_number);
   println("",0);


  rtl_net_card_init(device);
}
