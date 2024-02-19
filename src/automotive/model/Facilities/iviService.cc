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
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/


#include "iviService.h"
#include "ns3/asn_application.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/Setter.hpp"
#include "ns3/Encoding.hpp"
#include "ns3/SetOf.hpp"
#include "ns3/SequenceOf.hpp"
#include "ns3/BitString.hpp"
#include "ns3/View.hpp"
#include "ns3/Utils.hpp"
#include "ns3/snr-tag.h"
#include "ns3/sinr-tag.h"
#include "ns3/rssi-tag.h"
#include "ns3/timestamp-tag.h"
#include "ns3/rsrp-tag.h"


namespace ns3 {

  NS_LOG_COMPONENT_DEFINE("IVIBasicService");

  IVIBasicService::IVIBasicService()
  {
    m_station_id = ULONG_MAX;
    m_stationtype = LONG_MAX;
    m_seq_number = 0;
    m_socket_tx = NULL;
    m_real_time = false;
    m_btp = NULL;
    n_update=0;
    receivedIVIM = 0;
    b_trig=false;
    b_update=false;
    b_canc=false;
    b_neg=false;
  }

  bool
  IVIBasicService::CheckMainAttributes()
  {
    return m_station_id!=ULONG_MAX && m_stationtype!=LONG_MAX;
  }



  IVIBasicService::IVIBasicService(unsigned long fixed_stationid,long fixed_stationtype,Ptr<Socket> socket_tx)
  {
    m_station_id = (StationID_t) fixed_stationid;
    m_stationtype = (StationType_t) fixed_stationtype;
    m_seq_number = 0;
    m_socket_tx = socket_tx;
    m_real_time = false;
    m_btp = NULL;
  }

  void
  IVIBasicService::setStationProperties(unsigned long fixed_stationid,long fixed_stationtype)
  {
    m_station_id=fixed_stationid;
    m_stationtype=fixed_stationtype;
    m_btp->setStationProperties(fixed_stationid,fixed_stationtype);
  }

  void
  IVIBasicService::setFixedPositionRSU(double latitude_deg, double longitude_deg)
  {
    m_btp->setFixedPositionRSU(latitude_deg,longitude_deg);

  }

  void
  IVIBasicService::setStationID(unsigned long fixed_stationid)
  {
    m_station_id = fixed_stationid;
    m_btp->setStationID(fixed_stationid);
  }

  void
  IVIBasicService::setStationType(long fixed_stationtype)
  {
    m_stationtype=fixed_stationtype;
    m_btp->setStationType(fixed_stationtype);
  }

  void
  IVIBasicService::setSocketTx (Ptr<Socket> socket_tx)
  {
    m_btp->setSocketTx(socket_tx);
  }

  void
  IVIBasicService::setSocketRx (Ptr<Socket> socket_rx)
  {
    m_btp->setSocketRx(socket_rx);
    m_btp->addIVIMRxCallback (std::bind(&IVIBasicService::receiveIVIM,this,std::placeholders::_1,std::placeholders::_2));
  }

  ActionID
  IVIBasicService::getActionId ()
  {
    ActionID actionID;
    actionID.sequenceNumber = m_seq_number;
    actionID.originatingStationID = m_station_id;

    return actionID;
  }

  asn1cpp::Seq<PolygonalLine>
  IVIBasicService::fillPolyLine (iviData::IVI_glcP_PolygonalLine_t line)
  {

    asn1cpp::Seq<PolygonalLine> polyLine = asn1cpp::makeSeq(PolygonalLine);

    if(line.deltaPositions.isAvailable ())
      {
        auto deltaPos = line.deltaPositions.getData ();
        asn1cpp::setField(polyLine->present,PolygonalLine_PR_deltaPositions);
        for(auto deltaPos_it = deltaPos.begin (); deltaPos_it != deltaPos.end (); deltaPos_it++)
          {
            auto deltaSeq = asn1cpp::makeSeq(DeltaPosition);
            asn1cpp::setField(deltaSeq->deltaLatitude,deltaPos_it->deltaLat);
            asn1cpp::setField(deltaSeq->deltaLongitude,deltaPos_it->deltaLong);
            asn1cpp::sequenceof::pushList(polyLine->choice.deltaPositions,deltaSeq);
          }
      }
    else if(line.deltaPositionsWA.isAvailable ())
      {
        auto deltaPos = line.deltaPositionsWA.getData ();
        asn1cpp::setField(polyLine->present,PolygonalLine_PR_deltaPositionsWithAltitude);
        for(auto deltaPos_it = deltaPos.begin (); deltaPos_it != deltaPos.end (); deltaPos_it++)
          {
            auto deltaSeq = asn1cpp::makeSeq(DeltaReferencePosition);
            asn1cpp::setField(deltaSeq->deltaLatitude,deltaPos_it->deltaLat);
            asn1cpp::setField(deltaSeq->deltaLongitude,deltaPos_it->deltaLong);
            asn1cpp::setField(deltaSeq->deltaAltitude,deltaPos_it->deltaAltitude);
            asn1cpp::sequenceof::pushList(polyLine->choice.deltaPositionsWithAltitude,deltaSeq);
          }
      }
    else if(line.absPositions.isAvailable ())
      {
        auto absPos = line.absPositions.getData ();
        asn1cpp::setField(polyLine->present,PolygonalLine_PR_absolutePositions);
        for(auto absPos_it = absPos.begin (); absPos_it != absPos.end (); absPos_it++)
          {
            auto absSeq = asn1cpp::makeSeq(AbsolutePosition);
            asn1cpp::setField(absSeq->latitude,absPos_it->lat);
            asn1cpp::setField(absSeq->longitude,absPos_it->lon);
            asn1cpp::sequenceof::pushList(polyLine->choice.absolutePositions,absSeq);
          }
      }
    else if(line.absPositionsWA.isAvailable ())
      {
        auto absPos = line.absPositionsWA.getData ();
        asn1cpp::setField(polyLine->present,PolygonalLine_PR_absolutePositionsWithAltitude);
        for(auto absPos_it = absPos.begin (); absPos_it != absPos.end (); absPos_it++)
          {
            auto absSeq = asn1cpp::makeSeq(AbsolutePositionWAltitude);
            asn1cpp::setField(absSeq->latitude,absPos_it->lat);
            asn1cpp::setField(absSeq->longitude,absPos_it->lon);
            asn1cpp::setField(absSeq->altitude.altitudeValue,absPos_it->altitude);
            asn1cpp::setField(absSeq->altitude.altitudeConfidence,AltitudeConfidence_unavailable);
            asn1cpp::sequenceof::pushList(polyLine->choice.absolutePositionsWithAltitude,absSeq);
          }
      }
    else
      {
        asn1cpp::setField(polyLine->present,PolygonalLine_PR_NOTHING);
      }

    return polyLine;
  }

