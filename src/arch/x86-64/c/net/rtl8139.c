#include "pci.h"
#include "../core/port.h"
#include "rtl8139.h"
#include "../c_main.h"
#include "../interrupts/interrupt.h"
#include "../interrupts/isr.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define TX_ADDR_0 0x20  // +0x00, +0x04, +0x08, +0x0C for subsequent buffers
#define TX_STATUS_0 0x10 // +0x04, +0x08, +0x0C for subsequent buffers

static uint32_t current_tx_reg_addr;
static uint32_t current_status_reg_addr;

static uint32_t base_port_ioaddr = 0;


// uint8_t* rtl8139_get_mac_address();
// void rtl8139_transmit(uint8_t* data, uint32_t len);
// void rtl8139_receive();

static uint8_t tx_buffer[TX_BUFFER_SIZE] = {0};
static uint8_t rx_buffer[RX_BUFFER_SIZE] = {0};
uintptr_t rx_buffer_ptr = (uintptr_t) rx_buffer;

static uint64_t tx_buffer_addr = 0;
static uint32_t rx_index = 0;

static uint8_t transmit_start_registers[4] = { 0x20, 0x24, 0x28, 0x2C };
static uint8_t transmit_command_registers[4] = { 0x10, 0x14, 0x18, 0x1C };
static uint8_t current_transmit_pair = 0;

static uint8_t mac_address[6] = { 0 };




void rtl8139_receive()
{


 int16_t capr_value = port_word_in(base_port_ioaddr + RTL8139_REGISTER_RX_PTR);

  // 0x01 = buffer is empty
  while (true) {
   
    //Current Address of Packet Read pointer(CAPR)
    //CAPR register holds the offset from the start of the ring buffer
        println("-----------------------------------------------------: ",-1);
        println("rx_buffer_addr alligned: ",-1);
        print_hex((char*)rx_buffer_ptr,4);
        println("",-1);
        print_int_hex((int)rx_buffer_ptr);
        println("",-1);

        print("rtl8139_receive: capr_value: 0x",-1); //initial value is -16(FF)
        print_int_hex_t16(capr_value);
        print(" decimal: ",-1);
        print_int_t16(capr_value);
        println("",-1);
    
    
    // Calculate the offset into the receive buffer.
    //16-Byte Pre-Packet Gap:
    // This is a space before the packet's actual start.
    // It is reserved for DMA burst alignment and can be used by the card for internal purposes.
    // The driver typically skips these bytes to get to the actual packet data.
     
     uint32_t offset = capr_value + 0x10; // Get the current value and add 16 bytes to the header.
        print("rtl8139_receive: offset: ",-1);
        print_int(offset);
        println("",-1);
 
    // Read the packet header to get the packet length.
    // 4-Byte Packet Header:
    // The first two bytes of the header indicate the status of the received packet. It contains information such as whether the packet was received OK, whether it was multicast, broadcast, etc.
    // The next two bytes indicate the length of the packet, including the header itself.
    
    uint16_t *header_ptr = (uint16_t *)(rx_buffer_ptr + offset); 
    uint16_t pkt_length = header_ptr[1];  
    
        print("rtl8139_receive: pkt_length: ",-1);
        print_int_t16(pkt_length);
        print("  header: 0x",-1);
        print_hex((char*)header_ptr, 4);
        println("",-1);

     

    // Read the status from the first two bytes
    // Bit 0 (ROK): Receive OK. If this bit is set, it means that the packet has been received without any errors (including CRC and alignment errors).
    // Bit 1 (Fae): Frame Alignment Error. If this bit is set, it means that the incoming frame did not end on a byte boundary.
    // Bit 2 (Crc): CRC Error. Set if the cyclic redundancy check for the packet failed, indicating that the data may be corrupted.
    // Bit 3 (Long): Long Packet. Set if the received packet is longer than 4,096 bytes.
    // Bit 4 (Runt): Runt Packet. Set if the packet is less than the minimum Ethernet frame size, which is 64 bytes.
    // Bit 5 (Ise): Invalid Symbol Error. For Ethernet, this indicates problems with the preamble or there was no end-of-stream delimiter.
    // Bits 6-13: These bits are reserved or used for specific implementations or versions of the hardware and typically aren't used in general driver code.
    // Bit 14 (Bar): Broadcast Address Received. Set if the received packet is a broadcast frame, meaning the destination address was all '1's.
    // Bit 15 (Mar): Multicast Address Received. Set if the packet's destination is a multicast address.
     
     uint16_t status = header_ptr[0];

    if (!(status & (1 << 0))) { // Check if ROK bit is set
        //reset the buffer
    }
     
     if (pkt_length == 0) {
       //buffer empty
       break;
    }



    if (status & (1 << 1)) { // Check if FAE bit is set
    // Frame alignment error
    }

    
    char* body_ptr = (char*)rx_buffer_ptr + offset + 4; // 4 bytes for the packet header.

    println("body: ",-1);
    print_hex(body_ptr, pkt_length);
    println("",-1);

    uint16_t next_capr = (offset + 4 + pkt_length -16);
    print("rtl8139_receive: next capr: ",-1);
    print_int_t16(next_capr);
    println("",-1);

    // Update the CAPR register to the next packet.
    port_word_out(base_port_ioaddr + RTL8139_REGISTER_RX_PTR, next_capr); //pkt_length doesn't include header length 4 bytes -16 according to doc

   capr_value = next_capr;
   delay(900000000);


  }

  println("!!!!!!!!!!!!!rtl8139_receive: finish",-1);
}

