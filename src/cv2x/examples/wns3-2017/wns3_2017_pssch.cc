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

#include <ns3/applications-module.h>
#include <ns3/config-store.h>
#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/lte-helper.h>
#include <ns3/module.h>
#include "ns3/sl-resource-pool-factory.h"
#include "ns3/sl-preconfig-pool-factory.h"

/*
This example is used to validate the maximum achievable data rate on the Physical Sidelink Shared
Channel (PSSCH) between 2 UEs given a particular resource pool configuration.
Parameters include:
- period: duration of the sidelink period
- pscchLength: length of the pysical sidelink control channel (PSCCH)
- ktrp: repetition pattern defining how many subframe can be used for sidelink
- mcs: modulation and coding scheme for transmission on the sidelink shared channel
- rbSize: allocation size in resource block (RB)
*/
NS_LOG_COMPONENT_DEFINE ("wns3_2017_pssch");

using namespace ns3;

int
main (int argc, char *argv[])
{
  //DEBUG components
  LogComponentEnable("LteUeMac", LOG_LEVEL_ALL);

  // Initialize some values
  uint32_t mcs = 10;
  uint32_t rbSize = 2;
  uint32_t ktrp = 2;
  uint32_t pscchLength = 8;
  std::string period="sf40";
  double dist = 10;
  double simTime = 10.0;
  double ueTxPower = 23.0;
  uint32_t ueCount = 2;
  bool verbose = false;


  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("period", "Sidelink period", period);
  cmd.AddValue ("pscchLength", "Length of PSCCH.", pscchLength);
  cmd.AddValue ("ktrp", "Repetition.", ktrp);
  cmd.AddValue ("mcs", "MCS.", mcs);
  cmd.AddValue ("rbSize", "Allocation size.", rbSize);
  cmd.AddValue ("verbose", "Print time progress.", verbose);

  cmd.Parse(argc, argv);

  NS_LOG_INFO ("Configuring UE settings...");
  Config::SetDefault ("ns3::LteUeMac::SlGrantMcs", UintegerValue (mcs));
  Config::SetDefault ("ns3::LteUeMac::SlGrantSize", UintegerValue (rbSize));
  Config::SetDefault ("ns3::LteUeMac::Ktrp", UintegerValue (ktrp));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePowerControl::Pcmax", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePowerControl::PscchTxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePowerControl::PsschTxPower", DoubleValue (ueTxPower));
 
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  NS_LOG_INFO ("Creating helpers...");
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->DisableNewEnbPhy ();
  lteHelper->Initialize ();

  Ptr<LteProseHelper> proseHelper = CreateObject<LteProseHelper> ();
  proseHelper->SetLteHelper (lteHelper);

  NS_LOG_INFO ("Deploying UE's...");
  NodeContainer ueResponders;
  ueResponders.Create (ueCount);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0),
                                 "MinY", DoubleValue (-5.0),
                                 "DeltaX", DoubleValue (dist),
                                 "DeltaY", DoubleValue (dist),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueResponders);

  NS_LOG_INFO ("Installing UE network devices...");
  NetDeviceContainer ueDevs;
  NetDeviceContainer ueRespondersDevs = lteHelper->InstallUeDevice (ueResponders);
  ueDevs.Add (ueRespondersDevs);

  NS_LOG_INFO ("Installing IP stack...");
  InternetStackHelper internet;
  internet.Install (ueResponders);
  
  NS_LOG_INFO ("Allocating IP addresses and setting up network route...");
  Ipv4InterfaceContainer ueIpIface;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ueIpIface = epcHelper->AssignUeIpv4Address (ueDevs);
  for (uint32_t u = 0; u < ueResponders.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueResponders.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
  
  NS_LOG_INFO ("Attaching UE's to LTE network...");
  lteHelper->Attach (ueDevs);
  
  NS_LOG_INFO ("Installing applications...");
  // UDP application
  Ipv4Address groupAddress ("225.0.0.0"); //use multicast address as destination  
  UdpClientHelper udpClient (groupAddress , 8000); 
  udpClient.SetAttribute ("MaxPackets", UintegerValue (5000000));
  udpClient.SetAttribute ("Interval", TimeValue (Seconds (0.001)));
  udpClient.SetAttribute ("PacketSize", UintegerValue (1000));

  ApplicationContainer clientApps = udpClient.Install (ueResponders);
  clientApps.Get(0)->SetStartTime (Seconds (3.0));
  clientApps.Get(1)->SetStartTime (Seconds (1000.0));
 
  
  NS_LOG_INFO ("Creating sidelink configuration...");
  uint32_t groupL2Address = 0xFF;  
  Ptr<LteSlTft> tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress, groupL2Address);
  proseHelper->ActivateSidelinkBearer (Seconds(1.0), ueDevs, tft); 

  Ptr<LteUeRrcSl> ueSidelinkConfiguration = CreateObject<LteUeRrcSl> ();
  ueSidelinkConfiguration->SetSlEnabled (true);

  LteRrcSap::SlPreconfiguration preconfiguration;
  preconfiguration.preconfigGeneral.carrierFreq = 23330;
  preconfiguration.preconfigGeneral.slBandwidth = 50;
  preconfiguration.preconfigComm.nbPools = 1;
  SlPreconfigPoolFactory pfactory;
  //build PSCCH bitmap value
  uint64_t pscchBitmapValue = 0x0;
  for (uint32_t i = 0 ; i < pscchLength; i++) {
    pscchBitmapValue = pscchBitmapValue >> 1 | 0x8000000000;
  }
  std::cout << "bitmap=" << std::hex << pscchBitmapValue << '\n';
  pfactory.SetControlBitmap (pscchBitmapValue);
  pfactory.SetControlPeriod (period);
  pfactory.SetDataOffset (pscchLength);
  preconfiguration.preconfigComm.pools[0] = pfactory.CreatePool ();
  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);

  NS_LOG_INFO ("Installing sidelink configuration...");
  lteHelper->InstallSidelinkConfiguration (ueRespondersDevs, ueSidelinkConfiguration);

  NS_LOG_INFO ("Enabling LTE traces...");
  lteHelper->EnableTraces ();
  
  NS_LOG_INFO ("Starting simulation...");
  Simulator::Stop (Seconds (simTime+2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_INFO ("Done.");

  return 0;

}

