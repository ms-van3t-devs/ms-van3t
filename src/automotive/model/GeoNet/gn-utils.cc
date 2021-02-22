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

  Mac48Address getGNMac48(Address generic_eui)
  {
    Mac48Address mac48addr;

    // If only an EUI-64 identifier is available, try to compute a standard MAC-48 address
    if(generic_eui.GetLength ()==8)
    {
       uint8_t addr_buffer_64[8],addr_buffer_48[6];
       generic_eui.CopyTo (addr_buffer_64);

       // Shift again the 7th bit of the MAC address (from the left, i.e. the 2nd bit of addr_buffer_64[0])
       addr_buffer_64[0] ^= (1UL << 1);

       // Remove the center part of the MAC address and save to a new, smaller, buffer
       addr_buffer_48[0]=addr_buffer_64[0];
       addr_buffer_48[1]=addr_buffer_64[1];
       addr_buffer_48[2]=addr_buffer_64[2];
       addr_buffer_48[3]=addr_buffer_64[5];
       addr_buffer_48[4]=addr_buffer_64[6];
       addr_buffer_48[5]=addr_buffer_64[7];

       mac48addr.CopyFrom (addr_buffer_48);
    }
    else
    {
       mac48addr = Mac48Address::ConvertFrom (generic_eui);
    }

    return mac48addr;
  }
}
