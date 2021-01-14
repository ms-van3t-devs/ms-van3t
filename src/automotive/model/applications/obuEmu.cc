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
#include "obuEmu.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/vdpTraci.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("obuEmu");

  NS_OBJECT_ENSURE_REGISTERED(obuEmu);

  TypeId
  obuEmu::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::obuEmu")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<obuEmu> ()
        .AddAttribute ("SendCam",
            "If it is true, the branch sending the CAM is activated.",
            BooleanValue (true),
            MakeBooleanAccessor (&obuEmu::m_send_cam),
            MakeBooleanChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&obuEmu::m_client),
            MakePointerChecker<TraciClient> ());
        return tid;
  }

  obuEmu::obuEmu ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
  }

  obuEmu::~obuEmu ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  obuEmu::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  obuEmu::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);
    m_id = m_client->GetVehicleId (this->GetNode ());

    /* Create the socket for TX and RX (must be a PacketSocket in order to use GeoNet instead of IP) */
    TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");

    /* Socket used to send CAMs and receive DENMs */
    m_socket = Socket::CreateSocket (GetNode (), tid);

    /* Bind the socket to local address */
    PacketSocketAddress local;
    local.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
    local.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetAddress ());
    local.SetProtocol (0x8947);
    if (m_socket->Bind (local) == -1)
      {
        NS_FATAL_ERROR ("Failed to bind client socket");
      }

    /* Connect it to the server */
    // Set the socket to broadcast
    PacketSocketAddress remote;
    remote.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
    remote.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetBroadcast ());
    remote.SetProtocol (0x8947);

    m_socket->Connect(remote);

    //Create new btp and geoNet objects and set them in DENBasicService and CABasicService
    //m_btp = CreateObject <btp>();
    //m_geoNet = CreateObject <GeoNet>();
    //m_btp->setGeoNet(m_geoNet);
    //m_denService.setBTP(m_btp);
    //m_caService.setBTP(m_btp);

    /* Make the callback to handle received DENMs */
    //m_denService.setSocketRx (m_socket);
    m_socket->SetRecvCallback (MakeCallback (&DENBasicService::receiveDENM, &m_denService));

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);
    m_denService.addDENRxCallback (std::bind(&obuEmu::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));

    /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
    m_caService.setSocketTx (m_socket);
    m_caService.setStationProperties (std::stol(m_id.substr (3)), StationType_passengerCar);

    /* Set TraCI vdp for GeoNet object */
    VDPTraCI traci_vdp(m_client,m_id);
    m_caService.setVDP(traci_vdp);

    /* Schedule CAM dissemination */
    std::srand(Simulator::Now().GetNanoSeconds ());
    if (m_send_cam)
      {
         double desync = ((double)std::rand()/RAND_MAX);
         m_caService.startCamDissemination(desync);
      }
  }

  void
  obuEmu::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Remove(m_sendCamEvent);

    uint64_t cam_sent;

    cam_sent = m_caService.terminateDissemination ();

    std::cout<<"Number of CAMs sent for vehicle " <<m_id<< ": "<<cam_sent<<std::endl;;
    m_denService.cleanup();
  }

  void
  obuEmu::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  /* This is just a sample dummy receiveDENM function. The user can customize it to parse the content of a DENM when it is received. */
  void
  obuEmu::receiveDENM (denData denm, Address from)
  {
    (void) denm; // Contains the data received from the DENM
    (void) from; // Contains the address from which the DENM has been received
    std::cout<<"Received a new DENM."<<std::endl;
  }
}





