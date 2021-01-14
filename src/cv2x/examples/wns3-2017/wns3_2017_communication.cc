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
#include <ns3/spectrum-analyzer-helper.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <cfloat>
#include <sstream>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("d2d_communication_mode_2");

void
PacketSrcDstAddrsTrace (Ptr<OutputStreamWrapper> stream, const Ipv4Address &local_addrs, std::string contex, Ptr<const Packet> p, const Address &src_addrs, const Address &dst_addrs)
{
  std::ostringstream oss;

  *stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << contex << "\t" << p->GetSize () << "\t";
  if (InetSocketAddress::IsMatchingType (src_addrs))
    {
      //ss << os.rdbuf();
      oss << InetSocketAddress::ConvertFrom (src_addrs).GetIpv4 ();
      if(!oss.str ().compare("0.0.0.0")) //src_addrs not set
        {
          *stream->GetStream () << local_addrs << ":" << InetSocketAddress::ConvertFrom (src_addrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dst_addrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dst_addrs).GetPort () << std::endl;
        }
      else
        {
          oss.str("");
          oss << InetSocketAddress::ConvertFrom (dst_addrs).GetIpv4 ();
          if(!oss.str ().compare("0.0.0.0")) //dst_addrs not set
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (src_addrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (src_addrs).GetPort () << "\t" << local_addrs << ":" << InetSocketAddress::ConvertFrom (dst_addrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (src_addrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (src_addrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dst_addrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dst_addrs).GetPort () << std::endl;
            }
        }
    }
  else if (Inet6SocketAddress::IsMatchingType (src_addrs))
    {
      *stream->GetStream () << Inet6SocketAddress::ConvertFrom (src_addrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (src_addrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dst_addrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dst_addrs).GetPort () << std::endl;
    }
  else
    {
      *stream->GetStream () << "Unknown address type!" << std::endl;
    }
}

void
PacketTrace (Ptr<OutputStreamWrapper> stream, const Address &addrs, std::string contex, Ptr<const Packet> p)
{
  //*stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << contex << "\t" << p->GetSize () << "\t" << "Group Echo Server." << std::endl;
  *stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << contex << "\t" << p->GetSize () << "\t";
  if (InetSocketAddress::IsMatchingType (addrs))
    {
      *stream->GetStream () << InetSocketAddress::ConvertFrom (addrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (addrs).GetPort () << std::endl;
    }
  else if (Inet6SocketAddress::IsMatchingType (addrs))
    {
      *stream->GetStream () << Inet6SocketAddress::ConvertFrom (addrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (addrs).GetPort () << std::endl;
    }
  else
    {
      *stream->GetStream () << "Unknown address type!" << std::endl;
    }
}


void
PrintStatus (uint32_t s_period)
{
  std::cout<< Simulator::Now ().GetSeconds () << std::endl;
  Simulator::Schedule(Seconds(s_period),&PrintStatus, s_period);
}


int
main (int argc, char *argv[])
{
  LogComponentEnable ("d2d_communication_mode_2", LOG_INFO);

  // Initialize some values
  uint32_t simTime = 6;            // Simulation time in seconds
  double ueTxPower = 23.0;         // Transmission power in dBm
  uint32_t ue_responders_per_sector = 1; // Responder UEs per sectors
  uint32_t respondersStart = 1;    // Responders' applications start time
  bool verbose = false;            // Print time progress
  bool onoff = false;              // Use on-off applications
  double responderIntPack = 0.020; // Responders' application packet interval in seconds
  uint32_t responderMaxPack = 10;  // Responders' maximum number of packets
  uint32_t responderSizePack = 7;  // Number of payload bytes in packets
  uint32_t nbRings = 1;            // Number of rings in hexagon cell topology
  double isd = 10;                 // Inter Site Distance
  double minCenterDist = 1;        // Minimum deploy distance to center of cell site
  uint32_t nbgroups = 3;           // Number of D2D groups
  uint32_t nbreceivers = 0;        // Number of receivers per group. If zero, then group size is not limited.
  uint32_t mcs = 10;               // Modulation and Coding Scheme
  uint32_t rbSize = 2;             // PSSCH subchannel allocation size in RBs
  uint32_t ktrp = 1;               // Transmissions opportunities in a Time Repetition Pattern
  uint32_t pucchSize = 6;          // PUCCH size in RBs
  uint32_t pscch_rbs = 22;         // PSCCH pool size in RBs. Note, the PSCCH occupied bandwidth will be at least twice this amount.
  std::string pscch_bitmap_hexstring = "0xFF00000000"; // PSCCH time bitmap [40 bits]
  uint32_t sl_period = 40;         // Length of sidelink period in milliseconds
  bool CtrlErrorModelEnabled = true; // Enable error model in the PSCCH
  bool CtrlDropOnCollisionEnabled = false; // Drop PSCCH messages on conflicting scheduled resources

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("time", "Simulation time", simTime);
  cmd.AddValue ("responders", "Number of Responders per sector", ue_responders_per_sector);
  cmd.AddValue ("verbose", "Print time progress", verbose);
  cmd.AddValue ("ring", "Number of rings in hexagon cell topology", nbRings);
  cmd.AddValue ("isd", "Inter Site Distance", isd);
  cmd.AddValue ("mindist", "Minimum Center Distance for UEs", minCenterDist);
  cmd.AddValue ("groups", "Number of groups", nbgroups);
  cmd.AddValue ("receivers", "Number of receivers per group", nbreceivers);
  cmd.AddValue ("rpcksize", "Packets size in bytes", responderSizePack);
  cmd.AddValue ("rmaxpck", "maximum number of packets per UE", responderMaxPack);
  cmd.AddValue ("rpckint", "interval between packets", responderIntPack);
  cmd.AddValue ("mcs", "MCS", mcs);
  cmd.AddValue ("rbSize", "PSSCH allocation size", rbSize);
  cmd.AddValue ("ktrp", "Repetition", ktrp);
  cmd.AddValue ("pucchSize", "PUCCH size", pucchSize);
  cmd.AddValue ("pscch_rbs", "PSCCH RBs", pscch_rbs);
  cmd.AddValue ("pscch_trp", "PSCCH time bitmap", pscch_bitmap_hexstring);
  cmd.AddValue ("slperiod", "Length of SL period", sl_period );
  cmd.AddValue ("ctrlerror", "Enables PSCCH error model", CtrlErrorModelEnabled);
  cmd.AddValue ("ctrldroponcol", "Drop PSCCH messages on collisions", CtrlDropOnCollisionEnabled);
  cmd.Parse(argc, argv);

  NS_LOG_INFO ("Starting network configuration...");

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", StringValue ("100000"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (10000000));

  // Set the frequency to use for the Public Safety case (band 14 : 788 - 798 MHz for Uplink)
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", StringValue ("23330"));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", StringValue ("50"));

  // Set the UEs power in dBm
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePhy::RsrpUeMeasThreshold", DoubleValue (-10.0));

  // Set power control
  Config::SetDefault ("ns3::LteUePowerControl::Pcmax", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePowerControl::PoNominalPusch", IntegerValue (-106));
  Config::SetDefault ("ns3::LteUePowerControl::PsschTxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePowerControl::PscchTxPower", DoubleValue (ueTxPower));

  // Configure error model
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (CtrlErrorModelEnabled));
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlFullDuplexEnabled", BooleanValue (!CtrlErrorModelEnabled));
  Config::SetDefault ("ns3::LteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (CtrlDropOnCollisionEnabled));

  std::cout<<"CtrlErrorModel: " << ((CtrlErrorModelEnabled)?"Enabled":"Disabled") << std::endl;
  std::cout<<"CtrlDropOnCollision: " << ((CtrlDropOnCollisionEnabled)?"Enabled":"Disabled") << std::endl;

  // Configure for UE selected
  Config::SetDefault ("ns3::LteUeMac::SlGrantSize", UintegerValue (rbSize));
  Config::SetDefault ("ns3::LteUeMac::SlGrantMcs", UintegerValue (mcs));
  Config::SetDefault ("ns3::LteUeMac::Ktrp", UintegerValue (ktrp));
  Config::SetDefault ("ns3::LteUeMac::PucchSize", UintegerValue (pucchSize));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  NS_LOG_INFO ("Creating helpers...");
  // EPC
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  std::cout << "PGW id=" << pgw->GetId() << std::endl;

  // LTE Helper
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->DisableNewEnbPhy (); //Disable eNBs for out-of-coverage modeling

  // ProSe
  Ptr<LteProseHelper> proseHelper = CreateObject<LteProseHelper> ();
  proseHelper->SetLteHelper (lteHelper);

  // Topology (Hex Grid)
  Ptr<LteHexGridEnbTopologyHelper> topoHelper = CreateObject<LteHexGridEnbTopologyHelper> ();
  topoHelper->SetLteHelper (lteHelper);
  topoHelper->SetNbRings (nbRings);
  topoHelper->SetInterSiteDistance (isd);
  topoHelper->SetMinimumDistance (minCenterDist);

  // Configure eNBs' antenna parameters before deploying them.
  lteHelper->SetEnbAntennaModelType ("ns3::Parabolic3dAntennaModel");

  // Set pathloss model
  Config::SetDefault ("ns3::3gppPropagationLossModel::CacheLoss", BooleanValue (true));
  lteHelper->SetPathlossModelType ("ns3::3gppPropagationLossModel");

  // Create eNbs
  NodeContainer sectorNodes;
  sectorNodes.Create (topoHelper->GetNumberOfNodes ());

  std::cout << "eNb IDs=[";
  for (NodeContainer::Iterator it=sectorNodes.Begin (); it != sectorNodes.End (); it++)
    {
      if(it+1 != sectorNodes.End ())
        std::cout << (*it)->GetId () << ",";
      else
        std::cout << (*it)->GetId () << "]" << std::endl;
    }

  // Install mobility (eNB)
  MobilityHelper mobilityeNodeB;
  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.Install (sectorNodes);

  // Compute the position of each site and antenna orientation
  NetDeviceContainer enbDevs = topoHelper->SetPositionAndInstallEnbDevice(sectorNodes);

  // Create node container to hold all UEs
  NodeContainer ueAllNodes;

  // Responder users
  NodeContainer ueResponders;
  if (ue_responders_per_sector > 0)
    {
      // Create nodes (responders)
      ueResponders.Create (ue_responders_per_sector * sectorNodes.GetN ());
      ueAllNodes.Add (ueResponders);

      std::cout << "Responders IDs=[";
      for (NodeContainer::Iterator it=ueResponders.Begin (); it != ueResponders.End (); it++)
        {
          if(it+1 != ueResponders.End ())
            std::cout << (*it)->GetId () << ",";
          else
            std::cout << (*it)->GetId () << "]" << std::endl;
        }

      // Install mobility (responders)
      MobilityHelper mobilityResponders;
      mobilityResponders.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      //Do not set position here, the topology helper takes care of deploying the nodes when called ahead!
      mobilityResponders.Install(ueResponders);
    }

  // Required to use NIST 3GPP model
  BuildingsHelper::Install (sectorNodes);
  BuildingsHelper::Install (ueAllNodes);
  BuildingsHelper::MakeMobilityModelConsistent ();

  // Install LTE devices to all UEs and deploy them in the sectors.
  NS_LOG_INFO ("Installing UE's network devices and Deploying...");
  lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));
  NetDeviceContainer ueRespondersDevs = topoHelper->DropUEsUniformlyPerSector2(ueResponders);
  NetDeviceContainer ueDevs;
  ueDevs.Add (ueRespondersDevs);

  // Save nodes' positions
  std::ofstream m_outFile;
  if (ueAllNodes.GetN () != 0)
    {
      m_outFile.open ("nPositions.txt", std::ofstream::out);

      m_outFile << "Node\tID\tX\tY " << std::endl;
      for (uint32_t i = 0; i < sectorNodes.GetN (); ++i)
        {
          m_outFile << "eNB\t" << sectorNodes.Get (i)->GetId () << "\t" << sectorNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().x << "\t" << sectorNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().y << std::endl;
        }
      Ptr<Object> ueNetDevice;
      for (uint32_t i = 0; i < ueAllNodes.GetN (); ++i)
        {
          ueNetDevice = ueAllNodes.Get (i)->GetDevice (0);

          m_outFile << " UE\t" << ueNetDevice->GetObject<LteLteUeNetDevice> ()->GetImsi () << "\t" << ueAllNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().x << "\t" << ueAllNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().y << std::endl;
        }
      m_outFile.close ();
    }

  // Install the IP stack on the UEs
  NS_LOG_INFO ("Installing IP stack...");
  InternetStackHelper internet;
  internet.Install (ueAllNodes);

  // Assign IP address to UEs
  NS_LOG_INFO ("Allocating IP addresses and setting up network route...");
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (ueDevs);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  for (uint32_t u = 0; u < ueAllNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueAllNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  NS_LOG_INFO ("Attaching UE's to LTE network...");
  // Attach each UE to the best available eNB
  lteHelper->Attach (ueDevs);


  NS_LOG_INFO ("Creating sidelink groups...");
  // Create groups
  double ulEarfcn = enbDevs.Get (0)->GetObject<LteLteEnbNetDevice> ()->GetUlEarfcn ();
  double ulBandwidth = enbDevs.Get (0)->GetObject<LteLteEnbNetDevice> ()->GetUlBandwidth ();
  std::vector < NetDeviceContainer > createdgroups;
  if (nbreceivers > 0)
    {
      //groupcast
      createdgroups = proseHelper->AssociateForGroupcast (ueTxPower, ulEarfcn, ulBandwidth, ueRespondersDevs, -112, nbgroups, nbreceivers, LteLteProseHelper::SRSRP_EVAL);
    }
  else
    {
      //broadcast
      createdgroups = proseHelper->AssociateForBroadcastWithTxEnabledToReceive (ueTxPower, ulEarfcn, ulBandwidth, ueRespondersDevs, -112, nbgroups, LteLteProseHelper::SRSRP_EVAL);
    }
  //print groups created
  proseHelper->PrintGroups (createdgroups);

  //compute average number of receivers associated per transmitter and vice versa
  double totalRxs = 0;
  std::map < uint32_t, uint32_t> txPerUeMap;
  std::map < uint32_t, uint32_t> groupsPerUe;

  std::vector < NetDeviceContainer >::iterator gIt;
  for (gIt = createdgroups.begin() ; gIt != createdgroups.end() ; gIt++)
    {
      if(gIt->GetN() < nbreceivers){
          // Unable to satisfy group size requested!
          std::cerr << "Unable to satisfy group size requested for groupcast. Either the UEs are out of reach or there are not enough UEs!" << std::endl;
          return 1;
      }
      totalRxs += gIt->GetN ()-1;
      uint32_t nDevices = gIt->GetN ();
      for (uint32_t i = 1 ; i < nDevices; ++i)
        {
          uint32_t nId = gIt->Get (i)->GetNode ()->GetId ();
          txPerUeMap[nId]++;
        }
    }

  double totalTxPerUe = 0;
  std::map < uint32_t, uint32_t>::iterator mIt;
  std::cout << "Tx per UE:" << std::endl;
  for (mIt = txPerUeMap.begin (); mIt != txPerUeMap.end() ; mIt++)
    {
      totalTxPerUe+= mIt->second;
      std::cout << mIt->first << " " << mIt->second << std::endl;
      groupsPerUe [mIt->second]++;
    }
  std::cout << "Average number of receivers per transmitter = " << (totalRxs / nbgroups) << std::endl;
  std::cout << "Average number of transmitters per receiver = " << (totalTxPerUe / txPerUeMap.size()) << std::endl;
  std::cout << "Associated Groups per Rx UE" << std::endl;
  for (mIt = groupsPerUe.begin (); mIt != groupsPerUe.end (); mIt++)
    {
      std::cout << mIt->first << " " << mIt->second << std::endl;
    }

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("prose_connections.txt");
  proseHelper->PrintGroups (createdgroups, stream);

  NS_LOG_INFO ("Installing applications...");
  // Application Setup for Responders
  // Set Exponential Random Variables: ON(75% with mean 2.5s), OFF(25%)
  double ontime_mean = 2.5;
  double offtime_mean = ontime_mean/3;
  Ptr<ExponentialRandomVariable> onRv = CreateObject<ExponentialRandomVariable> ();
  onRv->SetAttribute ("Mean", DoubleValue (ontime_mean));
  Ptr<ExponentialRandomVariable> offRv = CreateObject<ExponentialRandomVariable> ();
  offRv->SetAttribute ("Mean", DoubleValue (offtime_mean));

  // VoIP model defined in 36.843 Table A.2.1.3-1
  double encoder_frame_length = responderIntPack; //0.020 s for VoIP
  double pck_rate= 1 / encoder_frame_length; //100
  uint16_t pck_bytes = responderSizePack;
  double pck_bitrate = pck_bytes * 8 * pck_rate;


  stream = ascii.CreateFileStream ("ue_pck.tr");
  //Trace file table header
  *stream->GetStream () << "time\ttx/rx\tNID\tIMSI\tUEtype\tsize\tIP[src]\tIP[dst]" << std::endl;

  std::vector<uint32_t> groupL2Addresses;
  uint32_t groupL2Address = 0x00;
  Ipv4AddressGenerator::Init(Ipv4Address ("225.0.0.0"), Ipv4Mask ("255.0.0.0"));
  Ipv4Address clientRespondersAddress = Ipv4AddressGenerator::NextAddress (Ipv4Mask ("255.0.0.0"));

  uint16_t application_port = 8000; //Application port to TX/RX
  ApplicationContainer clientRespondersApps;
  NetDeviceContainer activeTxUes;

  for (gIt = createdgroups.begin() ; gIt != createdgroups.end() ; gIt++)
    {
      //Create sidelink bearers
      //Use Tx for the group transmitter and Rx for all the receivers
      //split Tx/Rx

      NetDeviceContainer txUe ((*gIt).Get(0));
      activeTxUes.Add(txUe);
      NetDeviceContainer rxUes = proseHelper->RemoveNetDevice ((*gIt), txUe.Get (0));
      Ptr<LteSlTft> tft = Create<LteSlTft> (LteSlTft::TRANSMIT, clientRespondersAddress, groupL2Address);
      proseHelper->ActivateSidelinkBearer (Seconds(1.0), txUe, tft);
      tft = Create<LteSlTft> (LteSlTft::RECEIVE, clientRespondersAddress, groupL2Address);
      proseHelper->ActivateSidelinkBearer (Seconds(1.0), rxUes, tft);

      std::cout << "Created group L2Address=" << groupL2Address << " IPAddress=";
      clientRespondersAddress.Print(std::cout);
      std::cout << std::endl;

      // Install Application in the Responders' UEs
      if (onoff)
        {
          std::cout<<"Responder's OnOff App. bitrate " << pck_bitrate << std::endl;
          OnOffHelper clientOnOffHelper ("ns3::UdpSocketFactory",
                                             Address( InetSocketAddress (clientRespondersAddress, application_port)));
          clientOnOffHelper.SetAttribute ("PacketSize", UintegerValue (pck_bytes));
          clientOnOffHelper.SetAttribute ("DataRate", DataRateValue (pck_bitrate));
          clientOnOffHelper.SetAttribute ("OnTime", PointerValue (onRv));
          clientOnOffHelper.SetAttribute ("OffTime", PointerValue (offRv));

          //clientRespondersApps = clientOnOffHelper.Install ((*gIt).Get(0)->GetNode());
          clientRespondersApps.Add(clientOnOffHelper.Install (txUe.Get (0)->GetNode()));
        }
      else
        {
          // UDP application
          UdpEchoClientHelper echoClientHelper (clientRespondersAddress, application_port);
          echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (responderMaxPack));
          echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (responderIntPack)));
          echoClientHelper.SetAttribute ("PacketSize", UintegerValue (responderSizePack));

          // Install application to group transmitter
          //clientRespondersApps = echoClientHelper.Install ((*gIt).Get(0)->GetNode());
          clientRespondersApps.Add(echoClientHelper.Install (txUe.Get (0)->GetNode()));
        }

      //store and increment addresses
      groupL2Addresses.push_back (groupL2Address);
      groupL2Address++;
      clientRespondersAddress = Ipv4AddressGenerator::NextAddress (Ipv4Mask ("255.0.0.0"));
    } // end for each group in createdgroups

  clientRespondersApps.Start (Seconds (respondersStart));
  clientRespondersApps.Stop (Seconds (simTime));

  // Application to receive traffic
  PacketSinkHelper clientPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), application_port));
  ApplicationContainer clientRespondersSrvApps = clientPacketSinkHelper.Install(ueResponders);
  clientRespondersSrvApps.Start (Seconds (respondersStart));
  clientRespondersSrvApps.Stop (Seconds (simTime+1));


  NS_LOG_INFO ("Setting application traces...");
  std::ostringstream oss;
  // Set responders Tx traces
  for (uint16_t ac = 0; ac < clientRespondersApps.GetN (); ac++)
    {
      oss << "t\t" << activeTxUes.Get(ac)->GetNode()->GetId () << "\t" << activeTxUes.Get(ac)->GetObject<LteUeNetDevice> ()->GetImsi () << "\tresp";
      Ipv4Address local_addrs = activeTxUes.Get(ac)->GetNode()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal();
      std::cout<< "Tx address: " << local_addrs << std::endl;
      clientRespondersApps.Get (ac)->TraceConnect("TxWithAddresses", oss.str (), MakeBoundCallback(&PacketSrcDstAddrsTrace, stream, local_addrs));
      oss.str("");
    }

  // Set responders Rx traces
  for (uint16_t ac = 0; ac < clientRespondersSrvApps.GetN (); ac++)
    {
      Ipv4Address local_addrs =  ueRespondersDevs.Get (ac)->GetNode()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal();
      std::cout<< "Rx address: " << local_addrs << std::endl;
      oss << "r\t" << ueResponders.Get (ac)->GetId () << "\t" << ueResponders.Get (ac)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi () << "\tresp";
      clientRespondersSrvApps.Get (ac)->TraceConnect("RxWithAddresses", oss.str (), MakeBoundCallback(&PacketSrcDstAddrsTrace, stream, local_addrs));
      oss.str("");
    }


  NS_LOG_INFO ("Creating sidelink configuration...");
  Ptr<LteUeRrcSl> ueSidelinkConfiguration = CreateObject<LteUeRrcSl> ();
  ueSidelinkConfiguration->SetSlEnabled (true);

  LteRrcSap::SlPreconfiguration preconfiguration;
  preconfiguration.preconfigGeneral.carrierFreq = 23330;
  preconfiguration.preconfigGeneral.slBandwidth = 50;
  preconfiguration.preconfigComm.nbPools = 1;

  //Convert pscch_trp hex string representation to decimal.
  std::stringstream ss_hex;
  ss_hex << std::hex << pscch_bitmap_hexstring;
  uint64_t pscch_trpnumber;
  ss_hex >> pscch_trpnumber;
  std::cout<<"PSCCHRBS: " << pscch_rbs << ", PSCCH_hexstring: " << pscch_bitmap_hexstring << std::endl;
  uint64_t t_number = pscch_trpnumber;
  uint32_t zero_bit_counter = 0;
  while (t_number%2 == 0)
    {
      zero_bit_counter++;
      t_number/=2;
    }
  uint32_t pscch_length = 40 - zero_bit_counter;

  SlPreconfigPoolFactory pfactory;
  pfactory.SetControlBitmap (pscch_trpnumber);
  std::stringstream pstream;
  pstream << "sf" << sl_period;
  pfactory.SetControlPeriod (pstream.str());
  pfactory.SetControlPrbNum (pscch_rbs);
  pfactory.SetDataOffset (pscch_length);

  preconfiguration.preconfigComm.pools[0] = pfactory.CreatePool ();
  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);

  NS_LOG_INFO ("Installing sidelink configuration...");
  lteHelper->InstallSidelinkConfiguration (ueRespondersDevs, ueSidelinkConfiguration);

  NS_LOG_INFO ("Enabling LTE traces...");
  lteHelper->EnableTraces();

  // Display simulator time progress
  if (verbose)
    {
      uint32_t status_period = 1;
      Simulator::Schedule(Seconds(1),&PrintStatus, status_period);
    }

  NS_LOG_INFO ("Starting simulation...");
  Simulator::Stop(MilliSeconds(simTime*1000+40));
  Simulator::Run();
  Simulator::Destroy();

  NS_LOG_INFO ("Done.");
  return 0;

}

