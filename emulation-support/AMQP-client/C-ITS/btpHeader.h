//
// Created by carlos on 08/12/23.
//

#ifndef ZMQ_PROXY_BTPHEADER_H
#define ZMQ_PROXY_BTPHEADER_H

#include <cstdint>
#include <cstring>
#include "utils.h"

class btpHeader {
public:
    btpHeader();
    ~btpHeader();
    void removeHeader(unsigned char * buffer);

    //getters
    [[nodiscard]] uint16_t getDestPort() const {return m_destinationPort;}
    [[nodiscard]] uint16_t getSourcePort() const {return m_source_destInfo;}
    [[nodiscard]] uint16_t getDestPortInfo() const {return m_source_destInfo;}

    static uint32_t GetSerializedSize(void);
    void addHeader(uint16_t destPort, uint16_t destPortInfo, std::vector<unsigned char>& buffer);


private:
    uint16_t m_destinationPort; //!< Destination port
    uint16_t m_source_destInfo; //!< Source port/Destination port info

};


#endif //ZMQ_PROXY_BTPHEADER_H
