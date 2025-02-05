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
 * Diego Gasco, Politecnico di Torino (diego.gasco@polito.it, diego.gasco99@gmail.com)
*/
#include "simpleVAMSender-gps-tc.h"

#include "ns3/VAM.h"
#include "ns3/vrudpGPSTraceClient.h"
#include "ns3/vdpTraci.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/gn-utils.h"


namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("simpleVAMSender");

  NS_OBJECT_ENSURE_REGISTERED(simpleVAMSender);

  TypeId
  simpleVAMSender::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::simpleVAMSender")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<simpleVAMSender> ()
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&simpleVAMSender::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("GPSClient",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&simpleVAMSender::m_gps_tc_client),
            MakePointerChecker<GPSTraceClient> ());
        return tid;
  }

  simpleVAMSender::simpleVAMSender ()
  {
    NS_LOG_FUNCTION(this);
    m_gps_tc_client = nullptr;

    m_vam_sent = 0;
  }

  simpleVAMSender::~simpleVAMSender ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  simpleVAMSender::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  simpleVAMSender::StartApplication (void)
  {

    /*
     * This application connects with the gps-tc module and simply generates CAM.
     * In this case, the position of the vehicle will arrive from a GPS trace placed
     * in src/gps-tc/examples/GPS-Traces-Sample, NOT FROM SUMO!
     */

    NS_LOG_FUNCTION(this);

    // Ensure that the mobility client has been set
    if(m_gps_tc_client==nullptr)
    {
        NS_FATAL_ERROR("No mobility client specified in simpleCAMSender");
    }
    m_id = m_gps_tc_client->getID();

    // Create new BTP and GeoNet objects and set them in VRUBasicService
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();
    m_btp->setGeoNet(m_geoNet);
    m_vruService.setBTP(m_btp);

    /* Create the socket for TX and RX */
    TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);

    /* TX socket for CAMs */
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

    m_vruService.setStationProperties (std::stol(m_id), StationType_pedestrian);

    /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
    m_vruService.setSocketTx (m_socket);
    m_vruService.setSocketRx (m_socket);
    m_vruService.addVAMRxCallback (std::bind(&simpleVAMSender::receiveVAM,this,std::placeholders::_1,std::placeholders::_2));

    // TODO Create a VRUdpGPSTraceClient object and set it in the VRU Service
    VRUdp* gpstc_vdp = new VRUDPGPSTraceClient(m_gps_tc_client, m_id);
    m_vruService.setVRUdp(gpstc_vdp);

    m_vruService.SetLogTriggering(true, "vam-log.csv");

    /* Schedule VAM dissemination */
    std::srand(Simulator::Now().GetNanoSeconds ());
    double desync = ((double)std::rand()/RAND_MAX);
    m_vruService.startVamDissemination(desync);
  }

  void
  simpleVAMSender::StopApplication ()
  {
    NS_LOG_FUNCTION(this);

    uint64_t cam_sent;

    m_gps_tc_client->StopUpdates();

    cam_sent = m_vruService.terminateDissemination ();

    std::cout << "VRU " << m_id
              << " has sent " << cam_sent
              << " VAMs" << std::endl;
  }

  void
  simpleVAMSender::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  simpleVAMSender::receiveVAM (asn1cpp::Seq<VAM> vam, Address from)
  {
    /* Implement VAM strategy here */
    std::cout <<"VRU ID: " << m_id
            <<" | Rx VAM from "<<vam->header.stationId
            <<" | Remote VRU position: ("<<asn1cpp::getField(vam->vam.vamParameters.basicContainer.referencePosition.latitude,double)/DOT_ONE_MICRO<<","
            <<asn1cpp::getField(vam->vam.vamParameters.basicContainer.referencePosition.longitude,double)/DOT_ONE_MICRO<<")"<<std::endl;

  }
}



