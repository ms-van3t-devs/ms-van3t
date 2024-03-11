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

#include "emergencyVehicleWarningServer.h"
#include "emergencyVehicleWarningClient.h"

#include "ns3/CAM.h"
#include "ns3/DENM.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/gn-utils.h"
#include "ns3/iviService.h"

#define PROVIDER_IDENTIFIER 16383
#define COUNTRY_CODE 0000


namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("emergencyVehicleWarningServer");

  NS_OBJECT_ENSURE_REGISTERED(emergencyVehicleWarningServer);

  TypeId
  emergencyVehicleWarningServer::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::emergencyVehicleWarningServer")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<emergencyVehicleWarningServer> ()
        .AddAttribute ("AggregateOutput",
           "If it is true, the server will print every second an aggregate output about cam and denm",
           BooleanValue (false),
           MakeBooleanAccessor (&emergencyVehicleWarningServer::m_aggregate_output),
           MakeBooleanChecker ())
        .AddAttribute ("RealTime",
           "To compute properly timestamps",
           BooleanValue(false),
           MakeBooleanAccessor (&emergencyVehicleWarningServer::m_real_time),
           MakeBooleanChecker ())
        .AddAttribute ("CSV",
            "CSV log name",
            StringValue (),
            MakeStringAccessor (&emergencyVehicleWarningServer::m_csv_name),
            MakeStringChecker ())
        .AddAttribute ("PRRSupervisor",
            "PRR Supervisor to compute PRR according to 3GPP TR36.885 V14.0.0 page 70",
            PointerValue (0),
            MakePointerAccessor (&emergencyVehicleWarningServer::m_PRR_supervisor),
            MakePointerChecker<PRRSupervisor> ())
        .AddAttribute ("Client",
           "TraCI client for SUMO",
           PointerValue (0),
           MakePointerAccessor (&emergencyVehicleWarningServer::m_client),
           MakePointerChecker<TraciClient> ());
        return tid;
  }

  emergencyVehicleWarningServer::emergencyVehicleWarningServer ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_cam_received = 0;
    m_denm_sent = 0;
    m_ivim_sent = 0;
    m_isTransmittingDENM = false;
  }

  emergencyVehicleWarningServer::~emergencyVehicleWarningServer ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  emergencyVehicleWarningServer::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }


  void
  emergencyVehicleWarningServer::StartApplication (void)
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

    /* Compute GeoArea for IVIMs */
    GeoArea_t geoArea;
    // Longitude and Latitude in [0.1 microdegree]
    geoArea.posLong = map_center.x*DOT_ONE_MICRO;
    geoArea.posLat = map_center.y*DOT_ONE_MICRO;
    // Radius [m] of the circle that covers the whole square area of the map in (x,y)
    geoArea.distA = 6000;
    // DistB [m] and angle [deg] equal to zero because we are defining a circular area as specified in ETSI EN 302 636-4-1 [9.8.5.2]
    geoArea.distB = 0;
    geoArea.angle = 0;
    geoArea.shape = CIRCULAR;



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
    m_caService.setBTP(m_btp);
    m_iviService.setBTP(m_btp);

    /* Set sockets, callback and station properties in iviService */
    m_iviService.setSocketTx (m_socket);
    m_iviService.setSocketRx (m_socket);
    m_iviService.addIVIRxCallback (std::bind(&emergencyVehicleWarningServer::receiveIVIM,this,std::placeholders::_1,std::placeholders::_2));

    // Setting geoArea address for ivims and realTime
    m_iviService.setGeoArea (geoArea);
    m_iviService.setRealTime (m_real_time);


    // Setting a station ID (for instance, 777888999)
    m_iviService.setStationProperties (777888999, StationType_roadSideUnit);

    /* Set callback and station properties in CABasicService (which will only be used to receive CAMs) */
    m_caService.setStationProperties (777888999, StationType_roadSideUnit);
    m_caService.setSocketRx (m_socket);
    m_caService.addCARxCallback (std::bind(&emergencyVehicleWarningServer::receiveCAM,this,std::placeholders::_1,std::placeholders::_2));

    // Set the RSU position in the CA,DEN and IVI basic service (mandatory for any RSU object)
    // As the position must be specified in (lat, lon), we must take it from the mobility model and then convert it to Latitude and Longitude
    // As SUMO is used here, we can rely on the TraCIAPI for this conversion
    Ptr<MobilityModel> mob = GetNode ()->GetObject<MobilityModel>();
    libsumo::TraCIPosition rsuPos = m_client->TraCIAPI::simulation.convertXYtoLonLat (mob->GetPosition ().x,mob->GetPosition ().y);
    m_caService.setFixedPositionRSU (rsuPos.y,rsuPos.x);
    m_iviService.setFixedPositionRSU (rsuPos.y,rsuPos.x);

    if (!m_csv_name.empty ())
    {
      m_csv_ofstream_cam.open (m_csv_name+"-server.csv",std::ofstream::trunc);
      m_csv_ofstream_cam << "messageId,camId,timestamp,latitude,longitude,heading,speed,acceleration,x,y,stationtype" << std::endl;
    }

    /* If aggregate output is enabled, start it */
    if (m_aggregate_output)
      m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &emergencyVehicleWarningServer::aggregateOutput, this);


    TriggerIvim ();


  }

  void
  emergencyVehicleWarningServer::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel (m_aggegateOutputEvent);
    Simulator::Cancel (m_terminate_denm_ev);

    if (!m_csv_name.empty ())
      m_csv_ofstream_cam.close ();

    if (m_aggregate_output)
      std::cout << Simulator::Now () << "," << m_cam_received  << "," << m_denm_sent << std::endl;
  }

  void
  emergencyVehicleWarningServer::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  emergencyVehicleWarningServer::TriggerIvim ()
  {
    // Trigger the iviData
    m_ivim_sent++;

    libsumo::TraCIPosition refPos,deltaPosDet,deltaPosRel;

    /*
     *   ZONES DESCRIPTION
     *
     *      x=-65     Relevance Zone                 Detection Zone   x=130
     *       |<-------------------------------> <--------------------->|
     * ________________________________________|____________________________
     *                                         |
     *                                         |
     *                                         |
     * - - - - - - - - - - - x - - - - - - - - X - - - - - x - - - - - - - -  y = -1
     *                       |                 |           |
     *                       |                 |           |
     *                       |                 |           |
     * ----------------------|-----------------|-----------|----------------
     * x-axis               17.5              100         115
     *                  DeltaPosRel          RefPos     DeltaPosDet
     * */


    refPos = m_client->TraCIAPI::simulation.convertXYtoLonLat (100,-1); //Reference Position
    deltaPosDet = m_client->TraCIAPI::simulation.convertXYtoLonLat (115,-1);//Delta Position for Detection zone
    deltaPosRel = m_client->TraCIAPI::simulation.convertXYtoLonLat (17.5,-1);//Delta Position for Relevance zone


    /*IVI data creation */
    iviData Data;
    iviData::iviDataMandatory mandatoryData;
    mandatoryData.countryCode = COUNTRY_CODE;
    mandatoryData.providerIdentifier = PROVIDER_IDENTIFIER;
    Data.setIvimMandatory (mandatoryData);
    Data.setOptionalPresent(true,true,false,true,false);

    // General IVI container
    iviData::IVI_gic_t gic;
    iviData::IVI_gic_part_t gic_part;
    std::vector<iviData::gicPartRS_t> roadSignCodes;
    std::vector<long> detzoneIDs,relzoneIDs;
    gic_part.iviType = IviType_trafficRelatedInformationMessages;
    detzoneIDs.push_back (1); //ZoneID=1 --> DetectionZone
    gic_part.detectionZoneIds.setData (detzoneIDs);
    relzoneIDs.push_back (2); //ZoneID=2 --> RelevanceZone
    gic_part.relevanceZoneIds.setData (relzoneIDs);
    gic_part.direction.setData (Direction_sameDirection);

    iviData::gicPartRS RS;
    RS.RS_spm.setData (30);
    RS.RS_unit.setData (RSCUnit_kmperh);
    RS.RS_nature = 6;
    RS.RS_serialNumber = 59;
    RS.RS_trafficSignPictogram.setData (ISO14823Code__pictogramCode__serviceCategoryCode__trafficSignPictogram_informative);
    roadSignCodes.push_back (RS);

    gic_part.RS.push_back (RS);
    gic.GicPart.push_back (gic_part);

    Data.setIvimGic (gic);

    //Geographical Location Container
    iviData::IVI_glc_t glc;
    glc.referencePosition.latitude = refPos.y*DOT_ONE_MICRO;
    glc.referencePosition.longitude = refPos.x*DOT_ONE_MICRO;
    glc.referencePosition.altitude.setValue (AltitudeValue_unavailable);
    glc.referencePosition.altitude.setConfidence (AltitudeConfidence_unavailable);
    glc.referencePosition.positionConfidenceEllipse.semiMajorConfidence = SemiAxisLength_unavailable;
    glc.referencePosition.positionConfidenceEllipse.semiMinorConfidence = SemiAxisLength_unavailable;
    glc.referencePosition.positionConfidenceEllipse.semiMajorOrientation = HeadingValue_wgs84North;

    iviData::IVI_glc_part_t glc_part;
    iviData::IVI_glcP_zone_t zone;
    iviData::IVI_glcP_segment_t segment;
    std::vector<iviData::IVI_glcP_deltaPosition_t> deltaPositions;
    iviData::IVI_glcP_deltaPosition_t deltaPosition;

    //First GLC part --> ZoneID=1 --> Detection Zone
    glc_part.zoneId = 1;
    deltaPosition.deltaLat = (refPos.y-deltaPosDet.y)*DOT_ONE_MICRO;
    deltaPosition.deltaLong = (refPos.x-deltaPosDet.x)*DOT_ONE_MICRO;
    deltaPositions.push_back (deltaPosition);
    segment.line.deltaPositions.setData (deltaPositions);
    segment.laneWidth.setData((long) 9);
    zone.segment.setData(segment);
    glc_part.zone.setData(zone);
    glc.GlcPart.push_back(glc_part);

    deltaPositions.clear ();

    //Second GLC part --> ZoneID=2 --> Relevance Zone
    glc_part.zoneId = 2;
    deltaPosition.deltaLat = (refPos.y-deltaPosRel.y)*DOT_ONE_MICRO;
    deltaPosition.deltaLong = (refPos.x-deltaPosRel.x)*DOT_ONE_MICRO;
    deltaPositions.push_back (deltaPosition);

    segment.line.deltaPositions.setData (deltaPositions);
    segment.laneWidth.setData((long) 9);
    zone.segment.setData(segment);
    glc_part.zone.setData(zone);
    glc.GlcPart.push_back(glc_part);

    Data.setIvimGlc (glc);


//    iviData::IVI_tc textContainer;
//    iviData::IVI_tc_part_t tc_part;
//    std::vector<long> appLanesPos;
//    std::vector<iviData::IVI_tc_text_t> textCont;
//    iviData::IVI_tc_text_t text;
//    tc_part.detectionZoneIds.setData (detzoneIDs);
//    tc_part.relevanceZoneIds = relzoneIDs;
//    tc_part.direction.setData (Direction_sameDirection); //Direction_sameDirection;
//    tc_part.data = "Left and central lane only for emergency vehicles";
//    appLanesPos.push_back (LanePosition_secondLaneFromOutside);
//    tc_part.applicableLanesPos.setData (appLanesPos);
//    text.bitLanguage = 00000;
//    text.textCont = "textCon";
//    textCont.push_back (text);
//    tc_part.text.setData (textCont);
//    textContainer.IVItcPart.push_back (tc_part);
//    Data.setIvimtc (textContainer);

    m_iviData = Data;


    m_iviService.appIVIM_trigger(Data);

    Simulator::Schedule (Seconds (1), &emergencyVehicleWarningServer::RepeatIvim, this);

  }

  void
  emergencyVehicleWarningServer::RepeatIvim ()
  {

        iviData Data = m_iviData;

        m_iviService.appIVIM_repetition(Data);

        Simulator::Schedule (Seconds (1), &emergencyVehicleWarningServer::RepeatIvim, this);


  }

  void
  emergencyVehicleWarningServer::receiveCAM (asn1cpp::Seq<CAM> cam, Address from)
  {
    m_cam_received++;

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

  long
  emergencyVehicleWarningServer::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }

  void
  emergencyVehicleWarningServer::aggregateOutput()
  {
    std::cout << Simulator::Now () << "," << m_cam_received << "," << m_denm_sent << std::endl;
    m_aggegateOutputEvent = Simulator::Schedule (Seconds(1), &emergencyVehicleWarningServer::aggregateOutput, this);
  }

  void
  emergencyVehicleWarningServer::receiveIVIM (iviData ivim, Address from)
  {

    /* Must be modified such as if the vehicle is inside the 'restricted line zone', it must follow some specific instruction
     * depending if it is an emergency vehicle or a passenger vehicle*/
    /* This is just a sample dummy receiveIVIM function. The user can customize it to parse the content of a IVIM when it is received. */
    (void) ivim; // Contains the data received from the IVIM
    (void) from; // Contains the address from which the DENM has been received
  }

}








