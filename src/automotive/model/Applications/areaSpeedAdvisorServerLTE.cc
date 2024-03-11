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
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
 *  Carlos Mateo Risma Carletti, Politecnico di Torino (carlosrisma@gmail.com)
*/

#include "areaSpeedAdvisorServerLTE.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/socket.h"
#include "ns3/btpdatarequest.h"
#include "ns3/network-module.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("areaSpeedAdvisorServerLTE");

  NS_OBJECT_ENSURE_REGISTERED(areaSpeedAdvisorServerLTE);

  TypeId
  areaSpeedAdvisorServerLTE::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::areaSpeedAdvisorServerLTE")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<areaSpeedAdvisorServerLTE> ()
        .AddAttribute ("AggregateOutput",
           "If it is true, the server will print every second an aggregate output about cam and denm",
           BooleanValue (false),
           MakeBooleanAccessor (&areaSpeedAdvisorServerLTE::m_aggregate_output),
           MakeBooleanChecker ())
        .AddAttribute ("RealTime",
           "To compute properly timestamps",
           BooleanValue(false),
           MakeBooleanAccessor (&areaSpeedAdvisorServerLTE::m_real_time),
           MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name.",
            StringValue (),
            MakeStringAccessor (&areaSpeedAdvisorServerLTE::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("Client",
           "TraCI client for SUMO",
           PointerValue (0),
           MakePointerAccessor (&areaSpeedAdvisorServerLTE::m_client),
           MakePointerChecker<TraciClient> ())
        .AddAttribute ("PRRSupervisor",
            "PRR Supervisor to compute PRR according to 3GPP TR36.885 V14.0.0 page 70",
            PointerValue (0),
            MakePointerAccessor (&areaSpeedAdvisorServerLTE::m_PRR_supervisor),
            MakePointerChecker<PRRSupervisor> ());
        return tid;
  }

  areaSpeedAdvisorServerLTE::areaSpeedAdvisorServerLTE ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_cam_received = 0;
    m_denm_sent = 0;
  }

  areaSpeedAdvisorServerLTE::~areaSpeedAdvisorServerLTE ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  areaSpeedAdvisorServerLTE::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  areaSpeedAdvisorServerLTE::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    /*
     * In this example, the speed control is actuated in a small area at the center of the map (2/3 of the original map, with the same origin).
     * In the inner area the maximum allowed speed is 6.94 m/s (25 km/h) while outside is 20.83 m/s (75 km/h).
     * The application will first of all determine this area, then, in the receiveCAM() function, it will check every CAM
     * to see if a DENM should be transmitted.
     * Thus, DENM are transmitted to vehicles moving from the "slow" area to the "fast" area or vice versa.
     * In this case, since we are working in the ns-3 LTE framework, it is not possible
     * to generate broadcast V2X messages. Therefore, both CAMs (from vehicle to server) and DENMs (from server to vehicles) will be unicast.
     * The packets will be encapsulated inside BTP->GeoNet->UDP->IPv4.
    */

    /* First of all deterine the boundaries of the inner and outer areas */
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


    /* Compute GeoArea for DENMs
     * NOTE: In this case the GeoArea where the DENMs are valid should comprise all the points in which the transition between the "slow" and "fast" areas
     * are located. Otherwise, there will be "grey" areas, where the DENMs will be discarded by the GeoNet module, but that require the vehicles to take
     * some action (either slow-down or speed-up).
     * For this reason, the dimension of the GeoArea is very big (300m of radius): in this way, all the DENMs will be delivered to the application layer.
     */
    GeoArea_t geoArea;
    // Longitude and Latitude in [0.1 microdegree]
    geoArea.posLong = map_center.x*DOT_ONE_MICRO;
    geoArea.posLat = map_center.y*DOT_ONE_MICRO;
    // Radius [m] of the circle that covers the whole square area of the map in (x,y)
    geoArea.distA = 300;
    // DistB [m] and angle [deg] equal to zero because we are defining a circular area as specified in ETSI EN 302 636-4-1 [9.8.5.2]
    geoArea.distB = 0;
    geoArea.angle = 0;
    geoArea.shape = CIRCULAR;

    /* TX socket for DENMs and RX socket for CAMs */
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

    m_socket = Socket::CreateSocket (GetNode (), tid);

    /* Bind the socket to receive packets coming from every IP */
    InetSocketAddress local_denm = InetSocketAddress (Ipv4Address::GetAny (), 9);

    // Bind the socket to local address
    if (m_socket->Bind (local_denm) == -1)
    {
      NS_FATAL_ERROR ("Failed to bind server UDP socket");
    }

    // Create new BTP and GeoNet objects and set them in DENBasicService and CABasicService
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();

    if(m_PRR_supervisor!=nullptr)
    {
      m_geoNet->setPRRSupervisor(m_PRR_supervisor);
    }

    m_btp->setGeoNet(m_geoNet);
    m_denService.setBTP(m_btp);
    m_caService.setBTP(m_btp);

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setSocketTx (m_socket);
    m_denService.setSocketRx (m_socket);
    m_denService.addDENRxCallback (std::bind(&areaSpeedAdvisorServerLTE::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));
    // Setting geoArea address for DENMs
    m_denService.setGeoArea (geoArea);
    // Setting a station ID (for instance, 777888999)
    m_denService.setStationProperties (777888999, StationType_roadSideUnit);

    /* Set callback and station properties in CABasicService (which will only be used to receive CAMs) */
    m_caService.setSocketRx (m_socket);
    m_caService.setStationProperties (777888999, StationType_roadSideUnit);
    m_caService.addCARxCallback (std::bind(&areaSpeedAdvisorServerLTE::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));

    // Set the central position for the service (i.e. a reference fixed position used by GeoNetworking)
    // As reference position for GeoNetworking, we can take the center of the map (i.e. (0,0)), as if an RSU was there
    // As the position must be specified in (lat, lon), we must convert it to Latitude and Longitude
    // As SUMO is used here, we can rely on the TraCIAPI for this conversion
    // As functions to set this position to the CA/DEN Basic Service, we can use "setFixedPositionRSU", even if no 802.11p RSU exists
    // Then, it will be the Basic Services that will take care of setting this position in the GeoNet object
    libsumo::TraCIPosition servicePos = m_client->TraCIAPI::simulation.convertXYtoLonLat (0,0);
    m_denService.setFixedPositionRSU (servicePos.y,servicePos.x);
    m_caService.setFixedPositionRSU (servicePos.y,servicePos.x);
    
    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.open (m_csv_name+"-server.csv",std::ofstream::trunc);
      m_csv_ofstream_cam << "messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration" << std::endl;
    }

    /* If aggregate output is enabled, start it */
    if (m_aggregate_output)
      m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &areaSpeedAdvisorServerLTE::aggregateOutput, this);
  }

  void
  areaSpeedAdvisorServerLTE::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel (m_aggegateOutputEvent);

    m_denService.cleanup();

    if (!m_csv_name.empty ())
      m_csv_ofstream_cam.close ();

    if (m_aggregate_output)
      std::cout << Simulator::Now () << "," << m_cam_received  << "," << m_denm_sent << std::endl;
  }

  void
  areaSpeedAdvisorServerLTE::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  areaSpeedAdvisorServerLTE::TriggerDenm (speedmode_t speedmode, Address from)
  {
    denData data;
    denData::denDataAlacarte alacartedata;
    DEN_RoadWorksContainerExtended_t roadworks;
    DEN_ActionID_t actionid;
    DENBasicService_error_t trigger_retval;

    /* Build DENM data */
    data.setDenmMandatoryFields (compute_timestampIts(),Latitude_unavailable,Longitude_unavailable);

    // As there is no proper "SpeedLimit" field inside a DENM message, for an area speed Advisor, we rely on the
    // RoadWorksContainerExtended, inside the "A la carte" container, which actually has a "SpeedLimit" field


    roadworks.speedLimit.setData (speedmode);
    alacartedata.roadWorks.setData (roadworks);


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

  }

  void
  areaSpeedAdvisorServerLTE::receiveCAM (asn1cpp::Seq<CAM> cam, Address address)
  {
    m_cam_received++;

    /* Convert the values */
    double lat = asn1cpp::getField(cam->cam.camParameters.basicContainer.referencePosition.latitude,double)/DOT_ONE_MICRO;
    double lon = asn1cpp::getField(cam->cam.camParameters.basicContainer.referencePosition.longitude,double)/DOT_ONE_MICRO;

    if (!m_csv_name.empty ())
    {
      // messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration
      m_csv_ofstream_cam << asn1cpp::getField(cam->header.messageId,long) << "," << asn1cpp::getField(cam->header.stationId,long) << ",";
      m_csv_ofstream_cam << asn1cpp::getField(cam->cam.generationDeltaTime,long) << "," << lat << "," << lon << ",";
      m_csv_ofstream_cam << asn1cpp::getField(cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue,double)/DECI << "," << asn1cpp::getField(cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue,double)/CENTI << ",";
      m_csv_ofstream_cam << asn1cpp::getField(cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.value,double)/DECI << std::endl;
    }

    /* If is the first time the this veh sends a CAM, just check if it is inside or outside, then return */
    if (m_veh_position.find (address) == m_veh_position.end ())
    {
      if (areaSpeedAdvisorServerLTE::isInside (lon,lat))
        m_veh_position[address] = INSIDE;
      else
        m_veh_position[address] = OUTSIDE;
      return;
    }

    /* Otherwise, check if a transition between INSIDE->OUTSIDE or viceversa happened */
    if (areaSpeedAdvisorServerLTE::isInside (lon,lat))
    {
      /* The vehice is now in the low-speed area
       * If it was registered as in the high-speed area, then send a DENM telling him to slow down */
      if (m_veh_position[address] == OUTSIDE)
      {
        m_veh_position[address] = INSIDE;
        areaSpeedAdvisorServerLTE::TriggerDenm (slowSpeedkmph,address);
      }
    }
    else
    {
      /* The vehicle is now in the high-speed area
       * If it was registered as in the slow-speed area, then send a DENM telling him that to speed-up */
      if (m_veh_position[address] == INSIDE)
      {
        m_veh_position[address] = OUTSIDE;
        areaSpeedAdvisorServerLTE::TriggerDenm (highSpeedkmph,address);
      }
    }
  }

  void
  areaSpeedAdvisorServerLTE::receiveDENM (denData denm, Address from)
  {
    /* This is just a sample dummy receiveDENM function. The user can customize it to parse the content of a DENM when it is received. */
    (void) denm; // Contains the data received from the DENM
    (void) from; // Contains the address from which the DENM has been received
    std::cout<<"Received a new DENM."<<std::endl;
  }

  bool
  areaSpeedAdvisorServerLTE::isInside(double x, double y)
  {
    if (x > m_lowerLimit.x && x < m_upperLimit.x && y > m_lowerLimit.y && y < m_upperLimit.y)
      return true;
    else
      return false;
  }

  long
  areaSpeedAdvisorServerLTE::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }

  void
  areaSpeedAdvisorServerLTE::aggregateOutput()
  {
    std::cout << Simulator::Now () << "," << m_cam_received << "," << m_denm_sent << std::endl;
    m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &areaSpeedAdvisorServerLTE::aggregateOutput, this);
  }

}







