#ifndef BSCONTAINER_H
#define BSCONTAINER_H

#include "ns3/caBasicService.h"
#include "ns3/denBasicService.h"
#include "ns3/PRRSupervisor.h"

namespace ns3
{
  class BSContainer: public Object
  {
  public:
    BSContainer();
    ~BSContainer();
    BSContainer(unsigned long fixed_stationid,long fixed_stationtype,Ptr<TraciClient> mobility_client,bool real_time,Ptr<Socket> socket_rxtx);

    void setStationProperties(unsigned long fixed_stationid,long fixed_stationtype);
    void setStationID(unsigned long fixed_stationid);
    void setStationType(long fixed_stationtype);
    void setSocket(Ptr<Socket> socket_rxtx);
    void setMobilityClient(Ptr<TraciClient> mobility_client) {m_mobility_client=mobility_client;}

    void linkPRRSupervisor(Ptr<PRRSupervisor> prrsup) {m_prrsup_ptr=prrsup;}
    void enablePRRSupervisorForGNBeacons() {m_prrsup_beacons=true;}
    void disablePRRSupervisorForGNBeacons() {m_prrsup_beacons=false;}

    // This simple wrapper module supports only extended callbacks
    void addCAMRxCallback(std::function<void(asn1cpp::Seq<CAM>, Address, StationID_t, StationType_t, SignalInfo)> rx_callback) {m_CAReceiveCallbackExtended=rx_callback;}
    void addDENMRxCallback(std::function<void(denData,Address,unsigned long,long,SignalInfo)> rx_callback) {m_DENReceiveCallbackExtended=rx_callback;}

    void setRealTime(bool real_time){m_real_time=real_time;}

    // ETSI Basic Services
    // Additional Basic Service getters should be added here
    Ptr<CABasicService> getCABasicService() {return &m_cabs;}
    Ptr<DENBasicService> getDENBasicService() {return &m_denbs;}

    // Function to easily retrieve a pointer to the Mobility Client leveraged by this BSContainer
    // Currently, only SUMO/TraCI clients are supported
    Ptr<TraciClient> getTraCIclient() {return m_mobility_client;}

    StationID_t getVehicleID() {return m_station_id;}

    // This function can be used to change the prefix this module supposes for all vehicle IDs in SUMO
    // If a new Basic Service container is created for station ID = 7, for example, this module
    // assumes that it will be related to "veh7" in the XML file, if the prefix is "veh"
    // By default, the prefix is "veh"
    // If you want to change it to something else, you can leverage this function
    // For example, if vehicles in the XML file have id "carX", you should call "changeSUMO_ID_prefix("car")"
    void changeSUMO_ID_prefix(std::string new_prefix) {m_sumo_vehid_prefix=new_prefix;}

    void setupContainer(bool CABasicService_enabled,bool DENBasicService_enabled);

    // Function to setup a circular GeoArea for DENMs - it must be called at least once when sending/receiving DENMs
    // Then, it may be called as many times as desired to change the DENMs GeoArea
    // It should always be called after setupContainer(), otherwise it will generate an error
    void setDENMCircularGeoArea(double lat, double lon, int radius_meters);

    // Function to be called when this BSContainer is no longer being used (e.g., when a vehicle exits from the SUMO scenario)
    // This function should be called to automatically perform all the necessary cleanup operations (like stopping the CAM
    // dissemation, if enabled, and the GeoNetworking egoPV updates)
    void cleanup();
  private:
    // Message reception callbacks
    std::function<void(asn1cpp::Seq<CAM>, Address, StationID_t, StationType_t, SignalInfo)> m_CAReceiveCallbackExtended;
    std::function<void(denData,Address,unsigned long,long,SignalInfo)> m_DENReceiveCallbackExtended;

    // ETSI Transport and Networking layer pointers
    Ptr<btp> m_btp;
    Ptr<GeoNet> m_gn;

    // ETSI Basic Services
    // Additional Basic Services should be added here
    CABasicService m_cabs;
    DENBasicService m_denbs;

    bool m_real_time;
    Ptr<TraciClient> m_mobility_client;
    Ptr<PRRSupervisor> m_prrsup_ptr;
    bool m_prrsup_beacons;

    Ptr<Socket> m_socket; // Socket for reception and transmission of messages

    // Vehicle properties
    StationID_t m_station_id;
    StationType_t m_stationtype;

    // Prefix for the full SUMO vehicle ID (default: "veh")
    std::string m_sumo_vehid_prefix = "veh";

    VDP *m_vdp_ptr;

    bool m_is_configured = false;
    bool m_DENMs_enabled = false;
    bool m_CAMs_enabled = false;

  };
}

#endif // BSCONTAINER_H
