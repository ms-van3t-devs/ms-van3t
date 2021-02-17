#ifndef GNDATAINDICATION_H
#define GNDATAINDICATION_H
#include <stdint.h>
#include "ns3/packet.h"
#include "ns3/gn-address.h"
#include "longpositionvector.h"

namespace ns3 {

  //GN-DATA.indication primitive implementation as specified in ETSI EN 302 636-4-1 [J.4]

  typedef enum {
    BTP=0,
    GN6ASL=1
  } UpperProtocol_t;

  typedef struct _gndataRequest {
    UpperProtocol_t upperProtocol;
    //TransportType_t GNType; // GN Packet transport type -- GeoUnicast, SHB, TSB, GeoBroadcast or GeoAnycast
    GNAddress GnAddressDest; // GN destination adress -- destination adress(GeoUnicast) or geo. area (GeoBroadcast or GeoAnycast) with which the packet was generated with
    //CommProfile_t GNCommProfile; // GN Communication Profile -- determines de LL protocol entity
    GNlpv_t SourcePV;
    //Security report /OPCIONAL/
    //Certificate ID/OPCIONAL/
    int32_t GNITS_AIDL;//Length of the value provided in the ITS-AID parameter /OPCIONAL/
    int32_t GNITS_AID;// ITS-AID for the payload to be sent/OPCIONAL/
    int32_t GNSecurityPermL;// Length of the security permissions parameter/OPCIONAL/
    int32_t GNSecurityPerm;// SSP associated with the ITS-AID/OPCIONAL/
    int32_t GNSecurityContInfo;// Information to be used to selecting properties of the security protocol/OPCIONAL/

    uint8_t GNTraClass; // GN Traffic Class
    double GNRemainingLife; //GN Remaining Packet Lifetime in [s] /OPCIONAL/
    int16_t GNRemainingHL; // GN Remaining Hop Limit /OPCIONAL/

    uint32_t lenght; // Payload size
    Ptr<Packet> data; // Payload
  } GNDataIndication_t;
}
#endif // GNDATAINDICATION_H
