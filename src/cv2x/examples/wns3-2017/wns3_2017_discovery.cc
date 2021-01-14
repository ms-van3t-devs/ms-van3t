/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 This software was developed at the National Institute of Standards and
 Technology by employees of the Federal Government in the course of
 their official duties. Pursuant to titleElement 17 Section 105 of the United
 States Code this software is not subject to copyright protection and
 is in the public domain.
 NIST assumes no responsibility whatsoever for its use by other parties,
 and makes no guarantees, expressed or implied, about its quality,
 reliability, or any other characteristic.

 We would appreciate acknowledgement if the software is used.

 NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 FROM THE USE OF THIS SOFTWARE.
 */

#include "ns3/lte-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/lte-hex-grid-enb-topology-helper.h"
#include <ns3/buildings-helper.h>
#include <ns3/3gpp-propagation-loss-model.h>
#include <ns3/constant-position-mobility-model.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wns32017Discovery");

void SlBearerActivation (Ptr<LteHelper> helper, NetDeviceContainer ues, Ptr<LteSlTft> tft)
{
  helper->ActivateSidelinkBearer (ues, tft);
}
void SlStartDiscovery (Ptr<LteHelper> helper, Ptr<NetDevice> ue, std::list<uint32_t> apps, bool rxtx)
{
  helper->StartDiscovery (ue, apps, rxtx);
}

void SlStopDiscovery (Ptr<LteHelper> helper, Ptr<NetDevice> ue, std::list<uint32_t> apps, bool rxtx)
{
  helper->StopDiscovery (ue, apps, rxtx);
}

void DiscoveryMonitoringTrace (Ptr<OutputStreamWrapper> stream, uint64_t imsi, uint16_t cellId, uint16_t rnti, uint32_t proSeAppCode)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << imsi << "\t" << cellId << "\t" << rnti << "\t" << proSeAppCode << std::endl;
}

void DiscoveryAnnouncementPhyTrace (Ptr<OutputStreamWrapper> stream, std::string imsi, uint16_t cellId, uint16_t rnti, uint32_t proSeAppCode)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << imsi << "\t" << cellId << "\t"  << rnti << "\t" << proSeAppCode << std::endl;
}

void DiscoveryAnnouncementMacTrace (Ptr<OutputStreamWrapper> stream, std::string imsi, uint16_t rnti, uint32_t proSeAppCode)
{
	*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << imsi << "\t" << rnti << "\t" << proSeAppCode << std::endl;
}

