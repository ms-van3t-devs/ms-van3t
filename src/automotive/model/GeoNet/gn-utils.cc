#include "ns3/gn-utils.h"

namespace ns3 {
  PacketSocketAddress getGNAddress (uint32_t ifindex, Address physicalAddress)
  {
    PacketSocketAddress local;

    local.SetSingleDevice (ifindex);
    local.SetPhysicalAddress (physicalAddress);
    local.SetProtocol (GN_ETHERTYPE);

    return local;
  }
}