  IVIBasicService_error_t
  IVIBasicService::fillIVIM(asn1cpp::Seq<IVIM> &ivim, iviData Data, const ActionID_t actionID)
  {

    /* Header */
    asn1cpp::setField(ivim->header.messageID,FIX_IVIMID); // description in ITS-Container.asn
    //asn1cpp::setField(ivim->header.protocolVersion,protocolVersion_currentVersion); // description in ITS-Container.asn
    asn1cpp::setField(ivim->header.stationID,m_station_id); // description in ITS-Container.asn


    /* Mandatory */
    iviData::iviDataMandatory mandatoryData = Data.getIvimMandatory_asn_types();
    asn1cpp::bitstring::setBit(ivim->ivi.mandatory.serviceProviderId.countryCode,
                               mandatoryData.countryCode,0) ; // description in asn1_IS_ISO_TS_19321_IVI.asn
   // asn1cpp::bitstring::setBit(ivim->ivi.mandatory.serviceProviderId.countryCode,Data.getIvimMandatoryCountryCode2 (),1) ; // description in asn1_IS_ISO_TS_19321_IVI.asn
    asn1cpp::setField(ivim->ivi.mandatory.serviceProviderId.providerIdentifier,
                      mandatoryData.providerIdentifier) ;// description in asn1_IS_ISO_TS_14906_EfcDsrcApplication.asn and asn1_IS_ISO_TS_14816_AVIAEINumberingAndDataStructures.asn
    asn1cpp::setField(ivim->ivi.mandatory.iviIdentificationNumber,
                      actionID.sequenceNumber) ; // description in asn1_IS_ISO_TS_19321_IVI.asn

    timeStamp = compute_timestampIts(m_real_time);
    asn1cpp::setField(ivim->ivi.mandatory.timeStamp,timeStamp) ;

    if(mandatoryData.validFrom.isAvailable ())
      asn1cpp::setField(ivim->ivi.mandatory.validFrom,mandatoryData.validFrom.getData ());
    if(mandatoryData.validTo.isAvailable ())
      asn1cpp::setField(ivim->ivi.mandatory.validTo, mandatoryData.validTo.getData ());

    asn1cpp::setField(ivim->ivi.mandatory.iviStatus, mandatoryData.iviStatus) ; // description in asn1_IS_ISO_TS_19321_IVI.asn




    /* Optional */

    /* IviContainer definition */

    /* Geographic Location Container (glc)*/

    if (Data.getOptionalPresent ()[0]){

      auto iviCont = asn1cpp::makeSeq(IviContainer) ;

      asn1cpp::setField(iviCont->present,IviContainer_PR_glc);

      auto glcData = Data.getIvimGlc ();
      auto ivimGlc = asn1cpp::makeSeq(GeographicLocationContainer) ;
      asn1cpp::setField(ivimGlc->referencePosition.latitude,
                        glcData.referencePosition.latitude) ;
      asn1cpp::setField(ivimGlc->referencePosition.longitude,
                        glcData.referencePosition.longitude) ;
      asn1cpp::setField(ivimGlc->referencePosition.positionConfidenceEllipse.semiMajorConfidence,
                        glcData.referencePosition.positionConfidenceEllipse.semiMajorConfidence) ;
      asn1cpp::setField(ivimGlc->referencePosition.positionConfidenceEllipse.semiMinorConfidence,
                        glcData.referencePosition.positionConfidenceEllipse.semiMinorConfidence) ;
      asn1cpp::setField(ivimGlc->referencePosition.positionConfidenceEllipse.semiMajorOrientation,
                        glcData.referencePosition.positionConfidenceEllipse.semiMajorOrientation) ;
      asn1cpp::setField(ivimGlc->referencePosition.altitude.altitudeValue,
                        glcData.referencePosition.altitude.getValue ()) ;
      asn1cpp::setField(ivimGlc->referencePosition.altitude.altitudeConfidence,
                        glcData.referencePosition.altitude.getConfidence ()) ;
      if(glcData.referencePositionTime.isAvailable ())
        asn1cpp::setField(ivimGlc->referencePositionTime,glcData.referencePositionTime.getData ());

      if(glcData.referencePositionHeading.isAvailable ())
        {
          auto heading = asn1cpp::makeSeq(Heading);
          asn1cpp::setField(heading->headingValue,glcData.referencePositionHeading.getData ().getValue ());
          asn1cpp::setField(heading->headingConfidence,glcData.referencePositionHeading.getData ().getConfidence ());
          asn1cpp::setField(ivimGlc->referencePositionHeading,heading);
        }

      if(glcData.referencePositionHeading.isAvailable ())
        {
          auto speed = asn1cpp::makeSeq(Speed);
          asn1cpp::setField(speed->speedValue,glcData.referencePositionHeading.getData ().getValue ());
          asn1cpp::setField(speed->speedConfidence,glcData.referencePositionHeading.getData ().getConfidence ());
          asn1cpp::setField(ivimGlc->referencePositionSpeed,speed);
        }

      for(auto glcPart_it = glcData.GlcPart.begin (); glcPart_it != glcData.GlcPart.end (); glcPart_it++)
        {
          auto glcPart = asn1cpp::makeSeq(GlcPart);

          asn1cpp::setField(glcPart->zoneId,glcPart_it->zoneId) ;

          if(glcPart_it->laneNumber.isAvailable ())
            asn1cpp::setField(glcPart->laneNumber,glcPart_it->laneNumber.getData ());
          if(glcPart_it->zoneExtension.isAvailable ())
            asn1cpp::setField(glcPart->zoneExtension,glcPart_it->zoneExtension.getData ());
          if(glcPart_it->zoneHeading.isAvailable ())
            asn1cpp::setField(glcPart->zoneHeading,glcPart_it->zoneHeading.getData ());

          if(glcPart_it->zone.isAvailable ())
            {
              auto zone = asn1cpp::makeSeq(Zone);
              auto zoneData = glcPart_it->zone.getData ();

              if(zoneData.segment.isAvailable ())
                {
                  auto segment = zoneData.segment.getData ();
                  asn1cpp::setField(zone->present,Zone_PR_segment);
                  if(segment.laneWidth.isAvailable ())
                    asn1cpp::setField(zone->choice.segment.laneWidth,segment.laneWidth.getData ());
                  asn1cpp::setField(zone->choice.segment.line,fillPolyLine (segment.line));
                }
              else if(zoneData.area.isAvailable ())
                {
                  asn1cpp::setField(zone->present,Zone_PR_area);
                  asn1cpp::setField(zone->choice.area,fillPolyLine (zoneData.area.getData ()));
                }
              else if(zoneData.computedSegment.isAvailable ())
                {
                  auto cSegment = zoneData.computedSegment.getData ();

                  asn1cpp::setField(zone->present,Zone_PR_computedSegment);
                  asn1cpp::setField(zone->choice.computedSegment.zoneId,cSegment.zoneID);
                  asn1cpp::setField(zone->choice.computedSegment.laneNumber,cSegment.laneNumber);
                  asn1cpp::setField(zone->choice.computedSegment.laneWidth,cSegment.laneWidth);

                  if(cSegment.offsetDist.isAvailable ())
                    asn1cpp::setField(zone->choice.computedSegment.offsetDistance,cSegment.offsetDist.getData ());

                  if(cSegment.offsetPosition.isAvailable ())
                    {
                      auto refPos = cSegment.offsetPosition.getData ();
                      auto offsetSeq = asn1cpp::makeSeq(DeltaReferencePosition);
                      asn1cpp::setField(offsetSeq->deltaLatitude,refPos.deltaLat);
                      asn1cpp::setField(offsetSeq->deltaLongitude,refPos.deltaLong);
                      asn1cpp::setField(offsetSeq->deltaAltitude,refPos.deltaAltitude);
                      asn1cpp::setField(zone->choice.computedSegment.offsetPosition,offsetSeq);
                    }
                }
              asn1cpp::setField(glcPart->zone,zone);
            }
          asn1cpp::sequenceof::pushList(ivimGlc->parts,glcPart);
        }
      asn1cpp::setField(iviCont->choice.glc,ivimGlc);
      asn1cpp::sequenceof::pushList(ivim->ivi.optional, iviCont);
    }


    /* General Ivi Container (gic) definitions */

    if (Data.getOptionalPresent ()[1]){

    auto gicData = Data.getGic ();
    auto iviCont = asn1cpp::makeSeq(IviContainer);
    asn1cpp::setField(iviCont->present,IviContainer_PR_giv);

    // gic
    auto gic = asn1cpp::makeSeq(GeneralIviContainer) ;


    // gicPart
    for(auto gicPart_it = gicData.GicPart.begin (); gicPart_it != gicData.GicPart.end (); gicPart_it++)
      {
        auto gicPart = asn1cpp::makeSeq(GicPart);

        if(gicPart_it->detectionZoneIds.isAvailable ())
          {
            auto dZ = gicPart_it->detectionZoneIds.getData ();
            for(auto dZ_it = dZ.begin (); dZ_it != dZ.end ();dZ_it++)
              asn1cpp::sequenceof::pushList(gicPart->detectionZoneIds,*dZ_it) ;
          }
        if(gicPart_it->relevanceZoneIds.isAvailable ())
          {
            auto rZ = gicPart_it->relevanceZoneIds.getData ();
            for(auto rZ_it = rZ.begin (); rZ_it != rZ.end ();rZ_it++)
              asn1cpp::sequenceof::pushList(gicPart->relevanceZoneIds,*rZ_it) ;
          }
        if(gicPart_it->direction.isAvailable ())
          asn1cpp::setField(gicPart->direction,gicPart_it->direction.getData ()) ;

        asn1cpp::setField(gicPart->iviType,gicPart_it->iviType) ;

        for(auto rsCode_it = gicPart_it->RS.begin ();rsCode_it != gicPart_it->RS.end (); rsCode_it++)
          {
            auto RS_0 = asn1cpp::makeSeq(RSCode);
            asn1cpp::setField(RS_0->code.present,RSCode__code_PR_iso14823);

            if(rsCode_it->RS_trafficSignPictogram.isAvailable ())
              {
                asn1cpp::setField(RS_0->code.choice.iso14823.pictogramCode.serviceCategoryCode.present,
                                     ISO14823Code__pictogramCode__serviceCategoryCode_PR_trafficSignPictogram);
                asn1cpp::setField(RS_0->code.choice.iso14823.pictogramCode.serviceCategoryCode.choice.trafficSignPictogram,
                                     rsCode_it->RS_trafficSignPictogram.getData ()) ;
              }
            asn1cpp::setField(RS_0->code.choice.iso14823.pictogramCode.pictogramCategoryCode.nature,
                                 rsCode_it->RS_nature) ;
            asn1cpp::setField(RS_0->code.choice.iso14823.pictogramCode.pictogramCategoryCode.serialNumber,
                                 rsCode_it->RS_serialNumber) ;
            if(rsCode_it->RS_unit.isAvailable ())
              {
                auto at = asn1cpp::makeSeq(ISO14823Attribute);

                asn1cpp::setField(at->present, ISO14823Attribute_PR_spe);
                if(rsCode_it->RS_spm.isAvailable ())
                  asn1cpp::setField(at->choice.spe.spm,rsCode_it->RS_spm.getData ());
                if(rsCode_it->RS_mns.isAvailable ())
                  asn1cpp::setField(at->choice.spe.mns,rsCode_it->RS_mns.getData ());

                asn1cpp::setField(at->choice.spe.unit,
                                  rsCode_it->RS_unit.getData());

                asn1cpp::sequenceof::pushList(RS_0->code.choice.iso14823.attributes,at);
              }
            asn1cpp::sequenceof::pushList(gicPart->roadSignCodes,RS_0);
          }
        asn1cpp::sequenceof::pushList(iviCont->choice.giv,gicPart);
      }
    asn1cpp::sequenceof::pushList(ivim->ivi.optional, iviCont);
}

    // Road Configuration Container

    if (Data.getOptionalPresent ()[2]){

    /* IviContainer definition */
    auto iviCont = asn1cpp::makeSeq(IviContainer) ;

    asn1cpp::setField(iviCont->present,IviContainer_PR_rcc);

    auto ivimRcc = asn1cpp::makeSeq(RoadConfigurationContainer) ;

    auto rccData = Data.getRcc ();

    for(auto rccPart_it = rccData.IVIrccPart.begin ();rccPart_it != rccData.IVIrccPart.end (); rccPart_it++)
      {
        auto rccPart = asn1cpp::makeSeq(RccPart);

        for(auto zIDs_it = rccPart_it->zoneID.begin ();zIDs_it != rccPart_it->zoneID.end ();zIDs_it++)
          asn1cpp::sequenceof::pushList(rccPart->zoneIds,*zIDs_it);

        asn1cpp::setField(rccPart->roadType,rccPart_it->roadType);

        for(auto laneinfo_it = rccPart_it->laneInformation.begin ();laneinfo_it != rccPart_it->laneInformation.end (); laneinfo_it++)
          {
             auto laneInfo = asn1cpp::makeSeq(LaneInformation);
             asn1cpp::setField(laneInfo->laneNumber,laneinfo_it->laneNumber);
             asn1cpp::setField(laneInfo->direction,laneinfo_it->direction);
             asn1cpp::setField(laneInfo->laneType,laneinfo_it->laneType);
             asn1cpp::setField(laneInfo->laneStatus,laneinfo_it->laneStatus);
             asn1cpp::sequenceof::pushList(rccPart->laneConfiguration, laneInfo) ;

          }
        asn1cpp::sequenceof::pushList(iviCont->choice.rcc,rccPart);
      }
    asn1cpp::sequenceof::pushList(ivim->ivi.optional, iviCont) ;

    }

    // Text Container

    if (Data.getOptionalPresent ()[3]){

    /* IviContainer definition */
    auto iviCont = asn1cpp::makeSeq(IviContainer) ;

    asn1cpp::setField(iviCont->present,IviContainer_PR_tc);

    auto ivimTcc = asn1cpp::makeSeq(TextContainer) ;
    auto tcData = Data.getTc ();

    for(auto tc_it = tcData.IVItcPart.begin ();tc_it != tcData.IVItcPart.end ();tc_it++)
      {
        auto tcPart = asn1cpp::makeSeq(TcPart);

        if(tc_it->detectionZoneIds.isAvailable ())
          {
            auto dZ = tc_it->detectionZoneIds.getData ();
            for(auto dZ_it = dZ.begin (); dZ_it != dZ.end ();dZ_it++)
              asn1cpp::sequenceof::pushList(tcPart->detectionZoneIds,*dZ_it) ;
          }
        auto rZ = tc_it->relevanceZoneIds;
        for(auto rZ_it = rZ.begin (); rZ_it != rZ.end ();rZ_it++)
          asn1cpp::sequenceof::pushList(tcPart->relevanceZoneIds,*rZ_it) ;

        if(tc_it->direction.isAvailable ())
          asn1cpp::setField(tcPart->direction,tc_it->direction.getData ());
        if(tc_it->driverAwarenessZoneIds.isAvailable ())
          {
            auto dZ = tc_it->driverAwarenessZoneIds.getData ();
            for(auto dZ_it = dZ.begin (); dZ_it != dZ.end ();dZ_it++)
              asn1cpp::sequenceof::pushList(tcPart->driverAwarenessZoneIds,*dZ_it) ;
          }
        if(tc_it->minimumAwarenessTime.isAvailable ())
          asn1cpp::setField(tcPart->minimumAwarenessTime,tc_it->minimumAwarenessTime.getData ());

        if(tc_it->applicableLanesPos.isAvailable ())
          {
            auto tcLanePosition = asn1cpp::makeSeq(LanePosition);
            auto lanes = tc_it->applicableLanesPos.getData ();
            for(auto lanes_it = lanes.begin ();lanes_it != lanes.end ();lanes_it++)
              asn1cpp::sequenceof::pushList(tcPart->applicableLanes,*lanes_it);
            //asn1cpp::setField(*tcLanePosition_0,LanePosition_secondLaneFromOutside);
            //asn1cpp::sequenceof::pushList(tcPart_0_0->applicableLanes, tcLanePosition_0);
          }

        if(tc_it->layoutId.isAvailable ())
          asn1cpp::setField(tcPart->layoutId,tc_it->layoutId.getData ());
        if(tc_it->preStoredlayoutId.isAvailable ())
          asn1cpp::setField(tcPart->preStoredlayoutId,tc_it->preStoredlayoutId.getData ());

        asn1cpp::setField(tcPart->data,tc_it->data);

        if(tc_it->text.isAvailable ())
          {
            auto text = tc_it->text.getData ();
            for(auto text_it = text.begin ();text_it != text.end (); text_it++)
              {
                auto tcText = asn1cpp::makeSeq(Text);
                if(text_it->layoutComponentId.isAvailable ())
                  asn1cpp::setField(tcText->layoutComponentId,text_it->layoutComponentId.getData ());
                asn1cpp::bitstring::setBit(tcText->language,text_it->bitLanguage,0);
                asn1cpp::setField(tcText->textContent,text_it->textCont);
                asn1cpp::sequenceof::pushList(tcPart->text, tcText);
              }
          }
        asn1cpp::sequenceof::pushList(iviCont->choice.tc, tcPart) ;
      }
    asn1cpp::sequenceof::pushList(ivim->ivi.optional, iviCont) ;

}
    // Layout Container

    if (Data.getOptionalPresent ()[4]){

    /* IviContainer definition */
    auto iviCont = asn1cpp::makeSeq(IviContainer) ;

    asn1cpp::setField(iviCont->present,IviContainer_PR_lac);

    auto ivimLac = asn1cpp::makeSeq(LayoutContainer) ;

    asn1cpp::setField(ivimLac->layoutId,Data.getLac ().layoutId);

    if(Data.getLac ().height.isAvailable ())
      asn1cpp::setField(ivimLac->height,Data.getLac ().height.getData ());
    if(Data.getLac ().height.isAvailable ())
      asn1cpp::setField(ivimLac->width,Data.getLac ().width.getData ());

    auto lacComp = Data.getLac ().IVIlacComp;
    for(auto lacComp_it = lacComp.begin ();lacComp_it != lacComp.end ();lacComp_it++)
      {
        auto lacComp = asn1cpp::makeSeq(LayoutComponent) ;
        asn1cpp::setField(lacComp->x,lacComp_it->x);
        asn1cpp::setField(lacComp->y,lacComp_it->y);
        asn1cpp::setField(lacComp->width,lacComp_it->width);
        asn1cpp::setField(lacComp->height,lacComp_it->height);
        asn1cpp::setField(lacComp->textScripting,lacComp_it->textScripting);
        asn1cpp::setField(lacComp->layoutComponentId,lacComp_it->layoutComponentId);
        asn1cpp::sequenceof::pushList(ivimLac->layoutComponents,lacComp);
      }
    asn1cpp::setField(iviCont->choice.lac, ivimLac) ;
    asn1cpp::sequenceof::pushList(ivim->ivi.optional, iviCont) ;

    }

    return IVIM_NO_ERROR;
  }