int
main (int argc, char *argv[])
{  
  // Set the frequency to use the Public Safety case (band 14 : 700 MHz)
  /*
  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", StringValue ("5330"));
  Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", StringValue ("5330"));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", StringValue ("23330"));
  */
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", StringValue ("50"));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", StringValue ("50"));
  Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue (2));

  // Set the UEs power in dBm
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (23.0));
  //Config::SetDefault ("ns3::LteUePowerControl::Pcmax", DoubleValue (24.0));
  // Set the eNBs power in dBm
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (46.0));
  // Set the SRS periodicity in ms (increase the capability of handling more UEs per cell)
  // Allowed values : 2, 5, 10, 20, 40, 80, 160 and 320
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (80));
    
  
  // Initialize some values
  double simTime = 10;  
  uint32_t nbUes = 10;
  uint32_t nbEnbs = 3;
  uint16_t txProb = 100;
  bool useRecovery = false;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("time", "Simulation time", simTime);
  cmd.AddValue ("ue", "Number of UEs", nbUes);
  cmd.AddValue ("tx", "initial transmission probability", txProb);
  cmd.AddValue ("recovery", "error model and HARQ for D2D Discovery", useRecovery);

  cmd.Parse(argc, argv);

  // Use error model and HARQ for D2D Discovery (recovery process)
  Config::SetDefault ("ns3::LteSpectrumPhy::ErrorModelHarqD2dDiscoveryEnabled", BooleanValue (useRecovery));

  // Create the helpers
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
 

  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  std::cout << "PGW id = " << pgw->GetId() << std::endl;


  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();
  
  //Disable eNBs for out-of-coverage modeling
  lteHelper->DisableNewEnbPhy ();

  // Configure antenna parameters not handled by the topology helper
  lteHelper->SetEnbAntennaModelType ("ns3::Parabolic3dAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute ("MechanicalTilt", DoubleValue (20)); 

  // Set pathloss model
  lteHelper->SetPathlossModelType ("ns3::FriisPropagationLossModel");

  lteHelper->SetAttribute ("UseDiscovery", BooleanValue (true));
  lteHelper->SetSchedulerType ("ns3::RrSlFfMacScheduler");

  // Create nodes (eNbs + UEs)
  NodeContainer threeSectorNodes;
  threeSectorNodes.Create (nbEnbs);
  std::cout << "eNb ids = [" << threeSectorNodes.Get(0)->GetId() <<"," << threeSectorNodes.Get(1)->GetId() << "," << threeSectorNodes.Get(2)->GetId() << "]" << std::endl;
  NodeContainer ues;
  ues.Create (nbUes);
 
  //Position of the nodes
  Ptr<ListPositionAllocator> positionAllocEnb = CreateObject<ListPositionAllocator> ();
  positionAllocEnb->Add (Vector (0.0, 0.0, 30.0));
  Ptr<ListPositionAllocator> positionAllocUe = CreateObject<ListPositionAllocator> ();
        
  AsciiTraceHelper asc;
  Ptr<OutputStreamWrapper> st = asc.CreateFileStream ("discovery_nodes.txt");
  *st->GetStream () << "id\tx\ty" << std::endl;

  for (uint32_t u = 0; u < ues.GetN (); ++u)
  {
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
    double x = rand->GetValue (-100,100);
    double y = rand->GetValue (-100,100);
    double z = 1.5;
    positionAllocUe->Add (Vector(x, y, z));
    uint32_t imsi = ues.Get(u)->GetId() - threeSectorNodes.GetN();
    std::cout << "UE "<< u << " id = " << ues.Get(u)->GetId() << " / imsi = " << imsi << " / position = [" << x << "," << y << "," << z << "]" << std::endl;
    *st->GetStream () << imsi << "\t" << x << "\t" << y << std::endl;
  }

  // Install mobility
  MobilityHelper mobilityeNodeB;
  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.SetPositionAllocator(positionAllocEnb);
  mobilityeNodeB.Install (threeSectorNodes);

  MobilityHelper mobilityUe;
  mobilityUe.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityUe.SetPositionAllocator(positionAllocUe);
  mobilityUe.Install(ues);

  // Required to use NIST 3GPP model
  BuildingsHelper::Install (threeSectorNodes);
  BuildingsHelper::Install (ues);
  BuildingsHelper::MakeMobilityModelConsistent ();


  //Install LTE devices to the nodes
  NetDeviceContainer enbDevs;
  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (0));
  enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (0)));
  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (120));
  enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (1)));
  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (240));
  enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (2)));

  NetDeviceContainer ueDevs = lteHelper->InstallUeDevice (ues);
  
  // Add X2 inteface
  lteHelper->AddX2Interface (threeSectorNodes);

  // Configure discovery for the UEs
  Ptr<LteUeRrcSl> ueSidelinkConfiguration = CreateObject<LteUeRrcSl> ();
  ueSidelinkConfiguration->SetDiscEnabled (true);
  
  //todo: specify parameters before installing in UEs if needed
  LteRrcSap::SlPreconfiguration preconfiguration;

  preconfiguration.preconfigGeneral.carrierFreq = 23330;
  preconfiguration.preconfigGeneral.slBandwidth = 50;
  preconfiguration.preconfigDisc.nbPools = 1; 
  preconfiguration.preconfigDisc.pools[0].cpLen.cplen = LteRrcSap::SlCpLen::NORMAL;
  preconfiguration.preconfigDisc.pools[0].discPeriod.period = LteRrcSap::SlPeriodDisc::rf32;
  preconfiguration.preconfigDisc.pools[0].numRetx = 0;
  preconfiguration.preconfigDisc.pools[0].numRepetition = 1;
  preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbNum = 10;
  preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbStart = 10;
  preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbEnd = 49;
  preconfiguration.preconfigDisc.pools[0].tfResourceConfig.offsetIndicator.offset = 0;
  preconfiguration.preconfigDisc.pools[0].tfResourceConfig.subframeBitmap.bitmap = std::bitset<40> (0x11111);
  preconfiguration.preconfigDisc.pools[0].txParameters.txParametersGeneral.alpha = LteRrcSap::SlTxParameters::al09;
  preconfiguration.preconfigDisc.pools[0].txParameters.txParametersGeneral.p0 = -40;
  //preconfiguration.preconfigDisc.pools[0].txParameters.txProbability.probability = LteRrcSap::TxProbability::p100;
  preconfiguration.preconfigDisc.pools[0].txParameters.txProbability = LteRrcSap::TxProbabilityFromInt (txProb);

  // Install configuration in UEs
  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs, ueSidelinkConfiguration);

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);
  std::cout << "Remote Host id = " << remoteHost->GetId() << std::endl;

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

  ///*** Configure applications ***///  
  std::map<Ptr<NetDevice>, std::list<uint32_t> > announceApps; 
  std::map<Ptr<NetDevice>, std::list<uint32_t> > monitorApps; 
  for (uint32_t i=1; i<= nbUes; ++i)
  {
    announceApps[ueDevs.Get(i-1)].push_back ((uint32_t)i);
    for (uint32_t j=1; j<= nbUes; ++j)
    {
      if (i != j)
      {
        monitorApps[ueDevs.Get(i-1)].push_back ((uint32_t)j);
      }
    }
  }

  for (uint32_t i = 0; i < nbUes; ++i) 
  {
    Simulator::Schedule (Seconds(2.0), &SlStartDiscovery, lteHelper, ueDevs.Get(i),announceApps.find(ueDevs.Get(i))->second, true); // true for announce
    Simulator::Schedule (Seconds(2.0), &SlStartDiscovery, lteHelper, ueDevs.Get(i), monitorApps.find(ueDevs.Get(i))->second, false); // false for monitor
  }
  
  ///*** End of application configuration ***///
  
  // Set Discovery Traces
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("discovery_out_monitoring.tr");
  *stream->GetStream () << "Time\tIMSI\tCellId\tRNTI\tProSeAppCode" << std::endl;
  
  AsciiTraceHelper ascii1;
  Ptr<OutputStreamWrapper> stream1 = ascii1.CreateFileStream ( "discovery_out_announcement_phy.tr");
  *stream1->GetStream () << "Time\tIMSI\tCellId\tRNTI\tProSeAppCode" << std::endl;
  
  AsciiTraceHelper ascii2;
  Ptr<OutputStreamWrapper> stream2 = ascii1.CreateFileStream ( "discovery_out_announcement_mac.tr");
  *stream2->GetStream () << "Time\tIMSI\tRNTI\tProSeAppCode" << std::endl;

  std::ostringstream oss;
  oss.str("");
  for (uint32_t i = 0; i < ueDevs.GetN (); ++i)
  {
    Ptr<LteUeRrc> ueRrc = DynamicCast<LteUeRrc>( ueDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetRrc () );    
    ueRrc->TraceConnectWithoutContext ("DiscoveryMonitoring", MakeBoundCallback (&DiscoveryMonitoringTrace, stream));
    oss << ueDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetImsi ();
    Ptr<LteUePhy> uePhy = DynamicCast<LteUePhy>( ueDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetPhy () );
    uePhy->TraceConnect ("DiscoveryAnnouncement", oss.str (), MakeBoundCallback (&DiscoveryAnnouncementPhyTrace, stream1));
    Ptr<LteUeMac> ueMac = DynamicCast<LteUeMac>( ueDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetMac () );
    ueMac->TraceConnect ("DiscoveryAnnouncement", oss.str (), MakeBoundCallback (&DiscoveryAnnouncementMacTrace, stream2));
    oss.str("");
  }

  // Enable traces
  lteHelper->EnableTraces();

  Simulator::Stop(Seconds(simTime));

  Simulator::Run();
  Simulator::Destroy();
  return 0;

}


