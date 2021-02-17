#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H
#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/gn-address.h"
#include "ns3/mac48-address.h"

namespace ns3 {


class GNCommonHeader : public Header
{
  public:
    GNCommonHeader();
    ~GNCommonHeader();
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

    //Setters
    void SetNextHeader(uint8_t NH) {m_nextHeader = NH;}
    void SetHeaderType(uint8_t HT) {m_headerType = HT;}
    void SetHeaderSubType(uint8_t HST) {m_headerSubType = HST;}
    void SetTrafficClass(uint8_t TC) {m_trafficClass = TC;}
    void SetFlag(bool flag) {m_flag = flag;}
    void SetPayload (uint16_t PL) {m_payload = PL;}
    void SetMaxHL(uint8_t MHL){m_maxHopLimit = MHL;}

    //Getters
    uint8_t GetNextHeader() {return m_nextHeader;}
    uint8_t GetHeaderType() {return m_headerType;}
    uint8_t GetHeaderSubType() {return m_headerSubType;}
    uint8_t GetTrafficClass() {return m_trafficClass;}
    bool GetFlag() {return m_flag;}
    uint16_t GetPayload() {return m_payload;}
    uint8_t GetMaxHopLimit() {return m_maxHopLimit;}

  private:
    uint8_t m_nextHeader : 4;
    uint8_t m_headerType : 4;
    uint8_t m_headerSubType : 4;
    uint8_t m_trafficClass;
    bool m_flag : 1;
    uint16_t m_payload;
    uint8_t m_maxHopLimit;
    uint8_t m_reserved;
  };
}
#endif // COMMON_HEADER_H
