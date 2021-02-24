#ifndef VDPGPSTRACECLIENT_H
#define VDPGPSTRACECLIENT_H

#include "vdp.h"
#include "ns3/gps-tc.h"

namespace ns3 {
  class VDPGPSTraceClient : public VDP
  {
  public:
    VDPGPSTraceClient(Ptr<GPSTraceClient>,std::string);
    VDPGPSTraceClient();

    CAM_mandatory_data_t getCAMMandatoryData();

    double getSpeedValue() {return m_gps_trace_client->getSpeedms ();}
    double getTravelledDistance() {return m_gps_trace_client->getTravelledDistance ();}
    double getHeadingValue() {return m_gps_trace_client->getHeadingdeg ();}

    // Added for GeoNet functionalities
    VDP_position_latlon_t getPosition();
    VDP_position_cartesian_t getPositionXY();
    VDP_position_cartesian_t getXY(double lon, double lat);

    AccelerationControl_t *getAccelerationControl() {return NULL;}
    LanePosition_t *getLanePosition() {return NULL;}
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
      Ptr<GPSTraceClient> m_gps_trace_client;
  };
}

#endif // VDPGPSTRACECLIENT_H
