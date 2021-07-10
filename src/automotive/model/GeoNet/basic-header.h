#ifndef BASIC_HEADER_H
#define BASIC_HEADER_H
#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/gn-address.h"
#include "ns3/mac48-address.h"

namespace ns3 {

  class GNBasicHeader : public Header
  {
    public:
      GNBasicHeader();
      ~GNBasicHeader();
      static TypeId GetTypeId (void);
      virtual TypeId GetInstanceTypeId (void) const;
      virtual void Print (std::ostream &os) const;
      virtual uint32_t GetSerializedSize (void) const;
      virtual void Serialize (Buffer::Iterator start) const;
      virtual uint32_t Deserialize (Buffer::Iterator start);

      //Setters
      void SetVersion(uint8_t version) {m_version = version;}
      void SetNextHeader(uint8_t NH) {m_nextHeader = NH;}
      void SetLifeTime(uint8_t LT) {m_lifeTime = LT;}
      void SetRemainingHL(uint8_t RHL){m_remainingHopLimit = RHL;}

      //Getters
      uint8_t GetVersion() {return m_version;}
      uint8_t GetNextHeader() {return m_nextHeader;}
      uint8_t GetLifeTime() {return m_lifeTime;}
      uint8_t GetRemainingHL(){return m_remainingHopLimit;}


    private:
      uint8_t m_version : 4;
      uint8_t m_nextHeader : 4;
      uint8_t m_reserved;
      uint8_t m_lifeTime;
      uint8_t m_remainingHopLimit;

  };

}
#endif // BASIC_HEADER_H
