#include "trafficManagerServerLTE.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/socket.h"
#include "ns3/btpdatarequest.h"
#include "ns3/network-module.h"

#define YELLOW_DURATION 4
#define NS_GREEN_PHASE 0
#define NS_YELLOW_PHASE 1
#define EW_GREEN_PHASE 2
#define EW_YELLOW_PHASE 3
#define PERCENTAGE 100

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("trafficManagerServerLTE");

  NS_OBJECT_ENSURE_REGISTERED(trafficManagerServerLTE);

  TypeId
  trafficManagerServerLTE::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::trafficManagerServerLTE")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<trafficManagerServerLTE> ()
        .AddAttribute ("RealTime",
           "To compute properly timestamps",
           BooleanValue(false),
           MakeBooleanAccessor (&trafficManagerServerLTE::m_real_time),
           MakeBooleanChecker ())
        .AddAttribute ("PhaseTime",
           "PhaseTime",
           UintegerValue (0),
           MakeUintegerAccessor (&trafficManagerServerLTE::m_phase_time),
           MakeUintegerChecker<uint16_t> ())
        .AddAttribute ("Threshold",
           "Threshold",
           DoubleValue (0),
           MakeDoubleAccessor (&trafficManagerServerLTE::m_threshold),
           MakeDoubleChecker<double> ())
        .AddAttribute ("Client",
           "TraCI client for SUMO",
           PointerValue (0),
           MakePointerAccessor (&trafficManagerServerLTE::m_client),
           MakePointerChecker<TraciClient> ())
        .AddAttribute ("MetricSupervisor",
            "Metric Supervisor to compute Metric according to 3GPP TR36.885 V14.0.0 page 70",
            PointerValue (0),
            MakePointerAccessor (&trafficManagerServerLTE::m_metric_supervisor),
            MakePointerChecker<MetricSupervisor> ());
        return tid;
  }

  trafficManagerServerLTE::trafficManagerServerLTE ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_cam_received = 0;
    m_threshold = 70;
    m_phase_time = 60;
    m_tls_lanes_number =0;
  }

  trafficManagerServerLTE::~trafficManagerServerLTE ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  trafficManagerServerLTE::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  trafficManagerServerLTE::StartApplication (void)
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

    /* TX socket for DENMs and RX socket for CAMs */
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

    m_socket = Socket::CreateSocket (GetNode (), tid);

    /* Bind the socket to receive packets coming from every IP */
    InetSocketAddress local_denm = InetSocketAddress (Ipv4Address::GetAny (), 9);

    // Bind the socket to local address
    if (m_socket->Bind (local_denm) == -1)
    {
      NS_FATAL_ERROR ("Failed to bind server UDP socket");
    }

    /* Create new BTP and GeoNet objects and set them in DENBasicService and CABasicService */
    m_btp = CreateObject <btp>();
    m_geoNet = CreateObject <GeoNet>();

    if(m_metric_supervisor!=nullptr)
    {
      m_geoNet->setMetricSupervisor(m_metric_supervisor);
    }

    m_btp->setGeoNet(m_geoNet);
    m_denService.setBTP(m_btp);
    m_caService.setBTP(m_btp);

    /* Set sockets, callback and station properties in DENBasicService */
    m_denService.setSocketTx (m_socket);
    m_denService.setSocketRx (m_socket);
    m_denService.addDENRxCallback (std::bind(&trafficManagerServerLTE::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));

    /* Setting a station ID (for instance, 777888999) */
    m_denService.setStationProperties (777888999, StationType_roadSideUnit);

    /* Set callback and station properties in CABasicService (which will only be used to receive CAMs) */
    m_caService.setStationProperties (777888999, StationType_roadSideUnit);
    m_caService.setSocketRx (m_socket);
    m_caService.addCARxCallback (std::bind(&trafficManagerServerLTE::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));

    /*
     * Set the RSU position in the CA and DEN basic service (mandatory for any RSU object)
     * As the position must be specified in (lat, lon), we must take it from the mobility model
     * and then convert it to Latitude and Longitude
     * As SUMO is used here, we can rely on the TraCIAPI for this conversion
     */
    libsumo::TraCIPosition servicePos = m_client->TraCIAPI::simulation.convertXYtoLonLat (0,0);
    m_denService.setFixedPositionRSU (servicePos.y,servicePos.x);
    m_caService.setFixedPositionRSU (servicePos.y,servicePos.x);

    std::vector<std::string> tls_id_vec = m_client->TraCIAPI::trafficlights.getIDList ();
    for (std::string tlsid : tls_id_vec)
      {
        for (std::string lanes : m_client->TraCIAPI::trafficlights.getControlledLanes (tlsid))
        {
            if(std::find(m_tls_lane_map[tlsid].begin (),m_tls_lane_map[tlsid].end (),lanes.substr(0,lanes.find_last_of("_")))==m_tls_lane_map[tlsid].end ())
              {
                m_tls_lane_map[tlsid].push_back(lanes.substr(0,lanes.find_last_of("_")));
                m_tls_lanes_number++;
              }
        }
        m_timeout_map[tlsid] = Simulator::Schedule(Seconds(m_phase_time),&trafficManagerServerLTE::phaseTimeout,this,tlsid);
      }
  }

  void
  trafficManagerServerLTE::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    m_denService.cleanup();
  }

  void
  trafficManagerServerLTE::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  trafficManagerServerLTE::receiveCAM (asn1cpp::Seq<CAM> cam, Address address)
  {
    (void) address;

    m_cam_received++;
    bool isInside = false;
    std::map<std::string,unsigned long> current_load_map = {};
    std::map<std::string,std::pair<double,double>> current_load_ratio_map = {};


    double lat = asn1cpp::getField(cam->cam.camParameters.basicContainer.referencePosition.latitude,double)/DOT_ONE_MICRO;
    double lon = asn1cpp::getField(cam->cam.camParameters.basicContainer.referencePosition.longitude,double)/DOT_ONE_MICRO;

    /* Based on the Lon,Lat exctracted by the map, get the edgeId using TraCI */
    libsumo::TraCIRoadPosition roadMap = m_client->TraCIAPI::simulation.convertLonLattoRoadmap (lon,lat);

    for (auto tls_it = m_tls_lane_map.begin (); tls_it != m_tls_lane_map.end (); tls_it++)
    {
        if(std::find(tls_it->second.begin (),tls_it->second.end (),roadMap.edgeID)!=tls_it->second.end ())
        {
          isInside=true;
        }
    }

    /* If is the first time the this veh sends a CAM, just check if it is inside and edge of interest(1-8) or outside(0), then return */
    if (m_veh_position.find (cam->header.stationId) == m_veh_position.end ())
    {
        if(isInside)
        {
          m_veh_position[cam->header.stationId] = roadMap.edgeID;
          m_veh_number[roadMap.edgeID]++;
        }
        else
        {
          // Empty string -> we are not interested in an edge which is outside the ones we are interested in
          m_veh_position[cam->header.stationId] = "";
          m_veh_number[""]++;
        }
        return;
    }

    /* Otherwise, check if a transition happened */
    if (isInside)
    {
      /* The vehice is now in an edge of interest
       * If it was registered in other edge, update traffic lights state */
      if (m_veh_position[cam->header.stationId] != roadMap.edgeID)
      {
          m_veh_number[m_veh_position[cam->header.stationId]]--;
          m_veh_number[roadMap.edgeID]++;
          m_veh_position[cam->header.stationId] = roadMap.edgeID;
      }
    }
    else
    {
      /* The vehicle is now outside of any edge of interest
       * If it was registered inside one, then update traffic lights state */
      if (!m_veh_position[cam->header.stationId].empty ())
      {
          m_veh_number[m_veh_position[cam->header.stationId]]--;
          m_veh_number[""]++;
          m_veh_position[cam->header.stationId] = "";
      }
    }

    /* Compute current traffic load on each intersection*/
    for (auto veh_number = m_veh_number.begin (); veh_number != m_veh_number.end (); veh_number++)
    {
        for(auto intersection = m_tls_lane_map.begin (); intersection != m_tls_lane_map.end (); intersection++)
          {
            auto intersection_name = std::find(intersection->second.begin (),intersection->second.end (),veh_number->first);
            if(intersection_name!=intersection->second.end ())
            {
                current_load_map[intersection->first] +=  veh_number->second;
            }
          }
    }

    /*If all lanes relative to the tls are populated*/
    if(m_veh_number.size ()> m_tls_lanes_number)
      {
        for( auto intersection = m_tls_lane_map.begin ();intersection != m_tls_lane_map.end (); intersection++)
          {
            /* In this example the intersections have North-South and East-West directions
             * This application will work with any direction as long as the tls lanes are introduced consecutively
             * in this case NORTH-EAST-SOUTH-WEST
             */
            std::pair<double,double> ratio;
            /*Compute North-South direction load ratio*/
            ratio.first = (double)(m_veh_number[intersection->second[NORTH]]+m_veh_number[intersection->second[SOUTH]])/current_load_map[intersection->first];
            /*Compute East-West direction load ratio*/
            ratio.second =(double)(m_veh_number[intersection->second[EAST]]+m_veh_number[intersection->second[WEST]])/current_load_map[intersection->first];

            m_load_Mutex.lock ();
            m_load[intersection->first] = ratio; //In case this map is being read by the phaseTimeout function
            m_load_Mutex.unlock ();

            /* Store current phase of this intersection*/
            m_phase_map[intersection->first] = m_client->TraCIAPI::trafficlights.getPhase (intersection->first);

            if(((m_load[intersection->first].first > m_threshold/PERCENTAGE) && (m_phase_map[intersection->first] == EW_GREEN_PHASE)) ||
                    ((m_load[intersection->first].second > m_threshold/PERCENTAGE) && (m_phase_map[intersection->first] == NS_GREEN_PHASE)))
              {
                 /*If any direction ratio is higher than the threshold, and we are in the green phase of the opposite direction, change phase now */
                  Simulator::Cancel (m_timeout_map[intersection->first]);
                  m_timeout_map[intersection->first] = Simulator::Schedule(MilliSeconds (1),&trafficManagerServerLTE::phaseTimeout,this,intersection->first);
              }
          }
      }
  }

  void
  trafficManagerServerLTE::receiveDENM (denData denm, Address from)
  {
    /* This is just a sample dummy receiveDENM function. The user can customize it to parse the content of a DENM when it is received. */
    (void) denm; // Contains the data received from the DENM
    (void) from; // Contains the address from which the DENM has been received
    std::cout<<"Received a new DENM."<<std::endl;
  }

  long
  trafficManagerServerLTE::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }

  void
  trafficManagerServerLTE::phaseTimeout (std::string intersection)
  {
    uint16_t phase;

    phase = m_client->TraCIAPI::trafficlights.getPhase (intersection);

    m_load_Mutex.lock ();
    if(phase == NS_GREEN_PHASE)
      {
        m_timeout_map[intersection] = Simulator::Schedule(Seconds((uint16_t)((m_phase_time * m_load[intersection].second) + YELLOW_DURATION)),&trafficManagerServerLTE::phaseTimeout,this,intersection);
        phase = NS_YELLOW_PHASE;
      }
    else
      {
        m_timeout_map[intersection] = Simulator::Schedule(Seconds((uint16_t)((m_phase_time * m_load[intersection].first) + YELLOW_DURATION)),&trafficManagerServerLTE::phaseTimeout,this,intersection);
        phase = EW_YELLOW_PHASE;
      }
    m_load_Mutex.unlock ();

    m_client->TraCIAPI::trafficlights.setPhase (intersection,phase);
  }


}
