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

    /* Compute x and y limit (in case */
    m_upperLimit.x = map_center.x + ((map_size_x)*2/3)/2;
    m_lowerLimit.x = map_center.x - ((map_size_x)*2/3)/2;
    m_upperLimit.y = map_center.y + ((map_size_y)*2/3)/2;
    m_lowerLimit.y = map_center.y - ((map_size_y)*2/3)/2;
  }

  void
  appServer::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  appServer::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  int
  appServer::receiveCAM (struct CAMinfo cam, Address address)
  {
    /* If is the first time the this veh sends a CAM, check if it is inside or outside */

    /* First of all, convert the values */
    if(m_lon_lat)
      {
        cam.position.x = cam.position.x/DOT_ONE_MICRO;
        cam.position.y = cam.position.y/DOT_ONE_MICRO;
      }
    else
      {
        cam.position.x = cam.position.x/MICRO;
        cam.position.y = cam.position.y/MICRO;
      }
    cam.speed = cam.speed/CENTI;

    if (m_veh_position.find (address) == m_veh_position.end ())
      {
        if (isInside (cam.position.x,cam.position.y))
          m_veh_position[address] = INSIDE;
        else
          m_veh_position[address] = OUTSIDE;
        return 0;
      }

    if (isInside (cam.position.x,cam.position.y))
      {
        /* The vehice is in the low-speed area */
        /* If it was registered as in the high-speed area, then send a DENM telling him to slow down,
           otherwise do nothing */
        if (m_veh_position[address] == OUTSIDE)
          {
            m_veh_position[address] = INSIDE;
            return SEND_0;
          }
        else
          return DO_NOT_SEND;
      }
    else
      {
        /* The vehicle is the high-speed area */
        /* If it was registered as in the slow-speed area, then send a DENM telling him that it can speed,
           otherwise do nothing */
        if (m_veh_position[address] == INSIDE)
          {
            m_veh_position[address] = OUTSIDE;
            return SEND_1;
          }
        else
          return DO_NOT_SEND;
      }

    return 0;
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


}







