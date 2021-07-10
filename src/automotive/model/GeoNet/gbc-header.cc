#include "gbc-header.h"
#include "shb-header.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/address-utils.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE ("GBCheader");
  NS_OBJECT_ENSURE_REGISTERED (GBCheader);

  GBCheader::GBCheader()
  {
    NS_LOG_FUNCTION (this);
  }

  GBCheader::~GBCheader()
  {
    NS_LOG_FUNCTION (this);
  }

  TypeId
  GBCheader::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::GBCheader")
      .SetParent<Header> ()
      .SetGroupName ("Automotive")
      .AddConstructor<GBCheader> ();
    return tid;
  }

  TypeId
  GBCheader::GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  uint32_t
  GBCheader::GetSerializedSize (void) const
  {
    return 44;
  }

  void
  GBCheader::Print (std::ostream &os) const
  {
    os << m_sourcePV.GnAddress;
  }

  void
  GBCheader::Serialize (Buffer::Iterator start) const
  {
    Buffer::Iterator i = start;
    //Source long position vector aux varaibles
    uint16_t pai_speed = 0;
    pai_speed = (m_sourcePV.positionAccuracy << 15) | ((m_sourcePV.speed)>>1); //Speed >> 1 so that sign isnt lost

    //ETSI EN 302 636-4-1 [9.8.5]
    //Sequence Number
    i.WriteHtonU16 (m_seqNumber);
    //Reserved
    i.WriteHtonU16 (0x0000);

    //Source long position vector
    WriteTo (i,m_sourcePV.GnAddress.ConvertTo ());
    i.WriteHtonU32 (m_sourcePV.TST);
    i.WriteHtonU32 (m_sourcePV.latitude);
    i.WriteHtonU32 (m_sourcePV.longitude);
    i.WriteHtonU16 (pai_speed);
    i.WriteHtonU16 (m_sourcePV.heading);
    //GeoArea position latitude
    i.WriteHtonU32 (m_posLat);
    //GeoArea position longitude
    i.WriteHtonU32 (m_posLong);
    //Distance A
    i.WriteHtonU16 (m_distA);
    //Distance B
    i.WriteHtonU16 (m_distB);
    //Angle
    i.WriteHtonU16 (m_angle);
    //Reserved
    i.WriteHtonU16 (0x0000);
  }

  uint32_t
  GBCheader::Deserialize (Buffer::Iterator start)
  {
    Buffer::Iterator i = start;
    //Sequence Number
    m_seqNumber= i.ReadNtohU16 ();
    //Reserved
    m_reserved = i.ReadNtohU16 ();

    //Source long position vector
    uint8_t addr[8];
    i.Read (addr,8);
    m_sourcePV.GnAddress.Set (addr);
    m_sourcePV.TST = i.ReadNtohU32 ();
    m_sourcePV.latitude = i.ReadNtohU32 ();
    m_sourcePV.longitude = i.ReadNtohU32 ();
    uint16_t pai_speed = 0;
    pai_speed = i.ReadU16 ();
    m_sourcePV.positionAccuracy = pai_speed >> 15;
    m_sourcePV.speed = pai_speed & 0x7fff;
    m_sourcePV.heading = i.ReadNtohU16 ();

    //GeoArea position latitude
    m_posLat = i.ReadNtohU32 ();
    //GeoArea position longitude
    m_posLong = i.ReadNtohU32 ();
    //Distance A
    m_distA = i.ReadNtohU16 ();
    //Distance B
    m_distB = i.ReadNtohU16 ();
    //Angle
    m_angle = i.ReadNtohU16 ();
    //Reserved
    m_reserved = i.ReadNtohU16 ();

    return GetSerializedSize ();
  }

  void
  GBCheader::SetGeoArea (GeoArea_t geoArea)
  {
    m_posLat = geoArea.posLat;
    m_posLong = geoArea.posLong;
    m_distA = geoArea.distA;
    m_distB = geoArea.distB;
    m_angle = geoArea.angle;
  }

  GeoArea_t
  GBCheader::GetGeoArea() const
  {
    GeoArea_t retval;
    retval.posLat = m_posLat;
    retval.posLong = m_posLong;
    retval.distA = m_distA;
    retval.distB = m_distB;
    retval.angle = m_angle;
    return retval;
  }
}
