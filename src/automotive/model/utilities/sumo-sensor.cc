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


#include "sumo-sensor.h"
#include <cmath>


namespace ns3 {

  const point_type frontLeftPoint(0.0, 0.5);
  const point_type frontRightPoint(0.0, -0.5);
  const point_type backRightPoint(-1.0, -0.5);
  const point_type backLeftPoint(-1.0, 0.5);
  const point_type boxCenterPoint(-0.5, 0.0);

  double compute_sensordist(double lat_a, double lon_a, double lat_b, double lon_b) {
      // 12742000 is the mean Earth radius (6371 km) * 2 * 1000 (to convert from km to m)
      return 12742000.0*asin(sqrt(sin(DEG_2_RAD(lat_b-lat_a)/2)*sin(DEG_2_RAD(lat_b-lat_a)/2)+cos(DEG_2_RAD(lat_a))*cos(DEG_2_RAD(lat_b))*sin(DEG_2_RAD(lon_b-lon_a)/2)*sin(DEG_2_RAD(lon_b-lon_a)/2)));
  }


  SUMOSensor::SUMOSensor()
  {
    m_stationID = 0;
    m_sensorRange = 50.0;
  }
  SUMOSensor::~SUMOSensor()
  {
    Simulator::Cancel(m_event_updateDetectedObjects);
  }

