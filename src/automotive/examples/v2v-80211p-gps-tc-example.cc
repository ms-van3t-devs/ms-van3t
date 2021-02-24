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
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/


#include "ns3/automotive-module.h"
#include "ns3/gps-tc-module.h"
#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include "ns3/mobility-module.h"
#include "ns3/packet-socket-helper.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("v2v-80211p-gps-tc-example");

int
main (int argc, char *argv[])
{
  /*
   * In this example instead of relying on SUMO, the vehicles' mobility will be managed by the "gps-tc" module.
   * "gps-tc" is a module that is able to load GPS traces in a specific CSV format, with few prerequisites, and use
   * them to simulate the presence of N vehicles from pre-recorded real GPS traces.
   * For the time being, the CSV format should be equal to the one available in "/src/gps-tc/examples/GPS-Traces-Sample" and
   * the positioning updates should occur, if possible at least at around 5-10 Hz (1 Hz would not be enough for an effective
   * dynamic frequency management for the CAMs).
   *
   * In this example the generated vehicles will simply broadcast their CAMs, relying on the application named simpleCAMSender-gps.
   * 802.11p is used as access technology.
   *
   * If the correct prerequisites for ns-3 PyViz are installed, the user can also see the moving nodes in a GUI by running this
   * example with the "--vis" option.
   */

  // Admitted data rates for 802.11p
  std::vector<float> rate_admitted_values{3,4.5,6,9,12,18,24,27};
  std::string datarate_config;

  /*** 0.a App Options ***/
  std::string trace_file_path = "src/gps-tc/examples/GPS-Traces-Sample/";
  std::string gps_trace = "sampletrace.csv";

  bool verbose = false;
  bool realtime = false;
  int txPower=26;
  float datarate=12;

  double simTime = 100;

  uint32_t nodeCounter = 0;

  CommandLine cmd;

  /* Cmd Line option for application */
  cmd.AddValue ("realtime", "Use the realtime scheduler or not", realtime);
  cmd.AddValue ("trace-folder","Position of GPS trace files",trace_file_path);
  cmd.AddValue ("gps-trace", "Name of the GPS trace file", gps_trace);

  /* Cmd Line option for 802.11p */
  cmd.AddValue ("tx-power", "OBUs transmission power [dBm]", txPower);
  cmd.AddValue ("datarate", "802.11p channel data rate [Mbit/s]", datarate);

  cmd.AddValue("sim-time", "Total duration of the simulation [s]", simTime);

  cmd.Parse (argc, argv);

  if(std::find(rate_admitted_values.begin(), rate_admitted_values.end(), datarate) == rate_admitted_values.end())
    {
      NS_FATAL_ERROR("Fatal error: invalid 802.11p data rate" << datarate << "Mbit/s. Valid rates are: 3, 4.5, 6, 9, 12, 18, 24, 27 Mbit/s.");
    }
  else
    {
      if(datarate==4.5)
        {
          datarate_config = "OfdmRate4_5MbpsBW10MHz";
        }
      else
        {
          datarate_config = "OfdmRate" + std::to_string((int)datarate) + "MbpsBW10MHz";
        }
    }

  if (verbose)
    {
      LogComponentEnable ("v2v-80211p-gps-tc", LOG_LEVEL_INFO);
      LogComponentEnable ("CABasicService", LOG_LEVEL_INFO);
      LogComponentEnable ("DENBasicService", LOG_LEVEL_INFO);
    }

  /* Use the realtime scheduler of ns3 */
  if(realtime)
      GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /*** 0.b Read from the GPS Trace and create a GPS Trace Client for each vehicle
  ***/
  NS_LOG_INFO("Reading the .rou file...");
  std::string path = trace_file_path + gps_trace;

  std::map<std::string,GPSTraceClient*> GPSTCMap;

  GPSTraceClientHelper GPSTCHelper;

  GPSTCHelper.setVerbose(verbose);

  GPSTCMap=GPSTCHelper.createTraceClientsFromCSV(path);

  int numberOfNodes=GPSTCMap.size ();
  NS_LOG_INFO("The .rou file has been read: " << numberOfNodes << " vehicles will be present in the simulation.");

  /* Set the simulation time (in seconds) */
  NS_LOG_INFO("Simulation will last " << simTime << " seconds");
  ns3::Time simulationTime (ns3::Seconds(simTime));

  /*** 1. Create containers for OBUs ***/
  NodeContainer obuNodes;
  obuNodes.Create(numberOfNodes);

  /*** 2. Create and setup channel   ***/
  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set ("TxPowerStart", DoubleValue (txPower));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (txPower));
  NS_LOG_INFO("Setting up the 802.11p channel @ " << datarate << " Mbit/s, 10 MHz, and tx power " << (int)txPower << " dBm.");

  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  /* To be removed when BPT is implemented */
  //Config::SetDefault ("ns3::ArpCache::DeadTimeout", TimeValue (Seconds (1)));

  /*** 3. Create and setup MAC ***/
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (datarate_config), "ControlMode", StringValue (datarate_config));
  NetDeviceContainer netDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, obuNodes);

  /*** 4. Create Internet and ipv4 helpers ***/
  PacketSocketHelper packetSocket;
  packetSocket.Install (obuNodes);

  /*** 6. Setup Mobility and position node pool ***/
  MobilityHelper mobility;
  mobility.Install (obuNodes);

  /*** 7. Setup interface and application for dynamic nodes ***/
  simpleCAMSenderHelper SimpleCAMSenderHelper;
  SimpleCAMSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));

  // Create vector with the GPS Trace Client map values
  std::vector<GPSTraceClient*> v_gps_tc;
  GPS_TC_MAP_ITERATOR(GPSTCMap,GPSTCit) {
    v_gps_tc.push_back (GPS_TC_IT_OBJECT(GPSTCit));
  }

  /* callback function for node creation */
  std::function<Ptr<Node> ()> setupNewWifiNode = [&] () -> Ptr<Node>
    {

      if (nodeCounter >= obuNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      Ptr<Node> includedNode = obuNodes.Get(nodeCounter);

      /* Install Application */
      SimpleCAMSenderHelper.SetAttribute ("GPSClient", PointerValue(v_gps_tc[nodeCounter]));
      ApplicationContainer setupAppSimpleSender = SimpleCAMSenderHelper.Install (includedNode);

      setupAppSimpleSender.Start (Seconds (0.0));
      setupAppSimpleSender.Stop (simulationTime - Simulator::Now () - Seconds (0.1));
      ++nodeCounter; // increment counter for next node
      return includedNode;
    };

  /* callback function for node shutdown */
  std::function<void (Ptr<Node>)> shutdownWifiNode = [] (Ptr<Node> exNode)
    {
      /* stop all applications */
      Ptr<simpleCAMSender> AppSimpleSender_ = exNode->GetApplication(0)->GetObject<simpleCAMSender>();

      if(AppSimpleSender_)
        AppSimpleSender_->StopApplicationNow();

       /* set position outside communication range */
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      /* NOTE: further actions could be required for a safe shut down! */
    };

  // Start "playing" the GPS Trace for each vehicle (i.e. make the vehicle start their movements)
  // "Seconds(0)" is specified to "playTrace" to reproduce all the traces since the beginning
  // of the simulation. A different amount of time for all the vehicles or a different amount of time
  // for each vehicle can also be specified to delay the reproduction of the traces
  GPS_TC_MAP_ITERATOR(GPSTCMap,GPSTCit) {
      GPS_TC_IT_OBJECT(GPSTCit)->GPSTraceClientSetup(setupNewWifiNode,shutdownWifiNode);
      GPS_TC_IT_OBJECT(GPSTCit)->playTrace(Seconds(0));
  }

  /*** 8. Start Simulation ***/
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
