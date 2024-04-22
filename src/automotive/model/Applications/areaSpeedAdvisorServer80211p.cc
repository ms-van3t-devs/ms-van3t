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

#include "areaSpeedAdvisorServer80211p.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/Setter.hpp"
#include "ns3/Encoding.hpp"
#include "ns3/SetOf.hpp"
#include "ns3/SequenceOf.hpp"
#include "ns3/socket.h"
#include "ns3/btpdatarequest.h"
#include "ns3/network-module.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("areaSpeedAdvisorServer80211p");

  NS_OBJECT_ENSURE_REGISTERED(areaSpeedAdvisorServer80211p);

  TypeId
  areaSpeedAdvisorServer80211p::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::areaSpeedAdvisorServer80211p")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<areaSpeedAdvisorServer80211p> ()
        .AddAttribute ("AggregateOutput",
           "If it is true, the server will print every second an aggregate output about cam and denm",
           BooleanValue (false),
           MakeBooleanAccessor (&areaSpeedAdvisorServer80211p::m_aggregate_output),
           MakeBooleanChecker ())
        .AddAttribute ("RealTime",
           "To compute properly timestamps",
           BooleanValue(false),
           MakeBooleanAccessor (&areaSpeedAdvisorServer80211p::m_real_time),
           MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&areaSpeedAdvisorServer80211p::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("Client",
           "TraCI client for SUMO",
           PointerValue (0),
           MakePointerAccessor (&areaSpeedAdvisorServer80211p::m_client),
           MakePointerChecker<TraciClient> ())
        .AddAttribute ("PRRSupervisor",
            "PRR Supervisor to compute PRR according to 3GPP TR36.885 V14.0.0 page 70",
            PointerValue (0),
            MakePointerAccessor (&areaSpeedAdvisorServer80211p::m_PRR_supervisor),
            MakePointerChecker<PRRSupervisor> ())
        .AddAttribute ("SendCAM",
             "To enable/disable the transmission of CAM messages",
             BooleanValue(true),
             MakeBooleanAccessor (&areaSpeedAdvisorServer80211p::m_send_cam),
             MakeBooleanChecker ());

        return tid;
  }

  areaSpeedAdvisorServer80211p::areaSpeedAdvisorServer80211p ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_cam_received = 0;
    m_denm_sent = 0;
    m_isTransmittingDENM = false;
  }

  areaSpeedAdvisorServer80211p::~areaSpeedAdvisorServer80211p ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  areaSpeedAdvisorServer80211p::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  areaSpeedAdvisorServer80211p::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    m_id = m_client->GetRSUId (this -> GetNode ());

    /* TX socket for DENMs and RX socket for CAMs (one socket only is necessary) */
    TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");

    m_socket = Socket::CreateSocket (GetNode (), tid);

    /* Bind the socket to local address */
    PacketSocketAddress local_denm;
    local_denm.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
    local_denm.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetAddress ());
    local_denm.SetProtocol (0x8947);
    if (m_socket->Bind (local_denm) == -1)
    {
      NS_FATAL_ERROR ("Failed to bind server socket");
    }

    /* Set socket to broacdcast */
    PacketSocketAddress remote;
    remote.SetSingleDevice (GetNode ()->GetDevice (0)->GetIfIndex ());
    remote.SetPhysicalAddress (GetNode ()->GetDevice (0)->GetBroadcast ());
    remote.SetProtocol (0x8947);
    m_socket->Connect(remote);

    /* Create new BTP and GeoNet objects and set them in DENBasicService and CABasicService */
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();

    if(m_PRR_supervisor!=nullptr)
    {
      m_geoNet->setPRRSupervisor(m_PRR_supervisor);
    }

    m_btp->setGeoNet(m_geoNet);
    m_denService.setBTP(m_btp);
    m_caService.setBTP(m_btp);

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setSocketTx (m_socket);
    m_denService.setSocketRx (m_socket);
    m_denService.addDENRxCallback (std::bind(&areaSpeedAdvisorServer80211p::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));

    uint64_t id = std::stoi(m_id.substr (m_id.find("_") + 1));
    m_denService.setStationProperties (m_stationId_baseline + id, StationType_roadSideUnit);

    /* Set callback and station properties in CABasicService */
    m_caService.setStationProperties (m_stationId_baseline + id, StationType_roadSideUnit);
    m_caService.setSocketRx (m_socket);
    m_caService.addCARxCallback (std::bind(&areaSpeedAdvisorServer80211p::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));

    libsumo::TraCIPosition rsuPosXY = m_client->TraCIAPI::poi.getPosition (m_id);
    libsumo::TraCIPosition rsuPosLonLat = m_client->TraCIAPI::simulation.convertXYtoLonLat (rsuPosXY.x,rsuPosXY.y);

    /* Compute GeoArea for DENMs */
    GeoArea_t geoArea;
    // Longitude and Latitude in [0.1 microdegree]
    geoArea.posLong = rsuPosLonLat.x*DOT_ONE_MICRO;
    geoArea.posLat = rsuPosLonLat.y*DOT_ONE_MICRO;
    // Radius [m] of the circle that covers the whole square area of the map in (x,y)
    geoArea.distA = 90;
    // DistB [m] and angle [deg] equal to zero because we are defining a circular area as specified in ETSI EN 302 636-4-1 [9.8.5.2]
    geoArea.distB = 0;
    geoArea.angle = 0;
    geoArea.shape = CIRCULAR;

    m_denService.setGeoArea (geoArea);

    m_denService.setFixedPositionRSU (rsuPosLonLat.y,rsuPosLonLat.x);
    m_caService.setFixedPositionRSU (rsuPosLonLat.y,rsuPosLonLat.x);

    VDP* traci_vdp = new VDPTraCI(m_client, m_id, true);

    m_caService.setVDP(traci_vdp);

    m_denService.setVDP(traci_vdp);

    /*if(m_PRR_supervisor!=nullptr)
    {
      m_PRR_supervisor->addExcludedID (777888999);
    }*/

    if(m_send_cam)
      {
        std::srand(Simulator::Now().GetNanoSeconds ());
        double desync = ((double)std::rand()/RAND_MAX);
        m_caService.startCamDissemination(desync);
      }

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.open (m_csv_name+"-server.csv",std::ofstream::trunc);
      m_csv_ofstream_cam << "messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration" << std::endl;
    }

    /* If aggregate output is enabled, start it */
    if (m_aggregate_output)
      m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &areaSpeedAdvisorServer80211p::aggregateOutput, this);
  }

  void
  areaSpeedAdvisorServer80211p::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel (m_aggegateOutputEvent);
    Simulator::Cancel (m_terminate_denm_ev);
    Simulator::Cancel (m_update_denm_ev);

    m_denService.cleanup();

    if (!m_csv_name.empty ())
      m_csv_ofstream_cam.close ();

    if (m_aggregate_output)
      std::cout << Simulator::Now () << "," << m_cam_received  << "," << m_denm_sent << std::endl;
  }

  void
  areaSpeedAdvisorServer80211p::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  areaSpeedAdvisorServer80211p::TriggerDenm ()
  {
    denData data;
    DEN_RoadWorksContainerExtended_t roadworks;
    denData::denDataAlacarte alacartedata;
    DENBasicService_error_t trigger_retval;

//    /* Build DENM data */
    data.setDenmMandatoryFields (compute_timestampIts(),Latitude_unavailable,Longitude_unavailable);


    //Roadworks
   // As there is no proper "SpeedLimit" field inside a DENM message, for an area speed Advisor, we rely on the
   // RoadWorksContainerExtended, inside the "A la carte" container, which actually has a "SpeedLimit" field

    // Set a speed limit Advisor inside the DENM message

    roadworks.speedLimit.setData (25);

    alacartedata.roadWorks.setData (roadworks); 

    data.setDenmAlacarteData_asn_types (alacartedata);

    trigger_retval=m_denService.appDENM_trigger(data,m_current_action_id);
    if(trigger_retval!=DENM_NO_ERROR)
    {
      NS_LOG_ERROR("Cannot trigger DENM. Error code: " << trigger_retval);
    }
    else
    {
      m_denm_sent++;
    }

    m_update_denm_ev = Simulator::Schedule (Seconds (1), &areaSpeedAdvisorServer80211p::UpdateDenm, this);
  }

  void
  areaSpeedAdvisorServer80211p::UpdateDenm()
  {
    denData data;
    DEN_RoadWorksContainerExtended_t roadworks;
    denData::denDataAlacarte alacartedata;
    DENBasicService_error_t update_retval;

    /* Build DENM data */
    data.setDenmMandatoryFields (compute_timestampIts(),Latitude_unavailable,Longitude_unavailable);

    //Roadworks
   // As there is no proper "SpeedLimit" field inside a DENM message, for an area speed Advisor, we rely on the
   // RoadWorksContainerExtended, inside the "A la carte" container, which actually has a "SpeedLimit" field

    // Set a speed limit Advisor inside the DENM message

    roadworks.speedLimit.setData (25);

    alacartedata.roadWorks.setData (roadworks);
    data.setDenmAlacarteData_asn_types (alacartedata);

    update_retval = m_denService.appDENM_update (data,m_current_action_id);
    if(update_retval!=DENM_NO_ERROR)
      {
        NS_LOG_ERROR("Cannot terminate DENM. Error code: " << update_retval);
      }
    else
      {
        m_denm_sent++;
      }

    m_update_denm_ev = Simulator::Schedule (Seconds (1), &areaSpeedAdvisorServer80211p::UpdateDenm, this);

  }

  void
  areaSpeedAdvisorServer80211p::TerminateDenmTransmission()
  {
    m_isTransmittingDENM=false;
    Simulator::Cancel (m_update_denm_ev);
  }

  void
  areaSpeedAdvisorServer80211p::receiveCAM (asn1cpp::Seq<CAM> cam, Address from)
  {
    /* The reception of a CAM, in this case, woarks as a trigger to generate DENMs.
     * If no CAMs are received, then no DENMs are generated */
    m_cam_received++;

    if(m_isTransmittingDENM)
    {
      /* If a CAM is received when the server is already sending DENMs, then reset the "timer" to stop the DENM generation */
      Simulator::Cancel (m_terminate_denm_ev);
      m_terminate_denm_ev = Simulator::Schedule (Seconds (5), &areaSpeedAdvisorServer80211p::TerminateDenmTransmission, this);
    }
    else
    {
        /* If a CAM is received when the server is not sending DENMs, then start the DENM generation and call the
         * TerminateDenmTransmission after 5 seconds */
        m_isTransmittingDENM=true;
        areaSpeedAdvisorServer80211p::TriggerDenm();
        m_terminate_denm_ev = Simulator::Schedule (Seconds (5), &areaSpeedAdvisorServer80211p::TerminateDenmTransmission, this);
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

//    ASN_STRUCT_FREE(asn_DEF_CAM,cam);
  }

  void
  areaSpeedAdvisorServer80211p::receiveDENM (denData denm, Address from)
  {
    /* This is just a sample dummy receiveDENM function. The user can customize it to parse the content of a DENM when it is received. */
    (void) denm; // Contains the data received from the DENM
    (void) from; // Contains the address from which the DENM has been received
    std::cout<<"Received a new DENM."<<std::endl;
  }

  long
  areaSpeedAdvisorServer80211p::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }

  void
  areaSpeedAdvisorServer80211p::aggregateOutput()
  {
    std::cout << Simulator::Now () << "," << m_cam_received << "," << m_denm_sent << std::endl;
    m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &areaSpeedAdvisorServer80211p::aggregateOutput, this);
  }

}