  void
  SUMOSensor::updateDetectedObjects ()
  {
    using namespace boost::geometry::strategy::transform;
    libsumo::TraCIPosition egoPosXY=m_client->TraCIAPI::vehicle.getPosition(m_id);
    libsumo::TraCIPosition egoPos = m_client->TraCIAPI::simulation.convertXYtoLonLat (egoPosXY.x,egoPosXY.y);
    std::vector<std::string> allIDs;
    std::vector<std::pair<std::string,double>> rangeIDs,sensedIDs;
    // Get all IDs in the simulation
    allIDs = m_client->vehicle.getIDList ();

    for(size_t i=0;i<allIDs.size ();i++)
      {
        //For all IDs, except the egoID
        if(allIDs[i].compare(m_id))
          {
            //Compute the vehicle distance from the egoVehicle's front bumper
            double f;
            libsumo::TraCIPosition geoPos=m_client->TraCIAPI::vehicle.getPosition(allIDs[i]);
            geoPos=m_client->TraCIAPI::simulation.convertXYtoLonLat (geoPos.x,geoPos.y);
            f = compute_sensordist (egoPos.y,egoPos.x,geoPos.y,geoPos.x);
            if (f<=m_sensorRange)
              {
                //If the vehicle is closer than the sensor range, add to preliminary in range list
                rangeIDs.push_back (std::pair<std::string,double>(allIDs[i],f));
              }
          }
      }
     // Sort rangeIDs list from closer to furthest vehicle
     sort(rangeIDs.begin (),rangeIDs.end (),[] (const std::pair<std::string, double>& a,
             const std::pair<std::string, double>& b){return a.second < b.second;});
     if(rangeIDs.size()>0)
       sensedIDs.push_back (rangeIDs[0]);

     if(rangeIDs.size ()>=2)
       {
         //If we have more than 2 vehicles in the rangeIDs list, we need to check if the furthest one/s, is/are actually in LoS
         for(size_t i=1;i<rangeIDs.size ();i++) //For every vehicle in range, except the closest
           {
             auto pointsTest = adjust(rangeIDs[i].first); //Get the points of the vehicle under test
             bool sensed=true;
             for(size_t j=0;j<sensedIDs.size ();j++) //For every 'already' sensed vehicle
               {
                 auto pointsSensed = adjust(sensedIDs[j].first);
                 //Create polygon of closest vehicle
                 polygon_type vehicle;
                 vehicle.outer().push_back(pointsSensed.front_left);
                 vehicle.outer().push_back(pointsSensed.back_left);
                 vehicle.outer().push_back(pointsSensed.back_right);
                 vehicle.outer().push_back(pointsSensed.front_right);
                 vehicle.outer().push_back(pointsSensed.front_left);

                 point_type ego_point(egoPosXY.x,egoPosXY.y);
                 std::vector<point_type> vehPoints;
                 vehPoints.push_back (pointsTest.front_left);
                 vehPoints.push_back (pointsTest.front_right);
                 vehPoints.push_back (pointsTest.back_left);
                 vehPoints.push_back (pointsTest.back_right);
                 bool los=false;
                 //Create linestring from sensor towards all 4 points of the furthest vehicle
                 for (int k = 0; k<4; k++)
                   {
                     linestring_type linestring;
                     linestring.push_back (ego_point);
                     linestring.push_back (vehPoints[k]);
                     //If there's at least one point in LoS, passes the check
                     if(!boost::geometry::intersects(vehicle,linestring))
                       {
                         los=true;
                         break;
                       }
                   }
                 //If none of the 4 points are in LoS, the vehicle can't be sensed
                 if(!los)
                   sensed=false;
               }
             //If the vehicle can be sensed, it is added to the sensedIDs list for following LoS tests
             if(sensed)
                 sensedIDs.push_back (rangeIDs[i]);
           }
       }

     for (size_t i=0;i<sensedIDs.size();i++)
       {
         LDM::returnedVehicleData_t retveh = {0};
         LDM::LDM_error_t retval = m_LDM->lookup(std::stol(sensedIDs[i].first.substr (3)),retveh);
         std::normal_distribution<double> dist_distance(m_mean,m_stddev_distance);
         std::normal_distribution<double> dist_angle(m_mean,m_stddev_angle);
         std::normal_distribution<double> dist_speed(m_mean,m_stddev_speed);


         //if (retval==LDM::LDM_ITEM_NOT_FOUND || (retval==LDM::LDM_OK && retveh.vehData.detected))
         if (retval==LDM::LDM_ITEM_NOT_FOUND || retval==LDM::LDM_OK)
           {
             vehicleData_t objectData = {0};
              long id = m_stationID;
              double dist_factor = 1-(sensedIDs[i].second/m_sensorRange);
              if (retval==LDM::LDM_OK && !retveh.vehData.detected)
                 objectData.detected = false;
              else
                objectData.detected = true;
              objectData.ID = sensedIDs[i].first;
              objectData.stationID = std::stol(objectData.ID.substr(3));

              //Get position with noise
              libsumo::TraCIPosition objectPosition = m_client->TraCIAPI::vehicle.getPosition(objectData.ID);
              objectPosition.x += (dist_distance(m_generator)*dist_factor);
              objectPosition.y += (dist_distance(m_generator)*dist_factor);            


              objectData.lon = m_client->TraCIAPI::simulation.convertXYtoLonLat (objectPosition.x
                                                                                 ,objectPosition.y).x;
              objectData.lat = m_client->TraCIAPI::simulation.convertXYtoLonLat (objectPosition.x
                                                                                 ,objectPosition.y).y;
              objectData.elevation = AltitudeValue_unavailable;
              objectData.heading = m_client->vehicle.getAngle (objectData.ID)+(dist_angle(m_generator)*dist_factor);
              objectData.speed_ms = m_client->vehicle.getSpeed (objectData.ID)+(dist_speed(m_generator)*dist_factor);
              objectData.timestamp_us = Simulator::Now ().GetMicroSeconds ();
              objectData.camTimestamp = objectData.timestamp_us;
              objectData.vehicleWidth = OptionalDataItem<long>(long ((m_client->vehicle.getWidth(objectData.ID)+(dist_distance(m_generator)*dist_factor/10))*DECI));
              objectData.vehicleLength = OptionalDataItem<long>(long ((m_client->vehicle.getLength(objectData.ID)+(dist_distance(m_generator)*dist_factor/10))*DECI));
              //Compute relative distance with x axis being defined by the egoVehicle's angle
              libsumo::TraCIPosition egoPosition = m_client->TraCIAPI::vehicle.getPosition(m_id);
              point_type egoReference(egoPosition.x,egoPosition.y);
              point_type relReference(objectPosition.x,objectPosition.y);
              rotate_transformer<boost::geometry::degree, double, 2, 2> rotate(90-m_client->vehicle.getAngle (m_id));

              boost::geometry::transform(egoReference, egoReference, rotate);// Transform both points to the SUMO (x,y) axises
              boost::geometry::transform(relReference, relReference, rotate);
              objectData.xDistance = OptionalDataItem<long>(long (boost::geometry::get<0>(relReference)*CENTI -
                                                                  boost::geometry::get<0>(egoReference)*CENTI));//X distance in centimeters
              objectData.yDistance = OptionalDataItem<long>(long (boost::geometry::get<1>(relReference)*CENTI -
                                                                  boost::geometry::get<1>(egoReference)*CENTI));//Y Distance in centimeters

              objectData.xDistAbs = OptionalDataItem<long>(long (objectPosition.x - egoPosition.x)*CENTI);
              objectData.yDistAbs = OptionalDataItem<long>(long (objectPosition.y - egoPosition.y)*CENTI);
              //Compute relative speed with x axis being defined by the egoVehicle's angle
              point_type egoSpeed(m_client->vehicle.getSpeed (m_id),0);
              point_type relSpeed(objectData.speed_ms,0);
              rotate_transformer<boost::geometry::degree, double, 2, 2> rotate_speed(90-m_client->vehicle.getAngle (m_id));
              boost::geometry::transform(egoSpeed, egoSpeed, rotate_speed);
              boost::geometry::transform(relSpeed, relSpeed, rotate_speed);

              double xspeed = (boost::geometry::get<0>(relSpeed)-boost::geometry::get<0>(egoSpeed))*CENTI;
              double yspeed = (boost::geometry::get<1>(relSpeed)-boost::geometry::get<1>(egoSpeed))*CENTI;

              objectData.xSpeed = OptionalDataItem <long>((long) xspeed);
              objectData.ySpeed = OptionalDataItem <long>((long) yspeed);
              objectData.xSpeedAbs = OptionalDataItem <long>((long) (objectData.speed_ms * cos(DEG_2_RAD(objectData.heading)))*CENTI);
              objectData.ySpeedAbs = OptionalDataItem <long>((long) (objectData.speed_ms * sin(DEG_2_RAD(objectData.heading)))*CENTI);

              objectData.longitudinalAcceleration = OptionalDataItem <long> (long (m_client->vehicle.getAcceleration (objectData.ID)));
              objectData.xAccAbs = OptionalDataItem <long> (long (m_client->vehicle.getAcceleration (objectData.ID) * cos(DEG_2_RAD(objectData.heading))));
              objectData.yAccAbs = OptionalDataItem <long> (long (m_client->vehicle.getAcceleration (objectData.ID) * sin(DEG_2_RAD(objectData.heading))));
              objectData.confidence = long (dist_factor*CENTI); //Distance based confidence
              objectData.perceivedBy = OptionalDataItem<long> ((long) m_stationID);
              long relAngle = (long) ((objectData.heading + dist_angle(m_generator) - m_client->vehicle.getAngle(m_id))*DECI);
              if(relAngle<0)
                objectData.angle = OptionalDataItem <long> (relAngle+3600);//Relative 'negative' Heading angle
              else
                objectData.angle = OptionalDataItem <long> (relAngle);//Relative Heading angle

              objectData.stationType = StationType_unknown;

              if (retveh.vehData.lastCPMincluded.isAvailable ())
                objectData.lastCPMincluded.setData(retveh.vehData.lastCPMincluded.getData());

              if(retveh.vehData.associatedCVs.isAvailable ())
                objectData.associatedCVs = OptionalDataItem<std::vector<long>>(retveh.vehData.associatedCVs.getData ());

              retval = m_LDM->insert(objectData);

              if(retval!=LDM::LDM_OK && retval!=LDM::LDM_UPDATED) {
                  std::cerr << "Warning! Insert on the database for detected object " << objectData.ID << "failed!" << std::endl;
              }
           }
       }

     m_event_updateDetectedObjects = Simulator::Schedule(MilliSeconds (100),&SUMOSensor::updateDetectedObjects,this);
  }

