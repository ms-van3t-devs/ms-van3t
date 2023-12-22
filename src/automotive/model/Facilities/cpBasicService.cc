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

#include "cpBasicService.h"
namespace ns3 {

  NS_LOG_COMPONENT_DEFINE("CPBasicService");

  CPBasicService::CPBasicService()
  {
    m_station_id = ULONG_MAX;
    m_stationtype = LONG_MAX;
    m_socket_tx=NULL;
    m_btp = NULL;
    m_LDM = NULL;
    m_real_time=false;

    // Setting a default value of m_T_CheckCpmGen_ms equal to 100 ms (i.e. T_GenCpmMin_ms)
    m_T_CheckCpmGen_ms=T_GenCpmMin_ms;

    m_prev_heading=-1;
    m_prev_speed=-1;
    m_prev_distance=-1;

    m_T_GenCpm_ms=T_GenCpmMax_ms;

    lastCpmGen=-1;
    lastCpmGenLowFrequency=-1;
    lastCpmGenSpecialVehicle=-1;

    m_T_LastSensorInfoContainer = -1;

    m_N_GenCpmMax=1000;
    m_N_GenCpm=100;

    m_vehicle=true;
    m_redundancy_mitigation = true;

    m_cpm_sent=0;
  }

  void
  CPBasicService::setStationID(unsigned long fixed_stationid)
  {
    m_station_id=fixed_stationid;
    m_btp->setStationID(fixed_stationid);
  }

  void
  CPBasicService::setStationType(long fixed_stationtype)
  {
    m_stationtype=fixed_stationtype;
    m_btp->setStationType(fixed_stationtype);
  }

  void
  CPBasicService::setStationProperties(unsigned long fixed_stationid,long fixed_stationtype)
  {
    m_station_id=fixed_stationid;
    m_stationtype=fixed_stationtype;
    m_btp->setStationProperties(fixed_stationid,fixed_stationtype);
  }

  void
  CPBasicService::setSocketRx (Ptr<Socket> socket_rx)
  {
    m_btp->setSocketRx(socket_rx);
    m_btp->addCPMRxCallback (std::bind(&CPBasicService::receiveCpm,this,std::placeholders::_1,std::placeholders::_2));
  }

  void
  CPBasicService::initDissemination()
  {
    std::srand(Simulator::Now().GetNanoSeconds ());
    double desync = ((double)std::rand()/RAND_MAX);
    m_event_cpmSend = Simulator::Schedule (Seconds(desync), &CPBasicService::generateAndEncodeCPM, this);
  }

  double
  CPBasicService::cartesian_dist(double lon1, double lat1, double lon2, double lat2)
  {
    libsumo::TraCIPosition pos1,pos2;
    pos1 = m_client->TraCIAPI::simulation.convertLonLattoXY(lon1,lat1);
    pos2 = m_client->TraCIAPI::simulation.convertLonLattoXY(lon2,lat2);
    return sqrt((pow((pos1.x-pos2.x),2)+pow((pos1.y-pos2.y),2)));
  }
  bool
  CPBasicService::checkCPMconditions(std::vector<LDM::returnedVehicleData_t>::iterator PO_data)
  {
    /*Perceived Object Container Inclusion Management as mandated by TR 103 562 Section 4.3.4.2*/
    std::map<uint64_t, PHData_t> phPoints = PO_data->phData.getPHpoints ();
    PHData_t previousCPM;
    /* 1.a The object has first been detected by the perception system after the last CPM generation event.*/
    if((PO_data->phData.getSize ()==1) && (PO_data->phData.getPHpoints ().begin ()->first > lastCpmGen))
      return true;

    /* Get the last position of the reference point of this object lastly included in a CPM from the object pathHistory*/
    std::map<uint64_t, PHData_t>::reverse_iterator it = phPoints.rbegin ();
    it ++;
    for(auto fromPrev = it; fromPrev!=phPoints.rend(); fromPrev++)
      {
        if (fromPrev->second.CPMincluded == true)
          {
            previousCPM = fromPrev->second;
          }
      }
    /* 1.b The Euclidian absolute distance between the current estimated position of the reference point of the
     * object and the estimated position of the reference point of this object lastly included in a CPM exceeds
     * 4 m. */
    if(cartesian_dist(previousCPM.lon,previousCPM.lat,PO_data->vehData.lon,PO_data->vehData.lat) > 4.0)
      return true;
    /* 1.c The difference between the current estimated absolute speed of the reference point of the object and the
     * estimated absolute speed of the reference point of this object lastly included in a CPM exceeds 0,5 m/s. */
    if(abs(previousCPM.speed_ms - PO_data->vehData.speed_ms) > 0.5)
      return true;
    /* 1.d The difference between the orientation of the vector of the current estimated absolute velocity of the
     * reference point of the object and the estimated orientation of the vector of the absolute velocity of the
     * reference point of this object lastly included in a CPM exceeds 4 degrees. */
    if(abs(previousCPM.heading - PO_data->vehData.heading) > 4)
      return true;
    /* 1.e The time elapsed since the last time the object was included in a CPM exceeds T_GenCpmMax. */
    if(PO_data->vehData.lastCPMincluded.isAvailable ())
      {
        if(PO_data->vehData.lastCPMincluded.getData() < ((computeTimestampUInt64 ()/NANO_TO_MILLI)-m_N_GenCpmMax))
          return true;
      }
    return false;
  }

