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

#include "v2i-DENM-sender.h"
extern "C"
{
  #include "asn1/CAM.h"
  #include "asn1/DENM.h"
}

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("v2i-DENM-sender");

  NS_OBJECT_ENSURE_REGISTERED(DENMSender);

  TypeId
  DENMSender::GetTypeId (void)
  {
  static TypeId tid = TypeId ("ns3::DENMSender")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<DENMSender> ()
    .AddAttribute ("Port", "Port on which we send packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&DENMSender::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("AggregateOutput",
                   "If it is true, the server will print every second an aggregate output about cam and denm",
                   BooleanValue (false),
                   MakeBooleanAccessor (&DENMSender::m_aggregate_output),
                   MakeBooleanChecker ())
    .AddAttribute ("Client", "TraCI client for SUMO",
                   PointerValue (0),
                   MakePointerAccessor (&DENMSender::m_client),
                   MakePointerChecker<TraciClient> ())
    .AddAttribute ("RealTime",
                   "To compute properly timestamps",
                   BooleanValue(false),
                   MakeBooleanAccessor (&DENMSender::m_real_time),
                   MakeBooleanChecker ())
    .AddAttribute ("ASN",
                   "If true, it uses ASN.1 to encode and decode CAMs and DENMs",
                   BooleanValue(false),
                   MakeBooleanAccessor (&DENMSender::m_asn),
                   MakeBooleanChecker ());
    return tid;
  }

  DENMSender::DENMSender ()
  {
    NS_LOG_FUNCTION(this);
    m_sendEvent = EventId ();
    m_port = 0;
    m_socket = 0;
    m_cam_received = 0;
    m_denm_sent = 0;
    m_client = nullptr;
  }

  DENMSender::~DENMSender ()
  {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
  }

  void
  DENMSender::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  DENMSender::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    if (m_socket == 0)
      {

        //Create the UDP socket for the server
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket (GetNode (), tid);

        //Bind the socket to receive packets coming from every IP
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
        if (m_socket->Bind (local) == -1)
          {
            NS_FATAL_ERROR ("Failed to bind server socket");
          }
        //Set the callback to call the function as a packet is received
        m_socket->SetRecvCallback (MakeCallback (&DENMSender::HandleRead, this));
      }

    /* If aggregate output is enabled, start it */
    if (m_aggregate_output)
      m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &DENMSender::aggregateOutput, this);

    /* If we are in realtime, save a base timestamp to start from */
    if(m_real_time)
      {
        struct timespec tv;
        clock_gettime (CLOCK_MONOTONIC, &tv);
        m_start_ms = tv.tv_sec*1000 + (tv.tv_nsec/1000000);
      }
  }

  void
  DENMSender::aggregateOutput()
  {
    std::cout << Simulator::Now () << "," << m_cam_received << "," << m_denm_sent << std::endl;
    m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &DENMSender::aggregateOutput, this);
  }

  void
  DENMSender::StopApplication ()
  {
    NS_LOG_FUNCTION(this);

    if (m_socket != 0)
      {
        m_socket->Close ();
        m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      }

    // Cancel all the send events scheduled (add here the events as they are created)
    Simulator::Cancel (m_sendEvent);
    Simulator::Cancel (m_aggegateOutputEvent);

    if (m_aggregate_output)
      std::cout << Simulator::Now () << "," << m_cam_received  << "," << m_denm_sent << std::endl;
  }

  void
  DENMSender::HandleRead (Ptr<Socket> socket)
  {
    CAMinfo cam;
    /* Debug print */
    //NS_LOG_INFO("SUMO time:" << m_client->simulation.getTime () << "\n");
    //NS_LOG_INFO("NS3 time:" << Simulator::Now () << "\n");
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    packet = socket->RecvFrom (from);
    uint8_t *buffer = new uint8_t[packet->GetSize ()-1];
    packet->CopyData (buffer, packet->GetSize ()-1);

    if (m_asn) //If ASN.1 is used
      {
        void *decoded_=NULL;
        asn_dec_rval_t rval;
        rval = uper_decode (NULL, &asn_DEF_CAM, &decoded_, buffer, packet->GetSize ()-1,0,1);

        if (rval.code == RC_FAIL)
          {
            std::cout << "CAM ASN.1 decoding failed!" << std::endl;
            return;
          }

        CAM_t *decoded = (CAM_t *) decoded_;

        /* Now in "decoded" you have the CAM */
        //Long = x, Lat = y

        /* Build your CAM strategy here! */
        cam.position.x = (double)decoded->cam.camParameters.basicContainer.referencePosition.longitude;
        cam.position.y = (double)decoded->cam.camParameters.basicContainer.referencePosition.latitude;
        cam.speed = (double)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue;

        m_cam_received++;
        ASN_STRUCT_FREE(asn_DEF_CAM,decoded);
    }
    else
      {
        /* A CAM in plain text is received */
        std::vector<std::string> values;
        std::string s = std::string ((char*) buffer);
        //std::cout << "Packet received - content:" << s << std::endl;
        std::stringstream ss(s);
        std::string element;
        while (std::getline(ss, element, ',')) {
            values.push_back (element);
          }

        /* Build your CAM strategy here! */
        cam.position.x = std::stod(values[2]);
        cam.position.y = std::stod(values[3]);
        cam.speed = std::stod(values[4]);
        m_cam_received++;
      }

    /* In this case, we pass to the server the information regarding position and speed of the vehicle, as well as a unique identifier
       (we used the IP address from which it sent the massage). The server will return DO_NOT_SEND if it is not necessary to send a DENM.
       In case a DENM with the information to slow down should be sent to the vehicle, the server returns SEND_0; in case a DENM containing
       the info to speed-up is needed, SEND_1 is returned.
    */
    cam.position.z=0;

    Ptr<appServer> app = GetNode()->GetApplication (1)->GetObject<appServer> ();

    int decision = app->receiveCAM (cam,from);

    if (decision == DO_NOT_SEND)
      return;

    if (decision == SEND_0)
      {
        if (m_asn)
          DENMSender::Populate_and_send_asn_denm (from,0);
        else
          DENMSender::Populate_and_send_normal_denm (from,0);
      }
    else if (decision == SEND_1)
      {
        if (m_asn)
          DENMSender::Populate_and_send_asn_denm (from,1);
        else
          DENMSender::Populate_and_send_normal_denm (from,1);
      }

  }

  void
  DENMSender::Populate_and_send_normal_denm(Address address,int speedmode)
  {
    // Generate the packet
    std::ostringstream msg;

    struct timespec tv = compute_timestamp ();

    msg << "This is a DENM,"
        << tv.tv_sec  << ","
        << tv.tv_nsec << ","
        << std::to_string (speedmode) << ",end\0";

    //Tweak: add +1, otherwise some random characters are received at the end of the packet
    uint16_t packetSize = msg.str ().length () + 1;
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) msg.str ().c_str (), packetSize);

    m_socket->SendTo (packet,2,address);
    m_denm_sent++;
  }

  void
  DENMSender::Populate_and_send_asn_denm(Address address, int speedmode)
  {
    struct timespec tv = compute_timestamp ();

    /* First DENM */
    DENM_t *denm1 = (DENM_t*) calloc(1, sizeof(DENM_t));

    /* The DENM here is filled with some example values. Be careful that some of the fields are mandatory */

    long gen_time;
    if(m_real_time)
      {
        long now_ms = tv.tv_sec*1000 + (tv.tv_nsec/1000000);
        gen_time=now_ms-m_start_ms;
      }
    else
      gen_time = (tv.tv_nsec/1000000)%65536;

    INTEGER_t decttime;
    memset(&decttime, 0, sizeof(decttime));
    asn_ulong2INTEGER(&decttime, int(tv.tv_sec));
    denm1->denm.management.detectionTime=decttime;

    INTEGER_t timeResult;
    memset(&timeResult, 0, sizeof(timeResult));
    asn_ulong2INTEGER(&timeResult, gen_time);
    denm1->denm.management.referenceTime=timeResult;

    denm1->header.stationID=0;
    denm1->header.protocolVersion=FIX_PROT_VERS;

    StationType_t stationType = StationType_passengerCar;
    denm1->denm.management.stationType=stationType;
    denm1->denm.management.actionID.originatingStationID=0;

    denm1->denm.management.eventPosition.latitude=Latitude_unavailable;
    denm1->denm.management.eventPosition.altitude.altitudeConfidence=AltitudeConfidence_unavailable;
    denm1->denm.management.eventPosition.altitude.altitudeValue=AltitudeValue_unavailable;
    denm1->denm.management.eventPosition.longitude=Longitude_unavailable;

    /* We encode the app information in sequenceNumber. This should not be done, use "à la carte container" instead (FIX THIS) */
    denm1->header.messageID=FIX_DENMID;
    denm1->denm.management.actionID.sequenceNumber=speedmode;

    /** Encoding **/
    void *buffer1 = NULL;
    asn_per_constraints_s *constraints = NULL;
    ssize_t ec = uper_encode_to_new_buffer(&asn_DEF_DENM, constraints, denm1, &buffer1);
    if (ec==-1)
      {
        std::cout << "Cannot encode DENM" << std::endl;
        return;
      }
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) buffer1, ec+1);
    m_socket->SendTo (packet,2,address);

    m_denm_sent++;
    ASN_STRUCT_FREE(asn_DEF_DENM,denm1);
  }

  /* This function is used to calculate the delay for packet reception */
  double
  DENMSender::time_diff(double sec1, double usec1, double sec2, double usec2)
    {
            double tot1 , tot2 , diff;
            tot1 = sec1 + (usec1 / 1000000000.0);
            tot2 = sec2 + (usec2 / 1000000000.0);
            diff = tot2 - tot1;
            return diff;
    }

  struct timespec
  DENMSender::compute_timestamp ()
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

} // Namespace ns3

