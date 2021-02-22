#ifndef GN_UTILS_H
#define GN_UTILS_H

#include "ns3/packet-socket-address.h"

#define GN_ETHERTYPE 0x8947

namespace ns3
{
  // This function returns a Packet Socket address, to be used in the sockets created for GeoNetworking,
  // given an interface index and a physical address. The correct Ethertype is automatically inserted.
  PacketSocketAddress getGNAddress (uint32_t ifindex, Address physicalAddress);

  // This function return a MAC-48 address, given any MAC-48 or EUI-64 ("mac64") identifier
  // In the second case, a MAC-48 address is calculated starting from the EUI-64 ("mac64") identifier, for usage inside GeoNetworking
  Mac48Address getGNMac48(Address generic_eui);
}

#endif // GN_UTILS_H

