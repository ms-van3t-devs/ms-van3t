#include "gn-address.h"
#include <iomanip>
#include <memory.h>

#include "ns3/log.h"
#include "ns3/assert.h"

#include "ns3/mac48-address.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE ("GNAddress");

  GNAddress::GNAddress()
  {
    NS_LOG_FUNCTION (this);
    memset (m_address, 0x00, 8);
  }

  GNAddress::GNAddress (uint8_t address[8])
  {
    NS_LOG_FUNCTION (this << &address);
    /* 64 bit => 8 bytes */
    memcpy (m_address, address, 8);
  }

  GNAddress GNAddress::MakeManagedconfiguredAddress (Mac48Address addr, uint8_t ITSType)
  {
    NS_LOG_FUNCTION (addr);
    GNAddress ret;
    uint8_t buf[8];
    uint8_t buf2[8];

    addr.CopyTo (buf);
    //ETSI EN 302 636-4-1 [6.3]
    buf2[0] = 0x00 | ITSType << 2; //Initial GeoNetAddress ->M=0 and 5bit ITS-S type
    buf2[1] = 0x00; //Reserved
    memcpy (buf2 + 2, buf, 6);

    ret.Set (buf2);
    return ret;
  }

  Mac48Address
  GNAddress::GetLLAddress ()
  {
    //Mac address located in the last 6 bytes of m_addres
    uint8_t buf[6];
    memcpy(buf,m_address+2,6);
    Mac48Address return_mac;
    return_mac.CopyFrom (buf);
    return return_mac;
  }

  void GNAddress::Set (uint8_t address[8])
  {
    /* 64 bit => 8 bytes */
    NS_LOG_FUNCTION (this << &address);
    memcpy (m_address, address, 8);
  }

  void GNAddress::Serialize (uint8_t buf[8]) const
  {
    //NS_LOG_FUNCTION (this << &buf);
    memcpy (buf, m_address, 8);
  }

  GNAddress GNAddress::Deserialize (const uint8_t buf[8])
  {
    NS_LOG_FUNCTION (&buf);
    GNAddress gn ((uint8_t*)buf);
    return gn;
  }

  GNAddress::operator Address () const
  {
    return ConvertTo ();
  }

  Address GNAddress::ConvertTo (void) const
  {
    NS_LOG_FUNCTION (this);
    uint8_t buf[8];
    Serialize (buf);
    return Address (GetType (), buf, 8);
  }

  GNAddress GNAddress::ConvertFrom (const Address &address)
  {
    NS_LOG_FUNCTION (address);
    NS_ASSERT (address.CheckCompatible (GetType (), 2));
    uint8_t buf[8];
    address.CopyTo (buf);
    return Deserialize (buf);
  }

  uint8_t GNAddress::GetType (void)
  {
    NS_LOG_FUNCTION_NOARGS ();
    static uint8_t type = Address::Register ();
    return type;
  }

}