  vehiclePoints_t
  SUMOSensor::adjust(std::string id)
  {
    using namespace boost::geometry::strategy::transform;
    libsumo::TraCIPosition egoPos=m_client->TraCIAPI::vehicle.getPosition(id);
    double width,length;
    vehiclePoints_t points;

    auto angle = m_client->vehicle.getAngle (id);
    width = m_client->vehicle.getWidth (id);
    length = m_client->vehicle.getLength (id);
    angle = -1.0 * (angle-90);


    // Scale with vehicle size
    scale_transformer<double, 2, 2> scale(length,width);
    boost::geometry::transform(boxCenterPoint, points.center, scale);
    boost::geometry::transform(frontLeftPoint, points.front_left,scale);
    boost::geometry::transform(frontRightPoint, points.front_right,scale);
    boost::geometry::transform(backLeftPoint, points.back_left, scale);
    boost::geometry::transform(backRightPoint, points.back_right,scale);


    // Rotate
    rotate_transformer<boost::geometry::degree, double, 2, 2> rotate(angle);
    boost::geometry::transform(points.center, points.center, rotate);
    boost::geometry::transform(points.front_left, points.front_left, rotate);
    boost::geometry::transform(points.front_right, points.front_right, rotate);
    boost::geometry::transform(points.back_left, points.back_left, rotate);
    boost::geometry::transform(points.back_right, points.back_right, rotate);

    //Translate to actual front bumper position
    translate_transformer<double, 2, 2> translate(egoPos.x,egoPos.y);
    boost::geometry::transform(points.center, points.center, translate);
    boost::geometry::transform(points.front_left, points.front_left, translate);
    boost::geometry::transform(points.front_right, points.front_right, translate);
    boost::geometry::transform(points.back_left, points.back_left, translate);
    boost::geometry::transform(points.back_right, points.back_right, translate);

    return points;
  }

  void
  SUMOSensor::cleanup()
  {
    Simulator::Cancel(m_event_updateDetectedObjects);
  }
}
