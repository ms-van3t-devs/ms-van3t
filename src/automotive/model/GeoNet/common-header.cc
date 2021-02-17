#include "common-header.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/address-utils.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE ("GNCommonHeader");
  NS_OBJECT_ENSURE_REGISTERED (GNCommonHeader);

  GNCommonHeader::GNCommonHeader()
  {
    NS_LOG_FUNCTION (this);
  }

  GNCommonHeader::~GNCommonHeader()
  {
    NS_LOG_FUNCTION (this);
  }

  TypeId
  GNCommonHeader::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::GNCommonHeader")
      .SetParent<Header> ()
      .SetGroupName ("Automotive")
      .AddConstructor<GNCommonHeader> ();
    return tid;
  }

  TypeId
  GNCommonHeader::GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  uint32_t
  GNCommonHeader::GetSerializedSize (void) const
  {
    return 8;
  }

  void
  GNCommonHeader::Print (std::ostream &os) const
  {
    os << m_headerType;
  }

  void
  GNCommonHeader::Serialize (Buffer::Iterator start) const
  {
    //ETSI EN 302 636-4-1 [9.8.4]
    Buffer::Iterator i = start;
    //Common Header aux variables
    uint8_t chNH = 0;
    chNH = m_nextHeader << 4;
    uint8_t headerType = 0;
    headerType = (m_headerType << 4) | (m_headerSubType);
    uint8_t chflag = 0;
    chflag = m_flag << 7;

    //Common Header      ETSI EN 302 636-4-1 [9.7]
    i.WriteU8 (chNH);
    i.WriteU8 (headerType);
    i.WriteU8 (m_trafficClass);
    i.WriteU8 (chflag);
    i.WriteHtonU16 (m_payload);
    i.WriteU8 (m_maxHopLimit);
    i.WriteU8 (0x00); //! Reserved
  }

  uint32_t
  GNCommonHeader::Deserialize (Buffer::Iterator start)
  {
    Buffer::Iterator i = start;
    //Common Header
    uint8_t chNH = 0;
    chNH = i.ReadU8 ();
    m_nextHeader = chNH >> 4;
    uint8_t headerType = 0;
    headerType = i.ReadU8 ();
    m_headerType = headerType >> 4;
    m_headerSubType = headerType & 0x0f;
    m_trafficClass = i.ReadU8 ();
    uint8_t chflag = 0;
    chflag = i.ReadU8 ();
    m_flag = chflag >>7;
    m_payload = i.ReadNtohU16 ();
    m_maxHopLimit = i.ReadU8 ();
    m_reserved = i.ReadU8 ();

    return GetSerializedSize ();
  }

}

