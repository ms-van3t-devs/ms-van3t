#ifndef VDPTRACI_H
#define VDPTRACI_H

#include "vdp.h"
#include "ns3/traci-client.h"

namespace ns3 {
  class VDPTraCI : public VDP
  {
  public:
    VDPTraCI(Ptr<TraciClient> traci_client, std::string node_id);
    VDPTraCI();

    void setProperties(Ptr<TraciClient> traci_client,std::string node_id) {m_traci_client=traci_client; m_id=node_id;}

    CAM_mandatory_data_t getCAMMandatoryData();

    double getSpeedValue() {return m_traci_client->TraCIAPI::vehicle.getSpeed (m_id);}
    double getTravelledDistance() {return m_traci_client->TraCIAPI::vehicle.getDistance (m_id);}
    double getHeadingValue() {return m_traci_client->TraCIAPI::vehicle.getAngle (m_id);}

    AccelerationControl_t *getAccelerationControl() {return NULL;}
    LanePosition_t *getLanePosition();
    SteeringWheelAngle_t *getSteeringWheelAngle() {return NULL;}
    LateralAcceleration_t *getLateralAcceleration() {return NULL;}
    VerticalAcceleration_t *getVerticalAcceleration() {return NULL;}
    PerformanceClass_t *getPerformanceClass() {return NULL;}
    CenDsrcTollingZone_t *getCenDsrcTollingZone() {return NULL;}

    void vdpFree(void* optional_field)
    {
      if(optional_field!=NULL)
        {
          free(optional_field);
        }
    }

    RSUContainerHighFrequency_t *getRsuContainerHighFrequency() {return NULL;}
    LowFrequencyContainer_t *getLowFrequencyContainer() {return NULL;}
    SpecialVehicleContainer_t *getSpecialVehicleContainer() {return NULL;}

    private:
      std::string m_id;
      Ptr<TraciClient> m_traci_client;
  };
}

#endif // VDPTRACI_H