  void
  CPBasicService::generateAndEncodeCPM()
  {
    VDP::CPM_mandatory_data_t cpm_mandatory_data;
    Ptr<Packet> packet;

    BTPDataRequest_t dataRequest = {};

    int64_t now = computeTimestampUInt64 ()/NANO_TO_MILLI;



    long numberOfPOs = 0;

    /* Collect data for mandatory containers */
    auto cpm = asn1cpp::makeSeq(CPM);

    if(bool(cpm)==false)
      {
        NS_LOG_ERROR("Warning: unable to encode CPM.");
        return;
      }

    //Schedule new CPM
    m_event_cpmSend = Simulator::Schedule (MilliSeconds (m_N_GenCpm), &CPBasicService::generateAndEncodeCPM, this);


    /* Process select Perceived Object Container Candidates as detailed in ETSI TR 103 562, ANNEX D (D.2) */
    if(m_LDM != NULL)
      {
        std::vector<LDM::returnedVehicleData_t> LDM_POs;
        if(m_LDM->getAllPOs (LDM_POs)) // If there are any POs in the LDM
          {
            auto POsContainer = asn1cpp::makeSeq(PerceivedObjectContainer);
            std::vector<LDM::returnedVehicleData_t>::iterator it;
            for(it = LDM_POs.begin (); it != LDM_POs.end ();it++)
              {               

                if(it->vehData.perceivedBy.getData () != (long) m_station_id)
                  break;
                if(!checkCPMconditions (it) && m_redundancy_mitigation)
                  break;
                else
                  {
                    auto PO = asn1cpp::makeSeq(PerceivedObject);
                    asn1cpp::setField(PO->objectID,it->vehData.stationID);
                    long timeOfMeasurement = (Simulator::Now ().GetMicroSeconds () - it->vehData.timestamp_us)/1000;// time of measuremente in ms
                    if(timeOfMeasurement > 1500)
                        timeOfMeasurement = 1500;
                    asn1cpp::setField(PO->timeOfMeasurement,timeOfMeasurement);
                    if(it->vehData.confidence.getData () < ObjectConfidence_unavailable && it->vehData.confidence.getData () > 0)
                      asn1cpp::setField(PO->objectConfidence,it->vehData.confidence.getData ());
                    else
                      asn1cpp::setField(PO->objectConfidence,ObjectConfidence_unavailable);

                    asn1cpp::setField(PO->xDistance.value,it->vehData.xDistance.getData ());
                    asn1cpp::setField(PO->xDistance.confidence,DistanceConfidence_unavailable);
                    asn1cpp::setField(PO->yDistance.value,it->vehData.yDistance.getData ());
                    asn1cpp::setField(PO->yDistance.confidence,DistanceConfidence_unavailable);
                    asn1cpp::setField(PO->xSpeed.value,it->vehData.xSpeed.getData ());
                    asn1cpp::setField(PO->xSpeed.confidence,SpeedConfidence_unavailable);
                    asn1cpp::setField(PO->ySpeed.value,it->vehData.ySpeed.getData ());
                    asn1cpp::setField(PO->ySpeed.confidence,SpeedConfidence_unavailable);
                    auto angle = asn1cpp::makeSeq(CartesianAngle);
                    if(it->vehData.angle.getData() < CartesianAngleValue_unavailable && it->vehData.angle.getData() > 0)
                      asn1cpp::setField(angle->value,it->vehData.angle.getData());
                    else
                      asn1cpp::setField(angle->value,CartesianAngleValue_unavailable);
                    asn1cpp::setField(angle->confidence,AngleConfidence_unavailable);
                    asn1cpp::setField(PO->yawAngle,angle);
                    auto OD1 = asn1cpp::makeSeq(ObjectDimension);
                    if(it->vehData.vehicleLength.getData() < 1023 && it->vehData.vehicleLength.getData() > 0)
                      asn1cpp::setField(OD1->value,it->vehData.vehicleLength.getData());
                    else
                      asn1cpp::setField(OD1->value,50);//usual value for SUMO vehicles
                    asn1cpp::setField(OD1->confidence,ObjectDimensionConfidence_unavailable);
                    asn1cpp::setField(PO->planarObjectDimension1,OD1);
                    auto OD2 = asn1cpp::makeSeq(ObjectDimension);
                    if(it->vehData.vehicleWidth.getData() < 1023 && it->vehData.vehicleWidth.getData() > 0)
                      asn1cpp::setField(OD2->value,it->vehData.vehicleWidth.getData());
                    else
                      asn1cpp::setField(OD2->value,18);//usual value for SUMO vehicles
                    asn1cpp::setField(OD2->confidence,ObjectDimensionConfidence_unavailable);
                    asn1cpp::setField(PO->planarObjectDimension2,OD2);
                    asn1cpp::setField(PO->objectRefPoint,ObjectRefPoint_topMid);

                    /*Rest of optional fields handling left as future work*/

                    //Push Perceived Object to the container
                    asn1cpp::sequenceof::pushList(*POsContainer,PO);
                    //Update the timestamp of the last time this PO was included in a CPM
                    m_LDM->updateCPMincluded (it->vehData.stationID,computeTimestampUInt64 ()/NANO_TO_MILLI);
                    //Increase number of POs for the numberOfPerceivedObjects field in cpmParameters container
                    numberOfPOs++;
                  }
              }
            if(numberOfPOs != 0)
              asn1cpp::setField(cpm->cpm.cpmParameters.perceivedObjectContainer,POsContainer);
          }
      }

    // Fill numberOfPerceivedObjects
    asn1cpp::setField(cpm->cpm.cpmParameters.numberOfPerceivedObjects,numberOfPOs);

    /* Process generate Sensor Information Container as detailed in ETSI TR 103 562, ANNEX D (D.3) */
    if(now-m_T_LastSensorInfoContainer >= m_T_AddSensorInformation)
      {
        auto sensorInfoContainer = asn1cpp::makeSeq(SensorInformationContainer);
        //For now we only consider one sensor
        //We assume a Lidar sensor of 50m sensing range from the vehicle front bumper
        auto sensorInfo = asn1cpp::makeSeq(SensorInformation);
        asn1cpp::setField(sensorInfo->sensorID,2);
        asn1cpp::setField(sensorInfo->type,SensorType_radar);
        auto detectionArea = asn1cpp::makeSeq(DetectionArea);
        asn1cpp::setField(detectionArea->present,DetectionArea_PR_vehicleSensor);
        asn1cpp::setField(detectionArea->choice.vehicleSensor.refPointId,0);
        asn1cpp::setField(detectionArea->choice.vehicleSensor.xSensorOffset,0);
        asn1cpp::setField(detectionArea->choice.vehicleSensor.ySensorOffset,0);
        auto property = asn1cpp::makeSeq(VehicleSensorProperties);
        asn1cpp::setField(property->range,50);
        asn1cpp::setField(property->horizontalOpeningAngleStart,0);
        asn1cpp::setField(property->horizontalOpeningAngleEnd,3600);//360 degrees
        asn1cpp::sequenceof::pushList(detectionArea->choice.vehicleSensor.vehicleSensorPropertyList,property);
        asn1cpp::setField(sensorInfo->detectionArea,detectionArea);
        //We ommit free space confidence
        asn1cpp::sequenceof::pushList(*sensorInfoContainer,sensorInfo);
        asn1cpp::setField(cpm->cpm.cpmParameters.sensorInformationContainer,sensorInfoContainer);

        m_T_LastSensorInfoContainer = now;
      }
    else
      {
        //If no sensorInformationContainer and no perceivedObjectsContainer
        if(numberOfPOs==0)
          return; //No CPM is generated in the current cycle
      }


    /* Fill the header */
    asn1cpp::setField(cpm->header.messageID, ItsPduHeader__messageID_cpm);
    asn1cpp::setField(cpm->header.protocolVersion, 1);
    asn1cpp::setField(cpm->header.stationID, m_station_id);

    /*
     * Compute the generationDeltaTime, "computed as the time corresponding to the
     * time of the reference position in the CPM, considered as time of the CPM generation.
     * The value of the generationDeltaTime shall be wrapped to 65 536. This value shall be set as the
     * remainder of the corresponding value of TimestampIts divided by 65 536 as below:
     * generationDeltaTime = TimestampIts mod 65 536"
    */
    asn1cpp::setField(cpm->cpm.generationDeltaTime, compute_timestampIts (m_real_time) % 65536);

    /* Fill the managementContainer's station type */
    asn1cpp::setField(cpm->cpm.cpmParameters.managementContainer.stationType, m_stationtype);

    cpm_mandatory_data=m_vdp->getCPMMandatoryData();

    /* Fill the managementContainer */
    asn1cpp::setField(cpm->cpm.cpmParameters.managementContainer.referencePosition.altitude.altitudeValue, cpm_mandatory_data.altitude.getValue ());
    asn1cpp::setField(cpm->cpm.cpmParameters.managementContainer.referencePosition.altitude.altitudeConfidence, cpm_mandatory_data.altitude.getConfidence ());
    asn1cpp::setField(cpm->cpm.cpmParameters.managementContainer.referencePosition.latitude, cpm_mandatory_data.latitude);
    asn1cpp::setField(cpm->cpm.cpmParameters.managementContainer.referencePosition.longitude, cpm_mandatory_data.longitude);
    asn1cpp::setField(cpm->cpm.cpmParameters.managementContainer.referencePosition.positionConfidenceEllipse.semiMajorConfidence, cpm_mandatory_data.posConfidenceEllipse.semiMajorConfidence);
    asn1cpp::setField(cpm->cpm.cpmParameters.managementContainer.referencePosition.positionConfidenceEllipse.semiMinorConfidence, cpm_mandatory_data.posConfidenceEllipse.semiMinorConfidence);
    asn1cpp::setField(cpm->cpm.cpmParameters.managementContainer.referencePosition.positionConfidenceEllipse.semiMajorOrientation, cpm_mandatory_data.posConfidenceEllipse.semiMajorOrientation);
    //TODO:  compute segmentInfo, get MTU and deal with needed segmentation

    /* Fill the stationDataContainer */
    auto stationDataContainer = asn1cpp::makeSeq(StationDataContainer);
    asn1cpp::setField(stationDataContainer->present, StationDataContainer_PR_originatingVehicleContainer);
    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.heading.headingValue, cpm_mandatory_data.heading.getValue ());
    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.heading.headingConfidence, cpm_mandatory_data.heading.getConfidence ());
    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.speed.speedValue, cpm_mandatory_data.speed.getValue ());
    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.speed.speedConfidence, cpm_mandatory_data.speed.getConfidence ());
    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.driveDirection, cpm_mandatory_data.driveDirection);

    auto vehicleLength = asn1cpp::makeSeq(VehicleLength);
    asn1cpp::setField(vehicleLength->vehicleLengthValue, cpm_mandatory_data.VehicleLength.getValue());
    asn1cpp::setField(vehicleLength->vehicleLengthConfidenceIndication, cpm_mandatory_data.VehicleLength.getConfidence());
    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.vehicleLength,vehicleLength);

    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.vehicleWidth, cpm_mandatory_data.VehicleWidth);

    auto longAcc = asn1cpp::makeSeq(LongitudinalAcceleration);
    asn1cpp::setField(longAcc->longitudinalAccelerationValue, cpm_mandatory_data.longAcceleration.getValue ());
    asn1cpp::setField(longAcc->longitudinalAccelerationConfidence, cpm_mandatory_data.longAcceleration.getConfidence ());
    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.longitudinalAcceleration,longAcc);

    auto yawRate = asn1cpp::makeSeq(YawRate);
    asn1cpp::setField(yawRate->yawRateValue, cpm_mandatory_data.yawRate.getValue ());
    asn1cpp::setField(yawRate->yawRateConfidence, cpm_mandatory_data.yawRate.getConfidence ());
    asn1cpp::setField(stationDataContainer->choice.originatingVehicleContainer.yawRate,yawRate);

    asn1cpp::setField(cpm->cpm.cpmParameters.stationDataContainer, stationDataContainer);


    std::string encode_result = asn1cpp::uper::encode(cpm);

    if(encode_result.size()<1)
    {
        NS_LOG_ERROR("Warning: unable to encode CPM.");
        return;
    }

    packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());

    dataRequest.BTPType = BTP_B; //!< BTP-B
    dataRequest.destPort = CP_PORT;
    dataRequest.destPInfo = 0;
    dataRequest.GNType = TSB;
    dataRequest.GNCommProfile = UNSPECIFIED;
    dataRequest.GNRepInt =0;
    dataRequest.GNMaxRepInt=0;
    dataRequest.GNMaxLife = 1;
    dataRequest.GNMaxHL = 1;
    dataRequest.GNTraClass = 0x02; // Store carry foward: no - Channel offload: no - Traffic Class ID: 2
    dataRequest.lenght = packet->GetSize ();
    dataRequest.data = packet;
    m_btp->sendBTP(dataRequest);

    m_cpm_sent++;

    // Store the time in which the last CPM (i.e. this one) has been generated and successfully sent
    m_T_GenCpm_ms=now-lastCpmGen;
    lastCpmGen = now;
  }
  void
  CPBasicService::startCpmDissemination()
  {
    // Old desync code kept just for reference
    // It may lead to nodes not being desynchronized properly in specific situations in which
    // Simulator::Now().GetNanoSeconds () returns the same seed for multiple nodes
    // std::srand(Simulator::Now().GetNanoSeconds ());
    // double desync = ((double)std::rand()/RAND_MAX);

    Ptr<UniformRandomVariable> desync_rvar = CreateObject<UniformRandomVariable> ();
    desync_rvar->SetAttribute ("Min", DoubleValue (0.0));
    desync_rvar->SetAttribute ("Max", DoubleValue (1.0));
    double desync = desync_rvar->GetValue ();
    m_event_cpmDisseminationStart = Simulator::Schedule (Seconds(desync), &CPBasicService::initDissemination, this);
  }

  uint64_t
  CPBasicService::terminateDissemination()
  {

    Simulator::Remove(m_event_cpmDisseminationStart);
    Simulator::Remove(m_event_cpmSend);
    return m_cpm_sent;
  }

  void
  CPBasicService::receiveCpm (BTPDataIndication_t dataIndication, Address from)
  {
    Ptr<Packet> packet;
    asn1cpp::Seq<CPM> decoded_cpm,cpm_test;

    uint8_t *buffer; //= new uint8_t[packet->GetSize ()];
    buffer=(uint8_t *)malloc((dataIndication.data->GetSize ())*sizeof(uint8_t));
    dataIndication.data->CopyData (buffer, dataIndication.data->GetSize ());
    std::string packetContent((char *)buffer,(int) dataIndication.data->GetSize ());

    /** Decoding **/
    decoded_cpm = asn1cpp::uper::decode(packetContent, CPM);

    if(bool(decoded_cpm)==false) {
        NS_LOG_ERROR("Warning: unable to decode a received CPM.");
        return;
      }

    m_CPReceiveCallback(decoded_cpm,from);
  }
  int64_t
  CPBasicService::computeTimestampUInt64()
  {
    int64_t int_tstamp=0;

    if (!m_real_time)
      {
        int_tstamp=Simulator::Now ().GetNanoSeconds ();
      }
    else
      {
        struct timespec tv;

        clock_gettime (CLOCK_MONOTONIC, &tv);

        int_tstamp=tv.tv_sec*1e9+tv.tv_nsec;
      }
    return int_tstamp;
  }
}
