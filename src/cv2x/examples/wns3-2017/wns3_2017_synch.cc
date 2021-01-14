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
//#include <sstream>

/*
  version ##.##

 */
using namespace ns3;

std::map < std::string,std::pair<int32_t,bool> > IpIdMap;
void
PacketAddrsTrace (Ptr<OutputStreamWrapper> stream, std::string contex, Ptr<const Packet> p, const Address &addrs)
{
  std::ostringstream oss;
  *stream->GetStream () << Simulator::Now ().GetMilliSeconds() << "\t" << contex << "\t" << p->GetSize () << "\t";

  if (InetSocketAddress::IsMatchingType (addrs))
    {
      InetSocketAddress::ConvertFrom (addrs).GetIpv4 ().Print (oss);
      *stream->GetStream () << InetSocketAddress::ConvertFrom (addrs).GetIpv4 () << "\t" << InetSocketAddress::ConvertFrom (addrs).GetPort () << "\t\t" << IpIdMap[oss.str ()].first << "\t" << IpIdMap[oss.str ()].second <<  std::endl;
    }
  else if (Inet6SocketAddress::IsMatchingType (addrs))
    {
      Inet6SocketAddress::ConvertFrom (addrs).GetIpv6 ().Print (oss);
      *stream->GetStream () << Inet6SocketAddress::ConvertFrom (addrs).GetIpv6 () << "\t" << Inet6SocketAddress::ConvertFrom (addrs).GetPort () << "\t\t" << IpIdMap[oss.str ()].first << "\t" << IpIdMap[oss.str ()].second << std::endl;
    }
  else
    {
      *stream->GetStream () << "Unknown address type!" << std::endl;
    }
}

