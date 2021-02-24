#ifndef BTP_H
#define BTP_H
#include "ns3/geonet.h"
#include "ns3/socket.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/vdpTraci.h"
#include "ns3/asn_utils.h"
#include "ns3/btpHeader.h"
#include "ns3/tsb-header.h"
#include "ns3/gn-address.h"
#include "ns3/btpdatarequest.h"

#define CA_PORT 2001
#define DEN_PORT 2002

namespace ns3
{
  class btp : public Object
  {

  public:
    static TypeId GetTypeId();
    btp();
    virtual ~btp();
    void setGeoNet(Ptr<GeoNet> geoNet){m_geonet = geoNet;}
    void setStationProperties(unsigned long fixed_stationid,long fixed_stationtype){m_geonet->setStationProperties(fixed_stationid,fixed_stationtype);}
    void setFixedPositionRSU(double latitude_deg, double longitude_deg) {m_geonet->setFixedPositionRSU(latitude_deg,longitude_deg);}
    void setStationID(unsigned long fixed_stationid) {m_geonet->setStationID(fixed_stationid);}
    void setStationType(long fixed_stationtype) {m_geonet->setStationType(fixed_stationtype);}
    void setVDP(VDP* vdp){m_geonet->setVDP(vdp);}
    void setSocketTx(Ptr<Socket> socket_tx) {m_geonet->setSocketTx(socket_tx);}
    void setSocketRx(Ptr<Socket> socket_rx);
    void addCAMRxCallback(std::function<void(BTPDataIndication_t,Address)> rx_callback) {m_cam_ReceiveCallback=rx_callback;}
    void addDENMRxCallback(std::function<void(BTPDataIndication_t,Address)> rx_callback) {m_denm_ReceiveCallback=rx_callback;}
    void sendBTP(BTPDataRequest_t dataRequest);
    void receiveBTP(GNDataIndication_t, Address address);
    void cleanup();

  private:

    Ptr<GeoNet> m_geonet;

    std::function<void(BTPDataIndication_t,Address)> m_cam_ReceiveCallback;
    std::function<void(BTPDataIndication_t,Address)> m_denm_ReceiveCallback;

  };
}

#endif // BTP_H
