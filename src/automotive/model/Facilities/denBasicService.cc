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

#include "denBasicService.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/Setter.hpp"
#include "ns3/Encoding.hpp"
#include "ns3/SetOf.hpp"
#include "ns3/SequenceOf.hpp"
#include "ns3/BitString.hpp"
#include "ns3/asn_application.h"
#include "ns3/snr-tag.h"
#include "ns3/rssi-tag.h"
#include "ns3/rsrp-tag.h"
#include "ns3/sinr-tag.h"
#include "ns3/timestamp-tag.h"
#include "ns3/size-tag.h"

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE("DENBasicService");

  DENBasicService::DENBasicService()
  {
    m_station_id = ULONG_MAX;
    m_stationtype = LONG_MAX;
    m_seq_number = 0;
    m_socket_tx = NULL;
    m_real_time = false;
    m_btp = NULL;

    m_DENReceiveCallback = nullptr;
    m_DENReceiveCallbackExtended = nullptr;
  }

  bool
  DENBasicService::CheckMainAttributes()
  {
    return m_station_id!=ULONG_MAX && m_stationtype!=LONG_MAX;
  }

    template<typename MEM_PTR> void
     DENBasicService::setDENTimer(Timer &timer,Time delay,MEM_PTR callback_fcn,DEN_ActionID_t actionID) {
      if(timer.IsRunning ())
        {
           timer.Cancel();
        }

      timer.SetFunction(callback_fcn,this);
      timer.SetArguments(actionID);
      timer.SetDelay (delay);

      timer.Schedule ();
    }

  DENBasicService_error_t
  DENBasicService::fillDENM(asn1cpp::Seq<DENM> &denm, denData &data, const DEN_ActionID_t actionID,long referenceTimeLong)
  {
    denData::denDataManagement mgmt_data;

    /* 1. Get Management Container */
    mgmt_data=data.getDenmMgmtData_asn_types ();

    /* 2. Transmission interval */
    if(mgmt_data.transmissionInterval.isAvailable ())
      asn1cpp::setField(denm->denm.management.transmissionInterval,mgmt_data.transmissionInterval.getData ());

    /* 3. Set all the containers [to be continued] */
    /* Header */
    asn1cpp::setField(denm->header.messageId,FIX_DENMID);
    asn1cpp::setField(denm->header.protocolVersion,2);
    asn1cpp::setField(denm->header.stationId,m_station_id);



    /* Management Container */
    asn1cpp::setField(denm->denm.management.actionID.originatingStationId,actionID.originatingStationID);
    asn1cpp::setField(denm->denm.management.actionID.sequenceNumber,actionID.sequenceNumber);
    asn1cpp::setField(denm->denm.management.detectionTime,mgmt_data.detectionTime);
    asn1cpp::setField(denm->denm.management.eventPosition.latitude,mgmt_data.latitude);
    asn1cpp::setField(denm->denm.management.eventPosition.longitude,mgmt_data.longitude);
    asn1cpp::setField(denm->denm.management.eventPosition.positionConfidenceEllipse.semiMajorConfidence,mgmt_data.posConfidenceEllipse.semiMajorConfidence);
    asn1cpp::setField(denm->denm.management.eventPosition.positionConfidenceEllipse.semiMinorConfidence,mgmt_data.posConfidenceEllipse.semiMinorConfidence);
    asn1cpp::setField(denm->denm.management.eventPosition.positionConfidenceEllipse.semiMajorOrientation,mgmt_data.posConfidenceEllipse.semiMajorOrientation);
    asn1cpp::setField(denm->denm.management.eventPosition.altitude.altitudeValue,mgmt_data.altitude.getValue ());
    asn1cpp::setField(denm->denm.management.eventPosition.altitude.altitudeConfidence,mgmt_data.altitude.getConfidence ());


    if(mgmt_data.relevanceDistance.isAvailable ())
      asn1cpp::setField(denm->denm.management.relevanceDistance,mgmt_data.relevanceDistance.getData ());

    if(mgmt_data.relevanceTrafficDirection.isAvailable ())
      asn1cpp::setField(denm->denm.management.relevanceTrafficDirection,mgmt_data.relevanceTrafficDirection.getData ());

    if(mgmt_data.termination.isAvailable ())
      asn1cpp::setField(denm->denm.management.termination,mgmt_data.termination.getData ());

    if(mgmt_data.validityDuration.isAvailable ())
      asn1cpp::setField(denm->denm.management.validityDuration,mgmt_data.validityDuration.getData ());

    asn1cpp::setField(denm->denm.management.referenceTime,referenceTimeLong);
    asn1cpp::setField(denm->denm.management.stationType,m_stationtype);

    /* Situation container */
    if(data.isDenmSituationDataSet ())
      {
         auto situation_data = data.getDenmSituationData_asn_types ();
         auto situation_seq = asn1cpp::makeSeq(SituationContainer);

         asn1cpp::setField(situation_seq->eventType.causeCode, situation_data.getData ().causeCode);
         asn1cpp::setField(situation_seq->eventType.subCauseCode, situation_data.getData ().subCauseCode);
         asn1cpp::setField(situation_seq->informationQuality, situation_data.getData().informationQuality);

         if(situation_data.getData ().linkedCauseCode.isAvailable ())
           {
             auto causeCode = asn1cpp::makeSeq(CauseCode);
             asn1cpp::setField(causeCode->causeCode,situation_data.getData ().linkedCauseCode.getData ());
             asn1cpp::setField(causeCode->subCauseCode,situation_data.getData ().linkedSubCauseCode.getData ());

             asn1cpp::setField(situation_seq->linkedCause,causeCode);
           }

         if(situation_data.getData ().eventHistory.isAvailable ())
           {
             for(size_t i=0;i<situation_data.getData ().eventHistory.getData ().size ();i++)
               {
                 auto eventPoint_data = situation_data.getData ().eventHistory.getData ()[i];
                 auto eventPoint_seq = asn1cpp::makeSeq(EventPoint);

                 asn1cpp::setField(eventPoint_seq->eventPosition.deltaLatitude,eventPoint_data.eventPosition.deltaLatitude);
                 asn1cpp::setField(eventPoint_seq->eventPosition.deltaLongitude,eventPoint_data.eventPosition.deltaLongitude);
                 asn1cpp::setField(eventPoint_seq->eventPosition.deltaAltitude,eventPoint_data.eventPosition.deltaAltitude);

                 asn1cpp::setField(eventPoint_seq->informationQuality,eventPoint_data.informationQuality);

                 if(eventPoint_data.eventDeltaTime.isAvailable ())
                   asn1cpp::setField(eventPoint_seq->eventDeltaTime,eventPoint_data.eventDeltaTime.getData ());

                 asn1cpp::sequenceof::pushList(situation_seq->eventHistory, eventPoint_seq);
               }
           }
         asn1cpp::setField(denm->denm.situation,situation_seq);
      }

    /* Location container */
    if(data.isDenmLocationDataSet ())
      {
        auto location_data=data.getDenmLocationData_asn_types ();
        auto location_seq = asn1cpp::makeSeq(LocationContainer);

        auto traces = asn1cpp::makeSeq(Traces);
        for(size_t i=0;i<location_data.getData ().traces.size ();i++)
          {
            auto pathHistory = asn1cpp::makeSeq(Path);
            for(size_t j=0;j<location_data.getData ().traces[i].size ();j++)
              {
                auto pathPoint = asn1cpp::makeSeq(PathPoint);
                asn1cpp::setField(pathPoint->pathPosition.deltaLatitude,
                                  location_data.getData ().traces[i][j].pathPosition.deltaLatitude);
                asn1cpp::setField(pathPoint->pathPosition.deltaLongitude,
                                  location_data.getData ().traces[i][j].pathPosition.deltaLongitude);
                asn1cpp::setField(pathPoint->pathPosition.deltaAltitude,
                                  location_data.getData ().traces[i][j].pathPosition.deltaAltitude);
                if(location_data.getData ().traces[i][j].pathDeltaTime.isAvailable ())
                  asn1cpp::setField(pathPoint->pathDeltaTime,location_data.getData ().traces[i][j].pathDeltaTime.getData ());

                asn1cpp::sequenceof::pushList(*pathHistory,pathPoint);
              }
            asn1cpp::sequenceof::pushList(location_seq->traces,pathHistory);
          }


        if(location_data.getData ().eventSpeed.isAvailable ())
          {
            auto eventSpeed = asn1cpp::makeSeq(Speed);
            asn1cpp::setField(eventSpeed->speedValue,location_data.getData ().eventSpeed.getData ().getValue ());
            asn1cpp::setField(eventSpeed->speedConfidence,location_data.getData ().eventSpeed.getData ().getConfidence ());

            asn1cpp::setField(location_seq->eventSpeed,eventSpeed);
          }

        if(location_data.getData ().eventPositionHeading.isAvailable ())
          {
            auto eventHeading = asn1cpp::makeSeq(Heading);
            asn1cpp::setField(eventHeading->headingValue,location_data.getData ().eventPositionHeading.getData ().getValue ());
            asn1cpp::setField(eventHeading->headingConfidence,location_data.getData ().eventPositionHeading.getData ().getConfidence ());

            asn1cpp::setField(location_seq->eventPositionHeading,eventHeading);
          }

        if(location_data.getData ().roadType.isAvailable ())
            asn1cpp::setField(location_seq->roadType,location_data.getData ().roadType.getData ());

        asn1cpp::setField(denm->denm.location,location_seq);
      }

    /* A la carte container */
    if(data.isDenmAlacarteDataSet ())
      {
        auto alacarte_data = data.getDenmAlacarteData_asn_types ();

        auto alacarte_seq = asn1cpp::makeSeq(AlacarteContainer);

        if(alacarte_data.getData ().lanePosition.isAvailable ())
          asn1cpp::setField(alacarte_seq->lanePosition,alacarte_data.getData ().lanePosition.getData ());

        if(alacarte_data.getData ().impactReduction.isAvailable ())
          {
            auto impactReduction = asn1cpp::makeSeq(ImpactReductionContainer);

            asn1cpp::setField(impactReduction->heightLonCarrRight,alacarte_data.getData ().impactReduction.getData ().heightLonCarrRight);
            asn1cpp::setField(impactReduction->heightLonCarrLeft,alacarte_data.getData ().impactReduction.getData ().heightLonCarrLeft);
            asn1cpp::setField(impactReduction->posLonCarrRight,alacarte_data.getData ().impactReduction.getData ().posLonCarrRight);
            asn1cpp::setField(impactReduction->posLonCarrLeft,alacarte_data.getData ().impactReduction.getData ().posLonCarrLeft);

            for(size_t i=0;i<alacarte_data.getData ().impactReduction.getData ().positionOfPillars.size ();i++)
              asn1cpp::sequenceof::pushList(impactReduction->positionOfPillars,alacarte_data.getData ().impactReduction.getData ().positionOfPillars[i]);

            asn1cpp::setField(impactReduction->posCentMass,alacarte_data.getData ().impactReduction.getData ().posCentMass);
            asn1cpp::setField(impactReduction->wheelBaseVehicle,alacarte_data.getData ().impactReduction.getData ().wheelBaseVehicle);
            asn1cpp::setField(impactReduction->turningRadius,alacarte_data.getData ().impactReduction.getData ().turningRadius);
            asn1cpp::setField(impactReduction->posFrontAx,alacarte_data.getData ().impactReduction.getData ().posFrontAx);

            uint32_t positionOfOccupants = alacarte_data.getData ().impactReduction.getData ().positionOfOccupants;
            asn1cpp::bitstring::setBit(impactReduction->positionOfOccupants,setByteMask(positionOfOccupants,0),0);
            asn1cpp::bitstring::setBit(impactReduction->positionOfOccupants,setByteMask(positionOfOccupants,1),1);
            asn1cpp::bitstring::setBit(impactReduction->positionOfOccupants,setByteMask(positionOfOccupants,2),2);

            asn1cpp::setField(impactReduction->vehicleMass,alacarte_data.getData ().impactReduction.getData ().vehicleMass);
            asn1cpp::setField(impactReduction->requestResponseIndication, alacarte_data.getData ().impactReduction.getData ().requestResponseIndication);

            asn1cpp::setField(alacarte_seq->impactReduction,impactReduction);
          }


        if(alacarte_data.getData ().externalTemperature.isAvailable ())
          asn1cpp::setField(alacarte_seq->externalTemperature,alacarte_data.getData ().externalTemperature.getData ());

        if(alacarte_data.getData ().roadWorks.isAvailable ())
          {
            auto roadworks = asn1cpp::makeSeq(RoadWorksContainerExtended);
            auto roadworks_data = alacarte_data.getData ().roadWorks.getData ();

            if(roadworks_data.lightBarSirenInUse.isAvailable ())
              asn1cpp::bitstring::setBit(roadworks->lightBarSirenInUse,setByteMask(roadworks_data.lightBarSirenInUse.getData ()),0);

            if(roadworks_data.innerhardShoulderStatus.isAvailable () ||
               roadworks_data.outerhardShoulderStatus.isAvailable ()||
               roadworks_data.drivingLaneStatus.isAvailable ())
              {
                auto closedLanes = asn1cpp::makeSeq (ClosedLanes);

                if(roadworks_data.innerhardShoulderStatus.isAvailable ())
                  asn1cpp::setField(closedLanes->innerhardShoulderStatus,roadworks_data.innerhardShoulderStatus.getData ());
                if(roadworks_data.outerhardShoulderStatus.isAvailable ())
                  {
                    asn1cpp::setField(closedLanes->outerhardShoulderStatus,roadworks_data.outerhardShoulderStatus.getData ());
                  }
                if(roadworks_data.drivingLaneStatus.isAvailable ())
                  {
                    asn1cpp::bitstring::setBit(closedLanes->drivingLaneStatus,setByteMask(roadworks_data.drivingLaneStatus.getData (),0),0);
                    asn1cpp::bitstring::setBit(closedLanes->drivingLaneStatus,setByteMask(roadworks_data.drivingLaneStatus.getData (),1),1);
                  }

                asn1cpp::setField(roadworks->closedLanes,closedLanes);
              }

            if(roadworks_data.restriction.isAvailable ())
              {
                for(size_t i=0;i<roadworks_data.restriction.getData ().size ();i++)
                    asn1cpp::sequenceof::pushList(roadworks->restriction, roadworks_data.restriction.getData ()[i]);
              }

            if(roadworks_data.speedLimit.isAvailable ())
              asn1cpp::setField(roadworks->speedLimit,roadworks_data.speedLimit.getData ());

            if(roadworks_data.causeCode.isAvailable ())
              {
                auto incidentInd_seq = asn1cpp::makeSeq(CauseCode);
                asn1cpp::setField(incidentInd_seq->causeCode,roadworks_data.causeCode.getData ());
                asn1cpp::setField(incidentInd_seq->subCauseCode,roadworks_data.subCauseCode.getData ());

                asn1cpp::setField(roadworks->incidentIndication,incidentInd_seq);
              }

            if(roadworks_data.recommendedPath.isAvailable ())
              {
                for(size_t i=0;i<roadworks_data.recommendedPath.getData ().size ();i++)
                  {
                    auto refPos_seq = asn1cpp::makeSeq(ReferencePosition);
                    auto refPos_data = roadworks_data.recommendedPath.getData ()[i];

                    asn1cpp::setField(refPos_seq->latitude,refPos_data.latitude);
                    asn1cpp::setField(refPos_seq->longitude,refPos_data.longitude);
                    asn1cpp::setField(refPos_seq->positionConfidenceEllipse.semiMajorConfidence,refPos_data.positionConfidenceEllipse.semiMajorConfidence);
                    asn1cpp::setField(refPos_seq->positionConfidenceEllipse.semiMinorConfidence,refPos_data.positionConfidenceEllipse.semiMinorConfidence);
                    asn1cpp::setField(refPos_seq->positionConfidenceEllipse.semiMajorOrientation,refPos_data.positionConfidenceEllipse.semiMajorOrientation);
                    asn1cpp::setField(refPos_seq->altitude.altitudeValue,refPos_data.altitude.getValue ());
                    asn1cpp::setField(refPos_seq->altitude.altitudeConfidence,refPos_data.altitude.getConfidence ());

                    asn1cpp::sequenceof::pushList(roadworks->recommendedPath,refPos_seq);
                  }
              }

            if(roadworks_data.startingPointSpeedLimit.isAvailable ())
              {
                auto startPointSLimit_seq = asn1cpp::makeSeq(DeltaReferencePosition);
                asn1cpp::setField(startPointSLimit_seq->deltaLatitude,roadworks_data.startingPointSpeedLimit.getData ().deltaLatitude);
                asn1cpp::setField(startPointSLimit_seq->deltaLongitude,roadworks_data.startingPointSpeedLimit.getData ().deltaLongitude);
                asn1cpp::setField(startPointSLimit_seq->deltaAltitude,roadworks_data.startingPointSpeedLimit.getData ().deltaAltitude);

                asn1cpp::setField(roadworks->startingPointSpeedLimit,startPointSLimit_seq);
              }

            if(roadworks_data.trafficFlowRule.isAvailable ())
              asn1cpp::setField(roadworks->trafficFlowRule,roadworks_data.trafficFlowRule.getData ());

            if(roadworks_data.referenceDenms.isAvailable ())
              {
                for(size_t i=0;i<roadworks_data.referenceDenms.getData ().size ();i++)
                  {
                    auto actionId_seq = asn1cpp::makeSeq(ActionID);
                    asn1cpp::setField(actionId_seq->originatingStationId,roadworks_data.referenceDenms.getData ()[i].originatingStationID);
                    asn1cpp::setField(actionId_seq->sequenceNumber,roadworks_data.referenceDenms.getData ()[i].sequenceNumber);

                    asn1cpp::sequenceof::pushList(roadworks->referenceDenms,actionId_seq);
                  }
              }

            asn1cpp::setField(alacarte_seq->roadWorks,roadworks);
          }

        if(alacarte_data.getData ().positioningSolution.isAvailable ())
          asn1cpp::setField(alacarte_seq->positioningSolution,alacarte_data.getData ().positioningSolution.getData ());

        if(alacarte_data.getData ().stationaryVehicle.isAvailable ())
          {
            auto stationary_veh_seq = asn1cpp::makeSeq(StationaryVehicleContainer);
            auto stationary_veh_data = alacarte_data.getData ().stationaryVehicle.getData ();

            if(stationary_veh_data.stationarySince.isAvailable ())
              asn1cpp::setField(stationary_veh_seq->stationarySince,stationary_veh_data.stationarySince.getData ());

            if(stationary_veh_data.causeCode.isAvailable ())
              {
                auto stationaryCause_seq = asn1cpp::makeSeq(CauseCode);
                asn1cpp::setField(stationaryCause_seq->causeCode,stationary_veh_data.causeCode.getData ());
                asn1cpp::setField(stationaryCause_seq->subCauseCode,stationary_veh_data.subCauseCode.getData ());

                asn1cpp::setField(stationary_veh_seq->stationaryCause,stationaryCause_seq);
              }

            if(stationary_veh_data.carryingDangerousGoods.isAvailable ())
              {

                auto dangerous_seq = asn1cpp::makeSeq(DangerousGoodsExtended);
                auto dangerous_data = stationary_veh_data.carryingDangerousGoods.getData ();

                asn1cpp::setField(dangerous_seq->dangerousGoodsType,dangerous_data.dangerousGoodsType);
                asn1cpp::setField(dangerous_seq->unNumber,dangerous_data.unNumber);
                asn1cpp::setField(dangerous_seq->elevatedTemperature,dangerous_data.elevatedTemperature);
                asn1cpp::setField(dangerous_seq->tunnelsRestricted,dangerous_data.tunnelsRestricted);
                asn1cpp::setField(dangerous_seq->limitedQuantity,dangerous_data.limitedQuantity);

                if(dangerous_data.emergencyActionCode.isAvailable ())
                  asn1cpp::setField(dangerous_seq->emergencyActionCode,dangerous_data.emergencyActionCode.getData ());

                if(dangerous_data.phoneNumber.isAvailable ())
                  asn1cpp::setField(dangerous_seq->phoneNumber,dangerous_data.phoneNumber.getData ());

                if(dangerous_data.companyName.isAvailable ())
                  asn1cpp::setField(dangerous_seq->companyName,dangerous_data.companyName.getData ());

                asn1cpp::setField(stationary_veh_seq->carryingDangerousGoods,dangerous_seq);
              }

            if(stationary_veh_data.numberOfOccupants.isAvailable ())
              asn1cpp::setField(stationary_veh_seq->numberOfOccupants,stationary_veh_data.numberOfOccupants.getData ());

            if(stationary_veh_data.vehicleIdentification.isAvailable ())
              {
                auto vehicleId_seq = asn1cpp::makeSeq(VehicleIdentification);

                if(stationary_veh_data.vehicleIdentification.getData ().wMInumber.isAvailable ())
                  asn1cpp::setField(vehicleId_seq->wMInumber,stationary_veh_data.vehicleIdentification.getData ().wMInumber.getData ());

                if(stationary_veh_data.vehicleIdentification.getData ().vDS.isAvailable ())
                  asn1cpp::setField(vehicleId_seq->vDS,stationary_veh_data.vehicleIdentification.getData ().vDS.getData ());

                asn1cpp::setField(stationary_veh_seq->vehicleIdentification,vehicleId_seq);
              }

            if(stationary_veh_data.energyStorageType.isAvailable ())
              asn1cpp::bitstring::setBit(stationary_veh_seq->energyStorageType,setByteMask(stationary_veh_data.energyStorageType.getData ()),0);


            //Add stationaryVehicle container to the Alacarte Container
            asn1cpp::setField(alacarte_seq->stationaryVehicle,stationary_veh_seq);
          }

        //Add the Alacarte container to the DENM
        asn1cpp::setField(denm->denm.alacarte,alacarte_seq);

      }

    return DENM_NO_ERROR;
  }

  template <typename T> int
  DENBasicService::asn_maybe_assign_optional_data(T *data, T **asn_structure, std::queue<void *> &ptr_queue)
  {
    if(data==NULL) {
        return 0;
      }

    *asn_structure = (T*)calloc(1, sizeof(T));

    if(*asn_structure == NULL)
        return -1;

    *(*asn_structure) = *data;

    ptr_queue.push ((void *) *asn_structure);

    return 1;
  }

  DENBasicService::DENBasicService(unsigned long fixed_stationid,long fixed_stationtype,Ptr<Socket> socket_tx)
  {
    m_station_id = (StationID_t) fixed_stationid;
    m_stationtype = (StationType_t) fixed_stationtype;
    m_seq_number = 0;
    m_socket_tx = socket_tx;
    m_real_time = false;
    m_btp = NULL;
  }

  void
  DENBasicService::setStationProperties(unsigned long fixed_stationid,long fixed_stationtype)
  {
    m_station_id=fixed_stationid;
    m_stationtype=fixed_stationtype;
    m_btp->setStationProperties(fixed_stationid,fixed_stationtype);
  }

  void
  DENBasicService::setFixedPositionRSU(double latitude_deg, double longitude_deg)
  {
    m_btp->setFixedPositionRSU(latitude_deg,longitude_deg);
  }

  void
  DENBasicService::setStationID(unsigned long fixed_stationid)
  {
    m_station_id=fixed_stationid;
    m_btp->setStationID(fixed_stationid);
  }

  void
  DENBasicService::setStationType(long fixed_stationtype)
  {
    m_stationtype=fixed_stationtype;
    m_btp->setStationType(fixed_stationtype);
  }

  void
  DENBasicService::setSocketTx (Ptr<Socket> socket_tx)
  {
    m_btp->setSocketTx(socket_tx);
  }

  void
  DENBasicService::setSocketRx (Ptr<Socket> socket_rx)
  {
    m_btp->setSocketRx(socket_rx);
    m_btp->addDENMRxCallback (std::bind(&DENBasicService::receiveDENM,this,std::placeholders::_1,std::placeholders::_2));
  }



  DENBasicService_error_t
  DENBasicService::appDENM_trigger(denData data, DEN_ActionID_t &actionid)
  {
    DENBasicService_error_t fillDENM_rval=DENM_NO_ERROR;
//    DENM_t *denm;

    if(!CheckMainAttributes ())
      {
        return DENM_ATTRIBUTES_UNSET;
      }

// To do: remove this part of commented code once we verify that it is really no more useful and it won't be in the future
//    if(m_socket_tx==NULL)
//      {
//        return DENM_TX_SOCKET_NOT_SET;
//      }

    if(!data.isDenDataRight())
        return DENM_WRONG_DE_DATA;

    auto denm = asn1cpp::makeSeq(DENM);
    if(bool(denm)==false)
      {
        return DENM_ALLOC_ERROR;
      }

    /* 1. If validity is expired return DENM_T_O_VALIDITY_EXPIRED */
    if (compute_timestampIts (m_real_time) > data.getDenmMgmtDetectionTime () + (data.getDenmMgmtValidityDuration ()*MILLI))
        return DENM_T_O_VALIDITY_EXPIRED;

    /* 2. Assign unused actionID value */
    actionid.originatingStationID = m_station_id;
    actionid.sequenceNumber = m_seq_number;

    std::pair <unsigned long, long> map_index = std::make_pair((unsigned long)m_station_id,(long)m_seq_number);

    m_seq_number++;

    /* 3. 4. 5. Manage Transmission interval and fill DENM */
    fillDENM_rval=fillDENM(denm,data,actionid,compute_timestampIts (m_real_time));

    if(fillDENM_rval!=DENM_NO_ERROR)
      {
        //freeDENM(denm);
        return fillDENM_rval;
      }

    /* 6. 7. Construct DENM and pass it to the lower layers (now UDP, in the future BTP and GeoNetworking, then UDP) */
    /** Encoding **/
    std::string encode_result = asn1cpp::uper::encode(denm);

    if(encode_result.size()<1)
    {
      return DENM_ASN1_UPER_ENC_ERROR;
    }


    Ptr<Packet> packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());
    //free(encode_result.buffer);

    BTPDataRequest_t dataRequest = {};
    dataRequest.BTPType = BTP_B; //!< BTP-B
    dataRequest.destPort = 2002;
    dataRequest.destPInfo = 0;
    dataRequest.GNType = GBC;
    dataRequest.GnAddress = m_geoArea;
    dataRequest.GNCommProfile = UNSPECIFIED;
    dataRequest.GNRepInt =0;
    dataRequest.GNMaxRepInt=0;
    dataRequest.GNMaxLife = 60;
    dataRequest.GNMaxHL = 1;
    dataRequest.GNTraClass = 0x01; // Store carry foward: no - Channel offload: no - Traffic Class ID: 1
    dataRequest.lenght = packet->GetSize ();
    dataRequest.data = packet;

    /* 8. 9. Create an entry in the originating ITS-S message table and set the state to ACTIVE and start the T_O_Validity timer. */
    /* As all the timers are stored, in this case, for each entry in the table, we have to set them before saving the new entry to a map, which is done as last operation. */
    /* We are basically adding the current entry, containing the already UPER encoded DENM packet, to a map of <ActionID,ITSSOriginatingTableEntry> */
    ITSSOriginatingTableEntry entry(*packet, ITSSOriginatingTableEntry::STATE_ACTIVE,actionid);

    m_btp->sendBTP(dataRequest);

    m_originatingTimerTable.emplace(map_index,std::tuple<Timer,Timer,Timer>());

    DENBasicService::setDENTimer(std::get<V_O_VALIDITY_INDEX>(m_originatingTimerTable[map_index]),Seconds(data.getDenmMgmtValidityDuration ()),&DENBasicService::T_O_ValidityStop,actionid);

    /* 10. Calculate and start timers T_RepetitionDuration and T_Repetition when both parameters in denData are > 0 */
    if(data.getDenmRepetitionDuration ()>0 && data.getDenmRepetitionInterval ()>0)
      {
        DENBasicService::setDENTimer(std::get<T_REPETITION_INDEX>(m_originatingTimerTable[map_index]),MilliSeconds(data.getDenmRepetitionInterval()),&DENBasicService::T_RepetitionStop,actionid);
        DENBasicService::setDENTimer(std::get<T_REPETITION_DURATION_INDEX>(m_originatingTimerTable[map_index]),MilliSeconds(data.getDenmRepetitionDuration ()),&DENBasicService::T_RepetitionDurationStop,actionid);
      }

    /* 11. Finally create the entry after starting all the timers (it refers to the step '8' of appDENM_trigger in ETSI EN302 637-3 V1.3.1 */

    m_originatingITSSTable[map_index]=entry;

    /* 12. Send actionID to the requesting ITS-S application. This is requested by the standard, but we are already reporting the actionID using &actionID */

    return DENM_NO_ERROR;
  }

  DENBasicService_error_t
  DENBasicService::appDENM_update(denData data, const DEN_ActionID_t actionid)
  {
    DENBasicService_error_t fillDENM_rval=DENM_NO_ERROR;
    std::pair <unsigned long, long> map_index = std::make_pair((unsigned long)actionid.originatingStationID,(long)actionid.sequenceNumber);

    if(!CheckMainAttributes ())
      {
        return DENM_ATTRIBUTES_UNSET;
      }

// To do: remove this part of commented code once we verify that it is really no more useful and it won't be in the future
//    if(m_socket_tx==NULL)
//      {
//        return DENM_TX_SOCKET_NOT_SET;
//      }

    auto denm = asn1cpp::makeSeq(DENM);
    if(bool(denm)==false)
      {
        return DENM_ALLOC_ERROR;
      }

    /* 1. If validity is expired return DENM_T_O_VALIDITY_EXPIRED */
    if (compute_timestampIts (m_real_time) > data.getDenmMgmtDetectionTime () + (data.getDenmMgmtValidityDuration ()*MILLI))
        return DENM_T_O_VALIDITY_EXPIRED;

    /* 2. Compare actionID in the application request with entries in the originating ITS-S message table (i.e. m_originatingITSSTable, implemented as a map) */
    /* Gather also the proper entry in the table, if available. */

    T_Repetition_Mutex.lock();
    std::map<std::pair<unsigned long,long>, ITSSOriginatingTableEntry>::iterator entry_map_it = m_originatingITSSTable.find(map_index);
    if (entry_map_it == m_originatingITSSTable.end())
      {
        T_Repetition_Mutex.unlock();
        return DENM_UNKNOWN_ACTIONID;
      }

    /* 3. Stop T_O_Validity, T_RepetitionDuration and T_Repetition (if they were started - this check is already performed by the setTimer* methods) */
    std::get<V_O_VALIDITY_INDEX>(m_originatingTimerTable[map_index]).Cancel();
    std::get<T_REPETITION_INDEX>(m_originatingTimerTable[map_index]).Cancel();
    std::get<T_REPETITION_DURATION_INDEX>(m_originatingTimerTable[map_index]).Cancel();

    /* 4. 5. 6. Manage transmission interval, reference time and fill DENM */
    fillDENM_rval=fillDENM(denm,data,actionid,compute_timestampIts (m_real_time));

    if(fillDENM_rval!=DENM_NO_ERROR)
      {
        T_Repetition_Mutex.unlock();
        return fillDENM_rval;
      }

    /* 7. 8. Construct DENM and pass it to the lower layers (now UDP, in the future BTP and GeoNetworking, then UDP) */
    /** Encoding **/
    std::string encode_result = asn1cpp::uper::encode(denm);

    if(encode_result.size()<1)
    {
      return DENM_ASN1_UPER_ENC_ERROR;
    }

    Ptr<Packet> packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());
    //free(encode_result.buffer);

    BTPDataRequest_t dataRequest = {};
    dataRequest.BTPType = BTP_B; //!< BTP-B
    dataRequest.destPort = 2002;
    dataRequest.destPInfo = 0;
    dataRequest.GNType = GBC;
    dataRequest.GnAddress = m_geoArea;
    dataRequest.GNCommProfile = UNSPECIFIED;
    dataRequest.GNRepInt =0;
    dataRequest.GNMaxRepInt=0;
    dataRequest.GNMaxLife = 60; // 60 seconds
    dataRequest.GNMaxHL = 1;
    dataRequest.GNTraClass = 0x01; // Store carry foward: no - Channel offload: no - Traffic Class ID: 1
    dataRequest.lenght = packet->GetSize ();
    dataRequest.data = packet;

    m_btp->sendBTP(dataRequest);

    /* 9. Update the entry in the originating ITS-S message table. */
    entry_map_it->second.setDENMPacket(*packet);

    /* 10. Start timer T_O_Validity. */
    DENBasicService::setDENTimer(std::get<V_O_VALIDITY_INDEX>(m_originatingTimerTable[map_index]),Seconds(data.getDenmMgmtValidityDuration ()),&DENBasicService::T_O_ValidityStop,actionid);

    /* 11. Calculate and start timers T_RepetitionDuration and T_Repetition when both parameters in denData are > 0 */
    if(data.getDenmRepetitionDuration ()>0 && data.getDenmRepetitionInterval ()>0)
      {
        DENBasicService::setDENTimer(std::get<T_REPETITION_INDEX>(m_originatingTimerTable[map_index]),MilliSeconds(data.getDenmRepetitionInterval()),&DENBasicService::T_RepetitionStop,actionid);
        DENBasicService::setDENTimer(std::get<T_REPETITION_DURATION_INDEX>(m_originatingTimerTable[map_index]),MilliSeconds(data.getDenmRepetitionDuration ()),&DENBasicService::T_RepetitionDurationStop,actionid);
      }

    T_Repetition_Mutex.unlock();
    return DENM_NO_ERROR;
  }

  DENBasicService_error_t
  DENBasicService::appDENM_termination(denData data, const DEN_ActionID_t actionid)
  {
    uint8_t termination=0;
    long asn_termination;
    long referenceTime;

    DENBasicService_error_t fillDENM_rval=DENM_NO_ERROR;

    if(!CheckMainAttributes ())
      {
        return DENM_ATTRIBUTES_UNSET;
      }

// To do: remove this part of commented code once we verify that it is really no more useful and it won't be in the future
//    if(m_socket_tx==NULL)
//      {
//        return DENM_TX_SOCKET_NOT_SET;
//      }

    auto denm = asn1cpp::makeSeq(DENM);
    if(bool(denm)==false)
      {
        return DENM_ALLOC_ERROR;
      }

    std::pair <unsigned long, long> map_index = std::make_pair((unsigned long)actionid.originatingStationID,(long)actionid.sequenceNumber);

    /* 1. If validity is expired return DENM_T_O_VALIDITY_EXPIRED */
    if (compute_timestampIts (m_real_time) > data.getDenmMgmtDetectionTime () + (data.getDenmMgmtValidityDuration ()*MILLI))
        return DENM_T_O_VALIDITY_EXPIRED;

    /* 2. Compare actionID in the application request with entries in the originating ITS-S message table and the receiving ITS-S message table */
    T_Repetition_Mutex.lock();

    std::map<std::pair<unsigned long,long>, ITSSOriginatingTableEntry>::iterator entry_originating_table = m_originatingITSSTable.find(map_index);
    /* 2a. If actionID exists in the originating ITS-S message table and the entry state is ACTIVE, then set termination to isCancellation.*/
    if (entry_originating_table != m_originatingITSSTable.end())
      {
        T_Repetition_Mutex.unlock();
        return DENM_UNKNOWN_ACTIONID_ORIGINATING;
      }
    else if(entry_originating_table->second.getStatus()==ITSSOriginatingTableEntry::STATE_ACTIVE)
      {
        asn_termination=Termination_isCancellation;
        if(!asn1cpp::setField(denm->denm.management.termination,asn_termination))
          {
            T_Repetition_Mutex.unlock();
            return DENM_ALLOC_ERROR;
          }
        else
          {
            termination=0;
          }
      }
    else
      {
        T_Repetition_Mutex.unlock();
        return DENM_NON_ACTIVE_ACTIONID_ORIGINATING;
      }

    /* 2b. If actionID exists in the receiving ITS-S message table and the entry state is ACTIVE, then set termination to isNegation.*/
    std::map<std::pair<unsigned long,long>, ITSSReceivingTableEntry>::iterator entry_receiving_table = m_receivingITSSTable.find(map_index);
    if (entry_receiving_table != m_receivingITSSTable.end())
      {
        T_Repetition_Mutex.unlock();
        return DENM_UNKNOWN_ACTIONID_RECEIVING;
      }
    else if(entry_receiving_table->second.getStatus()==ITSSReceivingTableEntry::STATE_ACTIVE)
      {
        asn_termination=Termination_isNegation;
        if(!asn1cpp::setField(denm->denm.management.termination,asn_termination))
          {
            T_Repetition_Mutex.unlock();
            return DENM_ALLOC_ERROR;
          }
        else
          {
            termination=1;
          }
      }
    else
      {
        T_Repetition_Mutex.unlock();
        return DENM_NON_ACTIVE_ACTIONID_RECEIVING;
      }

    if(termination==1)
      {
        referenceTime=entry_receiving_table->second.getReferenceTime();

        if(referenceTime==-1)
          {
            T_Repetition_Mutex.unlock();
            return DENM_WRONG_TABLE_DATA;
          }
      }
    else
      {
        referenceTime=compute_timestampIts (m_real_time);
      }

    /* 3. Set referenceTime to current time (for termination = 0) or to the receiving table entry's reference time (for termination = 1) and fill DENM */
    fillDENM_rval=fillDENM(denm,data,actionid,referenceTime);
    if(fillDENM_rval!=DENM_NO_ERROR)
      {
        T_Repetition_Mutex.unlock();
        //freeDENM(denm);
        return fillDENM_rval;
      }

    /* 4a. Gather the proper timers from the timers table */
    std::map<std::pair<unsigned long,long>, std::tuple<Timer,Timer,Timer>>::iterator entry_timers_table = m_originatingTimerTable.find(map_index);

    /* 4b. Stop T_O_Validity, T_RepetitionDuration and T_Repetition (if they were started - this check is already performed by the setTimer* methods) */
    std::get<V_O_VALIDITY_INDEX>(entry_timers_table->second).Cancel();
    std::get<T_REPETITION_INDEX>(entry_timers_table->second).Cancel();
    std::get<T_REPETITION_DURATION_INDEX>(entry_timers_table->second).Cancel();

    /* 5. Construct DENM and pass it to the lower layers (now UDP, in the future BTP and GeoNetworking, then UDP) */
    /** Encoding **/
    std::string encode_result = asn1cpp::uper::encode(denm);

    if(encode_result.size()<1)
    {
      return DENM_ASN1_UPER_ENC_ERROR;
    }


    Ptr<Packet> packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());
    //free(encode_result.buffer);

    BTPDataRequest_t dataRequest = {};
    dataRequest.BTPType = BTP_B; //!< BTP-B
    dataRequest.destPort = 2002;
    dataRequest.destPInfo = 0;
    dataRequest.GNType = GBC;
    dataRequest.GnAddress = m_geoArea;
    dataRequest.GNCommProfile = UNSPECIFIED;
    dataRequest.GNRepInt =0;
    dataRequest.GNMaxRepInt=0;
    dataRequest.GNMaxLife = 60;
    dataRequest.GNMaxHL = 1;
    dataRequest.GNTraClass = 0x01; // Store carry foward: no - Channel offload: no - Traffic Class ID: 1
    dataRequest.lenght = packet->GetSize ();
    dataRequest.data = packet;
    m_btp->sendBTP(dataRequest);

    /* 6a. If termination is set to 1, create an entry in the originating ITS-S message table and set the state to NEGATED. */
    if(termination==1)
      {
        ITSSOriginatingTableEntry entry(*packet, ITSSOriginatingTableEntry::STATE_NEGATED,actionid);
        m_originatingITSSTable[map_index]=entry;
      }
    /* 6b. If termination is set to 0, update the entry in the originating ITS-S message table and set the state to CANCELLED. */
    else
      {
        entry_originating_table->second.setDENMPacket(*packet);
        entry_originating_table->second.setStatus(ITSSOriginatingTableEntry::STATE_CANCELLED);
      }

    /* 7. Start timer T_O_Validity. */
    DENBasicService::setDENTimer(std::get<V_O_VALIDITY_INDEX>(entry_timers_table->second),Seconds(data.getDenmMgmtValidityDuration ()),&DENBasicService::T_O_ValidityStop,actionid);

    /* 8. Calculate and start timers T_RepetitionDuration and T_Repetition when both parameters in denData are > 0 */
    if(data.getDenmRepetitionDuration ()>0 && data.getDenmRepetitionInterval ()>0)
      {
        DENBasicService::setDENTimer(std::get<T_REPETITION_INDEX>(entry_timers_table->second),MilliSeconds(data.getDenmRepetitionInterval()),&DENBasicService::T_RepetitionStop,actionid);
        DENBasicService::setDENTimer(std::get<T_REPETITION_DURATION_INDEX>(entry_timers_table->second),MilliSeconds(data.getDenmRepetitionDuration ()),&DENBasicService::T_RepetitionDurationStop,actionid);
      }

    T_Repetition_Mutex.unlock();


    return DENM_NO_ERROR;
  }

  void
  DENBasicService::receiveDENM(BTPDataIndication_t dataIndication,Address from)
  {
    Ptr<Packet> packet;
    asn1cpp::Seq<DENM> decoded_denm;
    denData den_data;
    long validityDuration,termination;
    bool validity_ok,termination_ok;
    DEN_ActionID_t actionID;

    long detectionTime_long;
    long referenceTime_long;
    std::pair <unsigned long, long> map_index;

    packet = dataIndication.data;

    uint8_t *buffer; //= new uint8_t[packet->GetSize ()];
    buffer=(uint8_t *)malloc((packet->GetSize ())*sizeof(uint8_t));
    packet->CopyData (buffer, packet->GetSize ());
    std::string packetContent((char *)buffer,(int) dataIndication.data->GetSize ());

    RssiTag rssi;
    bool rssi_result = dataIndication.data->PeekPacketTag(rssi);

    SnrTag snr;
    bool snr_result = dataIndication.data->PeekPacketTag(snr);

    RsrpTag rsrp;
    bool rsrp_result = dataIndication.data->PeekPacketTag(rsrp);

    SinrTag sinr;
    bool sinr_result = dataIndication.data->PeekPacketTag(sinr);

    SizeTag size;
    bool size_result = dataIndication.data->PeekPacketTag(size);

    TimestampTag timestamp;
    dataIndication.data->PeekPacketTag(timestamp);

    if(!snr_result)
      {
        snr.Set(SENTINEL_VALUE);
      }
    if (!rssi_result)
      {
        rssi.Set(SENTINEL_VALUE);
      }
    if (!rsrp_result)
      {
        rsrp.Set(SENTINEL_VALUE);
      }
    if (!sinr_result)
      {
        sinr.Set(SENTINEL_VALUE);
      }
    if (!size_result)
      {
        size.Set(SENTINEL_VALUE);
      }

    SetSignalInfo(timestamp.Get(), size.Get(), rssi.Get(), snr.Get(), sinr.Get(), rsrp.Get());

    if(!CheckMainAttributes ())
      {
        NS_LOG_ERROR("DENBasicService has unset parameters. Cannot receive any data.");
        free(buffer);
        return;
      }

    /* Try to check if the received packet is really a DENM */
    if (buffer[1]!=FIX_DENMID)
      {
        NS_LOG_ERROR("Warning: received a message which has messageID '"<<buffer[1]<<"' but '1' was expected.");
        free(buffer);
        return;
      }

    /** Decoding **/
    free(buffer);
    decoded_denm = asn1cpp::uper::decodeASN(packetContent, DENM);

    if(bool(decoded_denm)==false) {
        NS_LOG_ERROR("Warning: unable to decode a received DENM.");
        return;
      }

    /* Compute T_R_Validity expiration time */
    validityDuration = asn1cpp::getField(decoded_denm->denm.management.validityDuration,long,&validity_ok);
    if(!validity_ok)
      validityDuration = DEN_DEFAULT_VALIDITY_S;
    detectionTime_long = asn1cpp::getField(decoded_denm->denm.management.detectionTime,long);
    referenceTime_long = asn1cpp::getField(decoded_denm->denm.management.referenceTime,long);

    long now = compute_timestampIts (m_real_time);
    /* 1. If validity is expired return without performing further steps */
    if (now > detectionTime_long + ((long)validityDuration*MILLI))
      {
        NS_LOG_ERROR("Warning: received a DENM with an expired validity. Detection time (ms): "<<detectionTime_long<<"; validity duration (s): "<<(long)validityDuration);
        NS_LOG_ERROR("Condition: '"<<now<<" > "<< detectionTime_long + ((long)validityDuration*MILLI)<<"' is true. Omitting further operations.");
        return;
      }

    /* Lookup entries in the receiving ITS-S message table with the received actionID */
    actionID.originatingStationID = asn1cpp::getField(decoded_denm->denm.management.actionID.originatingStationId,unsigned long);
    actionID.sequenceNumber = asn1cpp::getField(decoded_denm->denm.management.actionID.sequenceNumber,long);
    map_index = std::make_pair((unsigned long)actionID.originatingStationID ,(long)actionID.sequenceNumber);

    std::map<std::pair<unsigned long,long>, ITSSReceivingTableEntry>::iterator entry_rx_map_it = m_receivingITSSTable.find(map_index);

    termination = asn1cpp::getField(decoded_denm->denm.management.termination,long,&termination_ok);

    if (entry_rx_map_it == m_receivingITSSTable.end ())
      {
        /* a. If entry does not exist in the receiving ITS-S message table, check if termination data exists in the
            received DENM. */

        if(termination_ok && (termination == Termination_isCancellation || termination == Termination_isNegation))
          {
            /* if yes, discard the received DENM and omit execution of further steps. */
            NS_LOG_ERROR("Warning: received a new DENM with termination data (either cancelled or negated). Omitting further reception steps.");
            return;
          }
        else
          {
            /* if not, create an entry in the receiving ITS-S message table with the received DENM and set the state to ACTIVE (SSP is not yet implemented) */
            ITSSReceivingTableEntry entry(*packet,ITSSReceivingTableEntry::STATE_ACTIVE,actionID,referenceTime_long,detectionTime_long);
            m_receivingITSSTable[map_index]=entry;
          }
      }
    else
      {
        /* b. If entry does exist in the receiving ITS-S message table, check if the received referenceTime is less than the entry referenceTime,
         * or the received detectionTime is less than the entry detectionTime */
        long stored_reference_time = entry_rx_map_it->second.getReferenceTime ();
        long stored_detection_time =  entry_rx_map_it->second.getDetectionTime ();

        if (referenceTime_long < stored_reference_time || detectionTime_long < stored_detection_time)
          {
            /* i. if yes, discard received DENM and omit execution of further steps. */
            NS_LOG_ERROR("Warning: received a new DENM with reference time < entry reference time or  detection time < stored detection time.");
            NS_LOG_ERROR("reference time (ms): "<<referenceTime_long<<"; stored value: "<<stored_reference_time
                    <<"; detection time (ms): "<<detectionTime_long<<"; store value: "<<stored_detection_time);
            return;
          }
        else
          {
            /* ii. Otherwise, check if the received DENM is a repeated DENM of the entry, i.e. the received referenceTime equals to the entry
             * referenceTime, the received detectionTime equals to the entry
             * detectionTime, and the received termination value equals to the entry state */
            if(referenceTime_long == stored_reference_time &&
               detectionTime_long == stored_detection_time &&
               (
                 (!termination_ok &&  !entry_rx_map_it->second.isTerminationSet()) ||
                 (termination_ok && termination==entry_rx_map_it->second.getTermination ())
               ))
              {
                /* 1. If yes, discard received DENM and omit execution of further steps. */
                NS_LOG_ERROR("Warning: received a repeated DENM. It won't produce any new effect on the involved vehicle.");
                return;
              }
            else
              {
                /* 2. Otherwise, update the entry in receiving ITS-S message table, set entry state according
                 * to the termination value of the received DENM. (SSP is not yet implemented) */
                ITSSReceivingTableEntry entry(*packet,ITSSReceivingTableEntry::STATE_ACTIVE,actionID,referenceTime_long,detectionTime_long,decoded_denm->denm.management.termination);
                entry_rx_map_it->second=entry;
              }
          }

      }

    /* Start/restart T_R_Validity timer. */
    if(m_T_R_Validity_Table.find(map_index) == m_T_R_Validity_Table.end()) {
      m_T_R_Validity_Table[map_index] = Timer();
    }

    DENBasicService::setDENTimer(m_T_R_Validity_Table[map_index],Seconds((long)validityDuration),&DENBasicService::T_R_ValidityStop,actionID);

    /* Fill den_data with the received information */
    bool location_ok,situation_ok,alacarte_ok;

    auto header = asn1cpp::getSeq(decoded_denm->header,ItsPduHeader);
    DENBasicService::fillDenDataHeader (header, den_data);

    auto management = asn1cpp::getSeq(decoded_denm->denm.management,ManagementContainer);
    DENBasicService::fillDenDataManagement (management, den_data);


    auto location = asn1cpp::getSeqOpt(decoded_denm->denm.location,LocationContainer,&location_ok);
    if(location_ok)
        DENBasicService::fillDenDataLocation (location, den_data);

    auto situation = asn1cpp::getSeqOpt(decoded_denm->denm.situation,SituationContainer,&situation_ok);
    if(situation_ok)
        DENBasicService::fillDenDataSituation (situation, den_data);

    auto alacarte = asn1cpp::getSeqOpt(decoded_denm->denm.alacarte,AlacarteContainer,&alacarte_ok);
    if(alacarte_ok)
        DENBasicService::fillDenDataAlacarte (alacarte, den_data);

    if(m_DENReceiveCallback!=nullptr) {
      m_DENReceiveCallback(den_data,from);
    } else if(m_DENReceiveCallbackExtended!=nullptr) {
      m_DENReceiveCallbackExtended(den_data,from,m_station_id,m_stationtype,GetSignalInfo());
    }
  }

  void
  DENBasicService::T_O_ValidityStop(DEN_ActionID_t entry_actionid)
  {
    T_Repetition_Mutex.lock ();
    std::pair <unsigned long, long> map_index = std::make_pair((unsigned long)entry_actionid.originatingStationID,(long)entry_actionid.sequenceNumber);

    std::get<T_REPETITION_INDEX>(m_originatingTimerTable[map_index]).Cancel();
    std::get<T_REPETITION_DURATION_INDEX>(m_originatingTimerTable[map_index]).Cancel();

    // When an entry expires, discard also the information related to its (now stopped) timers
    m_originatingTimerTable.erase (map_index);
    m_originatingITSSTable.erase (map_index);
    T_Repetition_Mutex.unlock ();
  }

  void
  DENBasicService::T_RepetitionDurationStop(DEN_ActionID_t entry_actionid)
  {
    std::pair <unsigned long, long> map_index = std::make_pair((unsigned long)entry_actionid.originatingStationID,(long)entry_actionid.sequenceNumber);

    std::get<T_REPETITION_INDEX>(m_originatingTimerTable[map_index]).Cancel();
  }

  void
  DENBasicService::T_RepetitionStop(DEN_ActionID_t entry_actionid)
  {
    T_Repetition_Mutex.lock();
    std::pair <unsigned long, long> map_index = std::make_pair((unsigned long)entry_actionid.originatingStationID,(long)entry_actionid.sequenceNumber);
    T_Repetition_Mutex.unlock ();

    Ptr<Packet> packet = Create<Packet> (m_originatingITSSTable[map_index].getDENMPacket ());
    // We should never reach this point if m_socket_tx==NULL (i.e. the corresponding timer will never be started)
    // So, it should not be necessary to check that m_socket_tx!=NULL

    BTPDataRequest_t dataRequest = {};

    dataRequest.BTPType = BTP_B; //!< BTP-B
    dataRequest.destPort = DEN_PORT;
    dataRequest.destPInfo = 0;
    dataRequest.GNType = GBC;
    dataRequest.GnAddress = m_geoArea;
    dataRequest.GNCommProfile = UNSPECIFIED;
    dataRequest.GNRepInt = 0;
    dataRequest.GNMaxRepInt = 0;
    dataRequest.GNMaxLife = 60;
    dataRequest.GNMaxHL = 1;
    dataRequest.GNTraClass = 0x01; // Store carry foward: no - Channel offload: no - Traffic Class ID: 1
    dataRequest.lenght = packet->GetSize ();
    dataRequest.data = packet;
    m_btp->sendBTP(dataRequest);

    // Restart timer
    std::get<T_REPETITION_INDEX>(m_originatingTimerTable[map_index]).Schedule();
  }

  void
  DENBasicService::T_R_ValidityStop(DEN_ActionID_t entry_actionid)
  {
    T_Repetition_Mutex.lock();
    std::pair <unsigned long, long> map_index = std::make_pair((unsigned long)entry_actionid.originatingStationID,(long)entry_actionid.sequenceNumber);

    // When an entry expires, discard also the information related to its (now stopped) timers
    m_T_R_Validity_Table.erase (map_index);
    m_receivingITSSTable.erase (map_index);
    T_Repetition_Mutex.unlock ();
  }

  /* This cleanup function will attemp to stop any possibly still-running timer */
  void
  DENBasicService::cleanup(void)
  {
    for(std::map<std::pair<unsigned long,long>, std::tuple<Timer,Timer,Timer>>::iterator m_originatingTimerTable_it=m_originatingTimerTable.begin ();
        m_originatingTimerTable_it!=m_originatingTimerTable.end ();
        m_originatingTimerTable_it++)
      {
         std::get<V_O_VALIDITY_INDEX>(m_originatingTimerTable_it->second).Cancel();
         std::get<T_REPETITION_INDEX>(m_originatingTimerTable_it->second).Cancel();
         std::get<T_REPETITION_DURATION_INDEX>(m_originatingTimerTable_it->second).Cancel();
      }

    for(std::map<std::pair<unsigned long,long>, Timer>::iterator m_T_R_Validity_Table_it=m_T_R_Validity_Table.begin ();
        m_T_R_Validity_Table_it!=m_T_R_Validity_Table.end ();
        m_T_R_Validity_Table_it++)
      {
         m_T_R_Validity_Table_it->second.Cancel();
      }

    // Cleanup the BTP object (which will in turn perform the "cleanup" operation on the underlying GeoNet object)
    m_btp->cleanup();

  }

  void
  DENBasicService::fillDenDataHeader(asn1cpp::Seq<ItsPduHeader>denm_header, denData &denm_data)
  {
      denm_data.setDenmHeader (asn1cpp::getField(denm_header->messageId,long),
                               asn1cpp::getField(denm_header->protocolVersion,long),
                               asn1cpp::getField(denm_header->stationId,long));
  }

  void
  DENBasicService::fillDenDataManagement(asn1cpp::Seq<ManagementContainer> denm_mgmt_container, denData &denm_data)
  {
    denData::denDataManagement management;
    bool ok;
    management.detectionTime = asn1cpp::getField(denm_mgmt_container->detectionTime,long);
    management.longitude = asn1cpp::getField(denm_mgmt_container->eventPosition.longitude,long);
    management.latitude = asn1cpp::getField(denm_mgmt_container->eventPosition.latitude,long);
    management.altitude.setValue (asn1cpp::getField(denm_mgmt_container->eventPosition.altitude.altitudeValue,long));
    management.altitude.setConfidence (asn1cpp::getField(denm_mgmt_container->eventPosition.altitude.altitudeConfidence,long));
    management.posConfidenceEllipse.semiMajorConfidence = asn1cpp::getField(denm_mgmt_container->eventPosition.positionConfidenceEllipse.semiMajorConfidence,long);
    management.posConfidenceEllipse.semiMinorConfidence = asn1cpp::getField(denm_mgmt_container->eventPosition.positionConfidenceEllipse.semiMinorConfidence,long);
    management.posConfidenceEllipse.semiMajorOrientation = asn1cpp::getField(denm_mgmt_container->eventPosition.positionConfidenceEllipse.semiMajorOrientation,long);

    auto validityDuration = asn1cpp::getField(denm_mgmt_container->validityDuration,long,&ok);
    if(!ok)
      validityDuration = DEN_DEFAULT_VALIDITY_S;
    management.validityDuration.setData (validityDuration);

    auto transmissionInterval = asn1cpp::getField(denm_mgmt_container->transmissionInterval,long,&ok);
    if(ok)
      management.transmissionInterval.setData (transmissionInterval);

    management.stationID = asn1cpp::getField(denm_mgmt_container->actionID.originatingStationId,unsigned long);
    management.sequenceNumber = asn1cpp::getField(denm_mgmt_container->actionID.sequenceNumber,long);

    auto termination = asn1cpp::getField(denm_mgmt_container->termination,long,&ok);
    if(ok)
      management.termination.setData (termination);

    auto relevanceDistance = asn1cpp::getField(denm_mgmt_container->relevanceDistance,long,&ok);
    if(ok)
      management.relevanceDistance.setData (relevanceDistance);

    auto relevanceTrafficDir = asn1cpp::getField(denm_mgmt_container->relevanceTrafficDirection,long,&ok);
    if(ok)
      management.relevanceTrafficDirection.setData (relevanceTrafficDir);

    denm_data.setDenmMgmtData_asn_types (management);
  }

  void
  DENBasicService::fillDenDataSituation(asn1cpp::Seq<SituationContainer> denm_situation_container, denData &denm_data)
  {
    denData::denDataSituation situation;
    bool ok;
    situation.informationQuality = asn1cpp::getField(denm_situation_container->informationQuality,long);
    situation.causeCode = asn1cpp::getField(denm_situation_container->eventType.causeCode,long);
    situation.subCauseCode = asn1cpp::getField(denm_situation_container->eventType.subCauseCode,long);

    if(denm_situation_container->linkedCause!=nullptr)
      {
        auto linkedCause = asn1cpp::getField (denm_situation_container->linkedCause->causeCode, long, &ok);
        if (ok)
          {
            situation.linkedCauseCode.setData (linkedCause);
            situation.linkedSubCauseCode.setData (
                asn1cpp::getField (denm_situation_container->linkedCause->subCauseCode, long));
          }
      }

    auto eventHist = asn1cpp::getSeqOpt(denm_situation_container->eventHistory,EventHistory,&ok);
    if(ok)
      {
        int eventHist_size = asn1cpp::sequenceof::getSize(denm_situation_container->eventHistory);
        std::vector<DEN_EventPoint_t> eventHist;
        for(int i=0;i<eventHist_size;i++)
          {
            DEN_EventPoint_t eventPoint_data;
            bool deltaTime_ok;
            auto eventPoint_seq = asn1cpp::makeSeq(EventPoint);
            eventPoint_data.eventPosition.deltaLatitude = asn1cpp::sequenceof::getSeq(denm_situation_container->eventHistory,EventPoint,i);
            eventPoint_data.eventPosition.deltaLatitude = asn1cpp::getField(eventPoint_seq->eventPosition.deltaLatitude,long);
            eventPoint_data.eventPosition.deltaLongitude = asn1cpp::getField(eventPoint_seq->eventPosition.deltaLongitude,long);
            eventPoint_data.eventPosition.deltaAltitude = asn1cpp::getField(eventPoint_seq->eventPosition.deltaAltitude,long);
            auto deltaTime = asn1cpp::getField(eventPoint_seq->eventDeltaTime,long,&deltaTime_ok);
            if(deltaTime_ok)
              eventPoint_data.eventDeltaTime.setData (deltaTime);
            eventPoint_data.informationQuality = asn1cpp::getField(eventPoint_seq->informationQuality,long);
            eventHist.push_back (eventPoint_data);
          }
        situation.eventHistory.setData (eventHist);
      }

    denm_data.setDenmSituationData_asn_types (situation);
  }

  void
  DENBasicService::fillDenDataLocation(asn1cpp::Seq<LocationContainer> denm_location_container, denData &denm_data)
  {
    denData::denDataLocation location;
    bool ok;

    auto traces = asn1cpp::getSeq(denm_location_container->traces,Traces);
    for(int i=0; i<3;i++)
      {
        auto pathHistory = asn1cpp::sequenceof::getSeq(*traces,Path,i);
        std::vector<DEN_PathPoint_t> pathHistory_data;
        for(int j=0;j<3;j++)
          {
            auto pathPoint = asn1cpp::sequenceof::getSeq(*pathHistory,PathPoint,j);
            DEN_PathPoint_t pathPoint_data;
            bool deltaTime_ok;
            pathPoint_data.pathPosition.deltaLatitude = asn1cpp::getField(pathPoint->pathPosition.deltaLatitude,long);
            pathPoint_data.pathPosition.deltaLongitude = asn1cpp::getField(pathPoint->pathPosition.deltaLongitude,long);
            pathPoint_data.pathPosition.deltaAltitude = asn1cpp::getField(pathPoint->pathPosition.deltaAltitude,long);
            auto deltaTime = asn1cpp::getField(pathPoint->pathDeltaTime,long,&deltaTime_ok);
            if(deltaTime_ok)
              pathPoint_data.pathDeltaTime.setData (deltaTime);
            pathHistory_data.push_back (pathPoint_data);
          }
        location.traces.push_back (pathHistory_data);
      }

    auto speed = asn1cpp::getField(denm_location_container->eventSpeed->speedValue,long,&ok);
    if(ok)
      location.eventSpeed.setData (DENValueConfidence<long,long>(speed,
                  asn1cpp::getField(denm_location_container->eventSpeed->speedConfidence,long)));

    auto heading = asn1cpp::getField(denm_location_container->eventPositionHeading->headingValue,long,&ok);
    if(ok)
      location.eventPositionHeading.setData (DENValueConfidence<long,long>(heading,
                  asn1cpp::getField(denm_location_container->eventPositionHeading->headingConfidence,long)));

    auto roadType = asn1cpp::getField(denm_location_container->roadType,long,&ok);
    if(ok)
      location.roadType.setData (roadType);


    denm_data.setDenmLocationData_asn_types (location);
  }

  void
  DENBasicService::fillDenDataAlacarte(asn1cpp::Seq<AlacarteContainer> denm_alacarte_container, denData &denm_data)
  {
    denData::denDataAlacarte alacarte;
    bool ok;

    auto lanePos = asn1cpp::getField(denm_alacarte_container->lanePosition,long,&ok);
    if(ok)
      alacarte.lanePosition.setData (lanePos);

    auto impactReduction = asn1cpp::getSeqOpt(denm_alacarte_container->impactReduction,ImpactReductionContainer,&ok);
    if(ok)
      {
        DEN_ImpactReductionContainer_t impactReduction_data = {};

        impactReduction_data.heightLonCarrLeft = asn1cpp::getField(impactReduction->heightLonCarrLeft,long);
        impactReduction_data.heightLonCarrRight = asn1cpp::getField(impactReduction->heightLonCarrRight,long);
        impactReduction_data.posLonCarrLeft = asn1cpp::getField(impactReduction->posLonCarrLeft,long);
        impactReduction_data.posLonCarrRight = asn1cpp::getField(impactReduction->posLonCarrRight,long);

        int posPillars_size = asn1cpp::sequenceof::getSize(impactReduction->positionOfPillars);
        for(int i=0;i<posPillars_size;i++)
            impactReduction_data.positionOfPillars.push_back (asn1cpp::sequenceof::getField(impactReduction->positionOfPillars,long,i));

        impactReduction_data.posCentMass = asn1cpp::getField(impactReduction->posCentMass,long);
        impactReduction_data.wheelBaseVehicle = asn1cpp::getField(impactReduction->wheelBaseVehicle,long);
        impactReduction_data.turningRadius = asn1cpp::getField(impactReduction->turningRadius,long);
        impactReduction_data.posFrontAx = asn1cpp::getField(impactReduction->posFrontAx,long);


//        uint8_t byte0,byte1,byte2;
//        byte0 = asn1cpp::bitstring::getterByteMask(impactReduction->positionOfOccupants,0);
//        byte1 = asn1cpp::bitstring::getterByteMask(impactReduction->positionOfOccupants,1);
//        byte2 = asn1cpp::bitstring::getterByteMask(impactReduction->positionOfOccupants,2);
        impactReduction_data.positionOfOccupants = getFromMasks (asn1cpp::bitstring::getterByteMask(impactReduction->positionOfOccupants,0),
                                                                 asn1cpp::bitstring::getterByteMask(impactReduction->positionOfOccupants,1),
                                                                 asn1cpp::bitstring::getterByteMask(impactReduction->positionOfOccupants,2));

        impactReduction_data.vehicleMass = asn1cpp::getField(impactReduction->vehicleMass,long);
        impactReduction_data.requestResponseIndication = asn1cpp::getField(impactReduction->requestResponseIndication,long);

        alacarte.impactReduction.setData (impactReduction_data);
      }

    auto externalTemp = asn1cpp::getField(denm_alacarte_container->externalTemperature,long,&ok);
    if(ok)
      alacarte.externalTemperature.setData (externalTemp);

    auto roadworks = asn1cpp::getSeqOpt(denm_alacarte_container->roadWorks,RoadWorksContainerExtended,&ok);
    if(ok)
      {
        DEN_RoadWorksContainerExtended_t roadworks_data = {};
        bool roadworks_ok;
        auto lightBarSirenInUse = asn1cpp::bitstring::getterByteMask(roadworks->lightBarSirenInUse,0,&roadworks_ok);
        if(roadworks_ok)
          roadworks_data.lightBarSirenInUse.setData (getFromMask(lightBarSirenInUse));

        auto closedLanes = asn1cpp::getSeqOpt(roadworks->closedLanes,ClosedLanes,&roadworks_ok);
        if(roadworks_ok)
          {
            bool closedLanes_ok;
            auto innerHard = asn1cpp::getField(closedLanes->innerhardShoulderStatus,long,&closedLanes_ok);
            if(closedLanes_ok)
              roadworks_data.innerhardShoulderStatus.setData (innerHard);
            auto outerHand = asn1cpp::getField(closedLanes->outerhardShoulderStatus,long,&closedLanes_ok);
            if(closedLanes_ok)
              {
                roadworks_data.outerhardShoulderStatus.setData (outerHand);
              }
            auto drivingLaneStatus_0 = asn1cpp::bitstring::getterByteMask (closedLanes->drivingLaneStatus,0,&closedLanes_ok);
            if(closedLanes_ok)
                roadworks_data.drivingLaneStatus.setData (getFromMasks(drivingLaneStatus_0, asn1cpp::bitstring::getterByteMask (closedLanes->drivingLaneStatus,1)));
          }

        int restriction_size = asn1cpp::sequenceof::getSize(roadworks->restriction);
        std::vector<long> restriction;
        for(int i=0;i<restriction_size;i++)
            restriction.push_back (asn1cpp::sequenceof::getField(roadworks->restriction,long,i));
        roadworks_data.restriction.setData (restriction);

        auto speedLimit = asn1cpp::getField(roadworks->speedLimit,long,&roadworks_ok);
        if(roadworks_ok)
          roadworks_data.speedLimit.setData (speedLimit);

        auto incidentIndication = asn1cpp::getSeqOpt(roadworks->incidentIndication,CauseCode,&roadworks_ok);
        if(roadworks_ok)
          {
            roadworks_data.causeCode.setData (asn1cpp::getField(incidentIndication->causeCode,long));
            roadworks_data.subCauseCode.setData (asn1cpp::getField(incidentIndication->subCauseCode,long));
          }
        auto recommendedPath = asn1cpp::getSeqOpt(roadworks->recommendedPath,ItineraryPath,&roadworks_ok);
        if(roadworks_ok)
          {
            int recommended_size = asn1cpp::sequenceof::getSize(roadworks->recommendedPath);
            std::vector<DEN_ReferencePosition_t> recommendedPath_data;
            for(int i=0;i<recommended_size;i++)
              {
                DEN_ReferencePosition_t refPosition_data;
                auto refPos = asn1cpp::sequenceof::getSeq(roadworks->recommendedPath,ReferencePosition,i);

                refPosition_data.latitude = asn1cpp::getField(refPos->latitude,long);
                refPosition_data.longitude = asn1cpp::getField(refPos->longitude,long);
                refPosition_data.positionConfidenceEllipse.semiMajorConfidence = asn1cpp::getField(refPos->positionConfidenceEllipse.semiMajorConfidence,long);
                refPosition_data.positionConfidenceEllipse.semiMinorConfidence = asn1cpp::getField(refPos->positionConfidenceEllipse.semiMinorConfidence,long);
                refPosition_data.positionConfidenceEllipse.semiMajorOrientation = asn1cpp::getField(refPos->positionConfidenceEllipse.semiMajorOrientation,long);
                refPosition_data.altitude.setValue (asn1cpp::getField(refPos->altitude.altitudeValue,long));
                refPosition_data.altitude.setConfidence (asn1cpp::getField(refPos->altitude.altitudeConfidence,long));

                recommendedPath_data.push_back (refPosition_data);
              }
            roadworks_data.recommendedPath.setData (recommendedPath_data);
          }

        auto startingSpeedLimit = asn1cpp::getSeqOpt(roadworks->startingPointSpeedLimit,DeltaReferencePosition,&roadworks_ok);
        if(roadworks_ok)
          {
            DEN_DeltaReferencePosition_t deltaRef;
            deltaRef.deltaLatitude = asn1cpp::getField(startingSpeedLimit->deltaLatitude,long);
            deltaRef.deltaLongitude = asn1cpp::getField(startingSpeedLimit->deltaLongitude,long);
            deltaRef.deltaAltitude = asn1cpp::getField(startingSpeedLimit->deltaAltitude,long);
            roadworks_data.startingPointSpeedLimit.setData (deltaRef);
          }

        auto trafficFlowRule = asn1cpp::getField(roadworks->trafficFlowRule,long,&roadworks_ok);
        if(roadworks_ok)
          roadworks_data.trafficFlowRule.setData (trafficFlowRule);

        auto refDenms = asn1cpp::getSeqOpt(roadworks->referenceDenms,ReferenceDenms,&roadworks_ok);
        if(roadworks_ok)
          {
            int refDenms_size = asn1cpp::sequenceof::getSize(roadworks->referenceDenms);
            std::vector<DEN_ActionID_t> refDenms_data;

            for(int i=0;i<refDenms_size;i++)
              {
                DEN_ActionID_t actionId_data;
                auto actionId = asn1cpp::sequenceof::getSeq(roadworks->referenceDenms,ActionID,i);
                actionId_data.originatingStationID = asn1cpp::getField(actionId->originatingStationId,long);
                actionId_data.sequenceNumber = asn1cpp::getField(actionId->sequenceNumber,long);
                refDenms_data.push_back (actionId_data);
              }
            roadworks_data.referenceDenms.setData (refDenms_data);
          }
        alacarte.roadWorks.setData (roadworks_data);
      }

    auto positioningSol = asn1cpp::getField(denm_alacarte_container->positioningSolution,long,&ok);
    if(ok)
      alacarte.positioningSolution.setData (positioningSol);

    auto stationaryVehicle = asn1cpp::getSeqOpt(denm_alacarte_container->stationaryVehicle,StationaryVehicleContainer,&ok);
    if(ok)
      {
        bool stationary_ok;
        DEN_StationaryVehicleContainer_t stationaryVeh_data;
        auto stationarySince = asn1cpp::getField(stationaryVehicle->stationarySince,long,&stationary_ok);
        if(stationary_ok)
          stationaryVeh_data.stationarySince.setData (stationarySince);

        auto stationaryCause = asn1cpp::getSeqOpt(stationaryVehicle->stationaryCause,CauseCode,&stationary_ok);
        if(stationary_ok)
          {
            stationaryVeh_data.causeCode.setData (asn1cpp::getField(stationaryCause->causeCode,long));
            stationaryVeh_data.subCauseCode.setData (asn1cpp::getField(stationaryCause->subCauseCode,long));
          }

        auto dangerousGoods = asn1cpp::getSeqOpt(stationaryVehicle->carryingDangerousGoods,DangerousGoodsExtended,&stationary_ok);
        if(stationary_ok)
          {
            bool dangerous_ok;
            DEN_DangerousGoodsExtended_t dangerousGoods_data;
            dangerousGoods_data.dangerousGoodsType = asn1cpp::getField(dangerousGoods->dangerousGoodsType,long);
            dangerousGoods_data.unNumber = asn1cpp::getField(dangerousGoods->unNumber,long);
            dangerousGoods_data.elevatedTemperature = asn1cpp::getField(dangerousGoods->elevatedTemperature,bool);
            dangerousGoods_data.tunnelsRestricted = asn1cpp::getField(dangerousGoods->tunnelsRestricted,bool);
            dangerousGoods_data.limitedQuantity = asn1cpp::getField(dangerousGoods->limitedQuantity,bool);

            auto emergencyActionCode = asn1cpp::getField(dangerousGoods->emergencyActionCode,std::string,&dangerous_ok);
            if(dangerous_ok)
              dangerousGoods_data.emergencyActionCode.setData (emergencyActionCode);

            auto phoneNumber = asn1cpp::getField(dangerousGoods->phoneNumber,std::string,&dangerous_ok);
            if(dangerous_ok)
              dangerousGoods_data.phoneNumber.setData (phoneNumber);

            auto companyName = asn1cpp::getField(dangerousGoods->companyName,std::string,&dangerous_ok);
            if(dangerous_ok)
              dangerousGoods_data.companyName.setData (companyName);

            stationaryVeh_data.carryingDangerousGoods.setData (dangerousGoods_data);
          }

        auto numberOfOccupants = asn1cpp::getField(stationaryVehicle->numberOfOccupants,long,&stationary_ok);
        if(stationary_ok)
          stationaryVeh_data.numberOfOccupants.setData (numberOfOccupants);

        auto vehicleId = asn1cpp::getSeqOpt(stationaryVehicle->vehicleIdentification,VehicleIdentification,&stationary_ok);
        if(stationary_ok)
          {
            bool vehicleId_ok;
            DEN_VehicleIdentification_t vehicleId_data;

            auto wMInumber = asn1cpp::getField(vehicleId->wMInumber,std::string,&vehicleId_ok);
            if(vehicleId_ok)
              vehicleId_data.wMInumber.setData (wMInumber);

            auto vDS = asn1cpp::getField(vehicleId->vDS,std::string,&vehicleId_ok);
            if(vehicleId_ok)
              vehicleId_data.vDS.setData (vDS);

            stationaryVeh_data.vehicleIdentification.setData (vehicleId_data);
          }

        auto energyStorageType = asn1cpp::bitstring::getterByteMask (stationaryVehicle->energyStorageType,0,&stationary_ok);
        if(stationary_ok)
          stationaryVeh_data.energyStorageType.setData (getFromMask(energyStorageType));

        alacarte.stationaryVehicle.setData (stationaryVeh_data);
      }


   denm_data.setDenmAlacarteData_asn_types (alacarte);
  }
}
