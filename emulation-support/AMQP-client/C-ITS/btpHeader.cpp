//
// Created by carlos on 08/12/23.
//

#include <vector>
#include "btpHeader.h"
#include <netinet/in.h>

btpHeader::btpHeader() {
    m_source_destInfo = 0;
    m_destinationPort = 0;
}

btpHeader::~btpHeader() = default;

void btpHeader::removeHeader(unsigned char * buffer) {

    memcpy(&m_destinationPort, buffer, sizeof(uint16_t));
    buffer += 2;
    m_destinationPort = swap_16bit(m_destinationPort);

    memcpy(&m_source_destInfo, buffer, sizeof(uint16_t));
    buffer += 2;
    m_source_destInfo = swap_16bit(m_source_destInfo);
}

uint32_t
btpHeader::GetSerializedSize ()
{
    return 4;
}

void
btpHeader::addHeader (uint16_t destPort, uint16_t destPortInfo, std::vector<unsigned char>& buffer)
{
    std::vector<uint8_t> temp(GetSerializedSize());

    uint16_t destinationPortNBO = htons(destPort);
    uint16_t sourceDestInfoNBO = htons(destPortInfo);

    // Serialize the header into a temporary vector
    temp[0] = destinationPortNBO & 0xFF;
    temp[1] = destinationPortNBO >> 8;
    temp[2] = sourceDestInfoNBO & 0xFF;
    temp[3] = sourceDestInfoNBO >> 8;

    buffer.insert(buffer.begin(), temp.begin(), temp.end());

}