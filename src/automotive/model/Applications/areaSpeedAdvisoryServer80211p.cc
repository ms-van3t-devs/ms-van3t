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

#include "areaSpeedAdvisoryServer80211p.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/socket.h"
#include "ns3/btpdatarequest.h"
#include "ns3/network-module.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("areaSpeedAdvisoryServer80211p");

  NS_OBJECT_ENSURE_REGISTERED(areaSpeedAdvisoryServer80211p);

  TypeId
  areaSpeedAdvisoryServer80211p::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::areaSpeedAdvisoryServer80211p")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<areaSpeedAdvisoryServer80211p> ()
        .AddAttribute ("AggregateOutput",
           "If it is true, the server will print every second an aggregate output about cam and denm",
           BooleanValue (false),
           MakeBooleanAccessor (&areaSpeedAdvisoryServer80211p::m_aggregate_output),
           MakeBooleanChecker ())
        .AddAttribute ("RealTime",
           "To compute properly timestamps",
           BooleanValue(false),
           MakeBooleanAccessor (&areaSpeedAdvisoryServer80211p::m_real_time),
           MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&areaSpeedAdvisoryServer80211p::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("Client",
           "TraCI client for SUMO",
           PointerValue (0),
           MakePointerAccessor (&areaSpeedAdvisoryServer80211p::m_client),
           MakePointerChecker<TraciClient> ());
        return tid;
  }

  areaSpeedAdvisoryServer80211p::areaSpeedAdvisoryServer80211p ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_cam_received = 0;
    m_denm_sent = 0;
    m_isTransmittingDENM = false;
  }

  areaSpeedAdvisoryServer80211p::~areaSpeedAdvisoryServer80211p ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  areaSpeedAdvisoryServer80211p::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  areaSpeedAdvisoryServer80211p::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);

    /*
     * In this example, the speed control is actuated in a small area at the center of the map (a circular area with 90m of radius).
     * In the inner area the maximum allowed speed is 6.94 m/s (25 km/h) while outside is 20.83 m/s (75 km/h).
     * The application will first of all determine this area, and then it will GeoBroadcast DENMs in that area, by specifying
     * in the RoadWorks container, the speed limit to be applied.
     * The DENM dissemination starts only when the RSU receives CAMs from vehicles. The DENMs generation is stopped
     * if no CAM is received by the RSU for more than 5 seconds.
    */

    /* Compute GeoBroadcast area */
    libsumo::TraCIPositionVector net_boundaries = m_client->TraCIAPI::simulation.getNetBoundary ();
    libsumo::TraCIPosition pos1;
    libsumo::TraCIPosition pos2;
    libsumo::TraCIPosition map_center;
    /* Convert (x,y) to (long,lat) */
    // Long = x, Lat = y
    pos1 = m_client->TraCIAPI::simulation.convertXYtoLonLat (net_boundaries[0].x,net_boundaries[0].y);
    pos2 = m_client->TraCIAPI::simulation.convertXYtoLonLat (net_boundaries[1].x,net_boundaries[1].y);
    /* Check the center of the map */
    map_center.x = (pos1.x + pos2.x)/2;
    map_center.y = (pos1.y + pos2.y)/2;

    /* Compute GeoArea for DENMs */
    GeoArea_t geoArea;
    // Longitude and Latitude in [0.1 microdegree]
    geoArea.posLong = map_center.x*DOT_ONE_MICRO;
    geoArea.posLat = map_center.y*DOT_ONE_MICRO;
    // Radius [m] of the circle that covers the whole square area of the map in (x,y)
    geoArea.distA = 90;
    // DistB [m] and angle [deg] equal to zero because we are defining a circular area as specified in ETSI EN 302 636-4-1 [9.8.5.2]
    geoArea.distB = 0;
    geoArea.angle = 0;

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
    m_btp->setGeoNet(m_geoNet);
    m_denService.setBTP(m_btp);
    m_caService.setBTP(m_btp);

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setSocketTx (m_socket);
    m_denService.setSocketRx (m_socket);
    m_denService.addDENRxCallback (std::bind(&areaSpeedAdvisoryServer80211p::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));

    // Setting geoArea address for denms
    m_denService.setGeoArea (geoArea);

    // Setting a station ID (for instance, 777888999)
    m_denService.setStationProperties (777888999, StationType_roadSideUnit);

    /* Set callback and station properties in CABasicService (which will only be used to receive CAMs) */
    m_caService.setStationProperties (777888999, StationType_roadSideUnit);
    m_caService.setSocketRx (m_socket);
    m_caService.addCARxCallback (std::bind(&areaSpeedAdvisoryServer80211p::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.open (m_csv_name+"-server.csv",std::ofstream::trunc);
      m_csv_ofstream_cam << "messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration" << std::endl;
    }

    /* If aggregate output is enabled, start it */
    if (m_aggregate_output)
      m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &areaSpeedAdvisoryServer80211p::aggregateOutput, this);
  }

  void
  areaSpeedAdvisoryServer80211p::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel (m_aggegateOutputEvent);
    Simulator::Cancel (m_terminate_denm_ev);

    m_denService.cleanup();

    if (!m_csv_name.empty ())
      m_csv_ofstream_cam.close ();

    if (m_aggregate_output)
      std::cout << Simulator::Now () << "," << m_cam_received  << "," << m_denm_sent << std::endl;
  }

  void
  areaSpeedAdvisoryServer80211p::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  areaSpeedAdvisoryServer80211p::TriggerDenm ()
  {
    denData data;
    denData::denDataAlacarte alacartedata;
    DENBasicService_error_t trigger_retval;

    memset(&alacartedata,0,sizeof(denData::denDataAlacarte));

    /* Build DENM data */
    data.setDenmMandatoryFields (compute_timestampIts(),Latitude_unavailable,Longitude_unavailable);

    // As there is no proper "SpeedLimit" field inside a DENM message, for an area speed advisory, we rely on the
    // RoadWorksContainerExtended, inside the "A la carte" container, which actually has a "SpeedLimit" field
    alacartedata.roadWorks = (sRoadWorksContainerExtended_t *) calloc(sizeof(sRoadWorksContainerExtended_t),1);

    if(alacartedata.roadWorks == NULL)
      {
        NS_LOG_ERROR("Cannot send DENM. Unable to allocate memory for alacartedata.roadWorks.");
        return;
      }

    alacartedata.roadWorks->speedLimit = (SpeedLimit_t *) calloc(sizeof(SpeedLimit_t),1);

    if(alacartedata.roadWorks->speedLimit == NULL)
      {
        free(alacartedata.roadWorks);
        NS_LOG_ERROR("Cannot send DENM. Unable to allocate memory for alacartedata.roadWorks->speedLimit.");
        return;
      }

    // Set a speed limit advisory inside the DENM message
    *(alacartedata.roadWorks->speedLimit) = (SpeedLimit_t) 25;

    data.setDenmAlacarteData_asn_types (alacartedata);

    trigger_retval=m_denService.appDENM_trigger(data,m_current_action_id);
    if(trigger_retval!=DENM_NO_ERROR)
    {
      std::cout<<"EH! ERRORE!"<<std::endl;
      NS_LOG_ERROR("Cannot trigger DENM. Error code: " << trigger_retval);
    }
    else
    {
      m_denm_sent++;
    }

    if(alacartedata.roadWorks && alacartedata.roadWorks->speedLimit) free(alacartedata.roadWorks->speedLimit);
    if(alacartedata.roadWorks) free(alacartedata.roadWorks);

    data.denDataFree ();

    m_update_denm_ev = Simulator::Schedule (Seconds (1), &areaSpeedAdvisoryServer80211p::UpdateDenm, this);
  }

  void
  areaSpeedAdvisoryServer80211p::UpdateDenm()
  {
    denData data;
    denData::denDataAlacarte alacartedata;
    DENBasicService_error_t update_retval;

    memset(&alacartedata,0,sizeof(denData::denDataAlacarte));

    /* Build DENM data */
    data.setDenmMandatoryFields (compute_timestampIts(),Latitude_unavailable,Longitude_unavailable);

    // As there is no proper "SpeedLimit" field inside a DENM message, for an area speed advisory, we rely on the
    // RoadWorksContainerExtended, inside the "A la carte" container, which actually has a "SpeedLimit" field
    alacartedata.roadWorks = (sRoadWorksContainerExtended_t *) calloc(sizeof(sRoadWorksContainerExtended_t),1);

    if(alacartedata.roadWorks == NULL)
    {
      NS_LOG_ERROR("Cannot send DENM. Unable to allocate memory for alacartedata.roadWorks.");
      return;
    }

    alacartedata.roadWorks->speedLimit = (SpeedLimit_t *) calloc(sizeof(SpeedLimit_t),1);

    if(alacartedata.roadWorks->speedLimit == NULL)
    {
      free(alacartedata.roadWorks);
      NS_LOG_ERROR("Cannot send DENM. Unable to allocate memory for alacartedata.roadWorks->speedLimit.");
      return;
    }

    // Set a speed limit advisory inside the DENM message
    *(alacartedata.roadWorks->speedLimit) = (SpeedLimit_t) 25;

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

    data.denDataFree ();
    m_update_denm_ev = Simulator::Schedule (Seconds (1.1), &areaSpeedAdvisoryServer80211p::UpdateDenm, this);

  }

  void
  areaSpeedAdvisoryServer80211p::TerminateDenmTransmission()
  {
    m_isTransmittingDENM=false;
    Simulator::Cancel (m_update_denm_ev);
  }

  void
  areaSpeedAdvisoryServer80211p::receiveCAM (CAM_t *cam, Address address)
  {
    /* The reception of a CAM, in this case, woarks as a trigger to generate DENMs.
     * If no CAMs are received, then no DENMs are generated */
    m_cam_received++;

    if(m_isTransmittingDENM)
    {
      /* If a CAM is received when the server is already sending DENMs, then reset the "timer" to stop the DENM generation */
      Simulator::Cancel (m_terminate_denm_ev);
      m_terminate_denm_ev = Simulator::Schedule (Seconds (5), &areaSpeedAdvisoryServer80211p::TerminateDenmTransmission, this);
    }
    else
    {
        /* If a CAM is received when the server is not sending DENMs, then start the DENM generation and call the
         * TerminateDenmTransmission after 5 seconds */
        m_isTransmittingDENM=true;
        areaSpeedAdvisoryServer80211p::TriggerDenm();
        m_terminate_denm_ev = Simulator::Schedule (Seconds (5), &areaSpeedAdvisoryServer80211p::TerminateDenmTransmission, this);
    }

    if (!m_csv_name.empty ())
      {
        // messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration
        m_csv_ofstream_cam << cam->header.messageID << "," << cam->header.stationID << ",";
        m_csv_ofstream_cam << cam->cam.generationDeltaTime << "," << (double)cam->cam.camParameters.basicContainer.referencePosition.latitude/DOT_ONE_MICRO << ",";
        m_csv_ofstream_cam << (double)cam->cam.camParameters.basicContainer.referencePosition.longitude/DOT_ONE_MICRO << "," ;
        m_csv_ofstream_cam << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue/DECI << "," << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue/CENTI << ",";
        m_csv_ofstream_cam << (double)cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationValue/DECI << std::endl;
      }

    ASN_STRUCT_FREE(asn_DEF_CAM,cam);
  }

  void
  areaSpeedAdvisoryServer80211p::receiveDENM (denData denm, Address from)
  {
    /* This is just a sample dummy receiveDENM function. The user can customize it to parse the content of a DENM when it is received. */
    (void) denm; // Contains the data received from the DENM
    (void) from; // Contains the address from which the DENM has been received
    std::cout<<"Received a new DENM."<<std::endl;
  }

  long
  areaSpeedAdvisoryServer80211p::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }

  void
  areaSpeedAdvisoryServer80211p::aggregateOutput()
  {
    std::cout << Simulator::Now () << "," << m_cam_received << "," << m_denm_sent << std::endl;
    m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &areaSpeedAdvisoryServer80211p::aggregateOutput, this);
  }

}







