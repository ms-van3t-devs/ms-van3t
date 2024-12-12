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
 *  Giuseppe Avino, Politecnico di Torino (giuseppe.avino@polito.it)
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "ns3/core-module.h"
#include "ns3/gps-tc-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/vehicle-visualizer-module.h"
#include "ns3/traci-module.h"

using namespace ns3;


int 
main (int argc, char *argv[])
{
  bool verbose = false;
  double simTime = 100;
  bool realtime = false;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "Set a verbose mode to print additional GPSTraceClient-related information", verbose);
  cmd.AddValue ("sim-time", "Total duration of the simulation [s]", simTime);
  cmd.AddValue ("realtime", "Set to true to use the real-time scheduler", realtime);

  cmd.Parse (argc,argv);

  // Use the realtime scheduler of ns3, if required
  if(realtime)
  {
    GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  }

  // Set total simulation time
  ns3::Time simulationTime (ns3::Seconds(simTime));

  std::map<std::string,GPSTraceClient*> GPSTCMap;

  GPSTraceClientHelper GPSTCHelper;

  GPSTCHelper.setVerbose(verbose);

  /* Setup a VehicleVisualizer to show the vehicles moving on a map, following the specified CSV mobility traces */
  /* The VehicleVisualizer acts as a GUI for gps-tc */
  vehicleVisualizer vehicleVisObj;
  Ptr<vehicleVisualizer> vehicleVis = &vehicleVisObj;
  vehicleVis->startServer();
  vehicleVis->connectToServer ();

  GPSTCHelper.setVehicleVisualizer(vehicleVis);

  GPSTCMap=GPSTCHelper.createTraceClientsFromCSV("src/gps-tc/examples/GPS-Traces-Sample/sampletrace.csv");

  int numberOfNodes=GPSTCMap.size ();

  /*** 1. Create containers for OBUs ***/
  NodeContainer obuNodes;
  obuNodes.Create(numberOfNodes); //+1 for the server

  /*** 2. Setup Mobility and position node pool ***/
  MobilityHelper mobility;
  mobility.Install (obuNodes);

  uint32_t nodeCounter = 0;

  /* callback function for node creation */
  STARTUP_GPS_FCN setupNode = [&] (std::string vehicleID) -> Ptr<Node>
    {
      if (nodeCounter >= obuNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      /* don't create and install the protocol stack of the node at simulation time -> take from "node pool" */
      Ptr<Node> includedNode = obuNodes.Get(nodeCounter);
      ++nodeCounter; //increment counter for next node

      /* Install Application */
//      ApplicationContainer ClientApp = appClientHelper.Install (includedNode);
//      ClientApp.Start (Seconds (0.0));
//      ClientApp.Stop (simulationTime - Simulator::Now () - Seconds (0.1));

      return includedNode;
    };

  /* callback function for node shutdown */
  SHUTDOWN_GPS_FCN shutdownNode = [&] (Ptr<Node> exNode,std::string vehicleID)
    {
      /* stop all applications */
//      Ptr<appClient> appClient_ = exNode->GetApplication(0)->GetObject<appClient>();
//      if(appClient_)
//        appClient_->StopApplicationNow ();

       /* set position outside communication range */
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      /* NOTE: further actions could be required for a safe shut down! */
    };

      GPS_TC_MAP_ITERATOR(GPSTCMap,GPSTCit) {
      GPS_TC_IT_OBJECT(GPSTCit)->GPSTraceClientSetup(setupNode,shutdownNode);
      GPS_TC_IT_OBJECT(GPSTCit)->playTrace(Seconds(0));
  }


  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


