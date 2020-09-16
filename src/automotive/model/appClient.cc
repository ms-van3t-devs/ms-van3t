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
#include "appClient.h"

#include "asn1/CAM.h"
#include "asn1/DENM.h"
#include "ns3/vdpTraci.h"
#include "ns3/socket.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("appClient");

  NS_OBJECT_ENSURE_REGISTERED(appClient);

  long retValue(double value, int defValue, int fix, int fixNeeded)
  {
      if(fix<fixNeeded)
          return defValue;
      else
          return value;
  }

  TypeId
  appClient::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::appClient")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<appClient> ()
        .AddAttribute ("PrintSummary",
            "To print summary at the end of simulation",
            BooleanValue(false),
            MakeBooleanAccessor (&appClient::m_print_summary),
            MakeBooleanChecker ())
        .AddAttribute ("SendCam",
            "If it is true, the branch sending the CAM is activated.",
            BooleanValue (true),
            MakeBooleanAccessor (&appClient::m_send_cam),
            MakeBooleanChecker ())
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&appClient::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&appClient::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("ServerAddr",
            "Ip Addr of the server",
            Ipv4AddressValue("10.0.0.1"),
            MakeIpv4AddressAccessor (&appClient::m_server_addr),
            MakeIpv4AddressChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&appClient::m_client),
            MakePointerChecker<TraciClient> ());
        return tid;
  }

  appClient::appClient ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_print_summary = true;
    m_already_print = false;

    m_cam_sent = 0;
    m_denm_received = 0;
  }

  appClient::~appClient ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  appClient::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  appClient::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);
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
    // Set the socket to send to the server
    InetSocketAddress remote = InetSocketAddress (m_server_addr, 9);
    m_socket->Connect(remote);

    /* Make the callback to handle received DENMs */
    m_socket->SetRecvCallback (MakeCallback (&DENBasicService::receiveDENM, &m_denService));

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);
    m_denService.addDENRxCallback (std::bind(&appClient::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));

    /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
    m_caService.setSocketTx (m_socket);
    m_caService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);

    VDPTraCI traci_vdp(m_client,m_id);
    m_caService.setVDP(traci_vdp);

    /* Create CSV file, if requested */
    if (!m_csv_name.empty ())
      {
        m_csv_ofstream.open (m_csv_name+"-"+m_id+".csv",std::ofstream::trunc);
        m_csv_ofstream << "messageID,originatingStationID,sequence,referenceTime,detectionTime,stationID" << std::endl;
      }

    /* Schedule CAM dissemination */
    std::srand(Simulator::Now().GetNanoSeconds ());
    if (m_send_cam)
      {
         double desync = ((double)std::rand()/RAND_MAX);
         m_caService.startCamDissemination(desync);
      }
  }

  void
  appClient::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Remove(m_sendCamEvent);

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
  appClient::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  appClient::receiveDENM (denData denm, Address from)
  {
    /* This function extracts only one parameter from the DENM:
     */
    m_denm_received++;
    // Uncomment the following line to print a line to standard output for each DENM received by a vehicle
    //std::cout << "DENM received by " << m_id << std::endl;

    /*
     * Check the speed limit saved in the roadWorks container inside
     * the optional "A la carte" container
     * The division by 3.6 is used to convert the value stored in the DENM
     * from km/h to m/s, as required for SUMO
    */
    if(denm.getDenmAlacarteData_asn_types ().roadWorks->speedLimit == NULL)
      {
        NS_FATAL_ERROR("Error in appClient.cc. Received a NULL pointer for speedLimit.");
      }

    double speedLimit = *(denm.getDenmAlacarteData_asn_types ().roadWorks->speedLimit)/3.6;

    m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, speedLimit);

    // Change color for fast-moving vehicles to orange
    if(speedLimit>13)
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

  struct timespec
  appClient::compute_timestamp ()
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
  appClient::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }
}





