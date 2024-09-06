//
// Created by carlos on 08/12/23.
//

#ifndef ZMQ_PROXY_BTP_H
#define ZMQ_PROXY_BTP_H
#include "utils.h"
#include "btpHeader.h"
#include <cstdint>
#include <cstring>

#define CA_PORT 2001
#define CP_PORT 2009

class btp {

public :
    btp();
    ~btp();
    btpError_e decodeBTP(GNDataIndication_t dataIndication, BTPDataIndication_t* btpDataIndication);
    GNDataRequest_t encodeBTP(BTPDataRequest_t dataRequest);

};


#endif //ZMQ_PROXY_BTP_H
