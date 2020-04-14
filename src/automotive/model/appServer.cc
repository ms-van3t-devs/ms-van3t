/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 *
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

 * Edited by Marco Malinverno, Politecnico di Torino (marco.malinverno@polito.it)
*/
#include <errno.h>

#include "appServer.h"

extern "C"
{
  #include "asn1/CAM.h"
  #include "asn1/DENM.h"
}

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("appServer");

  NS_OBJECT_ENSURE_REGISTERED(appServer);

  TypeId
  appServer::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::appServer")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<appServer> ()
        .AddAttribute ("LonLat",
           "If it is true, position are sent through lonlat (not XY).",
           BooleanValue (false),
           MakeBooleanAccessor (&appServer::m_lon_lat),
           MakeBooleanChecker ())
        .AddAttribute ("AggregateOutput",
           "If it is true, the server will print every second an aggregate output about cam and denm",
           BooleanValue (false),
           MakeBooleanAccessor (&appServer::m_aggregate_output),
           MakeBooleanChecker ())
        .AddAttribute ("RealTime",
           "To compute properly timestamps",
           BooleanValue(false),
           MakeBooleanAccessor (&appServer::m_real_time),
           MakeBooleanChecker ())
        .AddAttribute ("Client",
           "TraCI client for SUMO",
           PointerValue (0),
           MakePointerAccessor (&appServer::m_client),
           MakePointerChecker<TraciClient> ());
        return tid;
  }

  appServer::appServer ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_cam_received = 0;
    m_denm_sent = 0;
  }

  appServer::~appServer ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  appServer::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  appServer::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    /*** In this very simple example, it is identified a smaller area (2/3 of the original map, with same origin)
     *   where a speed control is actuated: in this area the maximum allowed speed is 13.89 m/s while outside is 30 m/s.
     *   The application will first of all calculate this area, then, in the function receiveCAM, it will check every CAM
     *   to see if the limits are respected.
    ***/
    libsumo::TraCIPositionVector net_boundaries = m_client->TraCIAPI::simulation.getNetBoundary ();
    libsumo::TraCIPosition pos1;
    libsumo::TraCIPosition pos2;
    libsumo::TraCIPosition map_center;

    /* Check if LonLat or XY are used */
    //Long = x, Lat = y
    if (m_lon_lat)
      {
        pos1 = m_client->TraCIAPI::simulation.convertXYtoLonLat (net_boundaries[0].x,net_boundaries[0].y);
        pos2 = m_client->TraCIAPI::simulation.convertXYtoLonLat (net_boundaries[1].x,net_boundaries[1].y);
      }
    else
      {
        pos1 = net_boundaries[0];
        pos2 = net_boundaries[1];

      }

    /* Check the center of the map */
    map_center.x = (pos1.x + pos2.x)/2;
    map_center.y = (pos1.y + pos2.y)/2;

    /* Check the size of the map */
    double map_size_x = std::abs(pos1.x - pos2.x);
    double map_size_y = std::abs(pos1.y - pos2.y);

    /* Compute x and y limit */
    m_upperLimit.x = map_center.x + ((map_size_x)*2/3)/2;
    m_lowerLimit.x = map_center.x - ((map_size_x)*2/3)/2;
    m_upperLimit.y = map_center.y + ((map_size_y)*2/3)/2;
    m_lowerLimit.y = map_center.y - ((map_size_y)*2/3)/2;

    /* If aggregate output is enabled, start it */
    if (m_aggregate_output)
      m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &appServer::aggregateOutput, this);
  }

  void
  appServer::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel (m_aggegateOutputEvent);

    if (m_aggregate_output)
      std::cout << Simulator::Now () << "," << m_cam_received  << "," << m_denm_sent << std::endl;
  }

  void
  appServer::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  appServer::TriggerDenm (long detectionTime, int speedmode, Address address)
  {
    /* Build DENM data */
    /* In our case we encode the speedmode inside the stationID
     * FIX: implement other containers, and use them!! */
    den_data_t denm;
    denm.detectiontime = detectionTime;
    denm.messageid = FIX_DENMID;
    denm.proto = FIX_PROT_VERS;
    denm.stationid = speedmode;
    denm.stationtype = StationType_roadSideUnit;

    denm.validity = 10; // seconds

    Ptr<DENMSender> app = GetNode()->GetApplication (0)->GetObject<DENMSender> ();
    int app_ret = app->SendDenm(denm,address);

    if (app_ret)
      m_denm_sent++;
  }

  void
  appServer::receiveCAM (ca_data_t cam, Address address)
  {
    /* If is the first time the this veh sends a CAM, check if it is inside or outside */

    /* Convert the values */
    double lat = (double)cam.latitude/DOT_ONE_MICRO;
    double lon = (double)cam.longitude/DOT_ONE_MICRO;

    if (m_veh_position.find (address) == m_veh_position.end ())
      {
        if (isInside (lon,lat))
          m_veh_position[address] = INSIDE;
        else
          m_veh_position[address] = OUTSIDE;
        return;
      }

    long timestamp;
    if(m_real_time)
      {
        timestamp = compute_timestampIts ()%65536;
      }
    else
      {
        struct timespec tv = compute_timestamp ();
        timestamp = (tv.tv_nsec/1000000)%65536;
      }

    if (isInside (lon,lat))
      {
        /* The vehice is in the low-speed area */
        /* If it was registered as in the high-speed area, then send a DENM telling him to slow down,
           otherwise do nothing */
        if (m_veh_position[address] == OUTSIDE)
          {
            m_veh_position[address] = INSIDE;
            TriggerDenm (timestamp,0,address);
          }
      }
    else
      {
        /* The vehicle is the high-speed area */
        /* If it was registered as in the slow-speed area, then send a DENM telling him that it can speed,
           otherwise do nothing */
        if (m_veh_position[address] == INSIDE)
          {
            m_veh_position[address] = OUTSIDE;
            TriggerDenm (timestamp,1,address);
          }
      }
  }



  bool
  appServer::isInside(double x, double y)
  {
    if (x > m_lowerLimit.x && x < m_upperLimit.x && y > m_lowerLimit.y && y < m_upperLimit.y)
      return true;
    else
      return false;
    return 0;
  }

  struct timespec
  appServer::compute_timestamp ()
  {
    struct timespec tv;
    if (!m_real_time)
      {
        double nanosec =  Simulator::Now ().GetNanoSeconds ();
        tv.tv_sec = 0;
        tv.tv_nsec = nanosec;
      }
    else
      {
        clock_gettime (CLOCK_MONOTONIC, &tv);
      }
    return tv;
  }

  long
  appServer::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }

  void
  appServer::aggregateOutput()
  {
    std::cout << Simulator::Now () << "," << m_cam_received << "," << m_denm_sent << std::endl;
    m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &appServer::aggregateOutput, this);
  }

}







