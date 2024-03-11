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
 *  Carlos Mateo Risma Carletti, Politecnico di Torino (carlosrisma@gmail.com)
*/

#include "emergencyVehicleWarningClient.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/gn-utils.h"
#include "ns3/iviService.h"
#include "ns3/vdp.h"




#define DEG_2_RAD(val) ((val)*M_PI/180.0)

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("emergencyVehicleWarningClient");

  NS_OBJECT_ENSURE_REGISTERED(emergencyVehicleWarningClient);

  TypeId
  emergencyVehicleWarningClient::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::emergencyVehicleWarningClient")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<emergencyVehicleWarningClient> ()
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&emergencyVehicleWarningClient::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("IpAddr",
            "IpAddr",
            Ipv4AddressValue ("10.0.0.1"),
            MakeIpv4AddressAccessor (&emergencyVehicleWarningClient::m_ipAddress),
            MakeIpv4AddressChecker ())
        .AddAttribute ("PrintSummary",
            "To print summary at the end of simulation",
            BooleanValue(false),
            MakeBooleanAccessor (&emergencyVehicleWarningClient::m_print_summary),
            MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&emergencyVehicleWarningClient::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("Model",
            "Physical and MAC layer communication model",
            StringValue (""),
            MakeStringAccessor (&emergencyVehicleWarningClient::m_model),
            MakeStringChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&emergencyVehicleWarningClient::m_client),
            MakePointerChecker<TraciClient> ())
        .AddAttribute ("PRRSupervisor",
            "PRR Supervisor to compute PRR according to 3GPP TR36.885 V14.0.0 page 70",
            PointerValue (0),
            MakePointerAccessor (&emergencyVehicleWarningClient::m_PRR_supervisor),
            MakePointerChecker<PRRSupervisor> ())
        .AddAttribute ("SendCAM",
            "To enable/disable the transmission of CAM messages",
            BooleanValue(true),
            MakeBooleanAccessor (&emergencyVehicleWarningClient::m_send_cam),
            MakeBooleanChecker ());
        return tid;
  }

  emergencyVehicleWarningClient::emergencyVehicleWarningClient ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_print_summary = true;
    m_already_print = false;

    m_denm_sent = 0;
    m_cam_received = 0;
    m_denm_received = 0;
    m_denm_intertime = 0;
    m_ivim_received = 0;

    m_distance_threshold = 75; // Distance used in GeoNet to determine the radius of the circumference arounf the emergency vehicle where the DENMs are valid
    m_heading_threshold = 45; // Max heading angle difference between the normal vehicles and the emergenecy vehicle, that triggers a reaction in the normal vehicles
  }

  emergencyVehicleWarningClient::~emergencyVehicleWarningClient ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  emergencyVehicleWarningClient::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  emergencyVehicleWarningClient::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    /*
     * In this example, the vehicle can be either of type "passenger" or of type "emergency" (see cars.rou.xml in SUMO folder inside examples/sumo_files_v2v_map)
     * All the vehicles broadcast CAM messages. When a "passenger" car receives a CAM from an "emergency" vehicle, it checks the distance between them and
     * the difference in heading, and if it considers it to be close, it takes proper actions to facilitate the takeover maneuver.
     */

    /* Save the vehicles informations */
    m_id = m_client->GetVehicleId (this->GetNode ());
    m_type = m_client->TraCIAPI::vehicle.getVehicleClass (m_id);
    m_max_speed = m_client->TraCIAPI::vehicle.getMaxSpeed (m_id);

    // Create new BTP and GeoNet objects and set them in DENBasicService, CABasicService and iviService
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();

    if(m_PRR_supervisor!=nullptr)
    {
      m_geoNet->setPRRSupervisor(m_PRR_supervisor);
    }

    m_btp->setGeoNet(m_geoNet);
    m_caService.setBTP(m_btp);
    m_iviService.setBTP(m_btp);

    /* Create the Sockets for TX and RX */
    TypeId tid;
    if(m_model=="80211p")
      tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
    else if(m_model=="cv2x")
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
    if (m_type=="passenger") {
      stationtype = StationType_passengerCar;
    libsumo::TraCIColor red;
    red.r=255;red.g=0;red.b=0;red.a=255;
    m_client->TraCIAPI::vehicle.setColor (m_id,red);
      }else if (m_type=="emergency"){
      stationtype = StationType_specialVehicle;
      libsumo::TraCIColor pink;
      pink.r=255;pink.g=0;pink.b=239;pink.a=255;
      //orange.r=232;orange.g=126;orange.b=4;orange.a=255;
      m_client->TraCIAPI::vehicle.setColor (m_id,pink);
      } else
      stationtype = StationType_unknown;

    /* Set sockets, callback, station properties and TraCI VDP in CABasicService */
    m_caService.setSocketTx (m_socket);
    m_caService.setSocketRx (m_socket);
    m_caService.setStationProperties (std::stol(m_id.substr (3)), (long)stationtype);
    m_caService.addCARxCallback (std::bind(&emergencyVehicleWarningClient::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));
    m_caService.setRealTime (m_real_time);

    /* Set sockets, callback and station properties and TraCI VDP in iviService */
    m_iviService.setSocketTx (m_socket);
    m_iviService.setSocketRx (m_socket);
    m_iviService.setStationProperties (std::stol(m_id.substr (3)),(long)stationtype);
    m_iviService.addIVIRxCallback (std::bind(&emergencyVehicleWarningClient::receiveIVIM,this,std::placeholders::_1,std::placeholders::_2));
    m_iviService.setRealTime (m_real_time);



    /* Set TraCI VDP for GeoNet object */
    VDP* traci_vdp = new VDPTraCI(m_client,m_id);
    m_caService.setVDP(traci_vdp);
    m_geoNet->setVDP (traci_vdp);

    /* Schedule CAM dissemination */
    if(m_send_cam == true)
    {
      std::srand(Simulator::Now().GetNanoSeconds ());
      double desync = ((double)std::rand()/RAND_MAX);
      m_caService.startCamDissemination(desync);
    }

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.open (m_csv_name+"-"+m_id+"-"+std::to_string(stationtype)+"-CAM.csv",std::ofstream::trunc);
      m_csv_ofstream_cam << "messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration,x,y" << std::endl;
    }
  }

  void
  emergencyVehicleWarningClient::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_speed_ev);
    Simulator::Cancel(m_send_cam_ev);
    Simulator::Cancel(m_update_denm_ev);
    Simulator::Cancel(m_check_rel_zone_ev);


    uint64_t cam_sent;

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.close ();
    }

    cam_sent = m_caService.terminateDissemination ();

    if (m_print_summary && !m_already_print)
    {
      std::cout << "INFO-" << m_id << "-" << m_type
                << ",CAM-SENT:" << cam_sent
                << ",CAM-RECEIVED:" << m_cam_received
                << ",IVIM-RECEIVED:" << m_ivim_received
                << std::endl;
      m_already_print=true;
    }
  }

  void
  emergencyVehicleWarningClient::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  emergencyVehicleWarningClient::receiveCAM (asn1cpp::Seq<CAM> cam, Address from)
  {
    /* Implement CAM strategy here */
   m_cam_received++;


   /* If the CAM is received from an emergency vehicle, and the host vehicle is a "passenger" car, then process the CAM */
   if (cam->cam.camParameters.basicContainer.stationType==StationType_specialVehicle && m_type!="emergency")
   {
     libsumo::TraCIPosition pos=m_client->TraCIAPI::vehicle.getPosition(m_id);
     pos=m_client->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);
   }

   if (!m_csv_name.empty ())
     {

       libsumo::TraCIPosition pos,pos2;
       pos=m_client->TraCIAPI::vehicle.getPosition(m_id);

       // messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration
       m_csv_ofstream_cam << cam->header.messageId << "," << cam->header.stationId << ",";
       m_csv_ofstream_cam << cam->cam.generationDeltaTime << "," << (double)cam->cam.camParameters.basicContainer.referencePosition.latitude/DOT_ONE_MICRO << ",";
       m_csv_ofstream_cam << (double)cam->cam.camParameters.basicContainer.referencePosition.longitude/DOT_ONE_MICRO << "," ;
       m_csv_ofstream_cam << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue/DECI << "," << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue/CENTI << ",";
       m_csv_ofstream_cam << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.value/DECI << ",";
       m_csv_ofstream_cam << pos.x << "," << pos.y << std::endl;


     }
  }

  void
  emergencyVehicleWarningClient::receiveIVIM (iviData ivim, Address from)
  {
        //std::cout<<"Received a new IVIM."<<std::endl;
        m_ivim_received++;

        /* Once a vehicle receive the IVIM, check if it is in the relevanceZone
         * If true, check if the lane index is different from 0
         * If true, change to lane 0. Those vehicles will be showed with color = orange
         */
        long laneWidth;
        libsumo::TraCIPosition referencePosition,startDet;
        iviData::IVI_glc_t glc = ivim.getIvimGlc ();

        referencePosition = m_client->TraCIAPI::simulation.convertLonLattoXY((double) glc.referencePosition.longitude/DOT_ONE_MICRO,(double) glc.referencePosition.latitude/DOT_ONE_MICRO);

        auto gic = ivim.getGic ();
        for(auto gic_it = gic.GicPart.begin (); gic_it!= gic.GicPart.end (); gic_it++)
          {
            //Get detection zone ID from GIV and look for it in GLC
            if(gic_it->detectionZoneIds.isAvailable ())
              {
                for (auto glc_it = glc.GlcPart.begin (); glc_it!=glc.GlcPart.end ();glc_it++)
                  {
                    auto delta = glc_it->zone.getData().segment.getData ().line.deltaPositions.getData ()[0];
                    if(glc_it->zoneId == gic_it->detectionZoneIds.getData ()[0])
                      {
                        //Get the start of the Detection zone that has the center on ReferencePosition-DeltaPosition
                        startDet.x = (double) (glc.referencePosition.longitude - delta.deltaLong*2)/DOT_ONE_MICRO;
                        startDet.y = (double) (glc.referencePosition.latitude - delta.deltaLat*2)/DOT_ONE_MICRO;
                        startDet = m_client->TraCIAPI::simulation.convertLonLattoXY(startDet.x,startDet.y);
                       }
                  }
              }

            //Get relevance zone ID from GIV and look for it in GLC
            if(gic_it->relevanceZoneIds.isAvailable ())
              {
                for (auto glc_it = glc.GlcPart.begin (); glc_it!=glc.GlcPart.end ();glc_it++)
                  {
                    auto delta = glc_it->zone.getData().segment.getData ().line.deltaPositions.getData ()[0];
                    if(glc_it->zoneId == gic_it->relevanceZoneIds.getData ()[0])
                      {
                        //Get the start of the Relevance zone that has the center on ReferencePosition-DeltaPosition
                        //Store end of relevance zone
                        m_endRel.x = (double)  (glc.referencePosition.longitude - delta.deltaLong*2)/DOT_ONE_MICRO;
                        m_endRel.y = (double)  (glc.referencePosition.latitude - delta.deltaLat*2)/DOT_ONE_MICRO;
                        m_endRel = m_client->TraCIAPI::simulation.convertLonLattoXY(m_endRel.x,m_endRel.y);
                        laneWidth = glc_it->zone.getData ().segment.getData ().laneWidth.getData ();
                       }
                  }
              }

          }
        //Finish storing relevance zone for timeout checks
        m_startRel = referencePosition;
        m_segmentUpper = referencePosition.y + laneWidth/2;
        m_segmentLower = referencePosition.y - laneWidth/2;


        if (m_type != "emergency") {

            libsumo::TraCIPosition pos,posLL;
            pos=m_client->TraCIAPI::vehicle.getPosition(m_id);

            /*Check if client is inside the Detectionzone+RelevanceZone*/
            if ( pos.x <= (startDet.x) && pos.x>=(m_endRel.x) &&
                 pos.y <= (m_segmentUpper)  && pos.y>=(m_segmentLower)){

                libsumo::TraCIColor orange;
                orange.r=232;orange.g=126;orange.b=4;orange.a=255;
                m_client->TraCIAPI::vehicle.setColor (m_id,orange);

                if (m_client->TraCIAPI::vehicle.getLaneIndex (m_id) !=0){

                    /*If client is not on the slow lane, change lane towards it and set the speed to
                      the one specified in the RoadSign code of the IVIM*/
                    m_client->TraCIAPI::vehicle.changeLane (m_id,0,1);
                    double new_max_speed = gic.GicPart.back ().RS.back ().RS_spm.getData ()/3.6; //from km/h to m/s
                    m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, new_max_speed-3);

                  }

                /*Schedule timeout for Relevance Zone check*/
                Simulator::Schedule(Seconds (1),&emergencyVehicleWarningClient::checkRelevanceZone,this);

              }
          }

  }

  void
  emergencyVehicleWarningClient::checkRelevanceZone ()
  {
    /*Check if inside relevance zone */
    if ( m_client->TraCIAPI::vehicle.getPosition (m_id).x>=(m_endRel.x) &&
          m_client->TraCIAPI::vehicle.getPosition (m_id).y<=(m_segmentUpper) &&
          m_client->TraCIAPI::vehicle.getPosition (m_id).y>=(m_segmentLower)){
          /*Schedule timeout for Relevance Zone check*/
         Simulator::Schedule(Seconds (1),&emergencyVehicleWarningClient::checkRelevanceZone,this);

         if (m_client->TraCIAPI::vehicle.getLaneIndex (m_id) !=0){

             /* Keep on slower lane*/
             m_client->TraCIAPI::vehicle.changeLane (m_id,0,1);

           }
       } else {

        /*If client is outside the relevance zone, set speed back to normal */
         Simulator::Remove(m_speed_ev);
         m_speed_ev = Simulator::Schedule (Seconds (0.1), &emergencyVehicleWarningClient::SetMaxSpeed, this);
       }
  }

  void
  emergencyVehicleWarningClient::SetMaxSpeed ()
  {
    libsumo::TraCIColor normal;
    normal.r=255;normal.g=255;normal.b=0;normal.a=255;
    m_client->TraCIAPI::vehicle.setColor (m_id, normal);
    m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, m_max_speed);
  }

}





