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
#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <string>
#include <stdlib.h>
#include <algorithm>
#include "ns3/udp-socket-factory.h"
#include <sys/time.h>
#include <sys/types.h>
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"

#include <errno.h>

#include "v2i-CAM-sender.h"
extern "C"
{
  #include "asn1/CAM.h"
  #include "asn1/DENM.h"
}

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("v2i-CAM-sender");

  NS_OBJECT_ENSURE_REGISTERED(CAMSender);

  long setConfidence(double confidence, const int defConfidence, double value, const int defValue)
  {
      if(value==defValue)
          return defConfidence;
      else
          return confidence;
  }

  long retValue(double value, int defValue, int fix, int fixNeeded)
  {
      if(fix<fixNeeded)
          return defValue;
      else
          return value;
  }

  TypeId
  CAMSender::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::CAMSender")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<CAMSender> ()
        .AddAttribute ("Port",
            "The port on which the client will listen for incoming packets.",
            UintegerValue (9),
            MakeUintegerAccessor (&CAMSender::m_port),
            MakeUintegerChecker<uint16_t> ())
        .AddAttribute ("NodePrefix",
            "The prefix used to idefy vehicles in SUMO.",
            StringValue ("veh"),
            MakeStringAccessor (&CAMSender::m_veh_prefix),
            MakeStringChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&CAMSender::m_client),
            MakePointerChecker<TraciClient> ())
        .AddAttribute ("Index",
            "Index of the current node",
            IntegerValue (1),
            MakeIntegerAccessor (&CAMSender::m_index),
            MakeIntegerChecker<int> ())
        .AddAttribute ("SendCam",
            "If it is true, the branch sending the CAM is activated.",
            BooleanValue (true),
            MakeBooleanAccessor (&CAMSender::m_send_cam),
            MakeBooleanChecker ())
        .AddAttribute ("LonLat",
            "If it is true, position are sent through lonlat (not XY).",
            BooleanValue (false),
            MakeBooleanAccessor (&CAMSender::m_lon_lat),
            MakeBooleanChecker ())
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&CAMSender::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("PrintSummary",
            "To print summary at the end of simulation",
            BooleanValue(true),
            MakeBooleanAccessor (&CAMSender::m_print_summary),
            MakeBooleanChecker ())
        .AddAttribute ("ServerAddr",
            "Ip Addr of the server",
            Ipv4AddressValue("10.0.0.2"),
            MakeIpv4AddressAccessor (&CAMSender::m_server_addr),
            MakeIpv4AddressChecker ())
        .AddAttribute ("CAMIntertime",
            "Time between two consecutive CAMs",
            DoubleValue(0.1),
            MakeDoubleAccessor (&CAMSender::m_cam_intertime),
            MakeDoubleChecker<double> ())
        .AddAttribute ("ASN",
            "If true, it uses ASN.1 to encode and decode CAMs and DENMs",
            BooleanValue(false),
            MakeBooleanAccessor (&CAMSender::m_asn),
            MakeBooleanChecker ());

        return tid;
  }

  CAMSender::CAMSender ()
  {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    m_port = 0;
    m_client = nullptr;
    m_veh_prefix = "veh";
    m_cam_seq = 0;
    m_cam_sent = 0;
    m_denm_received = 0;
    m_print_summary = true;
    m_already_print = false;
  }

  CAMSender::~CAMSender ()
  {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
  }

  void
  CAMSender::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  CAMSender::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);
    RngSeedManager::SetSeed (m_index);

    // Create the UDP socket for the client
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);

    InetSocketAddress remote = InetSocketAddress (m_server_addr, m_port);

    // Bind the socket to receive packets coming from every IP
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
    if (m_socket->Bind (local) == -1)
      {
        NS_FATAL_ERROR ("Failed to bind client socket");
      }
    m_socket->Connect(remote);

    m_id = m_client->GetVehicleId (this->GetNode ());    
    // Schedule CAM dissemination
    if (m_send_cam)
       m_sendCamEvent = Simulator::Schedule (Seconds (1.0), &CAMSender::SendCam, this);

    // Make the callback to handle received packets
    m_socket->SetRecvCallback (MakeCallback (&CAMSender::HandleRead, this));

    /* If we are in realtime, save a base timestamp to start from */
    if(m_real_time)
      {
        struct timespec tv;
        clock_gettime (CLOCK_MONOTONIC, &tv);
        m_start_ms = tv.tv_sec*1000 + (tv.tv_nsec/1000000);
      }
  }

  void
  CAMSender::StopApplication ()
  {
    NS_LOG_FUNCTION(this);

    if (m_socket != 0)
      {
        m_socket->Close ();
        m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
        m_socket = 0;
      }
    Simulator::Remove(m_sendCamEvent);

    if (m_print_summary && !m_already_print)
      {
        std::cout << "INFO-" << m_id
                  << ",CAM-SENT:" << m_cam_sent
                  << ",DENM-RECEIVED:" << m_denm_received
                  << std::endl;
        m_already_print=true;
      }
  }

  void
  CAMSender::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  CAMSender::SendCam()
  {
    /*DEBUG: Print position*/
    //Ptr<MobilityModel> mob = this->GetNode ()->GetObject<MobilityModel> ();
    //std::cout << "x:" << mob->GetPosition ().x << std::endl;
    //std::cout << "y:" << mob->GetPosition ().y << std::endl;
    //std::cout << "x_sumo:" << m_client->TraCIAPI::vehicle.getPosition (m_id).x<< std::endl;
    //std::cout << "y_sumo:" << m_client->TraCIAPI::vehicle.getPosition (m_id).y << std::endl;

    if (m_asn)
      CAMSender::Populate_and_send_asn_cam();
    else
      CAMSender::Populate_and_send_normal_cam();

    // Schedule next CAM
    m_sendCamEvent = Simulator::Schedule (Seconds (m_cam_intertime), &CAMSender::SendCam, this);
  }

  void
  CAMSender::Populate_and_send_normal_cam()
  {
    struct timespec tv = compute_timestamp ();

    std::ostringstream msg;

    /* If lonlat is used, positions should be converted */
    /* Positions */
    libsumo::TraCIPosition pos = m_client->TraCIAPI::vehicle.getPosition(m_id);
    if (m_lon_lat)
      {
        pos = m_client->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);
        pos.x = pos.x * DOT_ONE_MICRO;
        pos.y = pos.y * DOT_ONE_MICRO;
      }
    else
      { /* For the standard you should use WGS84 co-ordinate system */
        pos.x = pos.x * MICRO;
        pos.y = pos.y * MICRO;
      }

    /* Create the message to be sent in plain text */
    msg << "CAM," << m_id << ","
        << pos.x << ","
        << pos.y << ","
        << m_client->TraCIAPI::vehicle.getSpeed(m_id)*CENTI << ","
        << m_client->TraCIAPI::vehicle.getAcceleration (m_id)*DECI << ","
        << m_client->TraCIAPI::vehicle.getAngle (m_id)*DECI << ","
        << tv.tv_sec << "," << tv.tv_nsec << ","
        << m_cam_seq << ",end\0";

    // Tweak: add +1, otherwise some strange character are received at the end of the packet
    uint16_t packetSize = msg.str ().length () + 1;
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) msg.str ().c_str (), packetSize);

    m_cam_seq++;
    m_cam_sent++;
    // Send packet through the interface
    m_socket->Send(packet);
  }

  void
  CAMSender::Populate_and_send_asn_cam()
  {
    struct timespec tv = compute_timestamp ();

    CAM_t *cam = (CAM_t*) calloc(1, sizeof(CAM_t));

    /* Install the high freq container */
    cam->cam.camParameters.highFrequencyContainer.present = HighFrequencyContainer_PR_basicVehicleContainerHighFrequency;

    /* Generation delta time (ms since time reference) */
    long timestamp;
    if(m_real_time)
      {
        timestamp = compute_timestampIts ()%65536;
      }
    else
      timestamp = (tv.tv_nsec/1000000)%65536;
    cam->cam.generationDeltaTime = (GenerationDeltaTime_t)timestamp;

    /* Station Type */
    cam->cam.camParameters.basicContainer.stationType = StationType_passengerCar;

    /* Positions */
    libsumo::TraCIPosition pos = m_client->TraCIAPI::vehicle.getPosition(m_id);
    if (m_lon_lat)
      {
        pos = m_client->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);
        pos.x = pos.x * DOT_ONE_MICRO;
        pos.y = pos.y * DOT_ONE_MICRO;
      }
    else
      { /* For the standard you should use WGS84 co-ordinate system */
        pos.x = pos.x * MICRO;
        pos.y = pos.y * MICRO;
      }

    //altitude
    cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeConfidence=AltitudeConfidence_unavailable;
    cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeValue=AltitudeValue_unavailable;

    //latitude
    Latitude_t latitudeT=(Latitude_t)retValue(pos.y,DEF_LATITUDE,0,0);
    cam->cam.camParameters.basicContainer.referencePosition.latitude=latitudeT;

    //longitude
    Longitude_t longitudeT=(Longitude_t)retValue(pos.x,DEF_LONGITUDE,0,0);
    cam->cam.camParameters.basicContainer.referencePosition.longitude=longitudeT;

    /* Heading */
    double angle = m_client->TraCIAPI::vehicle.getAngle (m_id);
    Heading heading;
    heading.headingValue=(HeadingValue_t)retValue (angle*DECI,DEF_HEADING,0,0);
    heading.headingConfidence=HeadingConfidence_unavailable;
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading=heading;

    /* Speed */
    Speed_t vel;
    double speed=m_client->TraCIAPI::vehicle.getSpeed(m_id);
    vel.speedValue=(SpeedValue_t)retValue(speed*CENTI,DEF_SPEED,0,0);
    vel.speedConfidence=SpeedConfidence_unavailable;
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed=vel;

    /* Acceleration */
    LongitudinalAcceleration_t longAcc;
    double acc=m_client->TraCIAPI::vehicle.getAcceleration (m_id);
    longAcc.longitudinalAccelerationValue=(LongitudinalAccelerationValue_t)retValue(acc*DECI,DEF_ACCELERATION,0,0);
    longAcc.longitudinalAccelerationConfidence=AccelerationConfidence_unavailable;
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration=longAcc;

    /* Length and width of car */
    double veh_length = m_client->TraCIAPI::vehicle.getLength (m_id);
    VehicleLength length;
    length.vehicleLengthConfidenceIndication=VehicleLengthConfidenceIndication_unavailable;
    length.vehicleLengthValue=(VehicleLengthValue_t)retValue (veh_length*DECI,DEF_LENGTH,0,0);
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength = length;
    double veh_width = m_client->TraCIAPI::vehicle.getWidth (m_id);
    VehicleWidth width = (VehicleWidth)retValue (veh_width*DECI,DEF_WIDTH,0,0);
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth = width;

    /* Other needed fields */
    cam->header.protocolVersion=FIX_PROT_VERS;
    cam->header.stationID = std::stol (m_id.substr (3));
    cam->header.messageID=FIX_CAMID;

    /* We filled just some fields, you can fill them all to match any purpose */
    /** Encoding **/
    void *buffer = NULL;
    asn_per_constraints_s *constraints = NULL;
    ssize_t ec = uper_encode_to_new_buffer(&asn_DEF_CAM, constraints, cam, &buffer);
    if (ec==-1)
      {
        std::cout << "Cannot encode CAM" << std::endl;
        return;
      }

    /** Packet creation **/
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) buffer, ec+1);

    m_socket->Send (packet);
    m_cam_seq++;
    m_cam_sent++;

    ASN_STRUCT_FREE(asn_DEF_CAM,cam);
  }


  void
  CAMSender::HandleRead (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    packet = socket->RecvFrom (from);

    uint8_t *buffer = new uint8_t[packet->GetSize ()];
    packet->CopyData (buffer, packet->GetSize ()-1);

    //In this example, the useful information is coded in the ASN.1 DENM in the field "sequenceNumber", while in plain text, as the third element
    int speedmode;

    if (m_asn)
      {
        /** Decoding **/
        void *decoded_=NULL;
        asn_dec_rval_t rval;

        rval = uper_decode(0, &asn_DEF_DENM, &decoded_, buffer, packet->GetSize ()-1, 0, 1);

        if (rval.code == RC_FAIL)
          {
            std::cout << "DENM ASN.1 decoding failed!" << std::endl;
            return;
          }

        DENM_t *decoded = (DENM_t *) decoded_;

        std::cout << "DENM in ASN.1 format received by " << m_id << std::endl;
        m_denm_received++;

        speedmode = (int)decoded->denm.management.actionID.sequenceNumber;

        /* Now in "decoded" you have the DENM */
        ASN_STRUCT_FREE(asn_DEF_DENM,decoded);
      }

    else
      {
        std::vector<std::string> values;
        std::string s = std::string ((char*) buffer);
        //std::cout << "Packet received - content:" << s << std::endl;
        std::stringstream ss(s);
        std::string element;
        while (std::getline(ss, element, ',')) {
            values.push_back (element);
          }

        std::cout << "DENM in plain text received by " << m_id << std::endl;
        m_denm_received++;

        speedmode = std::stoi (values[3]);
      }

    /* Build your DENM strategy here! */
    /* In this example, we pass to the application only the information about the speedmode */
    Ptr<appClient> app = GetNode()->GetApplication (1)->GetObject<appClient> ();
    app->receiveDENM (speedmode);
  }

  /* This function is used to calculate the delay for packet reception */
  double
  CAMSender::time_diff(double sec1, double usec1, double sec2, double usec2)
    {
            double tot1 , tot2 , diff;
            tot1 = sec1 + (usec1 / 1000000000.0);
            tot2 = sec2 + (usec2 / 1000000000.0);
            diff = tot2 - tot1;
            return diff;
    }

  struct timespec
  CAMSender::compute_timestamp ()
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
  CAMSender::compute_timestampIts ()
  {
    /* To et millisec since  2004-01-01T00:00:00:000Z*/
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    long elapsed_since_2004 = millis.count() - TIME_SHIFT;
    return elapsed_since_2004;
  }

}





