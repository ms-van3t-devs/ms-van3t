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

 * Edited by Francesco Raviglione and Marco Malinverno, Politecnico di Torino
 * (francescorav.es483@gmail.com)
 * (marco.malinverno@polito.it)
*/
#include "appServer.h"

#include "asn1/CAM.h"
#include "asn1/DENM.h"
#include "ns3/socket.h"

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
        .AddAttribute ("CSV",
            "CSV log name.",
            StringValue (),
            MakeStringAccessor (&appServer::m_csv_name),
            MakeStringChecker ())
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

    /*
     * In this very simple example, it is identified a smaller area (2/3 of the original map, with same origin)
     * where a speed control is actuated: in this area the maximum allowed speed is 6.94 m/s while outside is 20.83 m/s.
     * The application will first of all determine this area, then, in the function receiveCAM, it will check every CAM
     * to see if the limits are respected.
    */

    libsumo::TraCIPositionVector net_boundaries = m_client->TraCIAPI::simulation.getNetBoundary ();
    libsumo::TraCIPosition pos1;
    libsumo::TraCIPosition pos2;
    libsumo::TraCIPosition map_center;

    /* Convert (x,y) to (long,lat) */
    // Long = x, Lat = y
    pos1 = m_client->TraCIAPI::simulation.convertXYtoLonLat (net_boundaries[0].x,net_boundaries[0].y);
    pos2 = m_client->TraCIAPI::simulation.convertXYtoLonLat (net_boundaries[1].x,net_boundaries[1].y);

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

    /* TX socket for DENMs and RX socket for CAMs (one socket only is necessary) */
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

    m_socket = Socket::CreateSocket (GetNode (), tid);

    // Bind the socket to receive packets coming from every IP
    InetSocketAddress local_denm = InetSocketAddress (Ipv4Address::GetAny (), 9);
    if (m_socket->Bind (local_denm) == -1)
      {
        NS_FATAL_ERROR ("Failed to bind server socket");
      }
    m_socket->SetRecvCallback (MakeCallback (&CABasicService::receiveCam, &m_caService));

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setSocketTx (m_socket);

    // Setting a station ID (for instance, 777888999)
    m_denService.setStationProperties (777888999, StationType_roadSideUnit);

    /* Set callback and station properties in CABasicService (which will only be used to receive CAMs) */
    m_caService.setStationProperties (777888999, StationType_roadSideUnit);
    m_caService.addCARxCallback (std::bind(&appServer::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));

    if (!m_csv_name.empty ())
      {
        m_csv_ofstream_cam.open (m_csv_name+"-server.csv",std::ofstream::trunc);
        m_csv_ofstream_cam << "messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration" << std::endl;
      }

    /* If aggregate output is enabled, start it */
    if (m_aggregate_output)
      m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &appServer::aggregateOutput, this);
  }

  void
  appServer::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel (m_aggegateOutputEvent);

    if (!m_csv_name.empty ())
      m_csv_ofstream_cam.close ();

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
  appServer::TriggerDenm (speedmode_t speedmode, Address from)
  {
    denData data;
    denData::denDataAlacarte alacartedata;
    ActionID_t actionid;
    DENBasicService_error_t trigger_retval;

    memset(&alacartedata,0,sizeof(denData::denDataAlacarte));

    /* Build DENM data */
    data.setDenmMandatoryFields (compute_timestampIts(),Latitude_unavailable,Longitude_unavailable);

    // As there is no proper "SpeedLimit" field inside a DENM message, for an area speed advisory, we rely on the
    // RoadWorksContainerExtended, inside the "A la carte" container, which actually has a "SpeedLimit" field
    alacartedata.roadWorks = (sRoadWorksContainerExtended_t *) calloc(sizeof(sRoadWorksContainerExtended_t),1);

    if(alacartedata.roadWorks == NULL)
      {
        NS_LOG_ERROR("Cannot send DENM. Unable to allocate memory for alacartedata.roadWorks.");
        return;
      }

    alacartedata.roadWorks->speedLimit = (SpeedLimit_t *) calloc(sizeof(SpeedLimit_t),1);

    if(alacartedata.roadWorks->speedLimit == NULL)
      {
        free(alacartedata.roadWorks);
        NS_LOG_ERROR("Cannot send DENM. Unable to allocate memory for alacartedata.roadWorks->speedLimit.");
        return;
      }

    // Set a speed limit advisory inside the DENM message
    *(alacartedata.roadWorks->speedLimit) = (SpeedLimit_t) speedmode;

    data.setDenmAlacarteData_asn_types (alacartedata);

    m_socket->Connect (from);

    trigger_retval=m_denService.appDENM_trigger(data,actionid);
    if(trigger_retval!=DENM_NO_ERROR)
      {
        NS_LOG_ERROR("Cannot trigger DENM. Error code: " << trigger_retval);
      }
    else
      {
        m_denm_sent++;
      }

    if(alacartedata.roadWorks) free(alacartedata.roadWorks);
    if(alacartedata.roadWorks->speedLimit) free(alacartedata.roadWorks->speedLimit);
  }

  void
  appServer::receiveCAM (CAM_t *cam, Address address)
  {
    m_cam_received++;
    /* If is the first time the this veh sends a CAM, check if it is inside or outside */
    /* Convert the values */
    double lat = (double)cam->cam.camParameters.basicContainer.referencePosition.latitude/DOT_ONE_MICRO;
    double lon = (double)cam->cam.camParameters.basicContainer.referencePosition.longitude/DOT_ONE_MICRO;

    if (!m_csv_name.empty ())
      {
        // messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration
        m_csv_ofstream_cam << cam->header.messageID << "," << cam->header.stationID << ",";
        m_csv_ofstream_cam << cam->cam.generationDeltaTime << "," << (double)cam->cam.camParameters.basicContainer.referencePosition.latitude/DOT_ONE_MICRO << ",";
        m_csv_ofstream_cam << (double)cam->cam.camParameters.basicContainer.referencePosition.longitude/DOT_ONE_MICRO << "," ;
        m_csv_ofstream_cam << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue/DECI << "," << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue/CENTI << ",";
        m_csv_ofstream_cam << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationValue/DECI << std::endl;
      }

    if (m_veh_position.find (address) == m_veh_position.end ())
      {
        if (appServer::isInside (lon,lat))
          m_veh_position[address] = INSIDE;
        else
          m_veh_position[address] = OUTSIDE;
        return;
      }

    if (appServer::isInside (lon,lat))
      {
        /* The vehice is in the low-speed area */
        /* If it was registered as in the high-speed area, then send a DENM telling him to slow down,
           otherwise do nothing */
        if (m_veh_position[address] == OUTSIDE)
          {
            m_veh_position[address] = INSIDE;
            appServer::TriggerDenm (slowSpeedkmph,address);
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
            appServer::TriggerDenm (highSpeedkmph,address);
          }
      }

    ASN_STRUCT_FREE(asn_DEF_CAM,cam);
  }

  bool
  appServer::isInside(double x, double y)
  {
    if (x > m_lowerLimit.x && x < m_upperLimit.x && y > m_lowerLimit.y && y < m_upperLimit.y)
      return true;
    else
      return false;
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







