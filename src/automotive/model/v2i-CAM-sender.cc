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
        .AddAttribute ("ServerAddr",
            "Ip Addr of the server",
            Ipv4AddressValue("10.0.0.2"),
            MakeIpv4AddressAccessor (&CAMSender::m_server_addr),
            MakeIpv4AddressChecker ())
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
    // Create the UDP socket for the client
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);

    // Bind the socket to receive packets coming from every IP
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
    if (m_socket->Bind (local) == -1)
      {
        NS_FATAL_ERROR ("Failed to bind client socket");
      }

    // Connect it to the server
    InetSocketAddress remote = InetSocketAddress (m_server_addr, m_port);
    m_socket->Connect(remote);

    // Make the callback to handle received packets
    m_socket->SetRecvCallback (MakeCallback (&CAMSender::HandleRead, this));
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
  }

  void
  CAMSender::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  CAMSender::SendCam(ca_data_t cam)
  {
    if (m_asn)
      CAMSender::Populate_and_send_asn_cam(cam);
    else
      CAMSender::Populate_and_send_normal_cam(cam);
  }

  void
  CAMSender::Populate_and_send_normal_cam(ca_data_t cam)
  {
    std::ostringstream msg;

    /* Create the message to be sent in plain text */
    msg << "CAM," << cam.id << ","
        << cam.longitude << ","
        << cam.latitude << ","
        << cam.altitude_value << ","
        << cam.speed_value << ","
        << cam.longAcc_value << ","
        << cam.heading_value << ","
        << cam.timestamp << ",end\0";

    // Tweak: add +1, otherwise some strange character are received at the end of the packet
    uint16_t packetSize = msg.str ().length () + 1;
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) msg.str ().c_str (), packetSize);

    // Send packet through the interface
    m_socket->Send(packet);
  }

  void
  CAMSender::Populate_and_send_asn_cam(ca_data_t data)
  {
    /* Here a ASN.1 CAM is encoded, following ETSI EN 302 637-3, ETSI EN 302 637-2 and ETSI TS 102 894-2 encoding rules
     * in square brakets the unit used to transfer the data */

    CAM_t *cam = (CAM_t*) calloc(1, sizeof(CAM_t));

    /* Install the high freq container */
    cam->cam.camParameters.highFrequencyContainer.present = HighFrequencyContainer_PR_basicVehicleContainerHighFrequency;

    /* Generation delta time [ms since 2004-01-01]. In case the scheduler is not real time, we have to use simulation time,
     * otherwise timestamps will be not reliable */
    cam->cam.generationDeltaTime = (GenerationDeltaTime_t)data.timestamp;

    /* Station Type */
    cam->cam.camParameters.basicContainer.stationType = (StationType_t)data.type;

    /* Positions */
    //altitude [0,01 m]
    cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeConfidence = (AltitudeConfidence_t)data.altitude_conf;
    cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeValue = (AltitudeValue_t)data.altitude_value;

    //latitude WGS84 [0,1 microdegree]
    cam->cam.camParameters.basicContainer.referencePosition.latitude = (Latitude_t) data.latitude;

    //longitude WGS84 [0,1 microdegree]
    cam->cam.camParameters.basicContainer.referencePosition.longitude = (Longitude_t) data.longitude;

    /* Heading WGS84 north [0.1 degree] */
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingConfidence = (HeadingConfidence_t) data.heading_conf;
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue = (HeadingValue_t) data.heading_value;

    /* Speed [0.01 m/s] */
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedConfidence = (SpeedConfidence_t) data.speed_conf;
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue = (SpeedValue_t) data.speed_value;

    /* Acceleration [0.1 m/s^2] */
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationConfidence = (AccelerationConfidence_t) data.longAcc_conf;
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationValue = (LongitudinalAccelerationValue_t) data.longAcc_value;

    /* Length and width of car [0.1 m] */
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthConfidenceIndication = (VehicleLengthConfidenceIndication_t) data.length_conf;
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthValue = (VehicleLengthValue_t) data.length_value;
    cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth = (VehicleWidth_t) data.width;

    /* Other needed fields */
    cam->header.protocolVersion = data.proto;
    cam->header.stationID = (StationID_t) data.id;
    cam->header.messageID = data.messageid;

    /* We filled just some fields, it is possible to fill them all to match any purpose */

    /** Encoding **/
    void *buffer = NULL;
    asn_per_constraints_s *constraints = NULL;
    ssize_t ec = uper_encode_to_new_buffer(&asn_DEF_CAM, constraints, cam, &buffer);
    if (ec==-1)
      {
        std::cout << "Cannot encode CAM." << std::endl;
        return;
      }

    /** Packet creation **/
    Ptr<Packet> packet = Create<Packet> ((uint8_t*) buffer, ec+1);

    m_socket->Send (packet);

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

    if (m_asn)
      CAMSender::Decode_asn_denm(buffer,packet->GetSize ()-1);
    else
      CAMSender::Decode_normal_denm(buffer);
  }

  void
  CAMSender::Decode_asn_denm(uint8_t *buffer,uint32_t size)
  {
    /** Decoding **/
    void *decoded_=NULL;
    asn_dec_rval_t rval;
    den_data_t denm;

    rval = uper_decode(0, &asn_DEF_DENM, &decoded_, buffer, size, 0, 1);

    if (rval.code == RC_FAIL)
      {
        std::cout << "DENM ASN.1 decoding failed!" << std::endl;
        return;
      }

    DENM_t *decoded = (DENM_t *) decoded_;

    if (decoded->header.messageID==FIX_DENMID)
      {
        denm.proto = (int)decoded->header.protocolVersion;
        denm.stationid = (long)decoded->header.stationID;
        denm.messageid = (int)decoded->header.messageID;

        denm.sequence = (int)decoded->denm.management.actionID.sequenceNumber;
        denm.actionid = (long)decoded->denm.management.actionID.originatingStationID;

        long detection_time;
        memset(&detection_time, 0, sizeof(detection_time));
        asn_INTEGER2long (&decoded->denm.management.detectionTime,&detection_time);
        denm.detectiontime =detection_time;

        long ref_time;
        memset(&ref_time, 0, sizeof(ref_time));
        asn_INTEGER2long (&decoded->denm.management.referenceTime,&ref_time);
        denm.referencetime = ref_time;

        denm.stationtype = (long)decoded->denm.management.stationType;

        //[tbr]
        denm.evpos_lat = (long)decoded->denm.management.eventPosition.latitude;

        Ptr<appClient> app = GetNode()->GetApplication (1)->GetObject<appClient> ();
        app->receiveDENM (denm);
      }

    ASN_STRUCT_FREE(asn_DEF_DENM,decoded);
  }

  void
  CAMSender::Decode_normal_denm(uint8_t *buffer)
  {
    den_data_t denm;
    std::vector<std::string> values;
    std::string s = std::string ((char*) buffer);
    //std::cout << "Packet received - content:" << s << std::endl;
    std::stringstream ss(s);
    std::string element;
    while (std::getline(ss, element, ',')) {
        values.push_back (element);
      }

    if(values[0]=="DENM")
      {
        denm.messageid = FIX_DENMID;
        denm.detectiontime =  std::stol(values[1]);
        denm.referencetime = std::stol(values[2]);
        denm.stationid = std::stoi(values[3]);
        denm.sequence = std::stoi(values[4]);

        Ptr<appClient> app = GetNode()->GetApplication (1)->GetObject<appClient> ();
        app->receiveDENM (denm);
      }
  }
}