  IVIBasicService_error_t
  IVIBasicService::appIVIM_repetition (iviData Data)
  {

    auto ivim = asn1cpp::makeSeq(IVIM);

    /* Encode */

    /* Assign unused actionID value */
    ActionID actionid;
    actionid.originatingStationID = m_station_id;
    actionid.sequenceNumber = m_seq_number-1;

    //std::pair <unsigned long, long> map_index = std::make_pair((unsigned long)m_station_id,(long)m_seq_number);


    asn1cpp::setField(ivim->ivi.mandatory.timeStamp,timeStamp) ;

    if (b_trig) {
      //asn1cpp::setField(ivim->ivi.mandatory.iviStatus,IviStatus_new) ; // description in asn1_IS_ISO_TS_19321_IVI.asn, 0->new
      Data.setIvimIviStatus (IviStatus_new);
      } else if (b_update) {
    asn1cpp::setField(ivim->ivi.mandatory.iviStatus,IviStatus_update) ; // description in asn1_IS_ISO_TS_19321_IVI.asn, update
    Data.setIvimIviStatus (IviStatus_update);
      } else if (b_canc) {
      asn1cpp::setField(ivim->ivi.mandatory.iviStatus,IviStatus_cancellation) ; // description in asn1_IS_ISO_TS_19321_IVI.asn, cancellation
      Data.setIvimIviStatus (IviStatus_cancellation);
      } else if(b_neg) {
        asn1cpp::setField(ivim->ivi.mandatory.iviStatus,IviStatus_negation) ; // description in asn1_IS_ISO_TS_19321_IVI.asn, negation
        Data.setIvimIviStatus (IviStatus_negation);
      } else std::cout << "Repetition error" << std::endl;



    fillIVIM (ivim,Data,actionid);


    std::string encode_result = asn1cpp::uper::encode(ivim);

    Ptr<Packet> packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());
    //free(encode_result.buffer);

