//
// Created by carlos on 08/12/23.
//

#ifndef ZMQ_PROXY_ETSIMESSAGEHANDLER_H
#define ZMQ_PROXY_ETSIMESSAGEHANDLER_H

#include "utils.h"
#include <cinttypes>
#include <cstddef>
#include <nlohmann/json.hpp>
#include <iostream>
#include "geonet.h"
#include "btp.h"
#include "basicHeader.h"
#include "commonHeader.h"
#include "shbHeader.h"
#include "utils.h"
#include "Seq.hpp"
#include "Getter.hpp"
#include "Setter.hpp"
#include "Encoding.hpp"
#include "SetOf.hpp"
#include "SequenceOf.hpp"
#include "BitString.hpp"

#define FIX_DENMID          0x01
#define FIX_CAMID           0x02
#define FIX_IVIMID          0x06
#define DECI                10
#define CENTI               100
#define MILLI               1000
#define MICRO               1000000
#define DOT_ONE_MICRO       10000000
#define NANO_TO_MILLI       1000000
#define NANO_TO_CENTI       10000000

#define DEG_2_RAD(val) ((val)*M_PI/180.0)
extern "C" {
#include "ASN1/CAM.h"
#include "ASN1/CPM.h"
}

class ETSImessageHandler {

public:
    typedef enum {
        ETSI_DECODED_ERROR = 0,
        ETSI_DECODED_CAM = 1,
        ETSI_DECODED_CPM = 2,
    } etsiDecodedType_e;

    typedef struct etsiDecodedData {
        void *decoded_msg;
        etsiDecodedType_e type;
        asn1cpp::Seq<CAM> decoded_cam;
        asn1cpp::Seq<CPM> decoded_cpm;
        //json
        nlohmann::basic_json<> json_msg;
        uint32_t gnTimestamp;
    } etsiDecodedData_t;

    ETSImessageHandler();
    ~ETSImessageHandler();

    int decodeMessage(uint8_t *buffer,size_t buflen,etsiDecodedData_t &decoded_data);

    std::vector<unsigned char> encodeMessage(nlohmann::basic_json<> json_msg);

    long getTimestamp();

private:
    std::string getStationType(int stationType);
    std::vector<unsigned char> encodeCAM(nlohmann::basic_json<> json_msg);
    std::vector<unsigned char> encodeCPM(nlohmann::basic_json<> json_msg);

};


#endif //ZMQ_PROXY_ETSIMESSAGEHANDLER_H