void
PacketSrcDstAddrsTrace (Ptr<OutputStreamWrapper> stream, const Ipv4Address &local_addrs, std::string contex, Ptr<const Packet> p, const Address &src_addrs, const Address &dst_addrs)
{
  std::ostringstream oss;
  *stream->GetStream () << Simulator::Now ().GetMilliSeconds () << "\t" << contex << "\t" << p->GetSize () << "\t";

  if (InetSocketAddress::IsMatchingType (src_addrs)) //assumes dst_addrs is same type!
    {
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
  else if (Inet6SocketAddress::IsMatchingType (src_addrs)) //assumes dst_addrs is same type!
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
  *stream->GetStream () << Simulator::Now ().GetMilliSeconds()  << "\t" << contex << "\t" << p->GetSize () << "\t";

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


/*Synchronization traces*/
void
NotifyChangeOfSyncRef (Ptr<OutputStreamWrapper> stream, LteUeRrc::SlChangeOfSyncRefStatParameters param)
{
  *stream->GetStream () << Simulator::Now ().GetMilliSeconds () << "\t" << param.imsi << "\t" << param.prevSlssid << "\t" << param.prevRxOffset << "\t" << param.prevFrameNo << "\t" << param.prevSubframeNo <<
      "\t" << param.currSlssid << "\t" << param.currRxOffset << "\t" << param.currFrameNo << "\t" << param.currSubframeNo << std::endl;
}

std::multimap <uint64_t, std::map <uint64_t, uint64_t > > groupMap;  //IMSI, GroupId, SLSSID

void
NotifyChangeOfSyncRefAndReportNumberOfTimings (Ptr<OutputStreamWrapper> stream, LteUeRrc::SlChangeOfSyncRefStatParameters param)
{
  uint64_t imsi=param.imsi;
  uint64_t slssid=param.currSlssid;

  std::pair <std::multimap <uint64_t, std::map <uint64_t, uint64_t > >::iterator, std::multimap <uint64_t, std::map <uint64_t, uint64_t > >::iterator> ret;
  ret = groupMap.equal_range(imsi);

  //For each group where the UE is
  for (std::multimap <uint64_t, std::map <uint64_t, uint64_t > >::iterator it=ret.first; it!=ret.second; ++it){
      uint64_t group = it->second.begin()->first;
      if(slssid){
          it->second.begin()->second = slssid;
      }
      //identify the slssids in the group

      std::vector <uint16_t> slssidInGroup;
      std::map <uint64_t, std::map <uint64_t, uint64_t> >::iterator itMAp;
      for (itMAp = groupMap.begin() ; itMAp != groupMap.end() ; itMAp++)
        {
          if (itMAp->second.begin()->first == group){
              //stored slssid if different
              if (std::find(slssidInGroup.begin(), slssidInGroup.end(), itMAp->second.begin()->second ) == slssidInGroup.end()) {
                  slssidInGroup.push_back(itMAp->second.begin()->second);
              }
          }
        }
      //count how many slssid we have
      *stream->GetStream () << Simulator::Now ().GetMilliSeconds () << "\t" << group << "\t" << slssidInGroup.size() << std::endl;
  }
}

void
NotifySendOfSlss (Ptr<OutputStreamWrapper> stream, uint64_t imsi, uint64_t slssid, uint16_t txOffset, bool inCoverage, uint16_t frame, uint16_t subframe)
{
  *stream->GetStream () << Simulator::Now ().GetMilliSeconds () << "\t" << imsi << "\t"  << slssid << "\t" << txOffset << "\t" << inCoverage << "\t" << frame <<  "\t" << subframe << std::endl;
}


NS_LOG_COMPONENT_DEFINE ("ExampleD2DTestMergeCommAndSync");
int
main (int argc, char *argv[])
{

  //Set queue values
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", StringValue ("100000"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (10000000));

  // Set the frequency to use the Public Safety case (band 14 : 700 MHz)
  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", StringValue ("5330"));
  Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", StringValue ("5330"));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", StringValue ("23330"));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", StringValue ("50"));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", StringValue ("50"));
  Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue (1));// TxDiversity(1); SpatialMultiPlex(2)

  // Set the UEs power in dBm
  double ueTxPower = 31.0;// 31dBm for PS, 23.0 for non-PS
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePowerControl::Pcmax", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePowerControl::PoNominalPusch", IntegerValue (-106));
  Config::SetDefault ("ns3::LteUePowerControl::PsschTxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::LteUePowerControl::PscchTxPower", DoubleValue (ueTxPower));

  // Set threshold for detecting a eNodeB in dBm
  Config::SetDefault ("ns3::LteUePhy::RsrpUeMeasThreshold", DoubleValue (-10.0));

  // Set error models
  Config::SetDefault ("ns3::LteSpectrumPhy::SlDataBLERModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::NistErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::FadingModel", StringValue ("AWGN"));

  // Set the eNBs power in dBm
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (46.0));

  // Set the SRS periodicity in ms (increase the capability of handling more UEs per cell)
  // Allowed values : 2, 5, 10, 20, 40, 80, 160 and 320
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (160));

  //Default parameter values
  double simTime = 5.001;

  uint32_t nbRings = 1;
  double isd = 500;

  uint32_t ue_responders_per_sector = 1;
  int nbgroups = 1;

  bool onoff = false;
  double respondersStart = 20.0;
  double responderIntPack = 0.005;
  uint32_t responderMaxPack = 0xffffffff;
  uint32_t responderSizePack = 40;

  uint32_t mcs = 10;
  uint32_t rbSize = 2;
  uint32_t ktrp = 2;
  uint32_t pscch_rbs = 22;
  std::string pscch_trphexstring = "0xFF00000000";
  uint32_t sl_period = 40;
  bool CtrlErrorModelEnabled = true;
  bool CtrlDropOnCollisionEnabled = false;

  /*Synchronization*/
  int16_t syncTxThreshOoC = -60; //dBm
  uint16_t filterCoefficient = 0;  //k = 4 ==> a = 0.5, k = 0 ==> a = 1 No filter
  uint16_t syncRefMinHyst = 0; //dB
  uint16_t syncRefDiffHyst = 0; //dB
  uint32_t interScanTimeMin = 2000; //ms
  uint32_t interScanTimeMax = 2000; //ms
  uint32_t scanTime = 40; //ms
  uint32_t measTime = 400; //ms
  uint32_t evalTime = 400; //ms
  uint32_t firstScanTimeMin = 2000; //ms
  uint32_t firstScanTimeMax = 4000; //ms
  bool unsyncSl = true;
  bool slSyncActive = true;
  /*END Synchronization*/

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("time", "Simulation time", simTime);
  cmd.AddValue ("responders", "Number of Responders per sector", ue_responders_per_sector);
  cmd.AddValue ("ring", "Number of rings in hexagon", nbRings);
  cmd.AddValue ("isd", "Inter Site Distance", isd);
  cmd.AddValue ("groups", "Number of groups", nbgroups);
  cmd.AddValue ("rpcksize", "Packets size in bytes", responderSizePack);
  cmd.AddValue ("rmaxpck", "maximum number of packets per UE", responderMaxPack);
  cmd.AddValue ("rpckint", "interval between packets", responderIntPack);
  cmd.AddValue ("mcs", "MCS", mcs);
  cmd.AddValue ("rbSize", "Allocation size", rbSize);
  cmd.AddValue ("ktrp", "Repetition", ktrp);
  cmd.AddValue ("pscch_rbs", "PSCCH RBs", pscch_rbs);
  cmd.AddValue ("pscch_trp", "PSCCH trp bitmap", pscch_trphexstring);
  cmd.AddValue ("slperiod", "Length of SL period", sl_period );
  cmd.AddValue ("ctrlerror", "Enables PSCCH error model", CtrlErrorModelEnabled);
  cmd.AddValue ("ctrldroponcol", "Drop PSCCH messages on collisions", CtrlDropOnCollisionEnabled);
  cmd.AddValue ("onoff", "Type of traffic", onoff);

  /*Synchronization*/
  cmd.AddValue ("syncTxThreshOoC", "SL Sync: Threshold defining the “inner part” of the SyncRef UE cell", syncTxThreshOoC );
  cmd.AddValue ("filterCoefficient", "SL Sync: Weight of the last measurement => k ", filterCoefficient);
  cmd.AddValue ("syncRefMinHyst" , "SL Sync: How stronger than the minimum required the signal of a SyncRef UE should be to consider it", syncRefMinHyst);
  cmd.AddValue ("syncRefDiffHyst", "SL Sync: How stronger than the current selected SyncRef UE the signal of another SyncRef UE should be to consider it", syncRefDiffHyst);
  cmd.AddValue ("interScanTimeMin", "SL Sync: Min value of the uniform dist of the inter syncRef-selection-processes time", interScanTimeMin);
  cmd.AddValue ("interScanTimeMax", "SL Sync: Max value of the uniform dist of the inter syncRef-selection-processes time", interScanTimeMax);
  cmd.AddValue ("scanTime", "SL Sync: Length of the SyncRef search period", scanTime);
  cmd.AddValue ("measTime", "SL Sync: Length of the L1 measurement time", measTime);
  cmd.AddValue ("evalTime", "SL Sync: Length of the SyncRef evaluation period for cease/intiation of SLSS transmission", evalTime);
  cmd.AddValue ("firstScanTimeMin", "SL Sync: Min value of the uniform dist of the initial measurement", firstScanTimeMin);
  cmd.AddValue ("firstScanTimeMax", "SL Sync: Max value of the uniform dist of the initial measurement", firstScanTimeMax);
  cmd.AddValue ("unsyncSl", "SL Sync: unsynchronized scenario (random frame/subframe indication, and random SLSSID", unsyncSl);
  cmd.AddValue ("slSyncActive", "SL Sync: activate the SL synchronization protocol", slSyncActive);
  /*END Synchronization*/

  cmd.Parse(argc, argv);

  /* Synchronization*/
  //Configure synchronization protocol
  Config::SetDefault ("ns3::LteUePhy::UeSlssInterScanningPeriodMax", TimeValue (MilliSeconds(interScanTimeMax)));
  Config::SetDefault ("ns3::LteUePhy::UeSlssInterScanningPeriodMin", TimeValue (MilliSeconds(interScanTimeMin)));
  Config::SetDefault ("ns3::LteUePhy::UeSlssScanningPeriod", TimeValue (MilliSeconds(scanTime)));
  Config::SetDefault ("ns3::LteUePhy::UeSlssMeasurementPeriod", TimeValue (MilliSeconds(measTime)));
  Config::SetDefault ("ns3::LteUePhy::UeSlssEvaluationPeriod", TimeValue (MilliSeconds(evalTime)));
  Config::SetDefault ("ns3::LteUePhy::UeRandomInitialSubframeIndication", BooleanValue(unsyncSl));
  if (slSyncActive)
    {
      Config::SetDefault ("ns3::LteUeRrc::UeSlssTransmissionEnabled", BooleanValue(true));
    }
  Config::SetDefault ("ns3::LteUeRrc::MinSrsrp", DoubleValue(-125));
  Config::SetDefault ("ns3::LteUePhy::MinSrsrp", DoubleValue(-125));
  Config::SetDefault ("ns3::LteUePhy::NSamplesSrsrpMeas", UintegerValue(4));
  /* END Synchronization*/

  // Configure scheduler for UE selected
  Config::SetDefault ("ns3::LteUeMac::SlGrantSize", UintegerValue (rbSize));
  Config::SetDefault ("ns3::LteUeMac::SlGrantMcs", UintegerValue (mcs));
  Config::SetDefault ("ns3::LteUeMac::Ktrp", UintegerValue (ktrp));

  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (CtrlErrorModelEnabled));
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlFullDuplexEnabled", BooleanValue (!CtrlErrorModelEnabled));
  Config::SetDefault ("ns3::LteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (CtrlDropOnCollisionEnabled));//If true, PSCCH drop msg when collision occurs, regardless SINR

  //Convert pscch_trp string representation to decimal.
  std::stringstream ss_hex;
  ss_hex << std::hex << pscch_trphexstring;
  uint64_t pscch_trpnumber;
  ss_hex >> pscch_trpnumber;  
  uint64_t t_number = pscch_trpnumber;
  uint32_t zero_bit_counter = 0;
  while (t_number%2 == 0)
    {
      zero_bit_counter++;
      t_number/=2;
    }
  uint32_t pssch_min_offset = 40 - zero_bit_counter;

  // Create the helpers
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetSchedulerType ("ns3::RrSlFfMacScheduler");

  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  Ptr<LteProseHelper> proseHelper = CreateObject<LteProseHelper> ();
  proseHelper->SetLteHelper (lteHelper);

  Ptr<LteHexGridEnbTopologyHelper> topoHelper = CreateObject<LteHexGridEnbTopologyHelper> ();
  topoHelper->SetLteHelper (lteHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  //Disable eNBs for out-of-coverage modeling
  lteHelper->DisableNewEnbPhy ();

  // Configure antenna parameters not handled by the topology helper
  lteHelper->SetEnbAntennaModelType ("ns3::Parabolic3dAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute ("MechanicalTilt", DoubleValue (15));

  // Set pathloss model
  lteHelper->SetPathlossModelType ("ns3::Lte3gppPropagationLossModel");

  //Configure general values of the topology
  topoHelper->SetNbRings (nbRings);
  topoHelper->SetInterSiteDistance (isd);
  topoHelper->SetMinimumDistance (10);
  topoHelper->SetSiteHeight(32);// 3GPP specs TS36.814 Table A.2.1.1-2 indicates 32m
  // Create eNbs
  NodeContainer sectorNodes;
  sectorNodes.Create (topoHelper->GetNumberOfNodes ());
  // Install mobility (eNB)
  MobilityHelper mobilityeNodeB;
  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.Install (sectorNodes);
  // Compute the position of each site and antenna orientation
  NetDeviceContainer enbDevs = topoHelper->SetPositionAndInstallEnbDevice(sectorNodes);

  // Responder users
  NodeContainer ueResponders;
  if (ue_responders_per_sector > 0)
    {
      // Create nodes (responders)
      ueResponders.Create (ue_responders_per_sector * sectorNodes.GetN ());
      // Install mobility (responders)
      MobilityHelper mobilityResponders;
      mobilityResponders.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      //Do not set position here, the topology helper takes care of deploying the nodes when called ahead!
      mobilityResponders.Install(ueResponders);
    }

  // Required to use NIST 3GPP model
  BuildingsHelper::Install (sectorNodes);
  BuildingsHelper::Install (ueResponders);
  BuildingsHelper::MakeMobilityModelConsistent ();

  //Install LTE devices to all responders and deploy them in the sectors.
  lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));
  //NetDeviceContainer ueRespondersDevs = lteHelper->InstallUeDevice (ueResponders);
  NetDeviceContainer ueRespondersDevs = topoHelper->DropUEsUniformlyPerSector2(ueResponders);

  ///////////////////////////////////////////////////////////////////
  /*                   Save the nodes positions                    */ 
  //////////////////////////////////////////////////////////////////
  std::ofstream m_outFile;
  if (std::ifstream ("nPositions.txt"))
    {
      remove ("nPositions.txt");
    }
  if (ueResponders.GetN () != 0)
    {
      m_outFile.open ("nPositions.txt", std::ofstream::out | std::ofstream::app);
      m_outFile << "Node\tID\tX\tY " << std::endl;
      for (uint32_t i = 0; i < sectorNodes.GetN (); ++i)
        {
          m_outFile << "eNB\t" << sectorNodes.Get (i)->GetId () << "\t" << sectorNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().x << "\t" << sectorNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().y << std::endl;
        }
      Ptr<Object> ueNetDevice;
      for (uint32_t i = 0; i < ueResponders.GetN (); ++i)
        {
          ueNetDevice = ueResponders.Get (i)->GetDevice (0);

          m_outFile << " UE\t" << ueNetDevice->GetObject<LteUeNetDevice> ()->GetImsi () << "\t" << ueResponders.Get (i)->GetObject<MobilityModel> ()->GetPosition ().x << "\t" << ueResponders.Get (i)->GetObject<MobilityModel> ()->GetPosition ().y << std::endl;
        }
      m_outFile.close ();
    }
  ////////////////////////////////////////////////////////////////////

  // Add X2 inteface
  lteHelper->AddX2Interface (sectorNodes);

  // Install the IP stack on the UEs
  InternetStackHelper internet;
  internet.Install (ueResponders);

  // Assign IP address to UEs
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (ueRespondersDevs);
  std::ostringstream ossip;
  std::cout << "UE IPs:" << std::endl;
  for (uint32_t u = 0; u < ueIpIface.GetN (); ++u)
    {
      //std::pair<Ptr<Ipv4>, uint32_t> pair = ueIpIface.Get (u);
      ueIpIface.GetAddress (u).Print(std::cout);
      std::cout << std::endl;
      //Insert <IP,ID> in IpIdMap
      ueIpIface.GetAddress (u).Print(ossip);
      IpIdMap[ossip.str ()] = std::make_pair(ueRespondersDevs.Get (u)->GetNode ()->GetId (),0);
      ossip.str("");
      ossip.clear();
    }

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  for (uint32_t u = 0; u < ueResponders.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueResponders.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach each UE to the best available eNB
  lteHelper->Attach (ueRespondersDevs);

  // Create responders groups and applications
  double ulEarfcn = enbDevs.Get (0)->GetObject<LteEnbNetDevice> ()->GetDlEarfcn ();
  double ulBandwidth = enbDevs.Get (0)->GetObject<LteEnbNetDevice> ()->GetDlBandwidth ();
  std::vector < NetDeviceContainer > createdgroups;

  //createdgroups = proseHelper->AssociateForBroadcastWithTxEnabledToReceive (ueTxPower, ulEarfcn, ulBandwidth, ueRespondersDevs, -1000, nbgroups, LteProseHelper::SRSRP_EVAL);
  createdgroups = proseHelper->AssociateForBroadcast (ueTxPower, ulEarfcn, ulBandwidth, ueRespondersDevs, -112, nbgroups, LteProseHelper::SRSRP_STD);

  //print groups created
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("prose_connections.txt");
  proseHelper->PrintGroups (createdgroups, stream);

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

  //Packet trace file
  stream = ascii.CreateFileStream ("pscr_ue_pck.tr");
  *stream->GetStream () << "time\ttx/rx\tNID\tIMSI\tUEtype\tsize\tIP[src]\tIP[dst]" << std::endl;
  std::ostringstream oss;

  std::vector<uint32_t> groupL2Addresses;
  uint32_t groupL2Address = 0x00;

  Ipv4AddressGenerator::Init(Ipv4Address ("225.0.0.0"), Ipv4Mask ("255.0.0.0"));
  Ipv4Address groupRespondersIpv4Address = Ipv4AddressGenerator::NextAddress (Ipv4Mask ("255.0.0.0"));

  uint16_t echo_port = 8000; //where to listen
  uint16_t gecho_server_port = 8000; //where to send

  std::vector < NetDeviceContainer >::iterator gIt;
  for (gIt = createdgroups.begin() ; gIt != createdgroups.end() ; gIt++)
    {
      //Create sidelink bearers
      //Use Tx for the group transmitter and Rx for all the receivers      
      //split Tx/Rx
      NetDeviceContainer txUe ((*gIt).Get(0));
      NetDeviceContainer rxUes = proseHelper->RemoveNetDevice ((*gIt), (*gIt).Get(0));
      Ptr<LteSlTft> tft = Create<LteSlTft> (LteSlTft::TRANSMIT, groupRespondersIpv4Address, groupL2Address);
      proseHelper->ActivateSidelinkBearer (Seconds(1.0), txUe, tft); 
      tft = Create<LteSlTft> (LteSlTft::RECEIVE, groupRespondersIpv4Address, groupL2Address);
      proseHelper->ActivateSidelinkBearer (Seconds(1.0), rxUes, tft); 

      groupRespondersIpv4Address.Print(ossip);
      IpIdMap[ossip.str ()] = std::make_pair(groupL2Address,1);
      ossip.str("");
      ossip.clear();

      //Deploy applications
      // Application to generate traffic (installed only in transmitter)
      ApplicationContainer clientRespondersApps;
      if (onoff) 
        {
          OnOffHelper clientOnOffHelper ("ns3::UdpSocketFactory", 
                                             Address( InetSocketAddress (groupRespondersIpv4Address, gecho_server_port)));
          clientOnOffHelper.SetAttribute ("PacketSize", UintegerValue (pck_bytes));
          clientOnOffHelper.SetAttribute ("DataRate", DataRateValue (pck_bitrate));
          clientOnOffHelper.SetAttribute ("OnTime", PointerValue (onRv));
          clientOnOffHelper.SetAttribute ("OffTime", PointerValue (offRv));

          clientRespondersApps = clientOnOffHelper.Install ((*gIt).Get(0)->GetNode()); 
          clientRespondersApps.Start (Seconds (respondersStart));
          clientRespondersApps.Stop (Seconds (simTime));
        }
      else 
        {
          // UDP application
          UdpEchoClientHelper echoClientHelper (groupRespondersIpv4Address, gecho_server_port);
          echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (responderMaxPack));
          echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (responderIntPack)));
          echoClientHelper.SetAttribute ("PacketSize", UintegerValue (responderSizePack));//1280

          clientRespondersApps = echoClientHelper.Install ((*gIt).Get(0)->GetNode());
          clientRespondersApps.Start (Seconds (respondersStart));
          clientRespondersApps.Stop (Seconds (simTime));
        }

      // Set responders Tx traces
      for (uint16_t ac = 0; ac < clientRespondersApps.GetN (); ac++)
        {
          oss << "t\t" << (*gIt).Get(ac)->GetNode()->GetId () << "\t" << (*gIt).Get(ac)->GetObject<LteUeNetDevice> ()->GetImsi () << "\tresp";   
          Ipv4Address local_addrs = (*gIt).Get(ac)->GetNode()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal();
          std::cout<< "Tx address: " << local_addrs << std::endl;
          clientRespondersApps.Get (ac)->TraceConnect("TxWithAddresses", oss.str (), MakeBoundCallback(&PacketSrcDstAddrsTrace, stream, local_addrs));
          oss.str("");        
        }

      //store and increment addresses
      groupL2Addresses.push_back (groupL2Address);
      groupL2Address++;
      groupRespondersIpv4Address = Ipv4AddressGenerator::NextAddress (Ipv4Mask ("255.0.0.0"));
    }

  // Application to receive traffic (Installed in all responders)

  PacketSinkHelper clientPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), echo_port));
  ApplicationContainer clientRespondersSrvApps = clientPacketSinkHelper.Install(ueResponders);
  clientRespondersSrvApps.Start (Seconds (respondersStart));
  clientRespondersSrvApps.Stop (Seconds (simTime+1));  

  // Set responders Rx traces
  for (uint16_t ac = 0; ac < clientRespondersSrvApps.GetN (); ac++)
    {
      Ipv4Address local_addrs =  ueRespondersDevs.Get (ac)->GetNode()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal();
      std::cout<< "Rx address: " << local_addrs << std::endl;
      oss << "r\t" << ueResponders.Get (ac)->GetId () << "\t" << ueResponders.Get (ac)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi () << "\tresp";
      clientRespondersSrvApps.Get (ac)->TraceConnect("RxWithAddresses", oss.str (), MakeBoundCallback(&PacketSrcDstAddrsTrace, stream, local_addrs));
      oss.str("");
    }  


  //Sidelink configuration for the UEs
  Ptr<LteUeRrcSl> ueSidelinkConfiguration = CreateObject<LteUeRrcSl> ();
  ueSidelinkConfiguration->SetSlEnabled (true);

  LteRrcSap::SlPreconfiguration preconfiguration;

  preconfiguration.preconfigGeneral.carrierFreq = 23330;
  preconfiguration.preconfigGeneral.slBandwidth = 50;
  preconfiguration.preconfigComm.nbPools = 1;

  //control 
  preconfiguration.preconfigComm.pools[0].scCpLen.cplen = LteRrcSap::SlCpLen::NORMAL;
  //preconfiguration.preconfigComm.pools[0].scPeriod.period = LteRrcSap::SlPeriodComm::sf40;
  preconfiguration.preconfigComm.pools[0].scPeriod.period = LteRrcSap::PeriodAsEnum(sl_period).period;
  preconfiguration.preconfigComm.pools[0].scTfResourceConfig.prbNum = pscch_rbs;
  preconfiguration.preconfigComm.pools[0].scTfResourceConfig.prbStart = 3;
  preconfiguration.preconfigComm.pools[0].scTfResourceConfig.prbEnd = 46;
  preconfiguration.preconfigComm.pools[0].scTfResourceConfig.offsetIndicator.offset = 0;
  preconfiguration.preconfigComm.pools[0].scTfResourceConfig.subframeBitmap.bitmap = std::bitset<40> (0xF000000000);
  //data
  preconfiguration.preconfigComm.pools[0].dataCpLen.cplen = LteRrcSap::SlCpLen::NORMAL;
  preconfiguration.preconfigComm.pools[0].dataHoppingConfig.hoppingParameters = 0;
  preconfiguration.preconfigComm.pools[0].dataHoppingConfig.numSubbands = LteRrcSap::SlHoppingConfigComm::ns4;
  preconfiguration.preconfigComm.pools[0].dataHoppingConfig.rbOffset = 0;
  //UE selected parameters
  preconfiguration.preconfigComm.pools[0].trptSubset.subset = std::bitset<3> (0x7); //k0=1, k1=0, k2=0
  preconfiguration.preconfigComm.pools[0].dataTfResourceConfig.prbNum = 25;
  preconfiguration.preconfigComm.pools[0].dataTfResourceConfig.prbStart = 0;
  preconfiguration.preconfigComm.pools[0].dataTfResourceConfig.prbEnd = 49;
  preconfiguration.preconfigComm.pools[0].dataTfResourceConfig.offsetIndicator.offset = pssch_min_offset; //allow for control frames
  preconfiguration.preconfigComm.pools[0].dataTfResourceConfig.subframeBitmap.bitmap = std::bitset<40> (0xFFFFFFFFFF);

  preconfiguration.preconfigComm.pools[0].scTxParameters.alpha = LteRrcSap::SlTxParameters::al09;
  preconfiguration.preconfigComm.pools[0].scTxParameters.p0 = -40;
  preconfiguration.preconfigComm.pools[0].dataTxParameters.alpha = LteRrcSap::SlTxParameters::al09;
  preconfiguration.preconfigComm.pools[0].dataTxParameters.p0 = -40;

  /* Synchronization*/
  //Synchronization parameters
  preconfiguration.preconfigSync.syncOffsetIndicator1 = 18;
  preconfiguration.preconfigSync.syncOffsetIndicator2 = 29;
  preconfiguration.preconfigSync.syncTxThreshOoC = syncTxThreshOoC;
  preconfiguration.preconfigSync.syncRefDiffHyst = syncRefDiffHyst;
  preconfiguration.preconfigSync.syncRefMinHyst = syncRefMinHyst;
  preconfiguration.preconfigSync.filterCoefficient = filterCoefficient;
  /* END Synchronization*/

  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);
  lteHelper->InstallSidelinkConfiguration (ueRespondersDevs, ueSidelinkConfiguration);


  /*Synchronization*/
  //Tracing synchronization stuffs
  Ptr<OutputStreamWrapper> streamTimingEvent = ascii.CreateFileStream ("NumberOfTimingPerGroup.txt");
  *streamTimingEvent->GetStream () << "Time\tGruopId\tNoTimings" << std::endl;
  Ptr<OutputStreamWrapper> streamSyncRef = ascii.CreateFileStream ("SyncRef.txt");
  *streamSyncRef->GetStream () << "Time\tIMSI\tprevSLSSID\tprevRxOffset\tprevFrameNo\tprevSframeNo\tcurrSLSSID\tcurrRxOffset\tcurrFrameNo\tcurrSframeNo" << std::endl;
  Ptr<OutputStreamWrapper> streamSendOfSlss = ascii.CreateFileStream ("TxSlss.txt");
  *streamSendOfSlss->GetStream () << "Time\tIMSI\tSLSSID\ttxOffset\tinCoverage\tFrameNo\tSframeNo" << std::endl;
  Ptr<OutputStreamWrapper> streamFirstScan = ascii.CreateFileStream ("FirstScan.txt");
  *streamFirstScan->GetStream () << "Time\tIMSI" << std::endl;

  //Set initial SLSSID and start of the first scanning for all UEs
  Ptr<UniformRandomVariable> rndSlssid = CreateObject<UniformRandomVariable> ();
  rndSlssid->SetAttribute("Min", DoubleValue(100000.0));
  if(unsyncSl){
      rndSlssid->SetAttribute("Max", DoubleValue(200000.0));
  }
  else{
      rndSlssid->SetAttribute("Max", DoubleValue(100000.0));
  }

  Ptr<UniformRandomVariable> rndStartScanning = CreateObject<UniformRandomVariable> ();
  rndStartScanning->SetAttribute("Min", DoubleValue(firstScanTimeMin));
  rndStartScanning->SetAttribute("Max", DoubleValue(firstScanTimeMax));

  std::vector < NetDeviceContainer >::iterator groupIt;
  for (groupIt = createdgroups.begin() ; groupIt != createdgroups.end() ; groupIt++)
    {
      for (uint32_t i = 0; i < (*groupIt).GetN (); ++i)
        {
          uint32_t slssid = rndSlssid->GetInteger();
          (*groupIt).Get (i)->GetObject<LteUeNetDevice> ()->GetRrc ()->SetSlssid(slssid);

          if ((*groupIt).Get (i)->GetObject<LteUeNetDevice> ()->GetPhy ()->GetFirstScanningTime() == MilliSeconds (0)){
              uint32_t t = 0;
              if(slSyncActive){
                  t = rndStartScanning->GetInteger();
                  (*groupIt).Get (i)->GetObject<LteUeNetDevice> ()->GetPhy ()->SetFirstScanningTime(MilliSeconds(t));
              }
              *streamFirstScan->GetStream () << t <<"\t" << (*groupIt).Get (i)->GetObject<LteUeNetDevice> ()->GetRrc ()->GetImsi() << std::endl;
          }
        }
    }

  //Saving initial values
  for (groupIt = createdgroups.begin() ; groupIt != createdgroups.end() ; groupIt++)
    {
      uint64_t grupHeadImsi = (*groupIt).Get (0)->GetObject<LteUeNetDevice> ()->GetRrc ()->GetImsi();
      uint64_t grupHeadSlssid =  (*groupIt).Get (0)->GetObject<LteUeNetDevice> ()->GetRrc ()->GetSlssid();

      for (uint32_t i = 0; i < (*groupIt).GetN (); ++i)
        {
          uint64_t ueImsi =  (*groupIt).Get (i)->GetObject<LteUeNetDevice> ()->GetRrc ()->GetImsi();
          uint64_t ueSlssid =  (*groupIt).Get (i)->GetObject<LteUeNetDevice> ()->GetRrc ()->GetSlssid();

          std::map <uint64_t, uint64_t > tmp;
          tmp.insert(std::pair<uint64_t, uint64_t >(grupHeadImsi,ueSlssid));
          groupMap.insert (std::pair<uint64_t,  std::map <uint64_t, uint64_t > > (ueImsi, tmp ));
          tmp.clear();
        }
      LteUeRrc::SlChangeOfSyncRefStatParameters param;
      param.imsi=grupHeadImsi; param.currSlssid=grupHeadSlssid;
      param.prevSlssid =0;param.prevRxOffset=0;param.prevFrameNo=0;param.prevSubframeNo=0;param.currRxOffset=0;param.currFrameNo=0;param.currSubframeNo=0;
      NotifyChangeOfSyncRefAndReportNumberOfTimings (streamTimingEvent, param);
    }

  //Tracing the change of synchronization reference and the number of timings in the corresponding group when it happens
  for (uint32_t i = 0; i < ueRespondersDevs.GetN (); ++i)
    {

      Ptr<LteUeRrc> ueRrc =  ueRespondersDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetRrc ();
      ueRrc->TraceConnectWithoutContext ("ChangeOfSyncRef", MakeBoundCallback (&NotifyChangeOfSyncRef, streamSyncRef));
      ueRrc->TraceConnectWithoutContext ("ChangeOfSyncRef", MakeBoundCallback (&NotifyChangeOfSyncRefAndReportNumberOfTimings, streamTimingEvent));
      ueRrc->TraceConnectWithoutContext ("SendSLSS", MakeBoundCallback (&NotifySendOfSlss, streamSendOfSlss));

    }
  for (uint32_t i = 0; i < ueRespondersDevs.GetN (); ++i)
    {
      Ptr<LteUeRrc> ueRrc =  ueRespondersDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetRrc ();
      LteUeRrc::SlChangeOfSyncRefStatParameters param;
      param.imsi=ueRrc->GetImsi();
      param.prevSlssid =0;param.prevRxOffset=0;param.prevFrameNo=0;param.prevSubframeNo=0;
      param.currSlssid=ueRrc->GetSlssid();param.currRxOffset=0;param.currFrameNo=ueRrc->GetFrameNumber();param.currSubframeNo=ueRrc->GetSubFrameNumber();
      NotifyChangeOfSyncRef(streamSyncRef,param );
    }
  /*END Synchronization*/



  Simulator::Stop(Seconds(simTime));

  std::cout << "Simulation running..." << std::endl;
  Simulator::Run();

  /*Synchronization*/

  //Obtain status at the end of the sim
  std::cout << "Simulation finished..."<< std::endl;
  for (groupIt = createdgroups.begin() ; groupIt != createdgroups.end() ; groupIt++)
    {
      uint64_t grupHeadImsi = (*groupIt).Get (0)->GetObject<LteUeNetDevice> ()->GetRrc ()->GetImsi();
      uint64_t groupHeadSlssid =   (*groupIt).Get (0)->GetObject<LteUeNetDevice> ()->GetRrc ()->GetSlssid();
      std::cout << "Imsi: "<<grupHeadImsi <<" Slssid: "<<groupHeadSlssid << std::endl;

      LteUeRrc::SlChangeOfSyncRefStatParameters param;
      param.imsi=grupHeadImsi; param.currSlssid=groupHeadSlssid;
      param.prevSlssid =0;param.prevRxOffset=0;param.prevFrameNo=0;param.prevSubframeNo=0;param.currRxOffset=0;param.currFrameNo=0;param.currSubframeNo=0;

      NotifyChangeOfSyncRefAndReportNumberOfTimings (streamTimingEvent, param);
    }

  for (uint32_t i = 0; i < ueRespondersDevs.GetN (); ++i)
    {
      Ptr<LteUeRrc> ueRrc =  ueRespondersDevs.Get (i)->GetObject<LteUeNetDevice> ()->GetRrc ();

      LteUeRrc::SlChangeOfSyncRefStatParameters param;
           param.imsi=ueRrc->GetImsi();
           param.prevSlssid =ueRrc->GetSlssid();param.prevFrameNo=ueRrc->GetFrameNumber();param.prevSubframeNo=ueRrc->GetSubFrameNumber();
           param.currSlssid=0;param.currRxOffset=0;param.currFrameNo=0;param.currSubframeNo=0;

      NotifyChangeOfSyncRef(streamSyncRef,param);
    }
  /*END Synchronization*/


  Simulator::Destroy();
  groupMap.clear();
  return 0;

}

