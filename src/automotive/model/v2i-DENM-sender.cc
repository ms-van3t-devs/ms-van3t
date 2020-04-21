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
    m_sequence = 0;
    m_actionId = 0;
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
     }

  void
  DENMSender::HandleRead (Ptr<Socket> socket)
  {
    /* Debug print */
    //NS_LOG_INFO("SUMO time:" << m_client->simulation.getTime () << "\n");
    //NS_LOG_INFO("NS3 time:" << Simulator::Now () << "\n");
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    packet = socket->RecvFrom (from);
    uint8_t *buffer = new uint8_t[packet->GetSize ()-1];
    packet->CopyData (buffer, packet->GetSize ()-1);

    if (m_asn)
      DENMSender::Decode_asn_cam(buffer,packet->GetSize ()-1,from);
    else
      DENMSender::Decode_normal_cam(buffer,from);
  }

  int DENMSender::SendDenm(den_data_t denm, Address address)
  {
    if (m_asn)
      return DENMSender::Populate_and_send_asn_denm(denm, address);
    else
      return DENMSender::Populate_and_send_normal_denm(denm, address);
  }

  int
  DENMSender::Populate_and_send_normal_denm(den_data_t denm, Address address)
  {
    // Generate the packet
    std::ostringstream msg;

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

//    Overflow problem - FIX IT
    /* If validity is expired return 0 */
//    if ((timestamp - denm.detectiontime) > (denm.validity)*1000)
//      return 0;

    m_sequence++;
    m_actionId++;

    msg << "DENM,"
        << denm.detectiontime << ","
        << timestamp << ","
        << denm.stationid << ","
        << m_sequence << ",end\0";

    //Tweak: add +1, otherwise some random characters are received at the end of the packet
    uint16_t packetSize = msg.str ().length () + 1;
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) msg.str ().c_str (), packetSize);

    m_socket->SendTo (packet,2,address);
    return m_actionId;
  }

  int
  DENMSender::Populate_and_send_asn_denm(den_data_t data, Address address)
  {
    /* Here a ASN.1 DENM is encoded, following ETSI EN 302 637-3, ETSI EN 302 637-2 and ETSI TS 102 894-2 encoding rules
     * in square brakets the unit used to transfer the data */

    /* First you have to check if the DENM validity has expired */
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

//    Overflow problem - FIX IT
//    /* If validity is expired return 0 */
//    if ((timestamp - data.detectiontime) > (data.validity)*1000)
//        return 0;

    m_sequence++;
    m_actionId++;

    DENM_t *denm = (DENM_t*) calloc(1, sizeof(DENM_t));

    /* Header */
    denm->header.messageID = data.messageid;
    denm->header.stationID = data.stationid;
    denm->header.protocolVersion = data.proto;

    /* Management container */
    /* Set actionID and sequence */
    /* The actionID shall be the combination of an ITS-S ID and a sequence number. The ITS-S ID corresponds to stationID
     * of the originating ITS-S that detects an event for the first time. The sequence number is assigned to the actionID
     * for each new DENM. In our case we use the sequencenumber field to encode the speedmode. */

    denm->denm.management.actionID.sequenceNumber = m_sequence;
    denm->denm.management.actionID.originatingStationID = m_actionId;

    /* Detection time [ms since 2004-01-01] (time at which the event is detected). */
    INTEGER_t detection_time;
    memset(&detection_time, 0, sizeof(detection_time));
    asn_long2INTEGER(&detection_time, data.detectiontime);
    denm->denm.management.detectionTime=detection_time;

    /* Reference time [ms since 2004-01-01] (time at wich the DENM is generated). In case the scheduler is not real time,
     * we have to use simulation time, otherwise timestamps will be not reliable */
    INTEGER_t ref_time;
    memset(&ref_time, 0, sizeof(ref_time));
    asn_long2INTEGER(&ref_time, timestamp);
    denm->denm.management.referenceTime = ref_time;

    /* Station Type */
    denm->denm.management.stationType = data.stationtype;

    //[tbr]
    denm->denm.management.eventPosition.latitude=data.evpos_lat;

    /** Encoding **/
    void *buffer1 = NULL;
    asn_per_constraints_s *constraints = NULL;
    ssize_t ec = uper_encode_to_new_buffer(&asn_DEF_DENM, constraints, denm, &buffer1);
    if (ec==-1)
      {
        std::cout << "Cannot encode DENM" << std::endl;
        return 0;
      }
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) buffer1, ec+1);
    m_socket->SendTo (packet,2,address);

    ASN_STRUCT_FREE(asn_DEF_DENM,denm);
    return m_actionId;
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

  struct timespec
  DENMSender::compute_timestamp ()
  {
    struct timespec tv;
    if (true)
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

  void
  DENMSender::Decode_asn_cam(uint8_t *buffer, uint32_t size, Address address)
  {
    /** Decoding **/
    void *decoded_=NULL;
    asn_dec_rval_t rval;
    ca_data_t cam;

    rval = uper_decode(0, &asn_DEF_CAM, &decoded_, buffer, size, 0, 1);

    if (rval.code == RC_FAIL)
      {
        std::cout << "CAM ASN.1 decoding failed!"<< std::endl;
        return;
      }

    CAM_t *decoded = (CAM_t *) decoded_;

    if (decoded->header.messageID == FIX_CAMID)
      {
        /* Now in "decoded" you have the CAM */
        /* Build your CAM strategy here! */
        cam.longitude = (long)decoded->cam.camParameters.basicContainer.referencePosition.longitude;
        cam.latitude = (long)decoded->cam.camParameters.basicContainer.referencePosition.latitude;
        cam.altitude_conf = (long)decoded->cam.camParameters.basicContainer.referencePosition.altitude.altitudeConfidence;
        cam.altitude_value = (long)decoded->cam.camParameters.basicContainer.referencePosition.altitude.altitudeValue;
        cam.heading_conf = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingConfidence;
        cam.heading_value = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue;
        cam.id = (long)decoded->header.stationID;
        cam.length_conf = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthConfidenceIndication;
        cam.length_value = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthValue;
        cam.width = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth;
        cam.longAcc_conf = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationConfidence;
        cam.longAcc_value = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationValue;
        cam.messageid = (int)decoded->header.messageID;
        cam.proto = (int)decoded->header.protocolVersion;
        cam.speed_conf = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedConfidence;
        cam.speed_value = (long)decoded->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue;
        cam.timestamp = (long)decoded->cam.generationDeltaTime;
        cam.type = (long)decoded->cam.camParameters.basicContainer.stationType;

        Ptr<appServer> app = GetNode()->GetApplication (1)->GetObject<appServer> ();
        app->receiveCAM (cam,address);
      }

    ASN_STRUCT_FREE(asn_DEF_CAM,decoded);
  }

  void
  DENMSender::Decode_normal_cam(uint8_t *buffer, Address address)
  {
    ca_data_t cam;
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
        cam.messageid = FIX_CAMID;
        cam.id = std::stol(values[1]);
        cam.latitude = std::stol(values[2]);
        cam.longitude = std::stol(values[3]);
        cam.altitude_value = std::stol(values[4]);
        cam.speed_value = std::stol(values[5]);
        cam.longAcc_value = std::stol(values[6]);
        cam.heading_value = std::stol(values[7]);
        cam.timestamp = std::stol(values[8]);

        Ptr<appServer> app = GetNode()->GetApplication (1)->GetObject<appServer> ();
        app->receiveCAM (cam,address);
      }
  }



} // Namespace ns3

