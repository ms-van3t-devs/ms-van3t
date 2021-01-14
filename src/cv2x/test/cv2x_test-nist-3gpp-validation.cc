/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.

 * We would appreciate acknowledgement if the software is used.

 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 *
 * Modified by: NIST
 */

#include "ns3/cv2x_lte-helper.h"
#include "ns3/cv2x_epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/cv2x-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/cv2x_point-to-point-epc-helper.h"
#include "ns3/config-store.h"
#include "ns3/cv2x_lte-hex-grid-enb-topology-helper.h"
#include <ns3/buildings-helper.h>
#include <ns3/cv2x_3gpp-propagation-loss-model.h>
#include <ns3/constant-position-mobility-model.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("cv2x_TestNist3gppValidation");

class cv2x_SaveTraces : public Object
{
public:
    cv2x_SaveTraces (std::string path);
    ~cv2x_SaveTraces ();
    void GetGainTrace (std::string context, Ptr<MobilityModel> tx,Ptr<MobilityModel> rx, double txAntennaGain, double rxAntennaGain, double propagationGain, double pathloss);
private:
   std::ofstream m_outFile;   
};

cv2x_SaveTraces::cv2x_SaveTraces (std::string path)
{
  if (std::ifstream (path.c_str()))
  {
    remove (path.c_str ());
  }
  m_outFile.open (path.c_str (), std::ofstream::out | std::ofstream::app);
  if (!m_outFile.is_open ())
  {
    NS_FATAL_ERROR ("Can't open output file");
    return;
  }
  m_outFile << "time\ttransmitter\treceiver\ttxAntennaGain\trxAntennaGain\tpropagationGain\tpathloss" << std::endl;
}
cv2x_SaveTraces::~cv2x_SaveTraces ()
{
  m_outFile.close ();
}
void 
cv2x_SaveTraces::GetGainTrace  (std::string context, Ptr<MobilityModel> tx,Ptr<MobilityModel> rx, double txAntennaGain, double rxAntennaGain, double propagationGain, double pathloss)
{
  m_outFile << Simulator::Now ().GetSeconds () << "\t" ;
  m_outFile << tx->GetObject<Node> ()->GetId () << "\t";
  m_outFile << rx->GetObject<Node> ()->GetId () << "\t";
  m_outFile << txAntennaGain << "\t";
  m_outFile << rxAntennaGain << "\t";
  m_outFile << propagationGain << "\t"; 
  m_outFile << pathloss << std::endl;
}


