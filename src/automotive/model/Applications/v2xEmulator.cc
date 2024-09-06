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

#include "v2xEmulator.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/vdpTraci.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/asn_utils.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("v2xEmulator");

  NS_OBJECT_ENSURE_REGISTERED(v2xEmulator);

  TypeId
  v2xEmulator::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::v2xEmulator")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<v2xEmulator> ()
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&v2xEmulator::m_client),
            MakePointerChecker<TraciClient> ())
        .AddAttribute ("SendCAM",
            "Enable the CAM transmission",
            BooleanValue (true),
            MakeBooleanAccessor (&v2xEmulator::m_send_cam),
            MakeBooleanChecker ())
        .AddAttribute ("SendDENM",
            "Enable the DENM transmission",
            BooleanValue (true),
            MakeBooleanAccessor (&v2xEmulator::m_send_denm),
            MakeBooleanChecker ())
        .AddAttribute ("SendCPM",
           "Enable the CPM transmission",
           BooleanValue (true),
           MakeBooleanAccessor (&v2xEmulator::m_send_cpm),
           MakeBooleanChecker ())
        .AddAttribute ("DestinationIPv4",
            "Destination IPv4 address when working in UDP mode",
            Ipv4AddressValue ("192.168.1.1"),
            MakeIpv4AddressAccessor (&v2xEmulator::m_udpmode_ipAddress),
            MakeIpv4AddressChecker ())
        .AddAttribute ("DestinationPort",
            "Destination port when working in UDP mode",
            IntegerValue (65510),
            MakeIntegerAccessor (&v2xEmulator::m_udpmode_port),
            MakeIntegerChecker <int>(1,65535))
        .AddAttribute ("UDPmode",
            "Flag set to true to enable UDP mode",
            BooleanValue (false),
            MakeBooleanAccessor (&v2xEmulator::m_udpmode_enabled),
            MakeBooleanChecker ());
        return tid;
  }

  v2xEmulator::v2xEmulator ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_udpmode_enabled = false;
  }

  v2xEmulator::~v2xEmulator ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  v2xEmulator::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  v2xEmulator::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);
    TypeId tid;
    m_id = m_client->GetVehicleId (this->GetNode ());

    /* Create the socket for TX and RX (must be a P */
    if(!m_udpmode_enabled)
    {
      // PacketSocket in order to use GeoNet instead of IP)
      tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
    }
    else
    {
      // UDP socket in BTP+GeoNetworking+UDP+IPv4 mode
      tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    }

    /* Socket used to send CAMs and receive DENMs */
    m_socket = Socket::CreateSocket (GetNode (), tid);

    if(!m_udpmode_enabled)
    {
        /* UDP mode */
        /* Bind the socket to local address */
        PacketSocketAddress local;
        local.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
        local.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetAddress ());
        local.SetProtocol (0x8947);
        if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind client socket");
        }

        // Set the socket to broadcast
        PacketSocketAddress remote;
        remote.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
        remote.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetBroadcast ());
        remote.SetProtocol (0x8947);
        m_socket->Connect(remote);
    }
    else
    {
        if (m_socket->Bind () == -1)
        {
          NS_FATAL_ERROR ("Failed to bind UDP socket");
        }
        if(m_socket->Connect (InetSocketAddress(m_udpmode_ipAddress,m_udpmode_port))!=0)
        {
          NS_FATAL_ERROR ("Error: cannot connect UDP socket.");
        }
    }
    VDP* traci_vdp = new VDPTraCI(m_client,m_id);
    //Create LDM and sensor object
    m_LDM = CreateObject<LDM>();
    m_LDM->setStationID(m_id);
    m_LDM->setTraCIclient(m_client);
    m_LDM->setVDP(traci_vdp);

    m_sensor = CreateObject<SUMOSensor>();
    m_sensor->setStationID(m_id);
    m_sensor->setTraCIclient(m_client);
    m_sensor->setVDP(traci_vdp);
    m_sensor->setLDM (m_LDM);

    // Create new BTP and GeoNet objects and set them in DENBasicService and CABasicService
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();
    m_btp->setGeoNet(m_geoNet);
    m_denService.setBTP(m_btp);
    m_caService.setBTP(m_btp);
    m_cpService.setBTP(m_btp);

    m_caService.setLDM(m_LDM);
    m_cpService.setLDM(m_LDM);

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setSocketRx (m_socket);
    m_denService.setSocketTx (m_socket);
    m_denService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);
    m_denService.addDENRxCallback (std::bind(&v2xEmulator::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));
    m_denService.setRealTime (true);

    /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
    m_caService.setSocketRx (m_socket);
    m_caService.setSocketTx (m_socket);
    m_caService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);
    m_caService.addCARxCallback (std::bind(&v2xEmulator::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));
    m_caService.setRealTime (true);

    /* Set sockets, callback, station properties and TraCI VDP in CPBasicService */
    m_cpService.setSocketTx (m_socket);
    m_cpService.setSocketRx (m_socket);
    m_cpService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);
    m_cpService.addCPRxCallback (std::bind(&v2xEmulator::receiveCPM,this,std::placeholders::_1,std::placeholders::_2));
    m_cpService.setRealTime (true);
    m_cpService.setTraCIclient (m_client);

    /* Set TraCI vdp for GeoNet object */

    m_caService.setVDP(traci_vdp);
    m_denService.setVDP(traci_vdp);
    m_cpService.setVDP(traci_vdp);

    /* Schedule CAM dissemination */
    std::srand(Simulator::Now().GetNanoSeconds ());
    double desync = ((double)std::rand()/RAND_MAX);
    if (m_send_cam)
      m_caService.startCamDissemination(desync);

    /* Schedule CPM dissemination */
    if (m_send_cpm)
      m_cpService.startCpmDissemination();

    /* Schedule DENM dissemination */
    if (m_send_denm)
      v2xEmulator::TriggerDenm ();
  }

  void
  v2xEmulator::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel (m_sendDenmEvent);
    m_socket->ShutdownRecv ();
    uint64_t cam_sent, cpm_sent;
    cam_sent = m_caService.terminateDissemination ();
    cpm_sent = m_cpService.terminateDissemination ();
    m_denService.cleanup();
    m_LDM->cleanup();
    m_sensor->cleanup();

    std::cout<<"Number of CAMs sent for vehicle " <<m_id<< ": "<<cam_sent<<std::endl<< std::flush;
    std::cout<<"Number of CPMs sent for vehicle " <<m_id<< ": "<<cpm_sent<<std::endl<< std::flush;


    m_denService.cleanup();

  }

  void
  v2xEmulator::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  v2xEmulator::TriggerDenm()
  {
    denData data;
    DEN_ActionID_t actionid;
    DENBasicService_error_t trigger_retval;

    /* Set DENM mandatpry fields */
    data.setDenmMandatoryFields (compute_timestampIts(),Latitude_unavailable,Longitude_unavailable);

    /* Compute GeoArea for denms */
    GeoArea_t geoArea;
    // Longitude and Latitude in [0.1 microdegree]
    libsumo::TraCIPosition pos = m_client->TraCIAPI::vehicle.getPosition (m_id);
    pos = m_client->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);
    geoArea.posLong = pos.x*DOT_ONE_MICRO;
    geoArea.posLat = pos.y*DOT_ONE_MICRO;
    // Radius [m] of the circle around the vehicle, where the DENM will be received
    geoArea.distA = 50;
    // DistB [m] and angle [deg] equal to zero because we are defining a circular area as specified in ETSI EN 302 636-4-1 [9.8.5.2]
    geoArea.distB = 0;
    geoArea.angle = 0;
    geoArea.shape = CIRCULAR;
    m_denService.setGeoArea (geoArea);

    trigger_retval=m_denService.appDENM_trigger(data,actionid);

    if(trigger_retval!=DENM_NO_ERROR)
    {
      NS_LOG_ERROR("Cannot trigger DENM. Error code: " << trigger_retval);
    }

    m_sendDenmEvent = Simulator::Schedule (Seconds (1), &v2xEmulator::UpdateDenm, this, actionid);
  }

  void
  v2xEmulator::UpdateDenm (DEN_ActionID actionid)
  {
    denData data;
    DENBasicService_error_t update_retval;

    /* Set DENM mandatpry fields */
    data.setDenmMandatoryFields (compute_timestampIts(),Latitude_unavailable,Longitude_unavailable);

    /* Compute GeoArea for denms */
    GeoArea_t geoArea;
    // Longitude and Latitude in [0.1 microdegree]
    libsumo::TraCIPosition pos = m_client->TraCIAPI::vehicle.getPosition (m_id);
    pos = m_client->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);
    geoArea.posLong = pos.x*DOT_ONE_MICRO;
    geoArea.posLat = pos.y*DOT_ONE_MICRO;
    // Radius [m] of the circle around the vehicle, where the DENM will be received
    geoArea.distA = 50;
    // DistB [m] and angle [deg] equal to zero because we are defining a circular area as specified in ETSI EN 302 636-4-1 [9.8.5.2]
    geoArea.distB = 0;
    geoArea.angle = 0;
    geoArea.shape = CIRCULAR;
    m_denService.setGeoArea (geoArea);

    update_retval = m_denService.appDENM_update (data,actionid);
    if(update_retval!=DENM_NO_ERROR)
    {
      NS_LOG_ERROR("Cannot update DENM. Error code: " << update_retval);
    }

    m_sendDenmEvent = Simulator::Schedule (Seconds (1), &v2xEmulator::UpdateDenm, this, actionid);

  }

  void
  v2xEmulator::receiveCAM (asn1cpp::Seq<CAM> cam, Address from)
  {

   // Ignore messages coming from itself
   // This is needed as broadcasted packets over a promiscuous inteface are also received back on the same socket
   if(asn1cpp::getField(cam->header.stationId,StationID_t)==std::stoul(m_id.substr (3)))
       return;

    /* Implement CAM strategy here */
    std::cout << "Vehicle with ID "<< m_id << " received a new CAM with stationID: "<< asn1cpp::getField(cam->header.stationId,StationID_t) << std::endl;
  }

  void
  v2xEmulator::receiveCPM (asn1cpp::Seq<CollectivePerceptionMessage> cpm, ns3::Address from)
  {
        // Ignore messages coming from itself
        // This is needed as broadcasted packets over a promiscuous inteface are also received back on the same socket
        if(asn1cpp::getField(cpm->header.stationId,StationID_t)==std::stoul(m_id.substr (3)))
                return;

        /* Implement CPM strategy here */
        std::cout << "Vehicle with ID "<< m_id << " received a new CPM with stationID: "<< asn1cpp::getField(cpm->header.stationId,StationID_t) << std::endl;
  }

  void
  v2xEmulator::receiveDENM (denData denm, Address from)
  {
    // Ignore messages coming from itself
    // This is needed as broadcasted packets over a promiscuous inteface are also received back on the same socket
    if(denm.getDenmHeaderStationID()==std::stol(m_id.substr (3)))
    {
        return;
    }

    /* This is just a sample dummy receiveDENM function. The user can customize it to parse the content of a DENM when it is received. */
    std::cout << "Vehicle with ID "<< m_id << " received a new DENM." << std::endl;

    (void) denm; // Contains the data received from the DENM
    (void) from; // Contains the address from which the DENM has been received
  }

  long
  v2xEmulator::compute_timestampIts ()
  {

    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }
}





