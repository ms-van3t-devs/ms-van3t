#ifndef GN_UTILS_H
#define GN_UTILS_H

#include "ns3/packet-socket-address.h"

#define GN_ETHERTYPE 0x8947

namespace ns3
{
  PacketSocketAddress getGNAddress (uint32_t ifindex, Address physicalAddress);
}

#endif // GN_UTILS_H

