#ifndef GBCHEADER_H
#define GBCHEADER_H
#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/gn-address.h"
#include "ns3/mac48-address.h"
#include "ns3/longpositionvector.h"
#include "ns3/btpdatarequest.h"

namespace ns3 {

  class GBCheader : public Header
  {
    public:
      GBCheader();
      ~GBCheader();
      static TypeId GetTypeId (void);
      virtual TypeId GetInstanceTypeId (void) const;
      virtual void Print (std::ostream &os) const;
      virtual uint32_t GetSerializedSize (void) const;
      virtual void Serialize (Buffer::Iterator start) const;
      virtual uint32_t Deserialize (Buffer::Iterator start);

      //Setters
      void SetLongPositionV(GNlpv_t longPositionVector) {m_sourcePV = longPositionVector;}
      void SetSeqNumber(uint16_t seqNumber){m_seqNumber = seqNumber;}
      void SetPosLong(int32_t posLong){m_posLong = posLong;}
      void SetPosLat(int32_t posLat){m_posLat = posLat;}
      void SetDistA(uint16_t distA){m_distA = distA;}
      void SetDistB(uint16_t distB){m_distA = distB;}
      void SetAngle(uint16_t angle){m_angle = angle;}
      void SetGeoArea(GeoArea_t geoArea);

      //Getters
      GNlpv_t GetLongPositionV(void) const {return m_sourcePV;}
      uint16_t GetSeqNumber(void) const {return m_seqNumber;}
      int32_t GetPosLong(void) const {return m_posLong;}
      int32_t GetPosLat(void) const {return m_posLat;}
      uint16_t GetDistA(void) const {return m_distA;}
      uint16_t GetDistB(void) const {return m_distB;}
      uint16_t GetAngle(void) const {return m_angle;}
      GeoArea_t GetGeoArea(void) const;

    private:
      GNlpv_t m_sourcePV;  //!Source long position vector
      uint16_t m_seqNumber;
      int32_t m_posLong;
      int32_t m_posLat;
      uint16_t m_distA;
      uint16_t m_distB;
      uint16_t m_angle;
      uint16_t m_reserved; //! aux variable for reading reserved fields


  };
}
#endif // GBCHEADER_H
