/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

* Created by:
    *  Carlos Mateo Risma Carletti, Politecnico di Torino (carlosrisma@gmail.com)
        */


#include "vdpopencda.h"

namespace ns3 {
  VDPOpenCDA::VDPOpenCDA()
  {
    m_opencda_client=NULL;
    m_id=-1;
    m_string_id="";
  }
  VDPOpenCDA::VDPOpenCDA(Ptr<OpenCDAClient> opencda_client, std::string id)
  {
    m_opencda_client = opencda_client;
    m_string_id = id;
    m_id = std::stoi (id);
    carla::Vehicle egoVehicle = m_opencda_client->GetManagedActorById (m_id);
    m_width = egoVehicle.width ()*DECI;
    if (m_width == 0)
      m_width = VehicleWidth_unavailable;
    m_length = egoVehicle.length ()*DECI;
    if (m_length == 0)
      m_length = VehicleLengthValue_unavailable;
  }

  VDPOpenCDA::CAM_mandatory_data_t
  VDPOpenCDA::getCAMMandatoryData ()
  {
    CAM_mandatory_data_t CAMdata;
    carla::Vehicle egoVehicle = m_opencda_client->GetManagedActorById (m_id);

    /* Speed [0.01 m/s] */
    CAMdata.speed = VDPValueConfidence<>(sqrt(pow(egoVehicle.speed ().x (),2) + pow(egoVehicle.speed ().y (),2))*CENTI,
                                       SpeedConfidence_unavailable);

    /* Position */
    // longitude WGS84 [0,1 microdegree]
    CAMdata.longitude=(Longitude_t)(egoVehicle.longitude ()*DOT_ONE_MICRO);
    // latitude WGS84 [0,1 microdegree]
    CAMdata.latitude=(Latitude_t)(egoVehicle.latitude ()*DOT_ONE_MICRO);

    /* Altitude [0,01 m] */
    CAMdata.altitude = VDPValueConfidence<>(AltitudeValue_unavailable,
                                          AltitudeConfidence_unavailable);

    /* Position Confidence Ellipse */
    CAMdata.posConfidenceEllipse.semiMajorConfidence=SemiAxisLength_unavailable;
    CAMdata.posConfidenceEllipse.semiMinorConfidence=SemiAxisLength_unavailable;
    CAMdata.posConfidenceEllipse.semiMajorOrientation=HeadingValue_unavailable;

    /* Longitudinal acceleration [0.1 m/s^2] */
    double acc = sqrt(pow(egoVehicle.acceleration ().x (),2)+pow(egoVehicle.acceleration ().y (),2));
    if ((acc*DECI) > AccelerationValue_unavailable)
      CAMdata.longAcceleration = VDPValueConfidence<>(AccelerationValue_unavailable,
                                                    AccelerationConfidence_unavailable);
    else
      CAMdata.longAcceleration = VDPValueConfidence<>(acc * DECI,
                                                  AccelerationConfidence_unavailable);


    /* Heading WGS84 north [0.1 degree] */
    double heading = egoVehicle.heading ();
    if (heading < 0)
      heading += 360;
    CAMdata.heading = VDPValueConfidence<>(heading * DECI,
                                         HeadingConfidence_unavailable);

    /* Drive direction TODO */
    CAMdata.driveDirection = DriveDirection_unavailable;

    /* Curvature and CurvatureCalculationMode */
    CAMdata.curvature = VDPValueConfidence<>(CurvatureValue_unavailable,
                                             CurvatureConfidence_unavailable);
    CAMdata.curvature_calculation_mode = CurvatureCalculationMode_unavailable;

    /* Length and Width [0.1 m] */
    if(egoVehicle.length () != 0)
      CAMdata.VehicleLength = VDPValueConfidence<long,long>(egoVehicle.length ()*DECI,
                                                          VehicleLengthConfidenceIndication_unavailable);
    else
      CAMdata.VehicleLength = VDPValueConfidence<long,long>(m_length,
                                                          VehicleLengthConfidenceIndication_unavailable);

    if(egoVehicle.width ()!=0)
      CAMdata.VehicleWidth = (int) (egoVehicle.width ()*DECI);
    else
      CAMdata.VehicleWidth = (int) (m_width);

    /* Yaw Rate */
    CAMdata.yawRate = VDPValueConfidence<>(YawRateValue_unavailable,
                                           YawRateConfidence_unavailable);

    return CAMdata;
  }