    BTPDataRequest_t dataRequest = {};
    dataRequest.BTPType = BTP_B; //!< BTP-B
    dataRequest.destPort = IVIM_PORT;
    dataRequest.destPInfo = 0;
    dataRequest.GNType = GBC;
    dataRequest.GnAddress = m_geoArea;
    dataRequest.GNCommProfile = UNSPECIFIED;
    dataRequest.GNRepInt =0;
    dataRequest.GNMaxRepInt=0;
    dataRequest.GNMaxLife = 70;
    dataRequest.GNMaxHL = 1;
    dataRequest.GNTraClass = 0x01; // Store carry foward: no - Channel offload: no - Traffic Class ID: 1
    dataRequest.lenght = packet->GetSize ();
    dataRequest.data = packet;
    m_btp->sendBTP(dataRequest);


       return IVIM_NO_ERROR;
  }


  IVIBasicService_error_t
  IVIBasicService::appIVIM_trigger(iviData Data)
  {

      printf("\n  +++ Launching appIVIM_trigger +++\n");

      auto ivim = asn1cpp::makeSeq(IVIM);

      /* Encode */

      /* Assign unused actionID value */
      ActionID actionid;
      actionid.originatingStationID = m_station_id;
      actionid.sequenceNumber = m_seq_number;

      //std::cout << "New Ivim triggered with ID number :" << actionid.sequenceNumber << std::endl;

      b_trig=true;
      b_update=false;
      b_canc=false;
      b_neg=false;
      m_seq_number++;

      Data.setIvimIviStatus (IviStatus_new);


      fillIVIM (ivim,Data,actionid);


      std::string encode_result = asn1cpp::uper::encode(ivim);

      Ptr<Packet> packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());
      //free(encode_result.buffer);

      BTPDataRequest_t dataRequest = {};
      dataRequest.BTPType = BTP_B; //!< BTP-B
      dataRequest.destPort = IVIM_PORT;
      dataRequest.destPInfo = 0;
      dataRequest.GNType = GBC;
      dataRequest.GnAddress = m_geoArea;
      dataRequest.GNCommProfile = UNSPECIFIED;
      dataRequest.GNRepInt =0;
      dataRequest.GNMaxRepInt=0;
      dataRequest.GNMaxLife = 70;
      dataRequest.GNMaxHL = 1;
      dataRequest.GNTraClass = 0x01; // Store carry foward: no - Channel offload: no - Traffic Class ID: 1
      dataRequest.lenght = packet->GetSize ();
      dataRequest.data = packet;
      m_btp->sendBTP(dataRequest);


         return IVIM_NO_ERROR;
  }

  IVIBasicService_error_t
  IVIBasicService::appIVIM_update(iviData Data, ActionID actionID)
  {

      printf("\n  +++ Launched appIVIM_update +++\n");


      /* between 2 updates a repetition is needed */

      if(!b_trig && b_update && !b_canc && !b_neg) appIVIM_repetition (Data);

      b_trig=false;
      b_update=true;
      b_canc=false;
      b_neg=false;


      auto ivim = asn1cpp::makeSeq(IVIM);

      Data.setIvimIviStatus (IviStatus_update); // description in asn1_IS_ISO_TS_19321_IVI.asn, 1-> update

      fillIVIM (ivim,Data,actionID);

      /* Encode */


      std::string encode_result = asn1cpp::uper::encode(ivim);

      Ptr<Packet> packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());
      //free(encode_result.buffer);

      BTPDataRequest_t dataRequest = {};
      dataRequest.BTPType = BTP_B; //!< BTP-B
      dataRequest.destPort = IVIM_PORT;
      dataRequest.destPInfo = 0;
      dataRequest.GNType = GBC;
      dataRequest.GnAddress = m_geoArea;
      dataRequest.GNCommProfile = UNSPECIFIED;
      dataRequest.GNRepInt =0;
      dataRequest.GNMaxRepInt=0;
      dataRequest.GNMaxLife = 70;
      dataRequest.GNMaxHL = 1;
      dataRequest.GNTraClass = 0x01; // Store carry foward: no - Channel offload: no - Traffic Class ID: 1
      dataRequest.lenght = packet->GetSize ();
      dataRequest.data = packet;
      m_btp->sendBTP(dataRequest);


  return IVIM_NO_ERROR;
  }

  IVIBasicService_error_t
  IVIBasicService::appIVIM_termination(iviData Data,ivimTerminationType term,ActionID_t actionID)
  {

      printf("\n  +++ Launched appIVIM_termination +++\n");


      auto ivim = asn1cpp::makeSeq(IVIM);

      if (term == term_cancellation) {
      //asn1cpp::setField(ivim->ivi.mandatory.iviStatus, IviStatus_cancellation) ; // description in asn1_IS_ISO_TS_19321_IVI.asn,
      Data.setIvimIviStatus (IviStatus_cancellation);
          // 2-> cancellation: by the Service Provider;
      b_trig=false;
      b_update=false;
      b_canc=true;
      b_neg=false;
       } else if (term == term_negation) {
      //asn1cpp::setField(ivim->ivi.mandatory.iviStatus, IviStatus_negation) ; // description in asn1_IS_ISO_TS_19321_IVI.asn,
      Data.setIvimIviStatus (IviStatus_negation);
      // 3-> negation: by another organization;
      b_trig=false;
      b_update=false;
      b_canc=false;
      b_neg=true;
      } else {
          printf(" No correct iviTerminationType selected");
      }


      fillIVIM (ivim,Data,actionID);


      std::string encode_result = asn1cpp::uper::encode(ivim);

      Ptr<Packet> packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());
      //free(encode_result.buffer);

      BTPDataRequest_t dataRequest = {};
      dataRequest.BTPType = BTP_B; //!< BTP-B
      dataRequest.destPort = IVIM_PORT;
      dataRequest.destPInfo = 0;
      dataRequest.GNType = GBC;
      dataRequest.GnAddress = m_geoArea;
      dataRequest.GNCommProfile = UNSPECIFIED;
      dataRequest.GNRepInt =0;
      dataRequest.GNMaxRepInt=0;
      dataRequest.GNMaxLife = 70;
      dataRequest.GNMaxHL = 1;
      dataRequest.GNTraClass = 0x01; // Store carry foward: no - Channel offload: no - Traffic Class ID: 1
      dataRequest.lenght = packet->GetSize ();
      dataRequest.data = packet;
      m_btp->sendBTP(dataRequest);



  return IVIM_NO_ERROR;
  }

  iviData::IVI_glcP_PolygonalLine_t
  IVIBasicService::getPolyLine (asn1cpp::Seq<PolygonalLine> line)
  {

    iviData::IVI_glcP_PolygonalLine_t polyLine;

    PolygonalLine_PR present = asn1cpp::getField(line->present,PolygonalLine_PR);
    if(present == PolygonalLine_PR_deltaPositions)
      {
        std::vector<iviData::IVI_glcP_deltaPosition_t> deltaPos;
        int size = asn1cpp::sequenceof::getSize(line->choice.deltaPositions);
        for(int i=0;i<size;i++)
          {
            iviData::IVI_glcP_deltaPosition_t deltaData;
            auto deltaSeq = asn1cpp::sequenceof::getSeq(line->choice.deltaPositions,DeltaPosition,i);
            deltaData.deltaLat = asn1cpp::getField(deltaSeq->deltaLatitude,long);
            deltaData.deltaLong = asn1cpp::getField(deltaSeq->deltaLongitude,long);
            deltaPos.push_back (deltaData);
          }
        polyLine.deltaPositions.setData (deltaPos);
      }
    else if(present == PolygonalLine_PR_deltaPositionsWithAltitude)
      {
        std::vector<iviData::IVI_glcP_deltaPositionWA_t> deltaPos;
        int size = asn1cpp::sequenceof::getSize(line->choice.deltaPositionsWithAltitude);
        for(int i=0;i<size;i++)
          {
            iviData::IVI_glcP_deltaPositionWA_t deltaData;
            auto deltaSeq = asn1cpp::sequenceof::getSeq(line->choice.deltaPositionsWithAltitude,DeltaReferencePosition,i);
            deltaData.deltaLat = asn1cpp::getField(deltaSeq->deltaLatitude,long);
            deltaData.deltaLong = asn1cpp::getField(deltaSeq->deltaLongitude,long);
            deltaData.deltaAltitude = asn1cpp::getField(deltaSeq->deltaAltitude,long);
            deltaPos.push_back (deltaData);
          }
        polyLine.deltaPositionsWA.setData (deltaPos);
      }
    else if(present == PolygonalLine_PR_absolutePositions)
      {
        std::vector<iviData::IVI_glcP_absPosition_t> absPos;
        int size = asn1cpp::sequenceof::getSize(line->choice.absolutePositions);
        for(int i=0;i<size;i++)
          {
            iviData::IVI_glcP_absPosition_t absData;
            auto absSeq = asn1cpp::sequenceof::getSeq(line->choice.absolutePositions,AbsolutePosition,i);
            absData.lat = asn1cpp::getField(absSeq->latitude,long);
            absData.lon = asn1cpp::getField(absSeq->longitude,long);
            absPos.push_back (absData);
          }
        polyLine.absPositions.setData (absPos);
      }
    else if(present == PolygonalLine_PR_absolutePositionsWithAltitude)
      {
        std::vector<iviData::IVI_glcP_absPositionWA_t> absPos;
        int size = asn1cpp::sequenceof::getSize(line->choice.absolutePositionsWithAltitude);
        for(int i=0;i<size;i++)
          {
            iviData::IVI_glcP_absPositionWA_t absData;
            auto absSeq = asn1cpp::sequenceof::getSeq(line->choice.absolutePositionsWithAltitude,AbsolutePositionWAltitude,i);
            absData.lat = asn1cpp::getField(absSeq->latitude,long);
            absData.lon = asn1cpp::getField(absSeq->longitude,long);
            absData.altitude = asn1cpp::getField(absSeq->altitude.altitudeValue,long);
            absPos.push_back (absData);
          }
        polyLine.absPositionsWA.setData (absPos);
      }
    //else PolygonalLine_PR_NOTHING

    return polyLine;
  }

  void
  IVIBasicService::receiveIVIM(BTPDataIndication_t dataIndication,Address from)
  {
      Ptr<Packet> packet;
      asn1cpp::Seq<IVIM> decoded_ivim;

      receivedIVIM++;

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

      SetSignalInfo(timestamp.Get(), rssi.Get(), snr.Get(), sinr.Get(), rsrp.Get());

      /* Try to check if the received packet is really a IVIM */
       if (buffer[1]!=FIX_IVIMID) //FIX_IVIMID = 0x06;
         {
           NS_LOG_ERROR("Warning: received a message which has messageID '"<<buffer[1]<<"' but '6' was expected.");
           free(buffer);
           return;
         }

       /** Decoding **/
       free(buffer);
       decoded_ivim = asn1cpp::uper::decode(packetContent, IVIM);

       iviData decodedData;

       decodedData.setIvimHeader (decoded_ivim->header.messageID,decoded_ivim->header.protocolVersion,decoded_ivim->header.stationID);
       iviData::iviDataMandatory mandatory;
       mandatory.timeStamp = (long) decoded_ivim->ivi.mandatory.timeStamp;
       mandatory.providerIdentifier = (long) decoded_ivim->ivi.mandatory.serviceProviderId.providerIdentifier;
       mandatory.countryCode = asn1cpp::bitstring::getterByteMask (decoded_ivim->ivi.mandatory.serviceProviderId.countryCode,0);
       mandatory.identificationNumber = asn1cpp::getField(decoded_ivim->ivi.mandatory.iviIdentificationNumber,long);
       mandatory.iviStatus = asn1cpp::getField(decoded_ivim->ivi.mandatory.iviStatus,int);

       decodedData.setIvimMandatory (mandatory);

       bool optional_ok;
       asn1cpp::getSeqOpt(decoded_ivim->ivi.optional,IviContainer,&optional_ok);
       if(optional_ok)
         {
           iviData::iviDataOptional optionalData;
           int optional_size = asn1cpp::sequenceof::getSize(decoded_ivim->ivi.optional);
           for(int i=0;i<optional_size;i++)
           {
             auto optional_seq = asn1cpp::sequenceof::getSeq(decoded_ivim->ivi.optional,IviContainer,i);
             IviContainer_PR type = asn1cpp::getField(optional_seq->present,IviContainer_PR);
             switch (type)
             {
               case IviContainer_PR_NOTHING:
                   break;
               case IviContainer_PR_glc:
                 {
                   iviData::IVI_glc_t glcData;
                   auto glc = asn1cpp::getSeq(optional_seq->choice.glc,GeographicLocationContainer);
                   glcData.referencePosition.altitude.setValue (asn1cpp::getField(glc->referencePosition.altitude.altitudeValue,long));
                   glcData.referencePosition.altitude.setConfidence (asn1cpp::getField(glc->referencePosition.altitude.altitudeConfidence,long));
                   glcData.referencePosition.longitude = asn1cpp::getField(glc->referencePosition.longitude,long);
                   glcData.referencePosition.latitude = asn1cpp::getField(glc->referencePosition.latitude,long);
                   glcData.referencePosition.positionConfidenceEllipse.semiMajorConfidence = asn1cpp::getField(glc->referencePosition.positionConfidenceEllipse.semiMajorConfidence,long);
                   glcData.referencePosition.positionConfidenceEllipse.semiMinorConfidence = asn1cpp::getField(glc->referencePosition.positionConfidenceEllipse.semiMinorConfidence,long);
                   glcData.referencePosition.positionConfidenceEllipse.semiMajorOrientation = asn1cpp::getField(glc->referencePosition.positionConfidenceEllipse.semiMajorOrientation,long);

                   int glcPartSize = asn1cpp::sequenceof::getSize(glc->parts);
                   for (int j=0;j<glcPartSize;j++) {

                       bool ok;
                       auto glcPart = asn1cpp::sequenceof::getSeq(glc->parts,GlcPart,j);
                       iviData::IVI_glc_part glcPartData;
                       glcPartData.zoneId = asn1cpp::getField(glcPart->zoneId,long);

                       auto laneNumber = asn1cpp::getField(glcPart->laneNumber,long,&ok);
                       if(ok)
                         glcPartData.laneNumber.setData (laneNumber);

                       auto zoneExtension = asn1cpp::getField(glcPart->zoneExtension,long,&ok);
                       if(ok)
                         glcPartData.zoneExtension.setData (zoneExtension);

                       auto zoneHeading = asn1cpp::getField(glcPart->zoneHeading,long,&ok);
                       if(ok)
                         glcPartData.zoneHeading.setData (zoneHeading);


                       Zone_PR type = asn1cpp::getField(glcPart->zone->present,Zone_PR);
                       switch (type)
                         {
                         case Zone_PR_NOTHING:
                           break;
                         case Zone_PR_area:
                           {
                             iviData::IVI_glcP_zone zone;
                             zone.area.setData (getPolyLine (asn1cpp::getSeq(glcPart->zone->choice.area,PolygonalLine)));
                             glcPartData.zone.setData (zone);
                             break;
                           }
                         case Zone_PR_segment:
                           {
                             iviData::IVI_glcP_zone_t zone;
                             iviData::IVI_glcP_segment_t segment;
                             bool lw_ok;
                             segment.line = getPolyLine (asn1cpp::getSeq(glcPart->zone->choice.segment.line,PolygonalLine));
                             auto laneW = asn1cpp::getField(glcPart->zone->choice.segment.laneWidth,long,&lw_ok);
                             if(lw_ok)
                               segment.laneWidth.setData (laneW);
                             zone.segment.setData (segment);
                             glcPartData.zone.setData (zone);
                             break;
                           }
                         case Zone_PR_computedSegment:
                           {
                             iviData::IVI_glcP_zone_t zone;
                             iviData::IVI_glcP_cSegment_t cSegData;
                             bool off_ok;
                             auto cSegment = asn1cpp::getSeq(glcPart->zone->choice.computedSegment,ComputedSegment);
                             cSegData.zoneID = asn1cpp::getField(cSegment->zoneId,long);
                             cSegData.laneNumber = asn1cpp::getField(cSegment->laneNumber,long);
                             cSegData.laneWidth = asn1cpp::getField(cSegment->laneWidth,long);
                             auto offsetD = asn1cpp::getField(cSegment->offsetDistance,long,&off_ok);
                             if(off_ok)
                               cSegData.offsetDist.setData (offsetD);
                             auto offsetP = asn1cpp::getSeq(cSegment->offsetPosition,DeltaReferencePosition,&off_ok);
                             if(off_ok)
                               {
                                 iviData::IVI_glcP_deltaPositionWA_t deltaPos;
                                 deltaPos.deltaLat  = asn1cpp::getField(cSegment->offsetPosition->deltaLatitude,long);
                                 deltaPos.deltaLong = asn1cpp::getField(cSegment->offsetPosition->deltaLongitude,long);
                                 deltaPos.deltaAltitude = asn1cpp::getField(cSegment->offsetPosition->deltaAltitude,long);
                                 cSegData.offsetPosition.setData (deltaPos);
                               }
                             zone.computedSegment.setData (cSegData);
                             glcPartData.zone.setData (zone);
                             break;
                           }
                         default:
                           {
                             std::cout << "Unable to decode GLC-part zone " << std::endl;
                             break;
                           }
                         }
                       glcData.GlcPart.push_back (glcPartData);
                       optionalData.glc.setData (glcData);
                     }
                   break;
                 }
               case IviContainer_PR_giv:
                 {
                   iviData::IVI_gic_t gicData;
                   auto gic = asn1cpp::getSeq(optional_seq->choice.giv,GeneralIviContainer);
                   int gicPartSize = asn1cpp::sequenceof::getSize(optional_seq->choice.giv);
                   for (int j=0;j<gicPartSize;j++) {
                       bool ok;
                       iviData::IVI_gic_part_t gicPartData;
                       auto gicPart = asn1cpp::sequenceof::getSeq(*gic,GicPart,j);

                       auto detectionZoneIds = asn1cpp::getSeq(gicPart->detectionZoneIds,Zid,&ok);
                       if(ok)
                         {
                           int detZonSize = asn1cpp::sequenceof::getSize(gicPart->detectionZoneIds);
                           std::vector<long> detZoneIDs;
                           for (int i = 0;i<detZonSize;i++) {
                               //Zid_t detzoneid = asn1cpp::setof::getterField<Zid_t>(*detectionZoneIds,i);
                               detZoneIDs.push_back (asn1cpp::sequenceof::getField(gicPart->detectionZoneIds,long,i));
                             }
                           gicPartData.detectionZoneIds.setData (detZoneIDs);
                         }

                       auto relevanceZoneIds = asn1cpp::getSeq(gicPart->relevanceZoneIds,Zid,&ok);

                       if(ok)
                         {
                           int relZonSize = asn1cpp::sequenceof::getSize(gicPart->relevanceZoneIds);
                           std::vector<long> relZoneIDs;
                           for (int i = 0;i<relZonSize;i++) {
                               //Zid_t detzoneid = asn1cpp::setof::getterField<Zid_t>(*detectionZoneIds,i);
                               relZoneIDs.push_back (asn1cpp::sequenceof::getField(gicPart->relevanceZoneIds,long,i));
                             }
                           gicPartData.relevanceZoneIds.setData (relZoneIDs);
                         }

                       gicPartData.direction = asn1cpp::getField (gicPart->direction,long);
                       gicPartData.iviType = asn1cpp::getField (gicPart->iviType,long);

                       int RSsize = asn1cpp::sequenceof::getSize(gicPart->roadSignCodes);
                       for (int i=0;i<RSsize;i++) {
                           iviData::gicPartRS_t RSdata;
                           auto RSs = asn1cpp::sequenceof::getSeq(gicPart->roadSignCodes,RSCode,i);
                           RSCode__code_PR rsType = asn1cpp::getField(RSs->code.present,RSCode__code_PR);

                           switch (rsType)
                             {
                           case RSCode__code_PR_iso14823: {
                             ISO14823Code__pictogramCode__serviceCategoryCode_PR sccType =
                                 asn1cpp::getField(RSs->code.choice.iso14823.pictogramCode.serviceCategoryCode.present,
                                                   ISO14823Code__pictogramCode__serviceCategoryCode_PR);
                             switch (sccType)
                               {
                               case ISO14823Code__pictogramCode__serviceCategoryCode_PR_trafficSignPictogram:
                                 {
                                  RSdata.RS_trafficSignPictogram.setData (asn1cpp::getField(RSs->code.choice.iso14823.pictogramCode.serviceCategoryCode.choice.trafficSignPictogram,
                                                       long));
                                  RSdata.RS_serialNumber =
                                     asn1cpp::getField(RSs->code.choice.iso14823.pictogramCode.pictogramCategoryCode.serialNumber,
                                                   long);
                                  RSdata.RS_nature =
                                     asn1cpp::getField(RSs->code.choice.iso14823.pictogramCode.pictogramCategoryCode.nature,
                                                   long);


                                  auto attributes = asn1cpp::getSeq(RSs->code.choice.iso14823.attributes,ISO14823Attributes);
                                  int atSize = asn1cpp::sequenceof::getSize(RSs->code.choice.iso14823.attributes);
                                  for (int i = 0;i<atSize;i++) {
                                       auto att = asn1cpp::sequenceof::getSeq(RSs->code.choice.iso14823.attributes,ISO14823Attribute,i);
                                       if(asn1cpp::getField(att->present,ISO14823Attribute_PR) == ISO14823Attribute_PR_spe)
                                         {
                                           RSdata.RS_spm.setData (asn1cpp::getField(att->choice.spe.spm,long));
                                           RSdata.RS_unit.setData (asn1cpp::getField(att->choice.spe.unit,long));
                                         }
                                    }
                                 break;
                                 }
                               case ISO14823Code__pictogramCode__serviceCategoryCode_PR_NOTHING:
                                 //To be implemented
                                 break;
                               case ISO14823Code__pictogramCode__serviceCategoryCode_PR_publicFacilitiesPictogram:
                                 //To be implemented
                                 break;
                               case ISO14823Code__pictogramCode__serviceCategoryCode_PR_ambientOrRoadConditionPictogram:
                                 //To be implemented
                                 break;
                               }
                             break;
                             }
                           case RSCode__code_PR_NOTHING:
                               //To be implemented
                               break;
                           case RSCode__code_PR_itisCodes:
                               //To be implemented
                               break;
                           case RSCode__code_PR_anyCatalogue:
                               //To be implemented
                               break;
                           case RSCode__code_PR_viennaConvention:
                               //To be implemented
                               break;
                             }
                          gicPartData.RS.push_back (RSdata);
                         }
                       gicData.GicPart.push_back (gicPartData);
                     }
                   optionalData.gic.setData (gicData);
                   break;
                 }
               case IviContainer_PR_rcc:
                 {
                   iviData::IVI_rcc_t rccData;
                   auto rcc = asn1cpp::getSeq(optional_seq->choice.rcc,RoadConfigurationContainer);
                   int rccPartSize = asn1cpp::sequenceof::getSize(optional_seq->choice.rcc);
                   for (int j=0;j<rccPartSize;j++) {
                       iviData::IVI_rcc_part_t rccPartData;
                       auto rccPart = asn1cpp::sequenceof::getSeq(*rcc,RccPart,j);
                       int zoneIDsize = asn1cpp::sequenceof::getSize(rccPart->zoneIds);
                       for (int i=0;i<zoneIDsize;i++)
                          rccPartData.zoneID.push_back (asn1cpp::sequenceof::getField(rccPart->zoneIds,long,i));

                       rccPartData.roadType = asn1cpp::getField(rccPart->roadType,long);
                       int laneCsize = asn1cpp::sequenceof::getSize(rccPart->laneConfiguration);
                       for (int i=0;i<laneCsize;i++)
                         {
                           auto laneConfig = asn1cpp::sequenceof::getSeq(rccPart->laneConfiguration,LaneInformation,i);
                           iviData::laneInfo_t laneInfo;
                           laneInfo.laneNumber = asn1cpp::getField(laneConfig->laneNumber,long);
                           laneInfo.direction = asn1cpp::getField(laneConfig->direction,long);
                           laneInfo.laneType = asn1cpp::getField(laneConfig->laneType,long);
                           laneInfo.laneStatus = asn1cpp::getField(laneConfig->laneStatus,long);
                           rccPartData.laneInformation.push_back (laneInfo);
                         }
                       rccData.IVIrccPart.push_back (rccPartData);
                     }
                   optionalData.rcc.setData (rccData);
                   break;
                 }
               case IviContainer_PR_tc:
                 {
                   iviData::IVI_tc_t tcData;
                   auto tc = asn1cpp::getSeq(optional_seq->choice.tc,TextContainer);
                   int tc_size = asn1cpp::sequenceof::getSize(optional_seq->choice.tc);
                  for (int j=0;j<tc_size;j++){
                      iviData::IVI_tc_part_t tcPartData;
                      bool ok;
                      auto tcPart = asn1cpp::sequenceof::getSeq(*tc,TcPart,j);

                      auto detectionZoneIds = asn1cpp::getSeq(tcPart->detectionZoneIds,Zid,&ok);
                      if(ok)
                        {
                          int detZonSize = asn1cpp::sequenceof::getSize(tcPart->detectionZoneIds);
                          std::vector<long> detZoneIDs;
                          for (int i = 0;i<detZonSize;i++) {
                              //Zid_t detzoneid = asn1cpp::setof::getterField<Zid_t>(*detectionZoneIds,i);
                              detZoneIDs.push_back (asn1cpp::sequenceof::getField(tcPart->detectionZoneIds,long,i));
                            }
                          tcPartData.detectionZoneIds.setData (detZoneIDs);
                        }
                      auto releveanceZoneIds = asn1cpp::getSeq(tcPart->relevanceZoneIds,Zid,&ok);
                      if(ok)
                        {
                          int relZonSize = asn1cpp::sequenceof::getSize(tcPart->relevanceZoneIds);
                          std::vector<long> relZoneIDs;
                          for (int i = 0;i<relZonSize;i++) {
                              //Zid_t detzoneid = asn1cpp::setof::getterField<Zid_t>(*detectionZoneIds,i);
                              tcPartData.relevanceZoneIds.push_back (asn1cpp::sequenceof::getField(tcPart->relevanceZoneIds,long,i));
                            }
                        }

                      auto direction = asn1cpp::getField(tcPart->direction,long,&ok);
                      if(ok)
                        tcPartData.direction.setData(direction);

                      auto driverAwZoneIds = asn1cpp::getSeq(tcPart->driverAwarenessZoneIds,Zid,&ok);
                      if(ok)
                        {
                          int dAZonSize = asn1cpp::sequenceof::getSize(tcPart->driverAwarenessZoneIds);
                          std::vector<long> dAZoneIDs;
                          for (int i = 0;i<dAZonSize;i++) {
                              //Zid_t detzoneid = asn1cpp::setof::getterField<Zid_t>(*detectionZoneIds,i);
                              dAZoneIDs.push_back (asn1cpp::sequenceof::getField(tcPart->driverAwarenessZoneIds,long,i));
                            }
                          tcPartData.driverAwarenessZoneIds.setData (dAZoneIDs);
                        }

                      auto minAT = asn1cpp::getField(tcPart->minimumAwarenessTime,long,&ok);
                      if(ok)
                        tcPartData.minimumAwarenessTime.setData(minAT);

                      auto appLanes = asn1cpp::getSeq(tcPart->applicableLanes,LanePosition,&ok);
                      if(ok)
                        {
                          int appLanesSize = asn1cpp::sequenceof::getSize(tcPart->applicableLanes);
                          std::vector<long> appL;
                          for (int i = 0;i<appLanesSize;i++) {
                              //Zid_t detzoneid = asn1cpp::setof::getterField<Zid_t>(*detectionZoneIds,i);
                              appL.push_back (asn1cpp::sequenceof::getField(tcPart->applicableLanes,long,i));
                            }
                          tcPartData.applicableLanesPos.setData (appL);
                        }

                      auto layoutID = asn1cpp::getField(tcPart->layoutId,long,&ok);
                      if(ok)
                        tcPartData.layoutId.setData(layoutID);

                      auto playoutID = asn1cpp::getField(tcPart->preStoredlayoutId,long,&ok);
                      if(ok)
                        tcPartData.preStoredlayoutId.setData(playoutID);

                      auto text = asn1cpp::getSeq(tcPart->text,Text,&ok);
                      if(ok)
                        {
                          int textSize = asn1cpp::sequenceof::getSize(tcPart->text);
                          std::vector<iviData::IVI_tc_text_t> textContents;
                          for(int i = 0;i<textSize;i++)
                            {
                              iviData::IVI_tc_text_t textData;
                              auto texti = asn1cpp::sequenceof::getSeq(tcPart->text,Text,i);
                              textData.bitLanguage = asn1cpp::bitstring::getterByteMask (texti->language,0);
                              textData.textCont = asn1cpp::getField(texti->textContent,std::string);
                              auto layoutID = asn1cpp::getField(texti->layoutComponentId,long,&ok);
                              if(ok)
                                textData.layoutComponentId.setData(layoutID);
                              textContents.push_back (textData);
                            }
                          tcPartData.text.setData (textContents);
                        }

                      std::string data = asn1cpp::getField(tcPart->data, std::string);

                      tcData.IVItcPart.push_back (tcPartData);
                    }

                   optionalData.tc.setData (tcData);
                   break;
                 }
               case IviContainer_PR_lac:
                 {
                   iviData::IVI_lac_t lacData;
                   bool ok;
                   auto lac = asn1cpp::getSeq(optional_seq->choice.lac,LayoutContainer);
                   lacData.layoutId = asn1cpp::getField(lac->layoutId,long);
                   auto height = asn1cpp::getField(lac->height,long,&ok);
                   if(ok)
                     lacData.height.setData(height);
                   auto width = asn1cpp::getField(lac->width,long,&ok);
                   if(ok)
                     lacData.width.setData(width);

                   int lacSize = asn1cpp::sequenceof::getSize(lac->layoutComponents);
                   for (int j=0;j<lacSize;j++){
                       iviData::IVI_lac_comp_t lacCompData;
                       auto layoutComponent = asn1cpp::sequenceof::getSeq(lac->layoutComponents,LayoutComponent,j);
                       lacCompData.x = asn1cpp::getField(layoutComponent->x,long);
                       lacCompData.y = asn1cpp::getField(layoutComponent->y,long);
                       lacCompData.width = asn1cpp::getField(layoutComponent->width ,long);
                       lacCompData.height = asn1cpp::getField(layoutComponent->height,long);
                       lacCompData.layoutComponentId = asn1cpp::getField(layoutComponent->layoutComponentId,long);

                       lacData.IVIlacComp.push_back (lacCompData);
                     }
                   optionalData.lac.setData (lacData);
                   break;
                 }
             }

           }
           decodedData.setOptional (optionalData);
         }

       m_IVIReceiveCallback(decodedData,from);


  }


  /* This cleanup function will attemp to stop any possibly still-running timer */
  void
  IVIBasicService::cleanup(void)
  {

  }

}
