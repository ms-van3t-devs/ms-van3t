//
// Created by carlos on 08/12/23.
//

#include <cstdint>
#include "commonHeader.h"
#include <cstring>
#include <netinet/in.h>

commonHeader::commonHeader() {
    m_nextHeader = 0;
    m_headerType = 0;
    m_headerSubType = 0;
    m_trafficClass = 0;
    m_flag = false;
    m_payload = 0;
    m_maxHopLimit = 0;
    m_reserved = 0;
}

commonHeader::~commonHeader() = default;

void
commonHeader::removeHeader(unsigned char *buffer) {

    uint8_t chNH = 0;
    chNH = (uint8_t) *buffer;
    buffer++;
    m_nextHeader = chNH >> 4;
    uint8_t headerType = 0;
    headerType = (uint8_t) *buffer;
    buffer++;
    m_headerType = headerType >> 4;
    m_headerSubType = headerType & 0x0f;
    m_trafficClass = (uint8_t) *buffer;
    buffer++;
    uint8_t chflag = 0;
    chflag = (uint8_t) *buffer;
    buffer++;
    m_flag = chflag >> 7;
    memcpy(&m_payload, buffer, sizeof(uint16_t));
    buffer += 2;
    m_payload = swap_16bit(m_payload);
    m_maxHopLimit = (uint8_t) *buffer;
    buffer++;
    m_reserved = (uint8_t) *buffer;
    buffer++;
}

void commonHeader::addHeader(std::vector<unsigned char> &buffer) {
    uint8_t chNH = 0;
    uint8_t headerType = 0;
    uint8_t chflag = 0;
    std::vector<uint8_t> payload;
    chNH = (m_nextHeader << 4);

    headerType = (m_headerType << 4) | (m_headerSubType & 0x0f);

    chflag = (m_flag << 7) | (m_reserved & 0x7f);

    //payload = (m_payload << 8) | (m_payload & 0xff);
    payload.push_back(m_payload >> 8);
    payload.push_back(m_payload & 0xff);

    buffer.insert(buffer.begin(), m_reserved);
    buffer.insert(buffer.begin(), m_maxHopLimit);
    buffer.insert(buffer.begin(), payload.begin(), payload.end());
    buffer.insert(buffer.begin(), chflag);
    buffer.insert(buffer.begin(), m_trafficClass);
    buffer.insert(buffer.begin(), headerType);
    buffer.insert(buffer.begin(), chNH);
}