static void net_callback(isr_stack_t* stack)
{
  static int counter = 1;

  println("net_callback: net interrupt received",-1);
  print("net_callback: counter: ",-1); print_int(counter++); println("",-1);
  

  // The loop is there in case we have more than one "thing" to handle when the
  // interrupt is triggered.
  while (1) {
    uint16_t status = port_word_in(base_port_ioaddr + RTL8139_REGISTER_ISR);
        print("net_callback: status: ",-1);
        print_int_hex(status);
        println("",-1);

    // ack interrupt.
    //To acknowledge the interrupt, the driver must write the same bit value back to the ISR.
    port_word_out(base_port_ioaddr + RTL8139_REGISTER_ISR, status);

    if (status == 0) {
      break;
    }

    if (status & RTL8139_ISR_TOK) {
      println("net_callback: frame transmitted", -1);
      print("net_callback: current_status_reg_addr: ", 0);
      print_bits(port_dword_in(current_status_reg_addr));
      println("", 0);
    }

    if (status & RTL8139_ISR_ROK) {
      println("net_callback: frame received", -1);
      rtl8139_receive();
    }
  }
 
 
}


bool rtl_net_card_init(pci_device_t device)
{
  uint32_t ret = pci_read(device, PCI_BASE_ADDR_0_FIELD_OFFSET, 4);
  println("BAR0:", 5);
  print_int(ret);
  println("",0);
  base_port_ioaddr = ret & (~0x3);  //ignoring two last digits of ret gives base port address
  println("base port address", 17);
  print_int_hex(base_port_ioaddr);
  println("",0);
 
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

       current_status_reg_addr = base_port_ioaddr + TX_STATUS_0;
       println("transmit_comand/status register: ",-1);
       print_int_hex(port_dword_in(current_status_reg_addr));
       println("", 0);
       print_bits(port_dword_in(current_status_reg_addr));
       println("", 0);

  // Initialize transmit buffer.
  tx_buffer_addr = (uint64_t) &tx_buffer;
       println("tx_buffer_addr: ",-1);
       print_int_hex(tx_buffer_addr);
       println("", 0);

  // Initialize receive buffer.
    println("rx_buffer address: ",-1);
    print_int_hex((uintptr_t)rx_buffer);
    println("",-1);
  //allign buffer in memory to 4 byte
  if((uintptr_t)rx_buffer % 4 !=0){
     uintptr_t raw_address = (uintptr_t)rx_buffer;
     uintptr_t offset = 4 - (raw_address % 4);
     rx_buffer_ptr += offset; // Now 'buffer' is aligned
  }
 
    println("rx_buffer address alligned: ",-1);
    print_int_hex((uintptr_t)rx_buffer_ptr);
    println("",-1);
  port_dword_out(base_port_ioaddr + RTL8139_REGISTER_RBSTART,
                 rx_buffer_ptr);

  // Set IMR + ISR.
  // '0x0005' sets the TOK and ROK bits high.
  port_word_out(base_port_ioaddr + RTL8139_REGISTER_IMR, 0x0005);

  // Configure receive buffer (RCR).
  //When a NIC is powered on or reset, it will generally assume a default configuration. For the RTL8139, the default receive buffer size might be the smallest available buffer size, or some manufacturer-defined value that ensures the NIC operates in a known state. For optimal performance, it is always recommended to explicitly set the buffer size in the Rx Config register to match the allocated buffer space in the driver.
  //The RTL8139 uses a 3-bit field to select the buffer size:
    // 000: 8K + 16 bytes
    // 001: 16K + 16 bytes
    // 010: 32K + 16 bytes
    // 011: 64K + 16 bytes
    //If WRAP is 0, the Rx buffer is treated as a traditional ring buffer: if a packet is being written near the end of the buffer and the RTL8139 knows you've already handled data before this (thanks to CAPR), the packet will continue at the beginning of the buffer.
    //If WRAP is 1, the remainder of the packet will be written contiguously (overflowing the actual receive buffer) so that it can be handled more efficiently. This means the buffer must be an additional 1500 bytes (to hold the largest potentially overflowing packet). 
  
    // AB (Accept Broadcast): Bit 0
    // AM (Accept Multicast): Bit 1
    // APM (All Physical Match): Bit 2
    //AAP (Accept All Packets): Bit 3
  port_dword_out(base_port_ioaddr + 0x44, 0x06 | (1 << 7)); //(1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP 0x07 = AB+AM+APM

  // Enable Receive and Transmit 
  port_byte_out(base_port_ioaddr + RTL8139_REGISTER_COMMAND, 0x0C);

  // Register an interrupt handler.
  uint8_t irq_num = pci_read(device, PCI_INTERRUPT_LINE, 1);
  println("irq_num",-1);
  print_int(irq_num);
  println("",-1);
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
  print_int_hex(mac_address[0]);
  print(":",1);
  print_int_hex(mac_address[1]);
  print(":",1);
  print_int_hex(mac_address[2]);
  print(":",1);
  print_int_hex(mac_address[3]);
  print(":",1);
  print_int_hex(mac_address[4]);
  print(":",1);
  print_int_hex(mac_address[5]);
  println("",1);


  return true;
}



