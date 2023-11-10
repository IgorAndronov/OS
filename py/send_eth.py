from scapy.all import *

# Create an Ethernet frame with a broadcast destination MAC address
ether = Ether(dst="52:55:00:d1:55:01") #02:ca:38:5c:ae:55  52:55:00:d1:55:01

# You can add other layers (like IP) depending on your needs
# For example, to add an IP layer: ip = IP(dst="192.168.1.255")

# Craft the packet (just an Ethernet frame in this case)
packet = ether / "Your payload here"

# Send the packet
sendp(packet, iface="tap0")
