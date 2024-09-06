//
// Created by carlos on 08/12/23.
//

#ifndef ZMQ_PROXY_SHBHEADER_H
#define ZMQ_PROXY_SHBHEADER_H

#include <cstdint>
#include <cstring>
#include "utils.h"

class shbHeader {

public:
    shbHeader();
    ~shbHeader();
    void removeHeader(unsigned char * buffer);

    //Getters
    [[nodiscard]] GNlpv_t GetLongPositionV() const {return m_sourcePV;}

    //Setters
    void SetLongPositionV(GNlpv_t longPositionVector) {m_sourcePV = longPositionVector;}

    void addHeader(std::vector<unsigned char>& buffer);

private:
    GNlpv_t m_sourcePV{};  //!Source long position vector
    uint8_t m_reserved{}; //! aux variable for reading reserved fields
    uint32_t m_reserved32{};

};


#endif //ZMQ_PROXY_SHBHEADER_H
