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
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "ns3/BSContainer.h"
#include "ns3/vdp.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("BSContainer");

  BSContainer::~BSContainer() {
    NS_LOG_INFO("BSContainer object destroyed.");
  }

  BSContainer::BSContainer()
  {
    m_station_id = ULONG_MAX;
    m_stationtype = LONG_MAX;
    m_socket = nullptr;
    m_btp = nullptr;
    m_gn = nullptr;
    m_real_time = false;
    m_mobility_client = nullptr;

    m_prrsup_ptr = nullptr;
    m_prrsup_beacons = true;

    m_sumo_vehid_prefix = "veh";

    m_is_configured = false;
    m_DENMs_enabled = false;
  }

  BSContainer::BSContainer(unsigned long fixed_stationid, long fixed_stationtype, Ptr<TraciClient> mobility_client, bool real_time, Ptr<Socket> socket_rxtx)
  {
    m_btp = nullptr;
    m_gn = nullptr;

    m_prrsup_ptr = nullptr;
    m_prrsup_beacons = true;

    m_sumo_vehid_prefix = "veh";

    m_is_configured = false;
    m_DENMs_enabled = false;

    m_station_id = (StationID_t) fixed_stationid;
    m_stationtype = (StationType_t) fixed_stationtype;

    m_mobility_client=mobility_client;
    m_real_time=real_time;

    m_socket=socket_rxtx;
  }

  void
  BSContainer::setStationProperties(unsigned long fixed_stationid,long fixed_stationtype)
  {
    m_station_id=fixed_stationid;
    m_stationtype=fixed_stationtype;
  }

  void
  BSContainer::setStationID(unsigned long fixed_stationid)
  {
    m_station_id=fixed_stationid;
  }

  void
  BSContainer::setStationType(long fixed_stationtype)
  {
    m_stationtype=fixed_stationtype;
  }

  void
  BSContainer::setSocket(Ptr<Socket> socket_rxtx)
  {
    m_socket=socket_rxtx;
  }

  void
  BSContainer::setupContainer(bool CABasicService_enabled,bool DENBasicService_enabled) {
    if(CABasicService_enabled==false && DENBasicService_enabled==false) {
      NS_FATAL_ERROR("Error. Called setupContainer() asking for enabling zero Basic Services. Aborting simulation.");
    }

    if(m_socket==nullptr || m_mobility_client==nullptr || m_station_id==ULONG_MAX || m_stationtype==LONG_MAX) {
      NS_FATAL_ERROR("Error. Attempted to call setupContainer() on an uninitialized BSContainer. Aborting simulation.");
    }

    // Setup the required services
    // ETSI Transport and Networking layers
    m_btp = CreateObject <btp>();
    m_gn = CreateObject <GeoNet>();

    if(m_prrsup_ptr!=nullptr) {
      m_gn->setPRRSupervisor (m_prrsup_ptr);
    }

    if(m_prrsup_beacons==false) {
      m_gn->disablePRRsupervisorForBeacons();
    }

    m_btp->setGeoNet (m_gn);

    if(CABasicService_enabled==true) {
      m_cabs.setBTP (m_btp);
      m_cabs.setSocketTx (m_socket);
      m_cabs.setSocketRx (m_socket);

      // Remember that setStationProperties() must always be called *after* setBTP()
      m_cabs.setStationProperties (m_station_id, m_stationtype);

      if(m_CAReceiveCallbackExtended!=nullptr) {
        m_cabs.addCARxCallbackExtended (m_CAReceiveCallbackExtended);
      }

      m_CAMs_enabled = true;
    }

    m_vdp_ptr = new VDPTraCI(m_mobility_client,m_sumo_vehid_prefix + std::to_string(m_station_id));
    m_btp->setVDP(m_vdp_ptr);

    if(CABasicService_enabled==true) {
      m_cabs.setVDP(m_vdp_ptr);
    }

    if(DENBasicService_enabled==true) {
      m_denbs.setBTP (m_btp);
      m_denbs.setSocketTx (m_socket);
      m_denbs.setSocketRx (m_socket);
      m_denbs.setStationProperties (m_station_id, (long)m_stationtype);

      if(m_DENReceiveCallbackExtended!=nullptr) {
        m_denbs.addDENRxCallbackExtended (m_DENReceiveCallbackExtended);
      }

      m_DENMs_enabled = true;
    }

    m_is_configured = true;
  }

  void
  BSContainer::setDENMCircularGeoArea(double lat, double lon, int radius_meters)
  {
    if(m_is_configured==false) {
      NS_FATAL_ERROR("Error. setDENMCircularGeoArea() should not be called before setupContainer(). Please adjust your code.");
    }

    if(m_DENMs_enabled==true) {
      /* Compute GeoArea for DENMs */
      GeoArea_t geoArea;
      // Longitude and Latitude in [0.1 microdegree]
      geoArea.posLong = lon*DOT_ONE_MICRO;
      geoArea.posLat = lat*DOT_ONE_MICRO;
      // Radius [m] of the circle that covers the whole square area of the map in (x,y)
      geoArea.distA = radius_meters;
      // DistB [m] and angle [deg] equal to zero because we are defining a circular area as specified in ETSI EN 302 636-4-1 [9.8.5.2]
      geoArea.distB = 0;
      geoArea.angle = 0;
      geoArea.shape = CIRCULAR;

      m_denbs.setGeoArea (geoArea);
    }
  }

  void
  BSContainer::cleanup()
  {
    if(m_CAMs_enabled==true) {
      m_cabs.terminateDissemination();
    }

    if(m_DENMs_enabled==true) {
      m_denbs.cleanup ();
    }

    if(m_gn!=nullptr) {
      m_gn->cleanup();
    }
  }
}
