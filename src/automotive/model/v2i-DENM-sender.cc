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

#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/uinteger.h"
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <sys/time.h>
#include <sys/types.h>
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
    m_this_id = 0;
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

        if (decoded->header.messageID == FIX_CAMID)
          {
            /* Now in "decoded" you have the CAM */
            /* Build your CAM strategy here! */
            cam.position.x = (double)decoded->cam.camParameters.basicContainer.referencePosition.longitude;
            cam.position.y = (double)decoded->cam.camParameters.basicContainer.referencePosition.latitude;
            cam.speed = (double)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue;

            m_cam_received++;
            ASN_STRUCT_FREE(asn_DEF_CAM,decoded);
          }
        else
          {
            /* What you received is not a CAM, clean and exit */
            ASN_STRUCT_FREE(asn_DEF_CAM,decoded);
            return;
          }
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

        if(values[0]=="CAM")
          {
            /* Build your CAM strategy here! */
            cam.position.x = std::stod(values[2]);
            cam.position.y = std::stod(values[3]);
            cam.speed = std::stod(values[4]);
            m_cam_received++;
          }
        else
          {
            /* What you received is not a CAM, exit */
            return;
          }
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

    long timestamp;
    if(m_real_time)
      {
        timestamp = compute_timestampIts ()%65536;
      }
    else
      {
        struct timespec tv = compute_timestamp ();
        timestamp = (tv.tv_nsec/1000000)%65536;
      }

    if (decision == SEND_0)
      {
        if (m_asn)
          DENMSender::Populate_and_send_asn_denm (from,0,timestamp);
        else
          DENMSender::Populate_and_send_normal_denm (from,0);
      }
    else if (decision == SEND_1)
      {
        if (m_asn)
          DENMSender::Populate_and_send_asn_denm (from,1,timestamp);
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

    msg << "DENM,"
        << tv.tv_sec  << ","
        << tv.tv_nsec << ","
        << speedmode << ",end\0";

    //Tweak: add +1, otherwise some random characters are received at the end of the packet
    uint16_t packetSize = msg.str ().length () + 1;
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) msg.str ().c_str (), packetSize);

    m_socket->SendTo (packet,2,address);
    m_denm_sent++;
  }

  void
  DENMSender::Populate_and_send_asn_denm(Address address, int speedmode, long det_time)
  {
    /* Here a ASN.1 DENM is encoded, following ETSI EN 302 637-3, ETSI EN 302 637-2 and ETSI TS 102 894-2 encoding rules
     * in square brakets the unit used to transfer the data */

    DENM_t *denm = (DENM_t*) calloc(1, sizeof(DENM_t));

    /* Header */
    denm->header.protocolVersion=FIX_PROT_VERS;
    denm->header.stationID=m_this_id;
    denm->header.messageID=FIX_DENMID;

    /* Management container */
    /* The actionID shall be the combination of an ITS-S ID and a sequence number. The ITS-S ID corresponds to stationID
     * of the originating ITS-S that detects an event for the first time. The sequence number is assigned to the actionID
     * for each new DENM. In our case we use the sequencenumber field to encode the speedmode. */
    denm->denm.management.actionID.sequenceNumber=speedmode;
    denm->denm.management.actionID.originatingStationID=m_this_id;

    /* Detection time [ms since 2004-01-01] (time at which the event is detected). */
    INTEGER_t detection_time;
    memset(&detection_time, 0, sizeof(detection_time));
    asn_long2INTEGER(&detection_time, det_time);
    denm->denm.management.detectionTime=detection_time;

    /* Reference time [ms since 2004-01-01] (time at wich the DENM is generated). In case the scheduler is not real time,
     * we have to use simulation time, otherwise timestamps will be not reliable */
    long timestamp;
    if(m_real_time)
      {
        timestamp = compute_timestampIts ()%65536;
      }
    else
      {
        struct timespec tv = compute_timestamp ();
        timestamp = (tv.tv_nsec/1000000)%65536;
      }
    INTEGER_t ref_time;
    memset(&ref_time, 0, sizeof(ref_time));
    asn_long2INTEGER(&ref_time, timestamp);
    denm->denm.management.referenceTime=ref_time;

    /* Station Type */
    denm->denm.management.stationType=StationType_roadSideUnit;

    /** Encoding **/
    void *buffer1 = NULL;
    asn_per_constraints_s *constraints = NULL;
    ssize_t ec = uper_encode_to_new_buffer(&asn_DEF_DENM, constraints, denm, &buffer1);
    if (ec==-1)
      {
        std::cout << "Cannot encode DENM" << std::endl;
        return;
      }
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) buffer1, ec+1);
    m_socket->SendTo (packet,2,address);

    m_denm_sent++;
    ASN_STRUCT_FREE(asn_DEF_DENM,denm);
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

  long
  DENMSender::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }

} // Namespace ns3