  VDPOpenCDA::CPM_mandatory_data_t
  VDPOpenCDA::getCPMMandatoryData ()
  {
    CPM_mandatory_data_t CPMdata;
    carla::Vehicle egoVehicle = m_opencda_client->GetManagedActorById (m_id);

    /* Speed [0.01 m/s] */
    CPMdata.speed = VDPValueConfidence<>(m_opencda_client->getSpeed (m_id)*CENTI,
                                       SpeedConfidence_unavailable);

    /* Position */
    // longitude WGS84 [0,1 microdegree]
    CPMdata.longitude=(Longitude_t)(egoVehicle.longitude ()*DOT_ONE_MICRO);
    // latitude WGS84 [0,1 microdegree]
    CPMdata.latitude=(Latitude_t)(egoVehicle.latitude ()*DOT_ONE_MICRO);

    /* Altitude [0,01 m] */
    CPMdata.altitude = VDPValueConfidence<>(AltitudeValue_unavailable,
                                          AltitudeConfidence_unavailable);

    /* Position Confidence Ellipse */
    CPMdata.posConfidenceEllipse.semiMajorConfidence=SemiAxisLength_unavailable;
    CPMdata.posConfidenceEllipse.semiMinorConfidence=SemiAxisLength_unavailable;
    CPMdata.posConfidenceEllipse.semiMajorOrientation=HeadingValue_unavailable;

    /* Longitudinal acceleration [0.1 m/s^2] */
    double acc = sqrt(pow(egoVehicle.acceleration ().x (),2)+pow(egoVehicle.acceleration ().y (),2));
    // CARLA sometimes gives very high acceleration values https://github.com/carla-simulator/carla/issues/3168
    if ((acc*DECI) > AccelerationValue_unavailable)
      CPMdata.longAcceleration = VDPValueConfidence<>(AccelerationValue_unavailable,
                                                    AccelerationConfidence_unavailable);
    else
      CPMdata.longAcceleration = VDPValueConfidence<>(acc * DECI,
                                                  AccelerationConfidence_unavailable);

    /* Heading WGS84 north [0.1 degree] */
    double heading = egoVehicle.heading ();
    if (heading < 0)
      heading += 360;
    CPMdata.heading = VDPValueConfidence<>(heading * DECI,
                                         HeadingConfidence_unavailable);

    /* Drive direction (backward driving is not fully supported by SUMO, at the moment */
    CPMdata.driveDirection = DriveDirection_unavailable;

    /* Curvature and CurvatureCalculationMode */
    CPMdata.curvature = VDPValueConfidence<>(CurvatureValue_unavailable,
                                             CurvatureConfidence_unavailable);
    CPMdata.curvature_calculation_mode = CurvatureCalculationMode_unavailable;

    /* Length and Width [0.1 m] */
    if(egoVehicle.length () != 0)
      CPMdata.VehicleLength = VDPValueConfidence<long,long>(egoVehicle.length ()*DECI,
                                                          VehicleLengthConfidenceIndication_unavailable);
    else
      CPMdata.VehicleLength = VDPValueConfidence<long,long>(m_length,
                                                          VehicleLengthConfidenceIndication_unavailable);
    if(egoVehicle.width ()!=0)
      CPMdata.VehicleWidth = (int) (egoVehicle.width ()*DECI);
    else
      CPMdata.VehicleWidth = (int) (m_width);

    /* Yaw Rate */
    CPMdata.yawRate = VDPValueConfidence<>(YawRateValue_unavailable,
                                           YawRateConfidence_unavailable); //TODO: implement yaw since we can actually get it from CARLA
    return CPMdata;

  }

  VDP::VDP_position_latlon_t
  VDPOpenCDA::getPosition ()
  {
    carla::Vehicle vehicle = m_opencda_client->GetManagedActorById (m_id);
    VDP::VDP_position_latlon_t pos;
    pos.lat = vehicle.latitude ();
    pos.lon = vehicle.longitude ();
    pos.alt = vehicle.location ().z ();

    return pos;
  }

  VDP::VDP_position_cartesian_t
  VDPOpenCDA::getPositionXY ()
  {
    carla::Vehicle vehicle = m_opencda_client->GetManagedActorById (m_id);
    VDP_position_cartesian_t pos;
    pos.x = vehicle.location ().x ();
    pos.y = vehicle.location ().y ();
    pos.z = vehicle.location ().z ();

    return pos;
  }

  VDP::VDP_position_cartesian_t
  VDPOpenCDA::getXY (double lon, double lat)
  {
    carla::Vector pos = m_opencda_client->getCartesian (lon,lat);
    VDP::VDP_position_cartesian_t retpos;
    retpos.x = pos.x ();
    retpos.y = pos.y ();
    retpos.z = pos.z ();

    return retpos;
  }

  double
  VDPOpenCDA::getCartesianDist (double lon1, double lat1, double lon2, double lat2)
  {
    carla::Vector pos1,pos2;
    pos1 = m_opencda_client->getCartesian (lon1,lat1);
    pos2 = m_opencda_client->getCartesian (lon2,lat2);
    return sqrt((pow((pos1.x()-pos2.x()),2)+pow((pos1.y()-pos2.y()),2)));
  }

  VDPDataItem<int>
  VDPOpenCDA::getLanePosition()
  {
    carla::Vehicle vehicle = m_opencda_client->GetManagedActorById (m_id);
    VDPDataItem<int> lanePos(vehicle.lane ());
    return lanePos;
  }
}

