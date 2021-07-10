#include "btpHeader.h"
#include "ns3/address-utils.h"

namespace ns3
{

  btpHeader::btpHeader()
  {
      m_source_destInfo = 0;
      m_destinationPort = 0;

  }
  btpHeader::~btpHeader ()
  {
    m_source_destInfo = 0xfffe;
    m_destinationPort = 0xfffe;
  }

  void
  btpHeader::SetDestinationPort (uint16_t port)
  {
    m_destinationPort = port;
  }

  void
  btpHeader::SetDestinationPortInfo (uint16_t portInfo)
  {
    m_source_destInfo = portInfo;
  }

  void
  btpHeader::SetSourcePort (uint16_t port)
  {
    m_source_destInfo = port;
  }

  uint16_t
  btpHeader::GetSourcePort (void) const
  {
    return m_source_destInfo;
  }

  uint16_t
  btpHeader::GetDestinationPort (void) const
  {
    return m_destinationPort;
  }

  uint16_t
  btpHeader::GetDestinationPortInfo (void) const
  {
    return m_source_destInfo;
  }

  TypeId
  btpHeader::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::btpHeader")
      .SetParent<Header> ()
      .SetGroupName ("Automotive")
      .AddConstructor<btpHeader> ();
    return tid;
  }

  TypeId
  btpHeader::GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  void
  btpHeader::Print (std::ostream &os) const
  {
    os << m_source_destInfo << " > " << m_destinationPort;
  }

  uint32_t
  btpHeader::GetSerializedSize (void) const
  {
    return 4;
  }

  void
  btpHeader::Serialize (Buffer::Iterator start) const
  {
    Buffer::Iterator i = start;
    //ETSI EN 302 636-5-1 [7.2.1-2]
    i.WriteHtonU16 (m_destinationPort);
    i.WriteHtonU16 (m_source_destInfo);
  }

  uint32_t
  btpHeader::Deserialize (Buffer::Iterator start)
  {
    Buffer::Iterator i = start;
    m_destinationPort = i.ReadNtohU16 ();
    m_source_destInfo = i.ReadNtohU16 ();
    return GetSerializedSize ();
  }


}
