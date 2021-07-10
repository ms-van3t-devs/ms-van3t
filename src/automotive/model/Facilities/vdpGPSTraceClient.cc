#include "vdpGPSTraceClient.h"

extern "C" {
    #include "ns3/utmups.h"
    #include "ns3/CAM.h"
}

namespace ns3
{
  VDPGPSTraceClient::VDPGPSTraceClient()
  {
    m_gps_trace_client=NULL;
    m_id="(null)";
  }

  VDPGPSTraceClient::VDPGPSTraceClient(Ptr<GPSTraceClient> gps_tc,std::string id)
  {
    m_gps_trace_client=gps_tc;
    m_id=id;

    /* Length and width of car [0.1 m] */
    m_vehicle_length = VDPValueConfidence<long,long>(VehicleLengthValue_unavailable,
                                          VehicleLengthConfidenceIndication_unavailable);
//    m_vehicle_length.vehicleLengthValue = VehicleLengthValue_unavailable;
//    m_vehicle_length.vehicleLengthConfidenceIndication = VehicleLengthConfidenceIndication_unavailable;

    m_vehicle_width = VehicleWidth_unavailable;
  }

  VDPGPSTraceClient::CAM_mandatory_data_t
  VDPGPSTraceClient::getCAMMandatoryData ()
  {
    CAM_mandatory_data_t CAMdata;

    /* Speed [0.01 m/s] */
    CAMdata.speed = VDPValueConfidence<>(m_gps_trace_client->getSpeedms ()*CENTI,
                                         SpeedConfidence_unavailable);
//    CAMdata.speed.speedValue=m_gps_trace_client->getSpeedms ()*CENTI;
//    CAMdata.speed.speedConfidence=SpeedConfidence_unavailable;

    // longitude WGS84 [0,1 microdegree]
    CAMdata.longitude=(Longitude_t)(m_gps_trace_client->getLon ()*DOT_ONE_MICRO);
    // latitude WGS84 [0,1 microdegree]
    CAMdata.latitude=(Latitude_t)(m_gps_trace_client->getLat ()*DOT_ONE_MICRO);

    /* Altitude [0,01 m] */
    CAMdata.altitude = VDPValueConfidence<>(AltitudeValue_unavailable,
                                            AltitudeConfidence_unavailable);
//    CAMdata.altitude.altitudeValue=AltitudeValue_unavailable;
//    CAMdata.altitude.altitudeConfidence=AltitudeConfidence_unavailable;

    /* Position Confidence Ellipse */
    CAMdata.posConfidenceEllipse.semiMajorConfidence=SemiAxisLength_unavailable;
    CAMdata.posConfidenceEllipse.semiMinorConfidence=SemiAxisLength_unavailable;
    CAMdata.posConfidenceEllipse.semiMajorOrientation=HeadingValue_unavailable;

    /* Longitudinal acceleration [0.1 m/s^2] */
    CAMdata.longAcceleration = VDPValueConfidence<>(m_gps_trace_client->getAccelmsq ()* DECI,
                                                  AccelerationConfidence_unavailable);
//    CAMdata.longAcceleration.longitudinalAccelerationValue=m_gps_trace_client->getAccelmsq ()* DECI;
//    CAMdata.longAcceleration.longitudinalAccelerationConfidence=AccelerationConfidence_unavailable;

    /* Heading WGS84 north [0.1 degree] */
    CAMdata.heading = VDPValueConfidence<>(m_gps_trace_client->getHeadingdeg ()* DECI,
                                                      HeadingConfidence_unavailable);
//    CAMdata.heading.headingConfidence = HeadingConfidence_unavailable;

    /* Drive direction */
    CAMdata.driveDirection = DriveDirection_unavailable;

    /* Curvature and CurvatureCalculationMode */
    CAMdata.curvature = VDPValueConfidence<>(CurvatureValue_unavailable,
                                           CurvatureConfidence_unavailable);
//    CAMdata.curvature.curvatureValue = CurvatureValue_unavailable;
//    CAMdata.curvature.curvatureConfidence = CurvatureConfidence_unavailable;
    CAMdata.curvature_calculation_mode = CurvatureCalculationMode_unavailable;

    /* Length and Width [0.1 m] */
    CAMdata.VehicleLength = m_vehicle_length;
    CAMdata.VehicleWidth = m_vehicle_width;

    /* Yaw Rate */
    CAMdata.yawRate = VDPValueConfidence<>(YawRateValue_unavailable,
                                           YawRateConfidence_unavailable);
//    CAMdata.yawRate.yawRateValue = YawRateValue_unavailable;
//    CAMdata.yawRate.yawRateConfidence = YawRateConfidence_unavailable;

    return CAMdata;
  }

  VDP::VDP_position_latlon_t
  VDPGPSTraceClient::getPosition()
  {
    VDP_position_latlon_t poslatlon;
    poslatlon.lat = m_gps_trace_client->getLat ();
    poslatlon.lon = m_gps_trace_client->getLon ();
    return poslatlon;
  }

  VDP::VDP_position_cartesian_t
  VDPGPSTraceClient::getPositionXY()
  {
    VDP_position_cartesian_t posxy;
    posxy.x = m_gps_trace_client->getX();
    posxy.y = m_gps_trace_client->getY();
    return posxy;
  }

  VDP::VDP_position_cartesian_t
  VDPGPSTraceClient::getXY(double lon, double lat)
  {
    VDP_position_cartesian_t posxy;

    double tm_x,tm_y,tm_gamma,tm_kappa;

    // GeographicLib (C adaptation) pre-initialized Transverse Mercator structure
    transverse_mercator_t tmerc=UTMUPS_init_UTM_TransverseMercator ();
    TransverseMercator_Forward (&tmerc,m_gps_trace_client->getLon0(),lat,lon,&tm_x,&tm_y,&tm_gamma,&tm_kappa);
    posxy.x=tm_x;
    posxy.y=tm_y;
    return posxy;
  }


}
