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
 *  Giuseppe Avino, Politecnico di Torino (giuseppe.avino@polito.it)
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "gps-tc.h"
#include "ns3/geographic-positions.h"
#include <cmath>

extern "C" {
  #include "ns3/utmups.h"
  #include "ns3/utmups_math.h"
}

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("GPSTraceClient");

  GPSTraceClient::GPSTraceClient(std::string vehID)
  {
      //ctor
      m_lastvehicledataidx=0;
      m_includeNode=nullptr;
      m_excludeNode=nullptr;
      m_vehNode=nullptr;
      m_vehID=vehID;
      m_updatefirstiter=true;
      m_travelled_distance=0;
      m_vehicle_visualizer=nullptr;
      m_accelerationset=false;
  }

  GPSTraceClient::~GPSTraceClient()
  {
      m_lastvehicledataidx=0;
      m_includeNode=nullptr;
      m_excludeNode=nullptr;
      m_updatefirstiter=true;
      m_accelerationset=false;
  }

  std::string
  GPSTraceClient::getID()
  {
    return m_vehID;
  }

  void
  GPSTraceClient::setTimestamp(std::string utc_time)
  {
      int lastcell = vehiclesdata.size();
      double utc_time_d;

      // Init the struct for this cell
      // It should be initialized to all zeros,
      // as the helper expects this
      positioning_data_t data_t = {};
      // Add the struct to the vector
      vehiclesdata.push_back(data_t);
      // Convert from sec to us
      utc_time_d = std::stod(utc_time) * 1000000;

      // Set the timestamp
      vehiclesdata[lastcell].utc_time = utc_time_d;
  }

  void
  GPSTraceClient::setLat(std::string lat)
  {
      vehiclesdata[vehiclesdata.size()-1].lat = std::stod(lat);
  }

  void
  GPSTraceClient::setLon(std::string lon)
  {
      vehiclesdata[vehiclesdata.size()-1].lon = std::stod(lon);
  }

  void
  GPSTraceClient::setLat0(double lat0)
  {
      m_lat0 = lat0;
  }

  void
  GPSTraceClient::setLon0(double lon0)
  {
      // The difference between lon and lon0, is that lon0 is the reference longitude used to compute the Transverse Mercator Forward (from lon_lat to x_y)
      m_lon0 = lon0;
  }

  void
  GPSTraceClient::setX(double tm_x)
  {
      vehiclesdata[vehiclesdata.size()-1].tm_x = tm_x;
  }

  void
  GPSTraceClient::setY(double tm_y)
  {
      vehiclesdata[vehiclesdata.size()-1].tm_y = tm_y;
  }

  void
  GPSTraceClient::setSpeedms(std::string speedms)
  {
      vehiclesdata[vehiclesdata.size()-1].speedms = std::stod(speedms);
  }

  void
  GPSTraceClient::setheading(std::string heading)
  {
      vehiclesdata[vehiclesdata.size()-1].heading = std::stod(heading);
  }

  void
  GPSTraceClient::setAccelmsq(std::string accelmsq)
  {
      vehiclesdata[vehiclesdata.size()-1].accelmsq = std::stod(accelmsq);

      if(m_accelerationset == false)
      {
          m_accelerationset = true;
      }
  }

  long int
  GPSTraceClient::getTimestamp()
  {
      return vehiclesdata[m_lastvehicledataidx].utc_time;
  }

  double
  GPSTraceClient::getLat()
  {
      return vehiclesdata[m_lastvehicledataidx].lat;
  }

  double
  GPSTraceClient::getLon()
  {
      return vehiclesdata[m_lastvehicledataidx].lon;
  }

  double
  GPSTraceClient::getLat0()
  {
      return m_lat0;
  }

  double
  GPSTraceClient::getLon0()
  {
      return m_lon0;
  }

  double
  GPSTraceClient::getX()
  {
      return vehiclesdata[m_lastvehicledataidx].tm_x;
  }

  double
  GPSTraceClient::getY()
  {
      return vehiclesdata[m_lastvehicledataidx].tm_y;
  }

  double
  GPSTraceClient::getSpeedms()
  {
      return vehiclesdata[m_lastvehicledataidx].speedms;
  }

  double
  GPSTraceClient::getHeadingdeg()
  {
      return vehiclesdata[m_lastvehicledataidx].heading;
  }

  double
  GPSTraceClient::getAccelmsq()
  {
      return vehiclesdata[m_lastvehicledataidx].accelmsq;
  }

  uint64_t
  GPSTraceClient::getLastIndex()
  {
      return m_lastvehicledataidx;
  }

  double
  GPSTraceClient::getTravelledDistance() {
      return m_travelled_distance;
  }

  void
  GPSTraceClient::sortVehiclesdata()
  {
      std::sort(vehiclesdata.begin(), vehiclesdata.end());
  }

  void
  GPSTraceClient::shiftOrigin(double tm_x_origin,double tm_y_origin)
  {
      for(std::vector<int>::size_type i = 0; i != vehiclesdata.size(); i++) {
          vehiclesdata[i].tm_x-=tm_x_origin;
          vehiclesdata[i].tm_y-=tm_y_origin;
      }
  }

  void
  GPSTraceClient::printVehiclesdata()
  {
      std::cout << "Number of positions: " << vehiclesdata.size() << std::endl;
      for (unsigned int i=0; i<vehiclesdata.size(); i++) {
          std::cout << "time: " << std::setprecision(12) << vehiclesdata[i].utc_time << "; lat: " << std::setprecision(12) << vehiclesdata[i].lat
                    << "; lon: " << std::setprecision(12) << vehiclesdata[i].lon << "; speed[m/s]: " << std::setprecision(12) << vehiclesdata[i].speedms
                    << "; heading[rad]: " << std::setprecision(16) << vehiclesdata[i].heading << "; accel[m/s2]: "
                    << std::setprecision(12) << vehiclesdata[i].accelmsq << std::endl;
      }
  }

  void
  GPSTraceClient::GPSTraceClientSetup(std::function<Ptr<Node>()> create_fcn,std::function<void(Ptr<Node>)> destroy_fcn)
  {
      m_includeNode=create_fcn;
      m_excludeNode=destroy_fcn;
  }

  void
  GPSTraceClient::playTrace(Time const &delay)
  {
    if(m_accelerationset == false)
    {
      NS_FATAL_ERROR ("Error. Attempted to start a GPS Trace Client with undefined acceleration values.");
    }

    Simulator::Schedule(delay, &GPSTraceClient::CreateNode, this);
  }

  void
  GPSTraceClient::CreateNode()
  {
      m_vehNode=m_includeNode();

      // First position update
      m_lastvehicledataidx=0;

      UpdatePositions();
  }

  // "vehiclesdata" is expected to be ordered by utc_time
  // The helper should and must take care of that by calling sortVehiclesdata() at least once!
  void
  GPSTraceClient::UpdatePositions(void)
  {
    if(m_vehNode==nullptr)
    {
        if (m_vehicle_visualizer!=nullptr && m_vehicle_visualizer->isConnected())
        {
            m_vehicle_visualizer->terminateServer ();
        }
        NS_FATAL_ERROR("NULL vehicle node pointer passed to GPSTraceClient::UpdatePositions (vehicle ID: "<<m_vehID<<".");
    }
    Ptr<MobilityModel> mob = m_vehNode->GetObject<MobilityModel>();

    if(m_updatefirstiter==false)
      {
          m_lastvehicledataidx++;
          m_travelled_distance+=UTMUPS_Math_haversineDist(vehiclesdata[m_lastvehicledataidx].lat,
                                    vehiclesdata[m_lastvehicledataidx].lon,
                                    vehiclesdata[m_lastvehicledataidx-1].lat,
                                    vehiclesdata[m_lastvehicledataidx-1].lon);
      }
    else
      {
          m_updatefirstiter=false;
          m_travelled_distance=0;
      }

    mob->SetPosition(Vector(vehiclesdata[m_lastvehicledataidx].tm_x,vehiclesdata[m_lastvehicledataidx].tm_y,1.5));

    if (m_vehicle_visualizer!=nullptr && m_vehicle_visualizer->isConnected())
    {
        int rval = m_vehicle_visualizer->sendObjectUpdate (m_vehID,vehiclesdata[m_lastvehicledataidx].lat,vehiclesdata[m_lastvehicledataidx].lon,vehiclesdata[m_lastvehicledataidx].heading);
        if (rval<0)
        {
            NS_FATAL_ERROR("Error: cannot send the object update to the vehicle visualizer for vehicle: "<<m_vehID);
        }
    }
    if(m_lastvehicledataidx+1==vehiclesdata.size())
      {
        m_excludeNode(m_vehNode);
        m_vehNode=nullptr;

        NS_LOG_INFO("Trace terminated for vehicle: "<<m_vehID);

        return;
      }

    m_event_updatepos=Simulator::Schedule(MicroSeconds (vehiclesdata[m_lastvehicledataidx+1].utc_time-vehiclesdata[m_lastvehicledataidx].utc_time), &GPSTraceClient::UpdatePositions, this);
  }

  void
  GPSTraceClient::StopUpdates(void)
  {
      m_event_updatepos.Cancel ();
  }

  void
  GPSTraceClient::generateAccelerationValues()
  {
    if(m_accelerationset == true)
    {
      NS_FATAL_ERROR ("Error. gps-tc generateAccelerationValues() can only be used when no acceleration values are set so far.");
    }

    for(unsigned long int i=0;i<vehiclesdata.size()-1;i++)
    {
        vehiclesdata[i].accelmsq = (vehiclesdata[i+1].speedms-vehiclesdata[i].speedms)/((vehiclesdata[i+1].utc_time-vehiclesdata[i].utc_time)/1e6);
    }
    vehiclesdata[vehiclesdata.size()-1].accelmsq = 0.0;

    if(m_accelerationset == false)
    {
        m_accelerationset = true;
    }
  }

  void
  GPSTraceClient::setInterpolationPoints(int interval_ms)
  {
      std::vector<positioning_data_t> nextpts;
      transverse_mercator_t tmerc=UTMUPS_init_UTM_TransverseMercator ();

      if(m_accelerationset == false)
      {
        NS_FATAL_ERROR ("Error. Attempted to call setInterpolationPoints() on a GPS Trace Client with undefined acceleration values.");
      }

      // Process each couple of real points in the vehiclesdata structure (which should be already being completely filled in, via the setter methods)
      for(unsigned long int i=0;i<vehiclesdata.size()-1;i++)
      {
          // If two points are "far enough" in time (i.e. their timestamp difference is greater than interval_ms milliseconds), create a certain number of in-between linearly interpolated points
          if(vehiclesdata[i+1].utc_time-vehiclesdata[i].utc_time > interval_ms*1000.0)
          {
              // Find the number of points needed to have new data every around interval_ms
              // floor() is used, i.e. we try to keep the number of added points as low as possible (we will have updated every interval_ms ms or something a bit more than interval_ms ms)
              // ceil() could also be used here; the usage of floor() is just an initial choice and may change in the future versions
              // This is actually providing the number of points + 1 (including also in the count the final real point, corresponding to vehiclesdata[i+1]), thus we will need to use numpoints - 1 later on
              int numpoints = floor(((vehiclesdata[i+1].utc_time-vehiclesdata[i].utc_time)/1000.0)/interval_ms);

              // Compute the heading difference between the two successive points
              // The heading difference is computed taking into account angles between 0 and 360 degrees and the smallest angle between two heading values
              double head_diff = fabs(fmod((vehiclesdata[i+1].heading - vehiclesdata[i].heading),360.0));
              if(head_diff > 180)
              {
                  head_diff = 360 - head_diff;
              }

              // The computed heading difference is signed in order to correctly interpolate depending on the initial and final heading values, coming from the two real points
              if((vehiclesdata[i+1].heading - vehiclesdata[i].heading > -180 && vehiclesdata[i+1].heading - vehiclesdata[i].heading < 0) ||
                 vehiclesdata[i+1].heading - vehiclesdata[i].heading > 180)
              {
                  head_diff *= -1;
              }

              // Prepare an empty std::vector to accumulate the points to be added
              nextpts.clear ();

              // Add numpoints-1 total points by linearly interpolating the different variables
              for(int p=1;p<numpoints;p++)
              {
                positioning_data_t nextpoint;
                double head_val = vehiclesdata[i].heading + head_diff*p/numpoints;

                if(head_val < 0)
                {
                    head_val += 360.0;
                }
                else if(head_val >= 360)
                {
                    head_val = fmod(head_val,360.0);
                }

                // Linearly interpolated heading
                nextpoint.heading = head_val;
                // Linearly interpolated speed
                nextpoint.speedms = vehiclesdata[i].speedms + (vehiclesdata[i+1].speedms - vehiclesdata[i].speedms)*p/numpoints;

                // Constant acceleration
                // Linear would be: nextpoint.accelmsq = vehiclesdata[i].accelmsq + (vehiclesdata[i+1].accelmsq - vehiclesdata[i].accelmsq)*p/numpoints;
                nextpoint.accelmsq = vehiclesdata[i].accelmsq;
                // Linearly interpolated time
                nextpoint.utc_time = vehiclesdata[i].utc_time + (vehiclesdata[i+1].utc_time - vehiclesdata[i].utc_time)*p/numpoints;

                // Interpolate linearly also the position (in terms of x,y, which are then converted back to lat,lon)
                double lat,lon,tm_gamma,tm_kappa;
                nextpoint.tm_x = vehiclesdata[i].tm_x + (vehiclesdata[i+1].tm_x - vehiclesdata[i].tm_x)*p/numpoints;
                nextpoint.tm_y = vehiclesdata[i].tm_y + (vehiclesdata[i+1].tm_y - vehiclesdata[i].tm_y)*p/numpoints;
                if(TransverseMercator_Reverse (&tmerc,m_lon0,nextpoint.tm_x,nextpoint.tm_y,&lat,&lon,&tm_gamma,&tm_kappa)!= UTMUPS_OK)
                {
                    NS_FATAL_ERROR ("Error while interpolating. Cannot perform reverse Transverse Mercator projection from (x,y) to (lat,lon)");
                }
                nextpoint.lat = lat;
                nextpoint.lon = lon;

                // Accumulate the new points inside a std::vector
                nextpts.push_back (nextpoint);
              }

//              // Leftover code (commented out) just for debug purposes
//              std::cout << "--------- X -----------" << std::endl;
//              std::cout << "Initial X: " << vehiclesdata[i].tm_x << std::endl;
//              for(unsigned int ll=0;ll<nextpts.size ();ll++) {
//                  std::cout << "Interpolated X: "<< nextpts[ll].tm_x << std::endl;
//              }
//              std::cout << "Final X: " << vehiclesdata[i+1].tm_x << std::endl;
//              std::cout << "-----------------------" << std::endl;

              // Insert the std::vector with the new point in between the two real points, inside the main "vehiclesdata" vector,
              // which is the one which will be used to "play" the trace
              vehiclesdata.insert (vehiclesdata.begin() + i + 1,nextpts.begin(),nextpts.end());

              // For the next iteration, start from the next couple of real points
              // The element at [i+1], after the insertion of nextpts (of size nextpts.size()), will be now accessible at [i+1+nextpts.size()]
              i += nextpts.size();
          }
      }
  }
}

