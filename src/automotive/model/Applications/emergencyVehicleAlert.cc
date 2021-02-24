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

#include "emergencyVehicleAlert.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/gn-utils.h"

#define DEG_2_RAD(val) ((val)*M_PI/180.0)

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("emergencyVehicleAlert");

  NS_OBJECT_ENSURE_REGISTERED(emergencyVehicleAlert);

  // Function to compute the distance between two objects, given their Lon/Lat
  double appUtil_haversineDist(double lat_a, double lon_a, double lat_b, double lon_b) {
      // 12742000 is the mean Earth radius (6371 km) * 2 * 1000 (to convert from km to m)
      return 12742000.0*asin(sqrt(sin(DEG_2_RAD(lat_b-lat_a)/2)*sin(DEG_2_RAD(lat_b-lat_a)/2)+cos(DEG_2_RAD(lat_a))*cos(DEG_2_RAD(lat_b))*sin(DEG_2_RAD(lon_b-lon_a)/2)*sin(DEG_2_RAD(lon_b-lon_a)/2)));
  }

  // Function to compute the absolute difference between two angles (angles must be between -180 and 180)
  double appUtil_angDiff(double ang1, double ang2) {
      double angDiff;
      angDiff=ang1-ang2;

      if(angDiff>180)
      {
        angDiff-=360;
      }
      else if(angDiff<-180)
      {
        angDiff+=360;
      }
      return std::abs(angDiff);
  }

  TypeId
  emergencyVehicleAlert::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::emergencyVehicleAlert")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<emergencyVehicleAlert> ()
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&emergencyVehicleAlert::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("IpAddr",
            "IpAddr",
            Ipv4AddressValue ("10.0.0.1"),
            MakeIpv4AddressAccessor (&emergencyVehicleAlert::m_ipAddress),
            MakeIpv4AddressChecker ())
        .AddAttribute ("PrintSummary",
            "To print summary at the end of simulation",
            BooleanValue(false),
            MakeBooleanAccessor (&emergencyVehicleAlert::m_print_summary),
            MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&emergencyVehicleAlert::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("Model",
            "Physical and MAC layer communication model",
            StringValue (""),
            MakeStringAccessor (&emergencyVehicleAlert::m_model),
            MakeStringChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&emergencyVehicleAlert::m_client),
            MakePointerChecker<TraciClient> ());
        return tid;
  }

  emergencyVehicleAlert::emergencyVehicleAlert ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_print_summary = true;
    m_already_print = false;

    m_denm_sent = 0;
    m_cam_received = 0;
    m_denm_received = 0;
    m_denm_intertime = 0;

    m_distance_threshold = 75; // Distance used in GeoNet to determine the radius of the circumference arounf the emergency vehicle where the DENMs are valid
    m_heading_threshold = 45; // Max heading angle difference between the normal vehicles and the emergenecy vehicle, that triggers a reaction in the normal vehicles
  }

  emergencyVehicleAlert::~emergencyVehicleAlert ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  emergencyVehicleAlert::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  emergencyVehicleAlert::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    /*
     * In this example, the vehicle can be either of type "passenger" or of type "emergency" (see cars.rou.xml in SUMO folder inside examples/sumo_files_v2v_map)
     * All the vehicles broadcast CAM messages. When a "passenger" car receives a CAM from an "emergency" vehicle, it checks the distance between them and
     * the difference in heading, and if it considers it to be close, it takes proper actions to facilitate the takeover maneuver.
     */

    /* Save the vehicles informations */
    m_id = m_client->GetVehicleId (this->GetNode ());
    m_type = m_client->TraCIAPI::vehicle.getVehicleClass (m_id);
    m_max_speed = m_client->TraCIAPI::vehicle.getMaxSpeed (m_id);

    // Create new BTP and GeoNet objects and set them in DENBasicService and CABasicService
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();
    m_btp->setGeoNet(m_geoNet);
    m_denService.setBTP(m_btp);
    m_caService.setBTP(m_btp);

    /* Create the Sockets for TX and RX */
    TypeId tid;
    if(m_model=="80211p")
      tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
    else if(m_model=="cv2x")
      tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    else
      NS_FATAL_ERROR ("No communication model set - check simulation script - valid models: '80211p' or 'lte'");
    m_socket = Socket::CreateSocket (GetNode (), tid);

    if(m_model=="80211p")
    {
        /* Bind the socket to local address */
        PacketSocketAddress local = getGNAddress(GetNode ()->GetDevice (0)->GetIfIndex (),
                                                GetNode ()->GetDevice (0)->GetAddress () );
        if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind client socket for BTP + GeoNetworking (802.11p)");
        }
        // Set the socketAddress for broadcast
        PacketSocketAddress remote = getGNAddress(GetNode ()->GetDevice (0)->GetIfIndex (),
                                                GetNode ()->GetDevice (0)->GetBroadcast () );
        m_socket->Connect (remote);
    }
    else // m_model=="cv2x"
    {
        /* The C-V2X model requires the socket to be bind to "any" IPv4 address, and to be connected to the
         * IP address of the transmitting node. Then, the model will take care of broadcasting the packets.
        */
        if (m_socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 19)) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind client socket for C-V2X");
        }
        m_socket->Connect (InetSocketAddress(m_ipAddress,19));
    }

    /* Set Station Type in DENBasicService */
    StationType_t stationtype;
    if (m_type=="passenger")
      stationtype = StationType_passengerCar;
    else if (m_type=="emergency")
      stationtype = StationType_specialVehicles;
    else
      stationtype = StationType_unknown;

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setSocketTx (m_socket);
    m_denService.setSocketRx (m_socket);
    m_denService.setStationProperties (std::stol(m_id.substr (3)), (long)stationtype);
    m_denService.addDENRxCallback (std::bind(&emergencyVehicleAlert::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));
    m_denService.setRealTime (m_real_time);

    /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
    m_caService.setSocketTx (m_socket);
    m_caService.setSocketRx (m_socket);
    m_caService.setStationProperties (std::stol(m_id.substr (3)), (long)stationtype);
    m_caService.addCARxCallback (std::bind(&emergencyVehicleAlert::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));
    m_caService.setRealTime (m_real_time);

    /* Set TraCI VDP for GeoNet object */
    VDP* traci_vdp = new VDPTraCI(m_client,m_id);
    m_caService.setVDP(traci_vdp);
    m_denService.setVDP(traci_vdp);

    /* Schedule CAM dissemination */
    std::srand(Simulator::Now().GetNanoSeconds ());
    double desync = ((double)std::rand()/RAND_MAX);
    m_caService.startCamDissemination(desync);

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.open (m_csv_name+"-"+m_id+"-CAM.csv",std::ofstream::trunc);
      m_csv_ofstream_cam << "messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration" << std::endl;
    }
  }

  void
  emergencyVehicleAlert::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_speed_ev);
    Simulator::Cancel(m_send_cam_ev);
    Simulator::Cancel(m_update_denm_ev);

    uint64_t cam_sent;

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.close ();
    }

    cam_sent = m_caService.terminateDissemination ();
    m_denService.cleanup();

    if (m_print_summary && !m_already_print)
    {
      std::cout << "INFO-" << m_id
                << ",CAM-SENT:" << cam_sent
                << ",CAM-RECEIVED:" << m_cam_received
                << std::endl;
      m_already_print=true;
    }
  }

  void
  emergencyVehicleAlert::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  emergencyVehicleAlert::receiveCAM (CAM_t *cam, Address from)
  {
    /* Implement CAM strategy here */
   m_cam_received++;

   /* If the CAM is received from an emergency vehicle, and the host vehicle is a "passenger" car, then process the CAM */
   if (cam->cam.camParameters.basicContainer.stationType==StationType_specialVehicles && m_type!="emergency")
   {
     libsumo::TraCIPosition pos=m_client->TraCIAPI::vehicle.getPosition(m_id);
     pos=m_client->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);

     /* If the distance between the "passenger" car and the emergency vehicle and the difference in the heading angles
      * are below certain thresholds, then actuate the slow-down strategy */
     if (appUtil_haversineDist (pos.y,pos.x,(double)cam->cam.camParameters.basicContainer.referencePosition.latitude/DOT_ONE_MICRO,(double)cam->cam.camParameters.basicContainer.referencePosition.longitude/DOT_ONE_MICRO) < m_distance_threshold
         &&
         appUtil_angDiff (m_client->TraCIAPI::vehicle.getAngle (m_id),(double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue/DECI)<m_heading_threshold)
     {
       /* Slowdown only if you are not in the takeover lane,
        * otherwise the emergency vechicle may get stuck behind */
       if (m_client->TraCIAPI::vehicle.getLaneIndex (m_id) == 0)
       {
         m_client->TraCIAPI::vehicle.changeLane (m_id,0,3);
         m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, m_max_speed*0.5);
         libsumo::TraCIColor orange;
         orange.r=232;orange.g=126;orange.b=4;orange.a=255;
         m_client->TraCIAPI::vehicle.setColor (m_id,orange);

         Simulator::Remove(m_speed_ev);
         m_speed_ev = Simulator::Schedule (Seconds (3.0), &emergencyVehicleAlert::SetMaxSpeed, this);
       }
       else
       {
         m_client->TraCIAPI::vehicle.changeLane (m_id,0,3);
         m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, m_max_speed*1.5);
         libsumo::TraCIColor green;
         green.r=0;green.g=128;green.b=80;green.a=255;
         m_client->TraCIAPI::vehicle.setColor (m_id,green);

         Simulator::Remove(m_speed_ev);
         m_speed_ev = Simulator::Schedule (Seconds (3.0), &emergencyVehicleAlert::SetMaxSpeed, this);
       }
     }
   }

   if (!m_csv_name.empty ())
     {
       // messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration
       m_csv_ofstream_cam << cam->header.messageID << "," << cam->header.stationID << ",";
       m_csv_ofstream_cam << cam->cam.generationDeltaTime << "," << (double)cam->cam.camParameters.basicContainer.referencePosition.latitude/DOT_ONE_MICRO << ",";
       m_csv_ofstream_cam << (double)cam->cam.camParameters.basicContainer.referencePosition.longitude/DOT_ONE_MICRO << "," ;
       m_csv_ofstream_cam << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue/DECI << "," << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue/CENTI << ",";
       m_csv_ofstream_cam << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationValue/DECI << std::endl;
     }

   // Free the received CAM data structure
   ASN_STRUCT_FREE(asn_DEF_CAM,cam);
  }

  void
  emergencyVehicleAlert::receiveDENM (denData denm, Address from)
  {
    /* This is just a sample dummy receiveDENM function. The user can customize it to parse the content of a DENM when it is received. */
    (void) denm; // Contains the data received from the DENM
    (void) from; // Contains the address from which the DENM has been received
    std::cout<<"Received a new DENM."<<std::endl;
  }

  void
  emergencyVehicleAlert::SetMaxSpeed ()
  {
    libsumo::TraCIColor normal;
    normal.r=255;normal.g=255;normal.b=0;normal.a=255;
    m_client->TraCIAPI::vehicle.setColor (m_id, normal);
    m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, m_max_speed);
  }

}





