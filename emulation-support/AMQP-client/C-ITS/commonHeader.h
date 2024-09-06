//
// Created by carlos on 08/12/23.
//

#ifndef ZMQ_PROXY_COMMONHEADER_H
#define ZMQ_PROXY_COMMONHEADER_H

#include <cstdint>
#include "utils.h"


class commonHeader {
public:
    commonHeader();
    ~commonHeader();
    void removeHeader(unsigned char * buffer);

    //Getters
    [[nodiscard]] uint8_t GetNextHeader() const {return m_nextHeader;}
    [[nodiscard]] uint8_t GetHeaderType() const {return m_headerType;}
    [[nodiscard]] uint8_t GetHeaderSubType() const {return m_headerSubType;}
    [[nodiscard]] uint8_t GetTrafficClass() const {return m_trafficClass;}
    [[nodiscard]] bool GetFlag() const {return m_flag;}
    [[nodiscard]] uint16_t GetPayload() const {return m_payload;}
    [[nodiscard]] uint8_t GetMaxHopLimit() const {return m_maxHopLimit;}

    //Setters
    void SetNextHeader(uint8_t NH) {m_nextHeader = NH;}
    void SetHeaderType(uint8_t HT) {m_headerType = HT;}
    void SetHeaderSubType(uint8_t HST) {m_headerSubType = HST;}
    void SetTrafficClass(uint8_t TC) {m_trafficClass = TC;}
    void SetFlag(bool flag) {m_flag = flag;}
    void SetPayload (uint16_t PL) {m_payload = PL;}
    void SetMaxHL(uint8_t MHL){m_maxHopLimit = MHL;}

    void addHeader(std::vector<unsigned char>& buffer);


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


#endif //ZMQ_PROXY_COMMONHEADER_H
