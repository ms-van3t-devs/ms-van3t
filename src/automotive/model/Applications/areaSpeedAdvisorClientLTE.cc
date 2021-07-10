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

#include "areaSpeedAdvisorClientLTE.h"
#include "areaSpeedAdvisorServerLTE.h" // Needed in order to use the "speedmode_t" enum

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/vdpTraci.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("areaSpeedAdvisorClientLTE");

  NS_OBJECT_ENSURE_REGISTERED(areaSpeedAdvisorClientLTE);

  TypeId
  areaSpeedAdvisorClientLTE::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::areaSpeedAdvisorClientLTE")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<areaSpeedAdvisorClientLTE> ()
        .AddAttribute ("PrintSummary",
            "To print summary at the end of simulation",
            BooleanValue(false),
            MakeBooleanAccessor (&areaSpeedAdvisorClientLTE::m_print_summary),
            MakeBooleanChecker ())
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&areaSpeedAdvisorClientLTE::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&areaSpeedAdvisorClientLTE::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("ServerAddr",
            "Ip Addr of the server",
            Ipv4AddressValue("10.0.0.1"),
            MakeIpv4AddressAccessor (&areaSpeedAdvisorClientLTE::m_server_addr),
            MakeIpv4AddressChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&areaSpeedAdvisorClientLTE::m_client),
            MakePointerChecker<TraciClient> ());
        return tid;
  }

  areaSpeedAdvisorClientLTE::areaSpeedAdvisorClientLTE ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_print_summary = true;
    m_already_print = false;

    m_cam_sent = 0;
    m_denm_received = 0;
  }

  areaSpeedAdvisorClientLTE::~areaSpeedAdvisorClientLTE ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  areaSpeedAdvisorClientLTE::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  areaSpeedAdvisorClientLTE::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    /*
     * This application works as client for the areaSpeedAdvisorServerLTE. It is intended to be installed over vehicular nodes,
     * and it is set to generate unicast CAM messages toward the centralized server on top of BTP-GeoNet-UDP-IPv4.
     * As soon as a DENM is received, it reads the information inside the RoadWorks container and
     * sets the speed accordingly (see receiveDENM() function)
     */

    m_id = m_client->GetVehicleId (this->GetNode ());

    /* Create the socket for TX and RX */
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

    /* Socket used to send CAMs and receive DENMs */
    m_socket = Socket::CreateSocket (GetNode (), tid);

    /* Bind the socket to receive packets coming from every IP */
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 9);
    if (m_socket->Bind (local) == -1)
    {
      NS_FATAL_ERROR ("Failed to bind client socket");
    }

    /* Connect it to the server */
    InetSocketAddress remote = InetSocketAddress (m_server_addr, 9);
    m_socket->Connect(remote);

    // Create new BTP and GeoNet objects and set them in DENBasicService and CABasicService
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();
    m_btp->setGeoNet(m_geoNet);
    m_denService.setBTP(m_btp);
    m_caService.setBTP(m_btp);

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setSocketRx (m_socket);
    m_denService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);
    m_denService.addDENRxCallback (std::bind(&areaSpeedAdvisorClientLTE::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));
    m_denService.setRealTime (m_real_time);

    /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
    m_caService.setSocketTx (m_socket);
    m_caService.setSocketRx (m_socket);
    m_caService.addCARxCallback (std::bind(&areaSpeedAdvisorClientLTE::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));
    m_caService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);
    m_caService.setRealTime (m_real_time);

    VDP* traci_vdp = new VDPTraCI(m_client,m_id);
    m_caService.setVDP(traci_vdp);
    m_denService.setVDP(traci_vdp);

    /* Create CSV file, if requested */
    if (!m_csv_name.empty ())
    {
      m_csv_ofstream.open (m_csv_name+"-"+m_id+".csv",std::ofstream::trunc);
      m_csv_ofstream << "messageID,originatingStationID,sequence,referenceTime,detectionTime,stationID" << std::endl;
    }

    /* Schedule CAM dissemination */
    std::srand(Simulator::Now().GetNanoSeconds ());
    double desync = ((double)std::rand()/RAND_MAX);
    m_caService.startCamDissemination(desync);
  }

  void
  areaSpeedAdvisorClientLTE::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendCamEvent);

    uint64_t cam_sent;

    if (!m_csv_name.empty ())
      m_csv_ofstream.close ();

    cam_sent = m_caService.terminateDissemination ();
    m_denService.cleanup();

    if (m_print_summary && !m_already_print)
    {
      std::cout << "INFO-" << m_id
                << ",CAM-SENT:" << cam_sent
                << ",DENM-RECEIVED:" << m_denm_received
                << std::endl;
      m_already_print=true;
    }
  }

  void
  areaSpeedAdvisorClientLTE::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  areaSpeedAdvisorClientLTE::receiveDENM (denData denm, Address from)
  {
    m_denm_received++;

    // Uncomment the following line to print a line to standard output for each DENM received by a vehicle
    // std::cout << "DENM received by " << m_id << std::endl;

    /*
     * Check the speed limit saved in the roadWorks container inside
     * the optional "A la carte" container
     * The division by 3.6 is used to convert the value stored in the DENM
     * from km/h to m/s, as required by SUMO
    */
    if(!denm.getDenmAlacarteData_asn_types ().getData ().roadWorks.getData ().speedLimit.isAvailable ())
    {
      NS_FATAL_ERROR("Error in areaSpeedAdvisorClientLTE.cc. Received a NULL pointer for speedLimit.");
    }

    double speedLimit = denm.getDenmAlacarteData_asn_types ().getData ().roadWorks.getData ().speedLimit.getData ();

    m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, speedLimit/3.6);

    // Change color for fast-moving vehicles to orange
   if(speedLimit>=highSpeedkmph)
    {
      libsumo::TraCIColor orange;
      orange.r=255;orange.g=99;orange.b=71;orange.a=255;
      m_client->TraCIAPI::vehicle.setColor (m_id,orange);
    }
    // Change color for slow-moving vehicles to green
    else
    {
      libsumo::TraCIColor green;
      green.r=50;green.g=205;green.b=50;green.a=255;
      m_client->TraCIAPI::vehicle.setColor (m_id,green);
    }

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream << denm.getDenmHeaderMessageID () << ","
                     << denm.getDenmActionID ().originatingStationID << ","
                     << denm.getDenmActionID ().sequenceNumber << ","
                     << denm.getDenmMgmtReferenceTime () << ","
                     << denm.getDenmMgmtDetectionTime () << ","
                     << denm.getDenmHeaderStationID () << std::endl;
    }
  }

  void
  areaSpeedAdvisorClientLTE::receiveCAM (asn1cpp::Seq<CAM> cam, Address from)
  {
    /* Implement CAM strategy here */

    (void) cam;
    (void) from;
  }

  long
  areaSpeedAdvisorClientLTE::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }
}





