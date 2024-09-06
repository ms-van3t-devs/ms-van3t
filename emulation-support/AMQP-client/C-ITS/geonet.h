//
// Created by carlos on 08/12/23.
//

#ifndef ZMQ_PROXY_GEONET_H
#define ZMQ_PROXY_GEONET_H

#include <cstdint>
#include "utils.h"
#include <cstdint>
#include <map>
#include <set>
#include <mutex>
#include "basicHeader.h"
#include "commonHeader.h"
#include "shbHeader.h"

class GeoNet {

public:

    GeoNet();
    GeoNet(uint64_t stationID, uint8_t stationType) { m_stationID = stationID; m_stationType = stationType;}

    ~GeoNet();

    gnError_e decodeGN(unsigned char * packet, GNDataIndication_t* dataIndication);
    std::vector<unsigned char> encodeGN(GNDataRequest_t dataRequest);
    void setEPV(GNlpv_t epv) {m_epv = epv;}


private:
    GNDataIndication_t* processSHB(GNDataIndication_t* dataIndication);
    std::vector<unsigned char> encodeSHB(GNDataRequest_t dataRequest, commonHeader commonHeader, basicHeader basicHeader);


    bool decodeLT(uint8_t lifeTime, double * seconds);
    uint8_t encodeLT(double seconds);

    uint64_t m_stationID;
    uint8_t m_stationType;
    GNlpv_t m_epv;

    //ETSI 302 636-4-1 ANNEX H: GeoNetworking protocol constans
    uint8_t m_GnPtotocolVersion = 1;
    uint8_t m_GnIfType = 1;
    uint32_t m_GnPaiInterval = 80;
    uint32_t m_GnMaxSduSize = 1398;
    uint8_t m_GnMaxGeoNetworkingHeaderSize = 88;
    uint8_t m_GnSecurity = 0; //!Disabled
    uint16_t m_GnDefaultTrafficClass = 0;
    uint8_t m_GnDefaultHopLimit = 10;
    bool m_GnIsMobile=true; //!To set wether if Mobile(1) or Stationary(0)
};


#endif //ZMQ_PROXY_GEONET_H
