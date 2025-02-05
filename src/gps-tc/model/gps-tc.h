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
#include "ns3/traci-module.h"
#include "ns3/sionna_handler.h"

#include "ns3/vehicle-visualizer.h"


#define STARTUP_GPS_FCN std::function<Ptr<Node>(std::string)>
#define SHUTDOWN_GPS_FCN std::function<void(Ptr<Node>,std::string)>


namespace ns3 {

  class GPSTraceClient : public Object
  {
      public:
          GPSTraceClient(std::string vehID, std::string vehType);
          virtual ~GPSTraceClient();
          void sortVehiclesdata();
          void shiftOrigin(double,double);
          void printVehiclesdata();

          // This function can be used to artificially add GNSS Trace points, after setting up the GPS Trace Client module
          // with data coming from a real GPS/GNSS trace (i.e. this function should be called only AFTER "vehiclesdata"
          // had been filled up thanks to the setter methods - this function is actually automatically called, if needed,
          // inside the gps-tc-helper module, thus the user, normally, does not need to call it directly)
          // Adding additional points can be useful to make a GPS Trace more "smooth", when it is obtained through a device
          // which, for instance, can provide updates only at a rate of 1 Hz or lower
          // This function will add additional points in between two real GPS/GNSS Trace points, in order to have new data
          // every around interval_ms milliseconds; the additional points are inserted by linearly interpolating all the
          // involved variables (timestamps, latitude, longitude, heading, speed, ...), except the acceleration which is
          // supposed constant between the two real points
          // For instance, if a trace from a device updating every around 1 s is used, and interval_ms is 100 ms, this
          // function will add around 9/10 points between each couple of real points, in order to have, after the computation,
          // 1 point every around 100 ms
          // IMPORTANT: keep in mind that this is a very simple and purely linear interpolation between actual GNSS data, which
          // will very probably not correspond to the physical reality between each couple of real points (in which, very probably,
          // the acceleration is not constant, nor the speed or the position variation; in the physical world, it would also be
          // impossible to have a position and a speed which both increase linearly between couple of points)
          // Thus, never use setInterpolationPoints() when the physics involving speed and acceleration are involved, but rely
          // instead on the actual points given by the GNSS device only (or use another device providing more frequent updates)
          // It can be useful, instead, for applications in which only the actual position and heading of vehicles matters
          void setInterpolationPoints(int interval_ms);

          void generateAccelerationValues();

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
          std::string getVehId() {return m_vehID;};
          std::string getType() {return m_vehType;};
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
          void GPSTraceClientSetup(STARTUP_GPS_FCN create_fcn,SHUTDOWN_GPS_FCN destroy_fcn);

          // Stop "playing" the trace
          void StopUpdates();
          void setVehicleVisualizer(Ptr<vehicleVisualizer> vehicleVis) {m_vehicle_visualizer = vehicleVis;}

          void SetInputMicroseconds(bool use_microseconds) {m_input_microseconds = use_microseconds;};

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
          bool m_accelerationset;
          std::string m_vehID;
          std::string m_vehType;
          double m_travelled_distance;

          // Function pointers to node include/exclude functions
          STARTUP_GPS_FCN m_includeNode;
          SHUTDOWN_GPS_FCN m_excludeNode;

          EventId m_event_updatepos;

          Ptr<Node> m_vehNode;

          void CreateNode(void);
          void UpdatePositions(void);

          Ptr<vehicleVisualizer> m_vehicle_visualizer;

          bool m_input_microseconds = false;
  };


  }

#endif /* GPS_TC_H */

