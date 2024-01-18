// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#include <string>
#include "OpenCDAClient.h"
#include <unistd.h> // For usleep
#include <fcntl.h>
namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("OpenCDAClient");

  TypeId
  OpenCDAClient::GetTypeId(void)
  {
    static TypeId tid =
        TypeId("ns3::OpenCDAClient")
        .SetParent<Object>()
        .SetGroupName ("OpenCDAClient")
    .AddAttribute ("UpdateInterval",
                  "CARLA update interval",
                  DoubleValue (1.0),
                  MakeDoubleAccessor (&OpenCDAClient::m_updateInterval),
                  MakeDoubleChecker <double>())
    .AddAttribute ("CARLAHost",
                  "CARLA host",
                  StringValue ("localhost"),
                  MakeStringAccessor (&OpenCDAClient::m_host),
                  MakeStringChecker ())
    .AddAttribute ("PenetrationRate",
                  "Rate of vehicles, equipped with wireless communication devices",
                  DoubleValue (1.0),
                  MakeDoubleAccessor (&OpenCDAClient::m_penetration_rate),
                  MakeDoubleChecker <double>())
    .AddAttribute ("OpenCDA_HOME",
                   "OpenCDA",
                   StringValue("ms_van3t_example"),
                   MakeStringAccessor (&OpenCDAClient::m_opencda_home),
                   MakeStringChecker ())
    .AddAttribute ("PythonInterpreter",
                   "Python interpreter used to execute OpenCDA",
                   StringValue("python3.7"),
                   MakeStringAccessor (&OpenCDAClient::m_python_interpreter),
                   MakeStringChecker ())
    .AddAttribute ("OpenCDA_config",
                   "Name of opencda script",
                   StringValue("ms_van3t_example"),
                   MakeStringAccessor (&OpenCDAClient::m_opencda_config),
                   MakeStringChecker ())
    .AddAttribute ("ApplyML",
                   "Apply machine learning model for OpenCDA perception module.",
                   BooleanValue (false),
                   MakeBooleanAccessor(&OpenCDAClient::m_apply_ml),
                   MakeBooleanChecker ())
    .AddAttribute ("CARLA_HOME",
                   "Location of CARLA installation",
                   StringValue("CARLA_0.9.12"),
                   MakeStringAccessor (&OpenCDAClient::m_carla_home),
                   MakeStringChecker ())
    .AddAttribute ("Port",
                  "Port for connection.",
                  UintegerValue (1337),
                  MakeUintegerAccessor (&OpenCDAClient::m_port),
                  MakeUintegerChecker<uint32_t> ());

  ;
    return tid;
  }

  OpenCDAClient::OpenCDAClient(void)
  {
      NS_LOG_FUNCTION(this);
      m_updateInterval = 0.05;
      m_host = "localhost";
      m_port = 1338;
      m_adapter_debug = false;
      m_randVar = CreateObject<UniformRandomVariable>();
      m_randVar->SetAttribute("Min", DoubleValue(0.0));
      m_randVar->SetAttribute("Max", DoubleValue(1.0));
  }

  OpenCDAClient::~OpenCDAClient(void)
  {
      this->stopSimulation ();
      std::cout<<"OpenCDAClient object destroyed." << std::endl;
      NS_LOG_INFO("OpenCDAClient object destroyed.");
  }

  void
  OpenCDAClient::stopSimulation ()
  {
    kill(-m_opencda_pid, SIGKILL);
    kill(-m_carla_pid, SIGKILL);
  }
  void
  OpenCDAClient::startCarlaAdapter(std::function<Ptr<Node>(std::string)> includeNode, std::function<void (Ptr<Node>,std::string)> excludeNode)
  {
      m_includeNode = includeNode;
      m_excludeNode = excludeNode;
      if(!m_adapter_debug)
        {
          m_carla_pid = fork();

          if (m_carla_pid == 0) {

              std::string carla_command = "cd "+ m_carla_home +" && ./CarlaUE4.sh -prefernvidia";

              setpgid(0, 0); // Create a new process group

              std::ifstream file("/tmp/carla_output.txt");
              if (file.good()) {
                 file.close();
                 remove("/tmp/carla_output.txt");
               }

              std::cout <<"Executing: " << (carla_command).c_str() << std::endl;

              // Open a temporary file for CARLA output
              int fd = open("/tmp/carla_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
              if (fd < 0) {
                  perror("Opening temp file for CARLA output failed");
                  exit(1);
                }
              dup2(fd, STDOUT_FILENO);
              dup2(fd, STDERR_FILENO);
              close(fd);

              int r = execl("/bin/sh", "sh", "-c", (carla_command).c_str(), NULL);

              std::cout << "execl returned: " << r << std::endl;

              if (r == -1) {
                  NS_FATAL_ERROR("Error.system failed");
              }
              if (WEXITSTATUS(r) != 0) {
                  NS_FATAL_ERROR("Error.cannot run");
              }
              NS_FATAL_ERROR("Error.returned from exec");
              exit(1);
          }

          usleep(10000000);
          bool carla_ready=false;
          int carla_waits=0;
          while (!carla_ready && carla_waits < 5) {
                  std::ifstream file("/tmp/carla_output.txt");
                  if (file.good()) {
                      carla_ready = true;
                      file.close();
                      std::cout << "CARLA server ready." << std::endl;
                  } else {
                      // File does not exist, wait for 1 second before checking again
                      std::cout << "Waiting for CARLA server to be ready." << std::endl;
                      usleep(10000000);
                      carla_waits++;
                  }
              }
          if(!carla_ready)
            {
              NS_FATAL_ERROR("Could not start CARLA server.");
            }

          m_opencda_pid = fork();

          if (m_opencda_pid == 0) {
              std::string opencda_command = "cd " + m_opencda_home +" && " + m_python_interpreter +" -u opencda.py -t " + m_opencda_config + " -v 0.9.12";
              if (m_apply_ml)
                {
                  opencda_command = opencda_command + " --apply_ml";
                }

              setpgid(0, 0); // Create a new process group

              std::ifstream file("/tmp/opencdaCI_ready");
              if (file.good()) {
                 file.close();
                 remove("/tmp/opencdaCI_ready");
               }
              std::cout << "Executing: " << (opencda_command).c_str() << std::endl;
              // Open a temporary file for CARLA output
              int fd = open("/tmp/opencda_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
              if (fd < 0) {
                  perror("Opening temp file for OpenCDA output failed");
                  exit(1);
              }
              dup2(fd, STDOUT_FILENO);
              dup2(fd, STDERR_FILENO);
              close(fd);

              int r = execl("/bin/sh", "sh", "-c", (opencda_command).c_str(), NULL);
              std::cout << "execl returned: " << r << std::endl;

              if (r == -1) {
                  NS_FATAL_ERROR("Error.system failed");
              }
              if (WEXITSTATUS(r) != 0) {
                  NS_FATAL_ERROR("Error.cannot run");
              }
              NS_FATAL_ERROR("Error.returned from exec");

              exit(1);
          }

        }
      Simulator::Schedule(Seconds(m_updateInterval), &OpenCDAClient::startSimulation, this);
  }

  void
  OpenCDAClient::startSimulation()
  {
      std::cout <<"OpenCDAClient::startSimulation()" << std::endl;
      usleep(10000000);
      bool server_ready = false;
      while (!server_ready) {
              std::ifstream file("/tmp/opencdaCI_ready");
              if (file.good()) {
                  server_ready = true;
                  file.close();
                  std::cout << "OpenCDA Control Interface ready." << std::endl;
              } else {
                  // File does not exist, wait for 1 second before checking again
                  std::cout << "Waiting for OpenCDA Control Interface to be ready." << std::endl;
                  usleep(10000000);
              }
          }
      std::string hostAndPort = m_host+":" + std::to_string(m_port);
      m_channel = CreateChannel(hostAndPort, grpc::InsecureChannelCredentials());
      m_stub = carla::CarlaAdapter::NewStub(m_channel);
      auto state = m_channel->GetState(true);

      while (state != GRPC_CHANNEL_READY) {
          if (!m_channel->WaitForStateChange(state, std::chrono::system_clock::now() + std::chrono::seconds(15))) {
              kill(-m_opencda_pid, SIGKILL);
              kill(-m_carla_pid, SIGKILL);
              NS_FATAL_ERROR("Could not connect to gRPC");
          }
          state = m_channel->GetState(true);
          if (state == GRPC_CHANNEL_IDLE) {
              std::cout << "I do have a GRPC_CHANNEL_IDLE in WHILE -> " << state << std::endl;
          }
          else if (state == GRPC_CHANNEL_CONNECTING) {
              std::cout << "I do have a GRPC_CHANNEL_CONNECTING in WHILE -> " << state << std::endl;
          }
          else if (state == GRPC_CHANNEL_READY) {
              std::cout << "I do have a GRPC_CHANNEL_READY in WHILE -> " << state << std::endl;
          }
          else if (state == GRPC_CHANNEL_TRANSIENT_FAILURE) {
              std::cout << "I do have a GRPC_CHANNEL_TRANSIENT_FAILURE in WHILE -> " << state << std::endl;
          }
          else if (state == GRPC_CHANNEL_SHUTDOWN) {
              std::cout << "I do have a GRPC_CHANNEL_SHUTDOWN in WHILE -> " << state << std::endl;
          }
      }

      // Add all the Actors that are already part of the simulation
      carla::ActorIds actorIds = GetManagedHostIds();
      for (int i = 0; i < actorIds.actorid_size(); i++) {

          if (m_randVar->GetValue() <= m_penetration_rate || hasCARLALDM (actorIds.actorid(i)))
            {
              int actorId = actorIds.actorid(i);
              carla::Vehicle vehicle = GetManagedActorById(actorId);
              //printVehicle(vehicle);
              Vector location;
              location.x = vehicle.location().x();
              location.y = vehicle.location().y();
              location.z = vehicle.location().z();
              //std::cout << "Adding vehicle with location x = " << location.x << " y = " << location.y << " z = " << location.z << std::endl;

              Ptr<ns3::Node> inNode = m_includeNode(std::to_string(actorId));
              m_vehMap.insert(std::pair<int, Ptr<Node>>(actorId, inNode));
              Ptr<MobilityModel> mob = m_vehMap.at(actorId)->GetObject<MobilityModel>();
              // set ns3 node position
              mob->SetPosition(Vector(location.x, location.y, 1.5));
            }
          else
            {
              m_nonCVIDs.push_back (actorIds.actorid(i));  // To keep track of non Connected Vehicles
            }
      }
      executeOneTimestep();
      //this->testInsertVehicle ();
  }

  void
  OpenCDAClient::testInsertVehicle()
  {
      carla::Vehicle vehicle;
      carla::Transform v = getRandomSpawnPoint();
      carla::Vector* pos = new carla::Vector();
      pos->set_x(v.location().x());
      pos->set_y(v.location().y());
      pos->set_z(v.location().z());

      vehicle.set_allocated_location(pos);
      insertVehicle(vehicle);
  }


  void
  OpenCDAClient::UpdateVehicleFileMap(void)
  {
    NS_LOG_FUNCTION(this);

    try
      {
        // iterate over all sumo vehicles in map
        for (std::map<int, Ptr<Node> >::iterator it = m_vehMap.begin(); it != m_vehMap.end(); ++it)
          {
            // get current sumo vehicle from map
            std::string veh(std::to_string (it->first));

            auto it_files = m_fileMap.find (veh);

            if(it_files == m_fileMap.end())
              {
                std::string filename = "carla_data_" + veh + ".csv";
                // Create and open a new file stream and store it in the map
                std::ofstream file(filename);
                if(!file.is_open()) {
                    std::cerr << "Failed to create file: " << filename << std::endl;
                }
                m_fileMap[veh] = std::move(file);
                m_fileMap[veh]  << "time,posX,posY,speed,acceleration" << std::endl;
              }
            else{
                // get vehicle position from carla
                carla::Vehicle egoVehicle = GetManagedActorById (it->first);

                it_files->second << (double) Simulator::Now().GetMilliSeconds ()<< ","
                                 << egoVehicle.location ().x ()<< ","
                                 << egoVehicle.location ().y () << ","
                                 << sqrt(pow(egoVehicle.speed ().x (),2) + pow(egoVehicle.speed ().y (),2))<< ","
                                 << sqrt(pow(egoVehicle.acceleration ().x (),2) + pow(egoVehicle.acceleration ().y (),2))
                                 << std::endl;
              }
          }
      }
    catch (std::exception& e)
      {
        NS_FATAL_ERROR("SUMO was closed unexpectedly while writing vehicle info into file: " << e.what());
      }

  }

  void
  OpenCDAClient::printVehicle(carla::Vehicle vehicle)
  {
      std::cout << "=========================================" << std::endl;
      std::cout << vehicle.acceleration().x() << std::endl;
      std::cout << "Vehicle" << vehicle.id() << "\n"
          << "Acceleration.x = " << vehicle.acceleration().x() << "\n"
          << "Acceleration.y = " << vehicle.acceleration().y() << "\n"
          << "Acceleration.z = " << vehicle.acceleration().z() << "\n"
          << "Speed.x = " << vehicle.speed().x() << "\n"
          << "Speed.y = " << vehicle.speed().y() << "\n"
          << "Speed.z = " << vehicle.speed().z() << "\n"
          << "Position.x = " << vehicle.location().x() << "\n"
          << "Position.y = " << vehicle.location().y() << "\n"
          << "Position.z = " << vehicle.location().z() <<std::endl;
      std::cout << "=========================================" << std::endl;
  }

  carla::Vehicle
  OpenCDAClient::GetManagedActorById(int actorId)
  {
      carla::Vehicle vehicle;
      carla::Number vehicleId;
      grpc::ClientContext clientContext;
      vehicleId.set_num(actorId);

      grpc::Status status = m_stub->GetManagedActorById(&clientContext, vehicleId, &vehicle);
      if (status.ok()) {
          return vehicle;
      }
      else {
          NS_FATAL_ERROR((std::string("OpenCDAClient::GetManagedActorById() failed with error: " + std::string(status.error_message())).c_str()));
      }
  }

  void
  OpenCDAClient::insertVehicle(carla::Vehicle request)
  {
      carla::Number vehicleId;
      grpc::ClientContext clientContext;
      std::cout << "Inserting vehicle at location: " << request.location().x() << "/" << request.location().y() << "/" << request.location().z() << std::endl;
      grpc::Status status = m_stub->InsertVehicle(&clientContext, request, &vehicleId);
      if (status.ok()) {
          std::cout << "New vehicle inserted succesfully" << std::endl;
      }
      else {
          NS_FATAL_ERROR((std::string("OpenCDAClient::insertVehicle() failed with error: " + std::string(status.error_message())).c_str()));
      }
      Vector pos;
      pos.x = request.location().x();
      pos.y = request.location().y();
      pos.z = request.location().z();

      Ptr<ns3::Node> inNode = m_includeNode(std::to_string(vehicleId.num()));
      m_vehMap.insert(std::pair<int, Ptr<Node>>(vehicleId.num(), inNode));
      Ptr<MobilityModel> mob = m_vehMap.at(vehicleId.num())->GetObject<MobilityModel>();
      // set ns3 node position
      mob->SetPosition(Vector(pos.x, pos.y, pos.z));

  }

  carla::Transform
  OpenCDAClient::getRandomSpawnPoint()
  {
      google::protobuf::Empty request;
      carla::Transform response;
      grpc::ClientContext clientContext;
      grpc::Status status = m_stub->GetRandomSpawnPoint(&clientContext, request, &response);
      if (status.ok()) {
          std::cout << "getRandomSpawnPoint:" << response.location().x() << "/" << response.location().y() << "/" << response.location().z() << std::endl;
          return response;
      }
      else {
          NS_FATAL_ERROR((std::string("OpenCDAClient::getRandomSpawnPoint() failed with error: " + std::string(status.error_message())).c_str()));
      }
  }


  void
  OpenCDAClient::executeOneTimestep()
  {
      //std::cout << Simulator::Now().GetSeconds () << ": executeOneTimestep()" << std::endl;
      google::protobuf::Empty responseEmpty;
      carla::Boolean retval;
      google::protobuf::Empty empty;
      grpc::ClientContext clientContext;
      grpc::Status status = m_stub->ExecuteOneTimeStep(&clientContext, empty, &retval);

      //if (status.ok())
      if(!status.ok())
        {
          NS_FATAL_ERROR((std::string("OpenCDAClient::executeOneTimestep() failed with error: " + std::string(status.error_message())).c_str()));
        }
      if (retval.value ()) {

          carla::ActorIds actors = GetManagedHostIds();
          for (int i = 0; i < actors.actorid_size(); i++) {
              auto actorId = actors.actorid(i);
              carla::Vehicle v = GetManagedActorById(actorId);
              Vector location;
              location.x = v.location().x();
              location.y = v.location().y();
              location.z = v.location().z();
              double speed = 0; // speed and angle not considered by current ms-van3t channel models
              double angle = 0;
              processVehicleSubscription(v.id(), location, speed, angle);
          }
          UpdateVehicleFileMap();
          m_executeOneTimestepTrigger = Simulator::Schedule(Seconds(m_updateInterval), &OpenCDAClient::executeOneTimestep, this);

      }
      else {
          kill(-m_opencda_pid, SIGTERM);
          kill(-m_carla_pid, SIGKILL);
          Simulator::Stop ();
          std::cout << "OpenCDA simulation finished unexpectedly, check /tmp/opencda_output.txt to see OpenCDA logs" << std::endl;
        }
  }

  void
  OpenCDAClient::processVehicleSubscription(int actorId, Vector location, double speed, double angle)
  {

      if (m_vehMap.find(actorId) == m_vehMap.end()) {
          //New vehicle
          if(std::find(m_nonCVIDs.begin(), m_nonCVIDs.end(), actorId) == m_nonCVIDs.end())
            {
              if (m_randVar->GetValue() <= m_penetration_rate || hasCARLALDM (actorId))
                {
                  carla::Vehicle vehicle = GetManagedActorById(actorId);
                  printVehicle(vehicle);
                  Vector location;
                  location.x = vehicle.location().x();
                  location.y = vehicle.location().y();
                  location.z = vehicle.location().z();
                  //std::cout << "Adding vehicle with location x = " << location.x << " y = " << location.y << " z = " << location.z << std::endl;

                  Ptr<ns3::Node> inNode = m_includeNode(std::to_string(actorId));
                  m_vehMap.insert(std::pair<int, Ptr<Node>>(actorId, inNode));
                  Ptr<MobilityModel> mob = m_vehMap.at(actorId)->GetObject<MobilityModel>();
                  // set ns3 node position
                  mob->SetPosition(Vector(location.x, location.y, location.z));
                }
              else
                {
                  m_nonCVIDs.push_back (actorId);  // To keep track of non Connected Vehicles
                }
            }
          // IF found in nonCVIDs --> we skip
      }
      else {
          // Node existed - update position
          Ptr<MobilityModel> mob = m_vehMap.at(actorId)->GetObject<MobilityModel>();
          // set ns3 node position
          mob->SetPosition(Vector(location.x, location.y, location.z));
      }
  }

  carla::ActorIds
  OpenCDAClient::GetManagedHostIds()
  {
      carla::ActorIds actorIds;
      google::protobuf::Empty empty;
      grpc::ClientContext clientContext;
      grpc::Status status = m_stub->GetManagedActorsIds(&clientContext, empty, &actorIds);

      if (!status.ok()) {
          NS_FATAL_ERROR((std::string("OpenCDAClient::GetManagedHostIds() failed with error: " + std::string(status.error_message())).c_str()));
      }
      return actorIds;
  }


  carla::Vector
  OpenCDAClient::getCartesian (double lon, double lat)
  {
      carla::Vector pos, geoPos;
      grpc::ClientContext clientContext;
      geoPos.set_x (lat);
      geoPos.set_y (lon);
      geoPos.set_z (0.0);

      grpc::Status status = m_stub->GetCartesian (&clientContext, geoPos, &pos);
      if (status.ok()) {
          return pos;
      }
      else {
          NS_FATAL_ERROR((std::string("OpenCDAClient::GetCartesian() failed with error: " + std::string(status.error_message())).c_str()));
      }
  }

  carla::Vector
  OpenCDAClient::getGeo (double x, double y)
  {
      carla::Vector pos, cartesianPos;
      grpc::ClientContext clientContext;
      cartesianPos.set_x (x);
      cartesianPos.set_y (y);
      cartesianPos.set_z (0.0);

      grpc::Status status = m_stub->GetGeo (&clientContext, cartesianPos, &pos);
      if (status.ok()) {
          return pos;
      }
      else {
          NS_FATAL_ERROR((std::string("OpenCDAClient::GetGeo() failed with error: " + std::string(status.error_message())).c_str()));
      }
  }

  double
  OpenCDAClient::getSpeed (int id)
  {
    carla::Vehicle vehicle = GetManagedActorById(id);
    double speed = sqrt(pow(vehicle.speed ().x (),2) + pow(vehicle.speed ().y (),2));
    return speed;
  }

  double
  OpenCDAClient::getHeading (int id)
  {
    carla::Vehicle vehicle = GetManagedActorById(id);
    return vehicle.heading ();
  }

  bool
  OpenCDAClient::hasCARLALDM (int id)
  {
    carla::Boolean retval;
    carla::Number vehicleId;
    vehicleId.set_num (id);
    grpc::ClientContext clientContext;

    grpc::Status status = m_stub->hasLDM (&clientContext, vehicleId, &retval);

    return retval.value ();
  }

  carla::Objects
  OpenCDAClient::getDetectedObjects (int id)
  {
    carla::Objects retObjects;
    carla::Number vehicleId;
    vehicleId.set_num (id);
    grpc::ClientContext clientContext;

    grpc::Status status = m_stub->GetActorLDM (&clientContext, vehicleId, &retObjects);

    return retObjects;
  }

  int
  OpenCDAClient::getVehicleID (Ptr<Node> node)
  {
    int retval = -1;
    for (std::map<int, Ptr<Node>>::iterator it = m_vehMap.begin (); it != m_vehMap.end (); it++)
      {
        if (it->second == node)
          {
            retval = it->first;
            break;
          }
      }
    return retval;
  }

  bool
  OpenCDAClient::InsertObject (carla::ObjectIn object)
  {
    grpc::ClientContext clientContext;
    carla::Number ret;
    bool ret_b = false;
    grpc::Status status = m_stub->InsertObject (&clientContext, object, &ret);
    if (!status.ok()) {
        NS_FATAL_ERROR((std::string("OpenCDAClient::InsertObject () failed with error: " + std::string(status.error_message())).c_str()));
    }
    if(ret.num () == 1)
      ret_b = true;

    return ret_b;
  }

  void
  OpenCDAClient::setControl(int id, double speed, Vector position, double acceleration)
  {
    grpc::ClientContext clientContext;
    carla::Control control;
    carla::Vector* pos = new carla::Vector();
    google::protobuf::Empty responseEmpty;
    pos->set_x(position.x);
    pos->set_y(position.y);
    pos->set_z(position.z);
    control.set_allocated_waypoint (pos);
    control.set_id (id);
    control.set_speed (speed);
    control.set_acceleration (acceleration);

    grpc::Status status = m_stub->SetControl (&clientContext, control, &responseEmpty);
    if (!status.ok()) {
        NS_FATAL_ERROR((std::string("OpenCDAClient::setControl() failed with error: " + std::string(status.error_message())).c_str()));
    }

  }

  carla::ActorIds
  OpenCDAClient::GetManagedCAVsIds()
  {
    carla::ActorIds actorIds;
    google::protobuf::Empty empty;
    grpc::ClientContext clientContext;
    grpc::Status status = m_stub->GetManagedCAVsIds (&clientContext, empty, &actorIds);

    if (!status.ok()) {
        NS_FATAL_ERROR((std::string("OpenCDAClient::GetManagedCAVsIds() failed with error: " + std::string(status.error_message())).c_str()));
    }
    return actorIds;

  }

  std::vector<int>
  OpenCDAClient::getManagedConnectedIds()
  {
    std::vector<int> ret_ids;

    for(auto it = m_vehMap.begin (); it != m_vehMap.end (); it++){
        ret_ids.push_back (it->first);
      }

    return ret_ids;
  }

}

