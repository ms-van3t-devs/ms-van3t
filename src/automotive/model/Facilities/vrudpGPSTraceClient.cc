#include "vrudpGPSTraceClient.h"

extern "C" {
    #include "ns3/utmups.h"
    #include "ns3/VAM.h"
}

namespace ns3
{
  VRUDPGPSTraceClient::VRUDPGPSTraceClient()
  {
    m_gps_trace_client=NULL;
    m_id="(null)";
  }

  VRUDPGPSTraceClient::VRUDPGPSTraceClient(Ptr<GPSTraceClient> gps_tc,std::string id)
  {
    m_gps_trace_client=gps_tc;
    m_id=id;
  }

  VAM_mandatory_data_t
  VRUDPGPSTraceClient::getVAMMandatoryData ()
  {
    VAM_mandatory_data_t VAMdata;

    /* Speed [0.01 m/s] */
    VAMdata.speed = VRUdpValueConfidence<>(m_gps_trace_client->getSpeedms ()*CENTI,
                                         SpeedConfidence_unavailable);
//    CAMdata.speed.speedValue=m_gps_trace_client->getSpeedms ()*CENTI;
//    CAMdata.speed.speedConfidence=SpeedConfidence_unavailable;

    // longitude WGS84 [0,1 microdegree]
    VAMdata.longitude=(Longitude_t)(m_gps_trace_client->getLon ()*DOT_ONE_MICRO);
    // latitude WGS84 [0,1 microdegree]
    VAMdata.latitude=(Latitude_t)(m_gps_trace_client->getLat ()*DOT_ONE_MICRO);

    /* Altitude [0,01 m] */
    VAMdata.altitude = VRUdpValueConfidence<>(AltitudeValue_unavailable,
                                            AltitudeConfidence_unavailable);
//    CAMdata.altitude.altitudeValue=AltitudeValue_unavailable;
//    CAMdata.altitude.altitudeConfidence=AltitudeConfidence_unavailable;

    /* Position Confidence Ellipse */
    VAMdata.posConfidenceEllipse.semiMajorConfidence=SemiAxisLength_unavailable;
    VAMdata.posConfidenceEllipse.semiMinorConfidence=SemiAxisLength_unavailable;
    VAMdata.posConfidenceEllipse.semiMajorOrientation=HeadingValue_unavailable;

    /* Longitudinal acceleration [0.1 m/s^2] */
    VAMdata.longAcceleration = VRUdpValueConfidence<>(m_gps_trace_client->getAccelmsq ()* DECI,
                                                  AccelerationConfidence_unavailable);
//    CAMdata.longAcceleration.longitudinalAccelerationValue=m_gps_trace_client->getAccelmsq ()* DECI;
//    CAMdata.longAcceleration.longitudinalAccelerationConfidence=AccelerationConfidence_unavailable;

    /* Heading WGS84 north [0.1 degree] */
    VAMdata.heading = VRUdpValueConfidence<>(m_gps_trace_client->getHeadingdeg ()* DECI,
                                                      HeadingConfidence_unavailable);
//    CAMdata.heading.headingConfidence = HeadingConfidence_unavailable;

    return VAMdata;
  }

  VRUdp_position_latlon_t
  VRUDPGPSTraceClient::getPedPosition()
  {
    VRUdp_position_latlon_t poslatlon;
    poslatlon.lat = m_gps_trace_client->getLat ();
    poslatlon.lon = m_gps_trace_client->getLon ();
    return poslatlon;
  }

  VRUdp::VRUDP_position_cartesian_t
  VRUDPGPSTraceClient::getXY(double lon, double lat)
  {
    VRUDP_position_cartesian_t posxy;

    double tm_x,tm_y,tm_gamma,tm_kappa;

    // GeographicLib (C adaptation) pre-initialized Transverse Mercator structure
    transverse_mercator_t tmerc=UTMUPS_init_UTM_TransverseMercator ();
    TransverseMercator_Forward (&tmerc,m_gps_trace_client->getLon0(),lat,lon,&tm_x,&tm_y,&tm_gamma,&tm_kappa);
    posxy.x=tm_x;
    posxy.y=tm_y;
    return posxy;
  }


}
