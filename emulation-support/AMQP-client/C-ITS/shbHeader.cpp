//
// Created by carlos on 08/12/23.
//

#include "shbHeader.h"

shbHeader::shbHeader() {
    m_sourcePV = {};  //!Source long position vector
    m_reserved = 0; //! aux variable for reading reserved fields
    m_reserved32 = 0;
}

shbHeader::~shbHeader() = default;

void shbHeader::removeHeader(unsigned char *buffer) {

    //Source long position vector
    memcpy(m_sourcePV.GnAddress, buffer, 8);
    buffer += 8;
    memcpy(&m_sourcePV.TST, buffer, sizeof(uint32_t));
    buffer += 4;
    m_sourcePV.TST = swap_32bit(m_sourcePV.TST);
    memcpy(&m_sourcePV.latitude, buffer, sizeof(uint32_t));
    buffer += 4;
    m_sourcePV.latitude = swap_32bit(m_sourcePV.latitude);
    memcpy(&m_sourcePV.longitude, buffer, sizeof(uint32_t));
    buffer += 4;
    m_sourcePV.longitude = swap_32bit(m_sourcePV.longitude);
    uint16_t pai_speed = 0;
    memcpy(&pai_speed, buffer, sizeof(uint16_t));
    buffer += 2;
    pai_speed = swap_16bit(pai_speed);
    m_sourcePV.positionAccuracy = pai_speed >> 15;
    m_sourcePV.speed = pai_speed & 0x7f;
    memcpy(&m_sourcePV.heading, buffer, sizeof(uint16_t));
    buffer += 2;
    m_sourcePV.heading = swap_16bit(m_sourcePV.heading);
    buffer += 4; //Reserved
}

void shbHeader::addHeader(std::vector<unsigned char> &buffer) {
    //Source long position vector
    uint32_t TST = swap_32bit(m_sourcePV.TST);
    uint32_t latitude = swap_32bit(m_sourcePV.latitude);
    uint32_t longitude = swap_32bit(m_sourcePV.longitude);
    uint16_t pai_speed = 0;
    pai_speed = (m_sourcePV.positionAccuracy << 15) | (m_sourcePV.speed & 0x7f);
    pai_speed = swap_16bit(pai_speed);
    uint16_t heading = swap_16bit(m_sourcePV.heading);

    buffer.insert(buffer.begin(), 4, 0x00); //Reserved
    buffer.insert(buffer.begin(), (unsigned char *) &heading, (unsigned char *) &heading + sizeof(uint16_t));
    buffer.insert(buffer.begin(), (unsigned char *) &pai_speed, (unsigned char *) &pai_speed + sizeof(uint16_t));
    buffer.insert(buffer.begin(), (unsigned char *) &longitude, (unsigned char *) &longitude + sizeof(uint32_t));
    buffer.insert(buffer.begin(), (unsigned char *) &latitude, (unsigned char *) &latitude + sizeof(uint32_t));
    buffer.insert(buffer.begin(), (unsigned char *) &TST, (unsigned char *) &TST + sizeof(uint32_t));
    buffer.insert(buffer.begin(), m_sourcePV.GnAddress, m_sourcePV.GnAddress + 8);



}
