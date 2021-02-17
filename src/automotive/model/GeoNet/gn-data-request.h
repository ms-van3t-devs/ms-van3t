#ifndef GNDATAREQUEST_H
#define GNDATAREQUEST_H
#include <stdint.h>
#include "ns3/packet.h"
#include "ns3/gn-address.h"

namespace ns3 {

  //GN-DATA.request primitive implementation as specified in ETSI EN 302 636-4-1 [J.2]
  typedef enum {
    GUC=0,
    GAC=1,
    GBC=2,
    TSB=3,
    SHB=4
  } TransportType_t;

  typedef enum {
    UNSPECIFIED=0,
    ITS_G5=1
  } CommProfile_t;

  typedef enum {
    BTP=0,
    GN6ASL=1
  } UpperProtocol_t;

  typedef struct _dataRequest {
    UpperProtocol_t upperProtocol;
    TransportType_t GNType; // GN Packet transport type -- GeoUnicast, SHB, TSB, GeoBroadcast or GeoAnycast
    GNAddress GnAddress; // GN destination adress -- destination adress(GeoUnicast) or geo. area (GeoBroadcast or GeoAnycast)
    CommProfile_t GNCommProfile; // GN Communication Profile -- determines de LL protocol entity

    int16_t GNSecurityP; // GN Security Profile /OPCIONAL/
    int32_t GNITS_AIDL;//Length of the value provided in the ITS-AID parameter /OPCIONAL/
    int32_t GNITS_AID;// ITS-AID for the payload to be sent/OPCIONAL/
    int32_t GNSecurityPermL;// Length of the security permissions parameter/OPCIONAL/
    int32_t GNSecurityPerm;// SSP associated with the ITS-AID/OPCIONAL/
    int32_t GNSecurityContInfo;// Information to be used to selecting properties of the security protocol/OPCIONAL/
    int32_t GNSecurityTargetIDListL; //Length for the value in the security target id list parameter /OPCIONAL/
    int32_t GNSecurityTargetIDList;// Unordered collection of target IDs used by the security entity for specifying multiple recipients /OPCIONAL/
    double GNMaxLife; //GN Maximum Packet Lifetime in [s] /OPCIONAL/
    int16_t GNRepInt; // GN Repetition Interval /OPCIONAL/
    int16_t GNMaxRepTime; // GN maximum repetition time /OPCIONAL/
    int16_t GNMaxHL; // GN Max Hop Limit /OPCIONAL/
    uint8_t GNTraClass; // GN Traffic Class

    uint32_t lenght; // Payload size
    Ptr<Packet> data; // Payload
  } GNDataRequest_t;
}


#endif // GNDATAREQUEST_H
