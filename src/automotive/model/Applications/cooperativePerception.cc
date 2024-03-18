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
 *  Carlos Mateo Risma Carletti, Politecnico di Torino (carlosrisma@gmail.com)
*/

#include "cooperativePerception.h"

#include "ns3/CAM.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/gn-utils.h"

#define DEG_2_RAD(val) ((val)*M_PI/180.0)

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("cooperativePerception");

  NS_OBJECT_ENSURE_REGISTERED(cooperativePerception);


  TypeId
  cooperativePerception::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::cooperativePerception")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<cooperativePerception> ()
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&cooperativePerception::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("IpAddr",
            "IpAddr",
            Ipv4AddressValue ("10.0.0.1"),
            MakeIpv4AddressAccessor (&cooperativePerception::m_ipAddress),
            MakeIpv4AddressChecker ())
        .AddAttribute ("PrintSummary",
            "To print summary at the end of simulation",
            BooleanValue(false),
            MakeBooleanAccessor (&cooperativePerception::m_print_summary),
            MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&cooperativePerception::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("Model",
            "Physical and MAC layer communication model",
            StringValue (""),
            MakeStringAccessor (&cooperativePerception::m_model),
            MakeStringChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&cooperativePerception::m_traci_client),
            MakePointerChecker<TraciClient> ())
        .AddAttribute ("OpenCDAClient",
            "OpenCDA client",
            PointerValue (0),
            MakePointerAccessor (&cooperativePerception::m_opencda_client),
            MakePointerChecker<OpenCDAClient> ())
        .AddAttribute ("PRRSupervisor",
            "PRR Supervisor to compute PRR according to 3GPP TR36.885 V14.0.0 page 70",
            PointerValue (0),
            MakePointerAccessor (&cooperativePerception::m_PRR_supervisor),
            MakePointerChecker<PRRSupervisor> ())
        .AddAttribute ("SendCAM",
            "To enable/disable the transmission of CAM messages",
            BooleanValue(true),
            MakeBooleanAccessor (&cooperativePerception::m_send_cam),
            MakeBooleanChecker ());
        return tid;
  }

  cooperativePerception::cooperativePerception ()
  {
    NS_LOG_FUNCTION(this);
    m_traci_client = nullptr;
    m_opencda_client = nullptr;
    m_print_summary = true;
    m_already_print = false;
    m_send_cam = true;

    m_cam_received = 0;
    m_cpm_received = 0;

    m_distance_threshold = 75; // Distance used in GeoNet to determine the radius of the circumference arounf the emergency vehicle where the DENMs are valid
    m_heading_threshold = 45; // Max heading angle difference between the normal vehicles and the emergenecy vehicle, that triggers a reaction in the normal vehicles
  }

  cooperativePerception::~cooperativePerception ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  cooperativePerception::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  cooperativePerception::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    /* Save the vehicles informations */
    VDP* vdp;
    if(m_traci_client != nullptr)
      {
        // SUMO mobility
        m_id = m_traci_client->GetVehicleId (this->GetNode ());
        m_type = m_traci_client->TraCIAPI::vehicle.getVehicleClass (m_id);

        vdp = new VDPTraCI(m_traci_client,m_id);

        //Create LDM and sensor object
        m_LDM = CreateObject<LDM>();
        m_LDM->setStationID(m_id);
        m_LDM->setTraCIclient(m_traci_client);
        m_LDM->setVDP(vdp);

        m_sumo_sensor = CreateObject<SUMOSensor>();
        m_sumo_sensor->setStationID(m_id);
        m_sumo_sensor->setTraCIclient(m_traci_client);
        m_sumo_sensor->setVDP(vdp);
        m_sumo_sensor->setLDM (m_LDM);

      }
    else if(m_opencda_client != nullptr)
      {
        int stationID = m_opencda_client->getVehicleID (this->GetNode ());
        m_id = std::to_string (stationID);
        m_type = StationType_passengerCar; //TODO: add support for multiple vehicle types

        vdp = new VDPOpenCDA(m_opencda_client, m_id);
        m_LDM = CreateObject<LDM>();
        m_LDM->setStationID(stationID);
        m_LDM->setVDP(vdp);
        //m_LDM->enableOutputFile (m_id);

        m_opencda_sensor = CreateObject<OpenCDASensor>();
        m_opencda_sensor->setStationID(stationID);
        m_opencda_sensor->setOpenCDAClient (m_opencda_client);
        m_opencda_sensor->setVDP(vdp);
        m_opencda_sensor->setLDM (m_LDM);
      }
    else
      {
        NS_FATAL_ERROR ("No mobility set - check simulation script - valid mobilities: 'SUMO' or 'CARLA'");
      }

    // Create new BTP and GeoNet objects and set them in DENBasicService and CABasicService
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();

    if(m_PRR_supervisor!=nullptr)
    {
      m_geoNet->setPRRSupervisor(m_PRR_supervisor);
    }

    m_btp->setGeoNet(m_geoNet);
    m_denService.setBTP(m_btp);
    m_caService.setBTP(m_btp);
    m_cpService.setBTP(m_btp);
    m_caService.setLDM(m_LDM);
    m_cpService.setLDM(m_LDM);

    /* Create the Sockets for TX and RX */
    TypeId tid;
    if(m_model=="80211p")
      tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
    else if(m_model=="cv2x" || m_model=="nrv2x")
      tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    else
      NS_FATAL_ERROR ("No communication model set - check simulation script - valid models: '80211p' or 'lte'");
    m_socket = Socket::CreateSocket (GetNode (), tid);

    if(m_model=="80211p")
    {
        /* Bind the socket to local address */
        PacketSocketAddress local = getGNAddress(GetNode ()->GetDevice (0)->GetIfIndex (),
                                                GetNode ()->GetDevice (0)->GetAddress () );
        if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind client socket for BTP + GeoNetworking (802.11p)");
        }
        // Set the socketAddress for broadcast
        PacketSocketAddress remote = getGNAddress(GetNode ()->GetDevice (0)->GetIfIndex (),
                                                GetNode ()->GetDevice (0)->GetBroadcast () );
        m_socket->Connect (remote);
    }
    else // m_model=="cv2x"
    {
        /* The C-V2X model requires the socket to be bind to "any" IPv4 address, and to be connected to the
         * IP address of the transmitting node. Then, the model will take care of broadcasting the packets.
        */
        if (m_socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), 19)) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind client socket for C-V2X");
        }
        m_socket->Connect (InetSocketAddress(m_ipAddress,19));
    }

    /* Set Station Type in DENBasicService */
    StationType_t stationtype;
    if (m_type=="passenger")
      stationtype = StationType_passengerCar;
    else if (m_type=="emergency"){
      stationtype = StationType_specialVehicle;
      //m_LDM->enablePolygons (); // Uncomment to enable detected object polygon visualization for this specific vehicle
      }
    else
      stationtype = StationType_unknown;

    /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
    m_caService.setSocketTx (m_socket);
    m_caService.setSocketRx (m_socket);
    m_caService.setStationProperties (std::stol(m_id), (long)stationtype);
    m_caService.addCARxCallback (std::bind(&cooperativePerception::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));
    m_caService.setRealTime (m_real_time);

    /* Set sockets, callback, station properties and TraCI VDP in CPBasicService */
    m_cpService.setSocketTx (m_socket);
    m_cpService.setSocketRx (m_socket);
    m_cpService.setStationProperties (std::stol(m_id), (long)stationtype);
    m_cpService.addCPRxCallback (std::bind(&cooperativePerception::receiveCPM,this,std::placeholders::_1,std::placeholders::_2));
    m_cpService.setRealTime (m_real_time);
    m_cpService.setRedundancyMitigation (false);

    /* Set VDP for GeoNet object */
    m_caService.setVDP(vdp);
    m_denService.setVDP(vdp);
    m_cpService.setVDP(vdp);

    /* Schedule CAM dissemination */
    if(m_send_cam == true)
    {
      std::srand(Simulator::Now().GetNanoSeconds ());
      double desync = ((double)std::rand()/RAND_MAX);
      m_caService.startCamDissemination(desync);
    }

    /* Schedule CPM dissemination */
    m_cpService.startCpmDissemination ();

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.open (m_csv_name+"-"+m_id+"-CAM.csv",std::ofstream::trunc);
      m_csv_ofstream_cam << "messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration" << std::endl;
    }
  }

  void
  cooperativePerception::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_send_cam_ev);

    uint64_t cam_sent, cpm_sent;

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.close ();
    }

    cam_sent = m_caService.terminateDissemination ();
    cpm_sent = m_cpService.terminateDissemination ();
    m_denService.cleanup();

    if (m_print_summary && !m_already_print)
    {
      std::cout << "INFO-" << m_id
                << ",CAM-SENT:" << cam_sent
                << ",CAM-RECEIVED:" << m_cam_received
                << ",CPM-SENT: " << cpm_sent
                << ",CPM-RECEIVED" << m_cpm_received
                << std::endl;
      m_already_print=true;
    }
  }

  void
  cooperativePerception::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  cooperativePerception::receiveCAM (asn1cpp::Seq<CAM> cam, Address from)
  {
    /* Implement CAM strategy here */
   m_cam_received++;
   double fromLon = asn1cpp::getField(cam->cam.camParameters.basicContainer.referencePosition.longitude,double)/DOT_ONE_MICRO;
   double fromLat = asn1cpp::getField(cam->cam.camParameters.basicContainer.referencePosition.latitude,double)/DOT_ONE_MICRO;
   if(m_opencda_client)
     {
       carla::Vector carlaPosition = m_opencda_client->getCartesian (fromLon,fromLat);
       std::cout << "["<< Simulator::Now ().GetSeconds ()<<"] " << m_id <<" received a new CAM from vehicle "
                 << asn1cpp::getField(cam->header.stationId,long) << " --> GeoPosition: [" << fromLon << ", " << fromLat << "]"
                 << " CartesianPosition: [" << carlaPosition.x () << ", " << carlaPosition.y () << "]" <<std::endl;
     }

   if (!m_csv_name.empty ())
     {
       // messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration
       m_csv_ofstream_cam << cam->header.messageId << "," << cam->header.stationId << ",";
       m_csv_ofstream_cam << cam->cam.generationDeltaTime << "," << asn1cpp::getField(cam->cam.camParameters.basicContainer.referencePosition.latitude,double)/DOT_ONE_MICRO << ",";
       m_csv_ofstream_cam << asn1cpp::getField(cam->cam.camParameters.basicContainer.referencePosition.longitude,double)/DOT_ONE_MICRO << "," ;
       m_csv_ofstream_cam << asn1cpp::getField(cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue,double)/DECI << "," << asn1cpp::getField(cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue,double)/CENTI << ",";
       m_csv_ofstream_cam << asn1cpp::getField(cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.value,double)/DECI << std::endl;
     }

  }

  void
  cooperativePerception::receiveCPMV1 (asn1cpp::Seq<CPMV1> cpm, Address from)
  {
   /* Implement CPM strategy here */
   m_cpm_received++;
   (void) from;
   std::cout << "["<< Simulator::Now ().GetSeconds ()<<"] " << m_id <<" received a new CPMv1 from vehicle " << asn1cpp::getField(cpm->header.stationId,long) <<" with "<< asn1cpp::getField(cpm->cpm.cpmParameters.numberOfPerceivedObjects,long)<< " perceived objects." <<std::endl;
   int fromID = asn1cpp::getField(cpm->header.stationId,long);
   if (m_recvCPMmap.find(fromID) == m_recvCPMmap.end())
     m_recvCPMmap[fromID] = std::map<int,int>(); // First CPM from this vehicle
   //For every PO inside the CPM, if any
   bool POs_ok;
   auto PObjects = asn1cpp::getSeqOpt(cpm->cpm.cpmParameters.perceivedObjectContainer,PerceivedObjectContainer,&POs_ok);
   if (POs_ok)
     {
       int PObjects_size = asn1cpp::sequenceof::getSize(cpm->cpm.cpmParameters.perceivedObjectContainer);
       for(int i=0; i<PObjects_size;i++)
        {
          LDM::returnedVehicleData_t PO_data;
          auto PO_seq = asn1cpp::makeSeq(PerceivedObjectV1);
          PO_seq = asn1cpp::sequenceof::getSeq(cpm->cpm.cpmParameters.perceivedObjectContainer,PerceivedObjectV1,i);
          if(m_recvCPMmap[fromID].find(asn1cpp::getField(PO_seq->objectID,long)) == m_recvCPMmap[fromID].end())
            {
              // First time we have received this object from this vehicle
              //If PO id is already in local copy of LDM
              if(m_LDM->lookup(asn1cpp::getField(PO_seq->objectID,long),PO_data) == LDM::LDM_OK)
                {
                  // We need a new ID for object
                  std::set<int> IDs;
                  m_LDM->getAllIDs (IDs);
                  int newID = 1;
                  for (int num : IDs) {
                      if (num == newID) {
                          ++newID;
                        } else if (num > newID) {
                          break;
                        }
                    }
                  //Translate CPM data to LDM format
                  m_LDM->insert(translateCPMV1data(cpm,i,newID));
                  //Update recvCPMmap
                  m_recvCPMmap[fromID][asn1cpp::getField(PO_seq->objectID,long)] = newID;
                }
              else
                {
                  //Translate CPM data to LDM format
                  m_LDM->insert(translateCPMV1data(cpm,i,-1));
                  //Update recvCPMmap
                  m_recvCPMmap[fromID][asn1cpp::getField(PO_seq->objectID,long)] = asn1cpp::getField(PO_seq->objectID,long);
                }
            }
          else
            {
              // We have already receive this object from this vehicle
              m_LDM->insert(translateCPMV1data(cpm,i,m_recvCPMmap[fromID][asn1cpp::getField(PO_seq->objectID,long)]));
            }
        }
     }
  }

  vehicleData_t
  cooperativePerception::translateCPMV1data (asn1cpp::Seq<CPMV1> cpm, int objectIndex, int newID)
  {
   vehicleData_t retval;
   auto PO_seq = asn1cpp::makeSeq(PerceivedObjectV1);
   using namespace boost::geometry::strategy::transform;
   PO_seq = asn1cpp::sequenceof::getSeq(cpm->cpm.cpmParameters.perceivedObjectContainer,PerceivedObject,objectIndex);
   retval.detected = true;
   if(newID == -1)
     retval.stationID = asn1cpp::getField(PO_seq->objectID,long);
   else
     retval.stationID = newID;
   retval.ID = std::to_string(retval.stationID);
   retval.vehicleLength = asn1cpp::getField(PO_seq->planarObjectDimension1->value,long);
   retval.vehicleWidth = asn1cpp::getField(PO_seq->planarObjectDimension2->value,long);
   retval.heading = asn1cpp::getField(cpm->cpm.cpmParameters.stationDataContainer->choice.originatingVehicleContainer.heading.headingValue,double)/10 +
                    asn1cpp::getField(PO_seq->yawAngle->value,double)/10;
   if (retval.heading > 360.0)
     retval.heading -= 360.0;

   retval.speed_ms = (double) (asn1cpp::getField(cpm->cpm.cpmParameters.stationDataContainer->choice.originatingVehicleContainer.speed.speedValue,long) +
                               asn1cpp::getField(PO_seq->xSpeed.value,long))/CENTI;

   double fromLon = asn1cpp::getField(cpm->cpm.cpmParameters.managementContainer.referencePosition.longitude,double)/DOT_ONE_MICRO;
   double fromLat = asn1cpp::getField(cpm->cpm.cpmParameters.managementContainer.referencePosition.latitude,double)/DOT_ONE_MICRO;

   VDP::VDP_position_cartesian_t objectPosition;
   if(m_traci_client != nullptr)
     {
       libsumo::TraCIPosition traciPosition = m_traci_client->TraCIAPI::simulation.convertLonLattoXY (fromLon,fromLat);
       objectPosition.x = traciPosition.x;
       objectPosition.y = traciPosition.y;
     }
   else
     {
       carla::Vector carlaPosition = m_opencda_client->getCartesian (fromLon,fromLat);
       objectPosition.x = carlaPosition.x ();
       objectPosition.y = carlaPosition.y ();
     }

   point_type objPoint(asn1cpp::getField(PO_seq->xDistance.value,double)/CENTI,asn1cpp::getField(PO_seq->yDistance.value,double)/CENTI);
   double fromAngle = asn1cpp::getField(cpm->cpm.cpmParameters.stationDataContainer->choice.originatingVehicleContainer.heading.headingValue,double)/10;
   rotate_transformer<boost::geometry::degree, double, 2, 2> rotate(fromAngle-90);
   boost::geometry::transform(objPoint, objPoint, rotate);// Transform points to the reference (x,y) axises
   VDP::VDP_position_latlon_t objectPositionGeo;
   if(m_traci_client != nullptr)
     {
       objectPosition.x += boost::geometry::get<0>(objPoint);
       objectPosition.y += boost::geometry::get<1>(objPoint);
       libsumo::TraCIPosition traciPosition = m_traci_client->TraCIAPI::simulation.convertXYtoLonLat (objectPosition.x,objectPosition.y);
       objectPositionGeo.lon = traciPosition.x;
       objectPositionGeo.lat = traciPosition.y;
     }
   else
     {
       objectPosition.x += asn1cpp::getField(PO_seq->xDistance.value,double)/CENTI;
       objectPosition.y += asn1cpp::getField(PO_seq->yDistance.value,double)/CENTI;
       carla::Vector carlaPosition = m_opencda_client->getGeo (objectPosition.x,objectPosition.y);
       objectPositionGeo.lat = carlaPosition.x ();
       objectPositionGeo.lon = carlaPosition.y ();
     }

   retval.lon = objectPositionGeo.lon;
   retval.lat = objectPositionGeo.lat;

   point_type speedPoint(asn1cpp::getField(PO_seq->xSpeed.value,double)/CENTI,asn1cpp::getField(PO_seq->ySpeed.value,double)/CENTI);
   boost::geometry::transform(speedPoint, speedPoint, rotate);// Transform points to the reference (x,y) axises
   retval.speed_ms = asn1cpp::getField(cpm->cpm.cpmParameters.stationDataContainer->choice.originatingVehicleContainer.speed.speedValue,double)/CENTI + boost::geometry::get<0>(speedPoint);

   retval.camTimestamp = asn1cpp::getField(cpm->cpm.generationDeltaTime,long);
   retval.timestamp_us = Simulator::Now().GetMicroSeconds () - (asn1cpp::getField(PO_seq->timeOfMeasurement,long)*1000);
   retval.stationType = StationType_passengerCar;
   retval.perceivedBy.setData(asn1cpp::getField(cpm->header.stationId,long));
   retval.confidence = asn1cpp::getField(PO_seq->objectConfidence,long);

   // FOR DEBUGGING
   if(m_traci_client == nullptr)
     {
       carla::Vector pos = m_opencda_client->getCartesian (retval.lon, retval.lat);
       std::cout << "[" << retval.stationID << "] --> Position: [" << pos.x ()<<", "<< pos.y () << "]" << std::endl;
     }

   return retval;

  }

  void
  cooperativePerception::receiveCPM (asn1cpp::Seq<CollectivePerceptionMessage> cpm, Address from)
  {
   /* Implement CPM strategy here */
   m_cpm_received++;
   (void) from;
   int fromID = asn1cpp::getField(cpm->header.stationId,long);
   if (m_recvCPMmap.find(fromID) == m_recvCPMmap.end())
     m_recvCPMmap[fromID] = std::map<int,int>(); // First CPM from this vehicle

   //For every PO inside the CPM, if any
   bool POs_ok;
   //auto wrappedContainer = asn1cpp::makeSeq(WrappedCpmContainer);
   int wrappedContainer_size = asn1cpp::sequenceof::getSize(cpm->payload.cpmContainers);
   for (int i=0; i<wrappedContainer_size; i++)
     {
       auto wrappedContainer = asn1cpp::sequenceof::getSeq(cpm->payload.cpmContainers,WrappedCpmContainer,i);
       WrappedCpmContainer__containerData_PR present = asn1cpp::getField(wrappedContainer->containerData.present,WrappedCpmContainer__containerData_PR);
       if(present == WrappedCpmContainer__containerData_PR_PerceivedObjectContainer)
        {
          auto POcontainer = asn1cpp::getSeq(wrappedContainer->containerData.choice.PerceivedObjectContainer,PerceivedObjectContainer);
          int PObjects_size = asn1cpp::sequenceof::getSize(POcontainer->perceivedObjects);
          std::cout << "["<< Simulator::Now ().GetSeconds ()<<"] " << m_id <<" received a new CPMv2 from " << asn1cpp::getField(cpm->header.stationId,long) << " with " << PObjects_size << " perceived objects." << std::endl;
          for(int j=0; j<PObjects_size;j++)
            {
              LDM::returnedVehicleData_t PO_data;
              auto PO_seq = asn1cpp::makeSeq(PerceivedObject);
              PO_seq = asn1cpp::sequenceof::getSeq(POcontainer->perceivedObjects,PerceivedObject,j);
              //If PO is already in local copy of vLDM
              if(m_recvCPMmap[fromID].find(asn1cpp::getField(PO_seq->objectId,long)) == m_recvCPMmap[fromID].end())
                {
                  // First time we have received this object from this vehicle
                  //If PO id is already in local copy of LDM
                  if(m_LDM->lookup(asn1cpp::getField(PO_seq->objectId,long),PO_data) == LDM::LDM_OK)
                    {
                      // We need a new ID for object
                      std::set<int> IDs;
                      m_LDM->getAllIDs (IDs);
                      int newID = 1;
                      for (int num : IDs) {
                          if (num == newID) {
                              ++newID;
                            } else if (num > newID) {
                              break;
                            }
                        }
                      //Translate CPM data to LDM format
                      m_LDM->insert(translateCPMdata(cpm,PO_seq,i,newID));
                      //Update recvCPMmap
                      m_recvCPMmap[fromID][asn1cpp::getField(PO_seq->objectId,long)] = newID;
                    }
                  else
                    {
                      //Translate CPM data to LDM format
                      m_LDM->insert(translateCPMdata(cpm,PO_seq,i,-1));
                      //Update recvCPMmap
                      m_recvCPMmap[fromID][asn1cpp::getField(PO_seq->objectId,long)] = asn1cpp::getField(PO_seq->objectId,long);
                    }
                }
              else
                {
                  // We have already receive this object from this vehicle
                  m_LDM->insert(translateCPMdata(cpm,PO_seq,i,m_recvCPMmap[fromID][asn1cpp::getField(PO_seq->objectId,long)]));
                }
            }
        }
     }
  }
  vehicleData_t
  cooperativePerception::translateCPMdata (asn1cpp::Seq<CollectivePerceptionMessage> cpm,
                                           asn1cpp::Seq<PerceivedObject> object, int objectIndex, int newID)
  {
   vehicleData_t retval;
   retval.detected = true;
   if(newID == -1)
     retval.stationID = asn1cpp::getField(object->objectId,long);
   else
     retval.stationID = newID;
   retval.ID = std::to_string(retval.stationID);
   retval.vehicleLength = asn1cpp::getField(object->objectDimensionX->value,long);
   retval.vehicleWidth = asn1cpp::getField(object->objectDimensionY->value,long);
   retval.heading = asn1cpp::getField(object->angles->zAngle.value,double) / DECI;
   retval.xSpeedAbs.setData (asn1cpp::getField(object->velocity->choice.cartesianVelocity.xVelocity.value,long));
   retval.xSpeedAbs.setData (asn1cpp::getField(object->velocity->choice.cartesianVelocity.yVelocity.value,long));
   retval.speed_ms = (sqrt (pow(retval.xSpeedAbs.getData(),2) +
                            pow(retval.ySpeedAbs.getData(),2)))/CENTI;

   VDP::VDP_position_cartesian_t objectPosition;
   double fromLon = asn1cpp::getField(cpm->payload.managementContainer.referencePosition.longitude,double)/DOT_ONE_MICRO;
   double fromLat = asn1cpp::getField(cpm->payload.managementContainer.referencePosition.latitude,double)/DOT_ONE_MICRO;
   if(m_traci_client != nullptr)
     {
       libsumo::TraCIPosition traciPosition = m_traci_client->TraCIAPI::simulation.convertLonLattoXY (fromLon,fromLat);
       objectPosition.x = traciPosition.x;
       objectPosition.y = traciPosition.y;
     }
   else
     {
       carla::Vector carlaPosition = m_opencda_client->getCartesian (fromLon,fromLat);
       objectPosition.x = carlaPosition.x ();
       objectPosition.y = carlaPosition.y ();
     }
   VDP::VDP_position_latlon_t objectPositionGeo;
   objectPosition.x += asn1cpp::getField(object->position.xCoordinate.value,long)/CENTI;
   objectPosition.y += asn1cpp::getField(object->position.yCoordinate.value,long)/CENTI;
   if(m_traci_client != nullptr)
     {
       libsumo::TraCIPosition traciPosition = m_traci_client->TraCIAPI::simulation.convertXYtoLonLat (objectPosition.x,objectPosition.y);
       objectPositionGeo.lon = traciPosition.x;
       objectPositionGeo.lat = traciPosition.y;
     }
   else
     {
       carla::Vector carlaPosition = m_opencda_client->getGeo (objectPosition.x,objectPosition.y);
       objectPositionGeo.lat = carlaPosition.x ();
       objectPositionGeo.lon = carlaPosition.y ();
     }

   retval.lon = objectPositionGeo.lon;
   retval.lat = objectPositionGeo.lat;

   retval.camTimestamp = asn1cpp::getField(cpm->payload.managementContainer.referenceTime,long);
   retval.timestamp_us = Simulator::Now().GetMicroSeconds () - (asn1cpp::getField(object->measurementDeltaTime,long)*1000);
   retval.stationType = StationType_passengerCar;
   retval.perceivedBy.setData(asn1cpp::getField(cpm->header.stationId,long));

   return retval;
  }

}