int network_init(){
   println("call network_init",-1);
   pci_device_t device = pci_get_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID); 
   println("bus_number/device_number/function_number:", 41);
   print_int(device.bus_number);
   print("/",1);
   print_int(device.device_number);
   print("/",1);
   print_int(device.function_number);
   println("",0);


  rtl_net_card_init(device);
}


void send_packet(void* packet, uint32_t length) {

         print("send_packet: length: ",-1);
         print_int(length);
         println("",-1);

         println("send_packet: packet: ",-1);
         print("   ",-1);
         println_hex(packet, length);
       

    static unsigned current_tx_buffer = 0; // RTL8139 has 4 tx buffers
    current_tx_reg_addr = base_port_ioaddr + TX_ADDR_0 + (current_tx_buffer * 4);
    current_status_reg_addr = base_port_ioaddr + TX_STATUS_0 + (current_tx_buffer * 4);

    // Copy packet to the TX buffer (needs DMA memory in real-world scenario)
    memcpy(tx_buffer, packet, length);

    // Tell the NIC where to find the data to send

    port_dword_out(current_tx_reg_addr, (uint32_t)tx_buffer_addr);

    // Start transmission: length of the packet and OWN bit set to 0
           println("send_packet: transmit_comand/status register before reset: ",-1);
           print("   ",-1);
           print_bits(port_dword_in(current_status_reg_addr));
           println("", -1);
    port_dword_out(current_status_reg_addr, length & 0xDFFF);
           println("send_packet: transmit_comand/status register after reset: ",-1);
           print("   ",-1);
           print_bits(port_dword_in(current_status_reg_addr));
           println("", -1);
           delay(10000000000);
           println("send_packet: transmit_comand/status register after delay: ",-1);
           print("   ",-1);
           print_bits(port_dword_in(current_status_reg_addr));
           println("", -1);

    current_tx_buffer = (current_tx_buffer + 1) % 4;
}



// Define a struct for an Ethernet frame header
struct ethernet_frame_header {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t type;
} __attribute__((packed));

// Define a struct for an Ethernet frame
struct ethernet_frame {
    struct ethernet_frame_header header;
    uint8_t data[1500]; // Max Ethernet payload size
} __attribute__((packed));

uint16_t htons(uint16_t hostshort) {
    // Check if we are on a little-endian machine
    int num = 1;
    if(*(char *)&num == 1) {
        // Swap the bytes
        return (hostshort >> 8) | (hostshort << 8);
    }
    // If already big-endian, return the number as is
    return hostshort;
}

// IPv4 header (simplified, without options)
struct ipv4_header {
    uint8_t version_ihl;      // Version and IHL
    uint8_t type_of_service;  // Type of service
    uint16_t total_length;    // Total length
    uint16_t identification;  // Identification
    uint16_t flags_fragment_offset; // Flags and Fragment offset
    uint8_t ttl;              // Time to live
    uint8_t protocol;         // Protocol
    uint16_t checksum;        // Header checksum
    uint32_t src_ip;          // Source IP address
    uint32_t dest_ip;         // Destination IP address
} __attribute__((packed));

