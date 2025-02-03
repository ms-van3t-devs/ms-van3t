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


//#include "ns3/automotive-module.h"
#include "ns3/carla-module.h"
#include "ns3/OpenCDAClient.h"
#include "ns3/simpleCAMSender-gps-tc.h"
#include "ns3/simpleVAMSender-gps-tc.h"
#include "ns3/simpleCAMSender-helper.h"
#include "ns3/simpleVAMSender-helper.h"
#include "ns3/gps-tc-module.h"
#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include "ns3/mobility-module.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/vehicle-visualizer-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("v2v-80211p-gps-tc-example");

int
main (int argc, char *argv[])
{
  /*
   * In this example instead of relying on SUMO, the vehicles' mobility will be managed by the "gps-tc" module.
   * "gps-tc" is a module that is able to load GPS traces in a specific CSV format, with few prerequisites, and use
   * them to simulate the presence of N vehicles from pre-recorded real GPS traces.
   * For the time being, the CSV format should be similar to the one available in "/src/gps-tc/examples/GPS-Traces-Sample" and
   * the positioning updates should occur, if possible, at least at around 5-10 Hz (1 Hz may impact the effectiveness of the dynamic
   * CAM generation). You can also select an example trace, however, taken from a low-end GNSS device, embedded inside a consumer
   * smartphone (in this case the trace is taken at 1 Hz).
   * This also showcases how the GPS Trace Client module is able to accept as input any kind of CSV trace, no matter if it is taken
   * with more professional devices (updating at least at 5-10 Hz - always the best option) or if it comes from the user's smartphone,
   * updating at 1 Hz (which can be used, for instance, to analyze the behaviour of a vehicular application when non-professional and
   * cheap GNSS devices are used, providing a low update rate).
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
  // If you want to use a sample GPS trace obtained using a high-end expensive device (with
  // high update rate and inertial sensors - 3 vehicles), uncomment this line (and comment
  // the "BiellaTrace.csv" one)
  std::string gps_trace = "union_long.csv";
  // If you want to use a sample GPS trace obtained using a normal smartphone, with the
  // Ultra GPS Logger app (update rate: 1 Hz, no acceleration, 2 vehicles), uncomment this
  // line (and comment the "sampletrace.csv" one)
  // std::string gps_trace = "BiellaTrace.csv";

  bool verbose = false;
  bool realtime = false;
  int txPower=26;
  float datarate=12;
  bool vehicle_vis = false;

  double simTime = 500;

  uint32_t nodeCounter = 0;

  CommandLine cmd;

  /* Cmd Line option for application */
  cmd.AddValue ("realtime", "Use the realtime scheduler or not", realtime);
  cmd.AddValue ("trace-folder","Position of GPS trace files",trace_file_path);
  cmd.AddValue ("gps-trace", "Name of the GPS trace file", gps_trace);
  cmd.AddValue ("vehicle-visualizer", "Activate the web-based vehicle visualizer for ms-van3t", vehicle_vis);

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

  /* Create and setup the web-based vehicle visualizer of ms-van3t */
  vehicleVisualizer vehicleVisObj;
  Ptr<vehicleVisualizer> vehicleVis = &vehicleVisObj;
  if (vehicle_vis)
  {
      vehicleVis->startServer();
      vehicleVis->connectToServer ();

      GPSTCHelper.setVehicleVisualizer(vehicleVis);
  }

  // If you want to use set up the GPS Trace Client Helper to read data from "BiellaTrace.csv", obtainer on a regular smartphone with
  // the Ultra GPS Logger app (update rate: 1 Hz, no acceleration), uncomment these lines
  // They will set the right columns names for the data inside the CSV file and turn on interpolation to get a more "smooth" trace
//  GPSTCHelper.setVehicleIDColumnName ("VehID");
//  GPSTCHelper.setLatitudeColumnName ("Lat");
//  GPSTCHelper.setLongitudeColumnName ("Lng");
//  // As not all the apps/low-end GNSS devices are able to output an acceleration value, you can also specify
//  // "unavailable": in this case, the acceleration will be estimated, between points, as contant, and calculated simply as delta(v)/delta(t)
//  // Of course this is an approximation of the acceleration and may not reflect the actual acceleration experienced by the vehicle
//  // in between two data points
//  GPSTCHelper.setAccelerationColumnName ("unavailable");
//  GPSTCHelper.setHeadingColumnName ("Bearing");
//  GPSTCHelper.setSpeedColumnName ("Speed");
//  GPSTCHelper.setTimestampColumnName ("TimeWithMS",true,"%H:%M:%S.%MSEC",{.tm_year=2020,.tm_mon=9,.tm_mday=28});

  GPSTCHelper.setVerbose (false);

  // Important to set the input time in microseconds if the trace has timestamps in microseconds (to avoid the conversion)
  GPSTCHelper.SetInputMicroseconds(true);

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

  wifiPhy.EnablePcap ("v2v-tracenx-long", netDevices);
  /*** 4. Create Internet and ipv4 helpers ***/
  PacketSocketHelper packetSocket;
  packetSocket.Install (obuNodes);

  /*** 6. Setup Mobility and position node pool ***/
  MobilityHelper mobility;
  mobility.Install (obuNodes);

  /*** 7. Setup interface and application for dynamic nodes ***/
  simpleCAMSenderHelper SimpleCAMSenderHelper;
  SimpleCAMSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));

  simpleVAMSenderHelper SimpleVAMSenderHelper;
  SimpleVAMSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));

  // Create vector with the GPS Trace Client map values
  std::vector<GPSTraceClient*> v_gps_tc;
  GPS_TC_MAP_ITERATOR(GPSTCMap,GPSTCit) {
    v_gps_tc.push_back (GPS_TC_IT_OBJECT(GPSTCit));
  }

  /* callback function for node creation */
  STARTUP_GPS_FCN setupNewWifiNode = [&] (std::string vehicleID) -> Ptr<Node>
    {

      if (nodeCounter >= obuNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      Ptr<Node> includedNode = obuNodes.Get(nodeCounter);

      /* Install Application */
      ApplicationContainer setupAppSimpleSender;
      if (v_gps_tc[nodeCounter]->getType() == "car")
        {
          SimpleCAMSenderHelper.SetAttribute ("GPSClient", PointerValue(v_gps_tc[nodeCounter]));
          setupAppSimpleSender = SimpleCAMSenderHelper.Install (includedNode);
        }
      else if (v_gps_tc[nodeCounter]->getType() == "vru")
        {
          SimpleVAMSenderHelper.SetAttribute ("GPSClient", PointerValue(v_gps_tc[nodeCounter]));
          setupAppSimpleSender = SimpleVAMSenderHelper.Install (includedNode);
        }
      setupAppSimpleSender.Start (Seconds (0.0));
      setupAppSimpleSender.Stop (simulationTime - Simulator::Now () - Seconds (0.1));
      ++nodeCounter; // increment counter for next node
      return includedNode;
    };

  // We can create a variable here and use it in the lambda functions below and main() will not terminate before the whole
  // simulation is terminated (thus the value of remainingNodes will always be accessible)
  int remainingNodes = obuNodes.GetN();

  /* callback function for node shutdown */
  SHUTDOWN_GPS_FCN shutdownWifiNode = [&] (Ptr<Node> exNode,std::string vehicleID)
    {
      /* stop all applications */
      Ptr<simpleCAMSender> AppSimpleSender_ = exNode->GetApplication(0)->GetObject<simpleCAMSender>();

      if(AppSimpleSender_) {
          AppSimpleSender_->StopApplicationNow();
          remainingNodes--;
      }

      if(remainingNodes==0) {
          Simulator::Stop();
      }

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
