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

extern "C" {
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
      m_vehNode=NULL;
      m_vehID=vehID;
      m_updatefirstiter=true;
      m_travelled_distance=0;
  }

  GPSTraceClient::~GPSTraceClient()
  {
      m_lastvehicledataidx=0;
      m_includeNode=nullptr;
      m_excludeNode=nullptr;
      m_updatefirstiter=true;
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
      positioning_data_t data_t;
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
    if(m_vehNode==NULL)
    {
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
    if(m_lastvehicledataidx+1==vehiclesdata.size())
      {
        m_excludeNode(m_vehNode);
        m_vehNode=NULL;

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

}