// Function to create an Ethernet frame
void create_ethernet_frame(struct ethernet_frame *frame, uint8_t *dst_mac, uint8_t *src_mac, uint16_t type, uint8_t *data, size_t data_length) {
    // Copy the destination MAC address
    memcpy(frame->header.dst_mac, dst_mac, sizeof(frame->header.dst_mac));

    // Copy the source MAC address
    memcpy(frame->header.src_mac, src_mac, sizeof(frame->header.src_mac));

    // Set the type (e.g., IPv4, ARP, etc.)
    frame->header.type = htons(type); // Ensure network byte order

    // Copy the data payload
    memcpy(frame->data, data, data_length);
}

uint32_t ip_str_to_num(const char *ip_str) {
    uint32_t ip = 0;
    uint8_t bytes[4] = {0};
    int byteIndex = 0;

    while (*ip_str) {
        if (*ip_str == '.') {
            byteIndex++;
            if (byteIndex > 3) {
                // Too many parts, not a valid IP.
                return 0;
            }
        } else if (*ip_str >= '0' && *ip_str <= '9') {
            bytes[byteIndex] = bytes[byteIndex] * 10 + (*ip_str - '0');
            if (bytes[byteIndex] > 255) {
                // Byte value too large, not a valid IP.
                return 0;
            }
        } else {
            // Invalid character in IP string.
            return 0;
        }
        ip_str++;
    }

    // Combine the four parts into a uint32_t.
    ip = (uint32_t)bytes[0] << 24 |
         (uint32_t)bytes[1] << 16 |
         (uint32_t)bytes[2] << 8  |
         (uint32_t)bytes[3];

    return ip;
}

// Function to check if the system is little-endian
int is_little_endian() {
    unsigned int x = 1;
    return *(char *)&x == 1;
}

// Custom htonl function
uint32_t htonl(uint32_t hostlong) {
    if (is_little_endian()) {
        // If the system is little-endian, swap the byte order
        return ((hostlong & 0xFF) << 24) | ((hostlong & 0xFF00) << 8) |
               ((hostlong & 0xFF0000) >> 8) | ((hostlong & 0xFF000000) >> 24);
    } else {
        // If the system is big-endian, no change is needed
        return hostlong;
    }
}



void send_ethernet_frame() {
    struct ethernet_frame frame;
    uint8_t dst_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Replace with actual destination MAC address
    uint16_t type = 0x0800; // Example EtherType for IPv4
 
  // IPv4 packet (payload) would be filled with actual data
    uint8_t ipv4_payload[sizeof(struct ipv4_header)] = {0}; // Dummy payload for example purposes

// Define source and destination IP addresses
    char src_ip_str[] = "192.168.1.2";
    char dest_ip_str[] = "192.168.1.12";
    // Convert IP address strings to uint32_t
    uint32_t src_ip = ip_str_to_num(src_ip_str);
    uint32_t dest_ip = ip_str_to_num(dest_ip_str);

     // Set some fields for IPv4 header for the sake of example
    struct ipv4_header *ip_hdr = (struct ipv4_header*)ipv4_payload;
    ip_hdr->version_ihl = 0x45; // IPv4, Header Length = 5 words (20 bytes)
    ip_hdr->ttl = 64; // Standard TTL value
    ip_hdr->protocol = 6; // TCP, for example
    ip_hdr->src_ip = htonl(src_ip); // Convert to network byte order
    ip_hdr->dest_ip = htonl(dest_ip); // Convert to network byte order
    
    // Set total length to the size of the header for now, as there's no additional data
    ip_hdr->total_length = htons(sizeof(struct ipv4_header));

    // The total length of the Ethernet frame = Ethernet header + payload
    size_t frame_length = sizeof(struct ethernet_frame_header) + sizeof(ipv4_payload);

    // Assemble the Ethernet frame
    create_ethernet_frame(&frame, dst_mac, mac_address, type, ipv4_payload, sizeof(ipv4_payload));

    unsigned char tx_buffer[] = {
    0xf5, 0xf2, 0x6d, 0x7f, 0x44, 0x02, 0xdc, 0x71,
    0x96, 0xc8, 0x65, 0x27, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x29, 0xe6, 0xdf, 0x40, 0x00, 0x80, 0x06,
    0xc8, 0x60, 0xc0, 0xa8, 0x00, 0x67, 0xad, 0xc2,
    0xdc, 0xbc, 0xe9, 0x10, 0x14, 0x6c, 0xf2, 0x54,
    0x89, 0xdf, 0xcd, 0x6a, 0xe5, 0x34, 0x50, 0x10,
    0x02, 0x01, 0x35, 0xf3, 0x00, 0x00, 0x00
};

 
    // while (true)
    // {
    //    send_packet(&frame,  frame_length);
    //    //send_packet(&tx_buffer,  55);
    //    delay(30000000000);
    //    print("send_ethernet_frame: delay", -1);
    // }
    
}

void send_packet_default(){
   send_ethernet_frame();

}