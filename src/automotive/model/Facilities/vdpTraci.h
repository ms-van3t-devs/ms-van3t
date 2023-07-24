#ifndef VDPTRACI_H
#define VDPTRACI_H

#include "vdp.h"
#include "ns3/traci-client.h"

namespace ns3 {
  class VDPTraCI : public VDP
  {
  public:

    // TraCI vehicle signals according to https://sumo.dlr.de/docs/TraCI/Vehicle_Signalling.html
    typedef enum {
    VEH_SIGNAL_BLINKER_RIGHT	=0b00000000000001,
    VEH_SIGNAL_BLINKER_LEFT	=0b00000000000010,
    VEH_SIGNAL_BLINKER_EMERGENCY=0b00000000000100,
    VEH_SIGNAL_BRAKELIGHT	=0b00000000001000,
    VEH_SIGNAL_FRONTLIGHT	=0b00000000010000,
    VEH_SIGNAL_FOGLIGHT         =0b00000000100000,
    VEH_SIGNAL_HIGHBEAM         =0b00000001000000,
    VEH_SIGNAL_BACKDRIVE	=0b00000010000000,
    VEH_SIGNAL_WIPER            =0b00000100000000,
    VEH_SIGNAL_DOOR_OPEN_LEFT	=0b00001000000000,
    VEH_SIGNAL_DOOR_OPEN_RIGHT	=0b00010000000000,
    VEH_SIGNAL_EMERGENCY_BLUE	=0b00100000000000,
    VEH_SIGNAL_EMERGENCY_RED	=0b01000000000000,
    VEH_SIGNAL_EMERGENCY_YELLOW =0b10000000000000,
    }TraciVehicleSignals_e;

    VDPTraCI(Ptr<TraciClient> traci_client, std::string node_id);
    VDPTraCI();

    void setProperties(Ptr<TraciClient> traci_client,std::string node_id) {m_traci_client=traci_client; m_id=node_id;}

    CAM_mandatory_data_t getCAMMandatoryData();
    CPM_mandatory_data_t getCPMMandatoryData();

    double getSpeedValue() {return m_traci_client->TraCIAPI::vehicle.getSpeed (m_id);}
    double getTravelledDistance() {return m_traci_client->TraCIAPI::vehicle.getDistance (m_id);}
    double getHeadingValue() {return m_traci_client->TraCIAPI::vehicle.getAngle (m_id);}

    // Added for GeoNet functionalities
    VDP_position_latlon_t getPosition();
    VDP_position_cartesian_t getPositionXY();
    VDP_position_cartesian_t getXY(double lon, double lat);

    VDPDataItem<uint8_t> getAccelerationControl() {return VDPDataItem<uint8_t>(false);}
    VDPDataItem<int> getLanePosition();
    VDPDataItem<VDPValueConfidence<int,int>> getSteeringWheelAngle() {return VDPDataItem<VDPValueConfidence<int,int>>(false);}
    VDPDataItem<VDPValueConfidence<int,int>> getLateralAcceleration() {return VDPDataItem<VDPValueConfidence<int,int>>(false);}
    VDPDataItem<VDPValueConfidence<int,int>> getVerticalAcceleration() {return VDPDataItem<VDPValueConfidence<int,int>>(false);}
    VDPDataItem<int> getPerformanceClass() {return VDPDataItem<int>(false);}
    VDPDataItem<VDP_CEN_DSRC_tolling_zone_t> getCenDsrcTollingZone() {return VDPDataItem<VDP_CEN_DSRC_tolling_zone_t>(false);}

    VDPDataItem<unsigned int> getVehicleRole() {return m_vehicleRole;}
    VDPDataItem<uint8_t> getExteriorLights();

    VDPDataItem<VDP_PublicTransportContainerData_t> getPublicTransportContainerData() {return m_publicTransportContainerData;}
    VDPDataItem<VDP_SpecialTransportContainerData_t> getSpecialTransportContainerData() {return m_specialTransportContainerData;}
    VDPDataItem<int> getDangerousGoodsBasicType() {return m_dangerousGoodsBasicType;}
    VDPDataItem<VDP_RoadWorksContainerBasicData_t> getRoadWorksContainerBasicData_t() {return m_roadWorksContainerBasicData;}
    VDPDataItem<uint8_t> getRescueContainerLightBarSirenInUse() {return m_rescueContainerLightBarSirenInUse;}
    VDPDataItem<VDP_EmergencyContainerData_t> getEmergencyContainerData() {return m_emergencyContainerData;}
    VDPDataItem<VDP_SafetyCarContainerData_t> getSafetyCarContainerData() {return m_safetyCarContainerData;}

    void setVehicleRole(unsigned int data){m_vehicleRole = VDPDataItem<unsigned int>(data);}

    // Special vehicle container
    void setPublicTransportContainerData(VDP_PublicTransportContainerData_t data) {m_publicTransportContainerData = VDPDataItem<VDP_PublicTransportContainerData_t>(data);}
    void setSpecialTransportContainerData(VDP_SpecialTransportContainerData_t data) {m_specialTransportContainerData = VDPDataItem<VDP_SpecialTransportContainerData_t>(data);}
    void setDangerousGoodsBasicType (int data) {m_dangerousGoodsBasicType = VDPDataItem<int>(data);} // For the DangerousGoodsContainer
    void setRoadWorksContainerBasicData(VDP_RoadWorksContainerBasicData_t data) {m_roadWorksContainerBasicData = VDPDataItem<VDP_RoadWorksContainerBasicData_t>(data);}
    void setRescueContainerLightBarSirenInUse(uint8_t data) {m_rescueContainerLightBarSirenInUse = VDPDataItem<uint8_t>(data);}
    void setEmergencyContainerData(VDP_EmergencyContainerData_t data) {m_emergencyContainerData = VDPDataItem<VDP_EmergencyContainerData_t>(data);}
    void setSafetyCarContainerData(VDP_SafetyCarContainerData_t data) {m_safetyCarContainerData = VDPDataItem<VDP_SafetyCarContainerData_t>(data);}

    private:
      std::string m_id;
      Ptr<TraciClient> m_traci_client;
  };
}

#endif // VDPTRACI_H