int
main (int argc, char *argv[])
{
  if (std::ifstream ("traffic.txt"))
  {
    remove ("traffic.txt");
  }

  // Set the run number of simulation and the value of the seed
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);  

  // Set the frequency to use the Public Safety case (band 14 : 700 MHz)
  // 3 MHz --> 15
  // 5 MHz --> 25
  // 10 MHz --> 50
  // 15 MHz --> 75
  // 20 MHz --> 100
  Config::SetDefault ("ns3::cv2x_LteEnbNetDevice::DlEarfcn", StringValue ("5330"));
  Config::SetDefault ("ns3::cv2x_LteEnbNetDevice::UlEarfcn", StringValue ("23330"));
  Config::SetDefault ("ns3::cv2x_LteEnbNetDevice::DlBandwidth", StringValue ("50"));
  Config::SetDefault ("ns3::cv2x_LteEnbNetDevice::UlBandwidth", StringValue ("50"));
  Config::SetDefault ("ns3::cv2x_LteUeNetDevice::DlEarfcn", StringValue ("5330"));

  // Set the UEs power in dBm 
  Config::SetDefault ("ns3::cv2x_LteUePhy::TxPower", DoubleValue (31.0));
  // Set the eNBs power in dBm 
  Config::SetDefault ("ns3::cv2x_LteEnbPhy::TxPower", DoubleValue (46.0));
  // Set the SRS periodicity in ms (increase the capability of handling more UEs per cell)
  // Allowed values : 2, 5, 10, 20, 40, 80, 160 and 320
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::SrsPeriodicity", UintegerValue (80));

  /*
  // Change the default HO algorithm (NoOpHandover) to the strongest cell HO algorithm
  Config::SetDefault ("ns3::cv2x_LteHelper::HandoverAlgorithm", StringValue ("ns3::cv2x_A3RsrpHandoverAlgorithm"));
  Config::SetDefault ("ns3::cv2x_A3RsrpHandoverAlgorithm::Hysteresis", DoubleValue (1.0));
  */

  // Configure the PO parameter of the uplink power control
  // no need to change alpha, because the default value matches the one specified in 3GPP
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::PoNominalPusch", IntegerValue (-106));

  // Configure the MIMO Spatial Multiplexing (2) 
  // the default model is SISO (0), and we have the MIMO Transmit Diversity (1)
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::DefaultTransmissionMode", UintegerValue (2));

  // Initialize some values
  uint32_t nbBldgs = 1;
  uint32_t nbRings = 1;
  uint32_t nbUesPerSector = 10;
  double simTime = 10;
  double intPack = 0.1;
  uint32_t maxPack = 1000;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("ring", "Number of rings in hexagon", nbRings);
  cmd.AddValue ("building", "Number of buildings in simulation", nbBldgs);
  cmd.AddValue ("time", "Simulation time", simTime);
  cmd.AddValue ("max", "maximum number of packets per UE", maxPack);
  cmd.AddValue ("int", "interval between packets", intPack);
  cmd.Parse(argc, argv);

  // Create the helpers
  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();

  Ptr<cv2x_PointToPointEpcHelper>  epcHelper = CreateObject<cv2x_PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
 
  Ptr<cv2x_LteHexGridEnbTopologyHelper> topoHelper = CreateObject<cv2x_LteHexGridEnbTopologyHelper> ();
  topoHelper->SetLteHelper (lteHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // Configure antenna parameters not handled by the topology helper
  lteHelper->SetEnbAntennaModelType ("ns3::cv2x_NistParabolic3dAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute ("ElectricalTilt", DoubleValue (15)); 
  topoHelper->SetNbRings (nbRings);
  topoHelper->SetMinimumDistance (10);

  // Set pathloss model
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::cv2x_NistUrbanmacrocellPropagationLossModel"));
  //lteHelper->SetPathlossModelType ("ns3::cv2x_NistUrbanmacrocellPropagationLossModel");

  // Choose the appropriate scheduler
  // In one hand, the default scheduler for the downlink is Proportional Fair (PF),
  // so we will have to set the downlink schedular to Round Robin 
  // It is 3GPP requirements for D2D communications
  // In the other hand, the default schedular for the uplink is Round Robin (RR)
  lteHelper->SetSchedulerType ("ns3::cv2x_RrFfMacScheduler");

  // Create nodes (eNbs + UEs)
  NodeContainer threeSectorNodes;
  threeSectorNodes.Create (topoHelper->GetNumberOfNodes());
  NodeContainer ues;
  ues.Create (nbUesPerSector * threeSectorNodes.GetN());

  // Define buildings
  for (uint32_t i = 0; i != nbBldgs; ++i)
  {
    Ptr<Building> building = CreateObject<Building> ();
    uint32_t xmin = -20;
    uint32_t xmax = 20;
    uint32_t ymin = 10;
    uint32_t ymax = -10;
    uint32_t zmin = 0;
    uint32_t zmax = 20;
    building->SetBoundaries (Box (xmin, xmax, ymin, ymax, zmin, zmax));
    building->SetBuildingType (Building::Residential);
    building->SetExtWallsType (Building::ConcreteWithWindows);
    building->SetNFloors (3);
  }

  // Install mobility
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (threeSectorNodes);
  mobility.Install (ues);

  BuildingsHelper::Install (threeSectorNodes);
  BuildingsHelper::Install (ues);
  BuildingsHelper::MakeMobilityModelConsistent ();

  // Compute the position of each site and antenna orientation
  NetDeviceContainer enbDevs = topoHelper->SetPositionAndInstallEnbDevice(threeSectorNodes);

  // Deploy the UEs uniformly (and equally among the cells)
  NetDeviceContainer ueDevs = topoHelper->DropUEsUniformlyPerSector(ues);
  
  // Add X2 inteface
  lteHelper->AddX2Interface (threeSectorNodes);
 
  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // Install the IP stack on the UEs
  internet.Install (ues);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
 
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ues.GetN (); ++u)
  {
    Ptr<Node> ueNode = ues.Get (u);
    // Set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

  // Attach each UE to the best available eNB
  lteHelper->Attach (ueDevs);
  //lteHelper->AttachToClosestEnb (ueDevs, enbDevs);

  ////////////////////////////////////////
  // Save the nodes IDs and positions (file name : nodesId.txt)
  std::ofstream m_outFile;
  if (std::ifstream ("nodesId.txt"))
  {
    remove ("nodesId.txt");
  }
  
  m_outFile.open ("nodesId.txt", std::ofstream::out | std::ofstream::app);
  if (!m_outFile.is_open ())
  {
    NS_FATAL_ERROR ("Can't open output file");
  }

  m_outFile << "eNBs Id = [";
  for (uint32_t i = 0; i < threeSectorNodes.GetN () - 1; ++i)
  {
    m_outFile << threeSectorNodes.Get (i)->GetId () << " , ";
  }
  m_outFile << threeSectorNodes.Get (threeSectorNodes.GetN () - 1)->GetId () << "]" << std::endl;
   
  m_outFile << "UEs Id = [";
  for (uint32_t i = 0; i < ues.GetN () - 1; ++i)
  {
    m_outFile << ues.Get (i)->GetId () << " (" << ueIpIface.GetAddress (i,0) << ") , ";
  }
  m_outFile << ues.Get (ues.GetN () - 1)->GetId () << " (" << ueIpIface.GetAddress (ues.GetN () - 1,0) << ")]" << std::endl;


  m_outFile << "eNBs and UEs Positions ID X Y " << std::endl;
  for (uint32_t i = 0; i < threeSectorNodes.GetN (); ++i)
  {
    m_outFile << "position eNB " << i+1 << " " << threeSectorNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().x << " " << threeSectorNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().y << std::endl;
  }  
  for (uint32_t i = 0; i < ues.GetN (); ++i)
  {
    m_outFile << "position UE " << i+1 << " " << ues.Get (i)->GetObject<MobilityModel> ()->GetPosition ().x << " " << ues.Get (i)->GetObject<MobilityModel> ()->GetPosition ().y << std::endl;
  }
  m_outFile.close ();
  //////////////////////////////////////////

  // Set Application in Server
  UdpEchoServerHelper echoServer (8000);
  ApplicationContainer serverApps = echoServer.Install (remoteHostContainer.Get (0));
  //ApplicationContainer serverApps = echoServer.Install (ues);
  serverApps.Start (Seconds (1.5));
  serverApps.Stop (Seconds (100.0));

  // Set Application in the UE
  UdpEchoClientHelper echoClient (remoteHostAddr, 8000); 
  echoClient.SetAttribute ("MaxPackets", UintegerValue (maxPack));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (intPack)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1280));

  ApplicationContainer clientApps = echoClient.Install (ues); 
  //ApplicationContainer clientApps = echoClient.Install (remoteHostContainer.Get (0)); 
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (50.0));

  // Enable traces
  lteHelper->EnableTraces();
  
  // Enable PCAP traces
  //p2ph.EnablePcapAll("test-validate");

  Simulator::Stop(Seconds(simTime));

  // Gain traces
  // Collect total pathloss (Prx - Ptx = txAntennaGain + rxAntennaGain + Pathloss + Shadowing)
  // in two different files : one the UL and another for the DL
  cv2x_SaveTraces dlFileTrace ("dl-gain.out");
  cv2x_SaveTraces ulFileTrace ("ul-gain.out");
  Config::Connect ("/ChannelList/0/Gain", MakeCallback (&cv2x_SaveTraces::GetGainTrace, &dlFileTrace));
  Config::Connect ("/ChannelList/1/Gain", MakeCallback (&cv2x_SaveTraces::GetGainTrace, &ulFileTrace));

  Simulator::Run();
  Simulator::Destroy();
  return 0;

}

