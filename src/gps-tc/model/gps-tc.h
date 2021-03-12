/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef GPS_TC_H
#define GPS_TC_H

// To use std::sort
#include <algorithm>

#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"

#include "ns3/vehicle-visualizer.h"

namespace ns3 {

  class GPSTraceClient : public Object
  {
      public:
          GPSTraceClient(std::string vehID);
          virtual ~GPSTraceClient();
          void sortVehiclesdata();
          void shiftOrigin(double,double);
          void printVehiclesdata();

          // Setter
          void setTimestamp(std::string);
          void setLat(std::string);
          void setLon(std::string);
          void setX(double);
          void setY(double);
          void setSpeedms(std::string);
          // This heading value should always be set in degrees, from 0 to 360, in a clockwise direction
          void setheading(std::string);
          void setAccelmsq(std::string);
          void setLat0(double);
          void setLon0(double);

          // Getter
          long int getTimestamp();
          double getLat();
          double getLon();
          double getX();
          double getY();
          double getSpeedms();
          double getHeadingdeg();
          double getAccelmsq();
          double getTravelledDistance();
          std::string getID();
          uint64_t getLastIndex();
          double getLat0();
          double getLon0();

          // Start "playing" the trace
          void playTrace(Time const &delay);

          // Set the functions to create/destroy node
          void GPSTraceClientSetup(std::function<Ptr<Node>()> create_fcn,std::function<void(Ptr<Node>)> destroy_fcn);

          // Stop "playing" the trace
          void StopUpdates();
          void setVehicleVisualizer(Ptr<vehicleVisualizer> vehicleVis) {m_vehicle_visualizer = vehicleVis;}

      private:
          typedef struct _positioning_data {
              double lat;
              double lon;
              double tm_x;
              double tm_y;
              double speedms;
              double accelmsq;
              double heading;
              long int utc_time; // UTC time as Unix timestamp since the epoch, in microseconds

              // To order the vector according to "utc_time"
              bool operator < (const _positioning_data& str) const
              {
                  return (utc_time < str.utc_time);
              }

          } positioning_data_t;

          double m_lat0;
          double m_lon0;

          std::vector<positioning_data_t> vehiclesdata;
          long unsigned int m_lastvehicledataidx;
          bool m_updatefirstiter;
          std::string m_vehID;
          double m_travelled_distance;

          // Function pointers to node include/exclude functions
          std::function<Ptr<Node>()> m_includeNode;
          std::function<void(Ptr<Node>)> m_excludeNode;

          EventId m_event_updatepos;

          Ptr<Node> m_vehNode;

          void CreateNode(void);
          void UpdatePositions(void);

          Ptr<vehicleVisualizer> m_vehicle_visualizer;


  };


  }

#endif /* GPS_TC_H */

