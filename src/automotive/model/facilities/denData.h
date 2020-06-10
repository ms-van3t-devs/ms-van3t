#ifndef DENDATA_H
#define DENDATA_H

#include "asn_utils.h"
#include "ns3/DENM.h"
#include <cstring>

typedef struct CauseCode sCauseCode_t;
typedef struct EventHistory sEventHistory_t;

typedef struct Speed sSpeed_t;
typedef struct Heading sHeading_t;

typedef struct ImpactReductionContainer sImpactReductionContainer_t;
typedef struct RoadWorksContainerExtended sRoadWorksContainerExtended_t;
typedef struct StationaryVehicleContainer sStationaryVehicleContainer_t;

#define DEN_DEFAULT_VALIDITY_S 600

class denData
{

public:
  typedef struct _internals {
      uint32_t repetitionDuration;
      uint32_t repetitionInterval;
      bool isMandatorySet;
  } denDataInternals;

  typedef struct _header {
    long messageID;
    long protocolVersion;
    StationID_t stationID;
  } denDataHeader;

  typedef struct _management {
      TimestampIts_t detectionTime;
      ReferencePosition_t eventPosition;
      ValidityDuration_t *validityDuration;
      TransmissionInterval_t *transmissionInterval;
      ActionID_t actionID;
      Termination_t *termination;
      RelevanceDistance_t *relevanceDistance;
      RelevanceTrafficDirection_t *relevanceTrafficDirection;
      TimestampIts_t referenceTime;
      // StationType_t stationType; // Defined during the creation of the DEN Basic service object
  } denDataManagement;

  typedef struct _situation {
      InformationQuality_t informationQuality;
      CauseCode_t eventType;
      sCauseCode_t *linkedCause;
      sEventHistory_t *eventHistory;
  } denDataSituation;

  typedef struct _location {
      sSpeed_t *eventSpeed;
      sHeading_t *eventPositionHeading;
      Traces_t traces;
      RoadType_t *roadType;
  } denDataLocation;

  typedef struct _alacarte {
      LanePosition_t *lanePosition;
      sImpactReductionContainer_t *impactReduction;
      Temperature_t *externalTemperature;
      sRoadWorksContainerExtended_t *roadWorks;
      PositioningSolutionType_t *positioningSolution;
      sStationaryVehicleContainer_t *stationaryVehicle;
  } denDataAlacarte;

public:
  denData();

  /* AppDENM_trigger mandatory setters */
  void setDenmMandatoryFields(long detectionTime_ms, double latReference_deg, double longReference_deg);
  void setDenmMandatoryFields(long detectionTime_ms, double latReference_deg, double longReference_deg, double altitude_m);
  void setDenmMandatoryFields_asn_types(TimestampIts_t detectionTime, ReferencePosition_t eventPosition);

  /*
   * Header setters (they can be used for experimentation purposes, but they shall not be called normally, as the header is typically
   * set inside the DEN Basic Service, without the need of a manual user intervention
  */
  void setDenmHeader(long messageID, long protocolVersion, StationID_t stationID) {m_header.messageID=messageID; m_header.protocolVersion=protocolVersion; m_header.stationID=stationID;}
  void setDenmMessageID(long messageID) {m_header.messageID=messageID;}
  void setDenmProtocolVersion(long protocolVersion) {m_header.protocolVersion=protocolVersion;}
  void setDenmStationID(StationID_t stationID) {m_header.stationID=stationID;}

  /* AppDENM_update mandatory setters */
  /* AppDENM_terminate mandatory setters */
  void setDenmMandatoryFields(long originatingStationID, long sequenceNumber, long detectionTime_ms, double latReference_deg, double longReference_deg);
  void setDenmMandatoryFields(long originatingStationID, long sequenceNumber, long detectionTime_ms, double latReference_deg, double longReference_deg, double altitude_m);
  void setDenmMandatoryFields_asn_types(ActionID_t actionID, TimestampIts_t detectionTime, ReferencePosition_t eventPosition);

  /* receiveDENM setters */
  void setDenmActionID(ActionID_t actionID) {m_management.actionID=actionID;}

  /* Optional information setters */

  /* Header getters */
  long getDenmHeaderMessageID() {return m_header.messageID;}
  long getDenmHeaderProtocolVersion() {return m_header.protocolVersion;}
  long getDenmHeaderStationID() {return m_header.stationID;}

  /* Container getters */
  denDataHeader getDenmHeader_asn_types() const{return m_header;}
  denDataManagement getDenmMgmtData_asn_types() const{return m_management;}
  denDataSituation getDenmSituationData_asn_types() const{return m_situation;}
  denDataLocation getDenmLocationData_asn_types() const{return m_location;}
  denDataAlacarte getDenmAlacarteData_asn_types() const{return m_alacarte;}

  bool isDenmSituationDataSet() const{return m_situation_set;}
  bool isDenmLocationDataSet() const{return m_location_set;}
  bool isDenmAlacarteDataSet() const{return m_alacarte_set;}

  long getDenmMgmtDetectionTime() const{long detectionTime=0;
                                        asn_INTEGER2long (&m_management.detectionTime,&detectionTime);
                                        return detectionTime;}

  long getDenmMgmtValidityDuration() const{long validityDuration = m_management.validityDuration!=NULL ? *(m_management.validityDuration) : DEN_DEFAULT_VALIDITY_S;
                                           return validityDuration;}

  long getDenmMgmtReferenceTime() const{long referenceTime=0;
                                        asn_INTEGER2long (&m_management.referenceTime,&referenceTime);
                                        return referenceTime;}

  long getDenmMgmtLatitude() const{return m_management.eventPosition.latitude;}
  long getDenmMgmtLongitude() const{return m_management.eventPosition.longitude;}
  long getDenmMgmtAltitude() const{return m_management.eventPosition.altitude.altitudeValue;}

  long getDenmHeaderMessageID() const{return m_header.messageID;}
  long getDenmHeaderProtocolVersion () const{return m_header.protocolVersion;}
  unsigned long getDenmHeaderStationID () const{return (unsigned long)m_header.stationID;}

  ActionID_t getDenmActionID() const{return m_management.actionID;}

  /* Internals setters */
  void setDenmRepetition(uint32_t repetitionDuration,uint32_t repetitionInterval) {m_internals.repetitionInterval=repetitionInterval; m_internals.repetitionDuration=repetitionDuration;}
  void setDenmRepetitionInterval(uint32_t repetitionInterval) {m_internals.repetitionInterval=repetitionInterval;}
  void setDenmRepetitionDuration(uint32_t repetitionDuration) {m_internals.repetitionDuration=repetitionDuration;}
  int setValidityDuration(long validityDuration_s);

  /* Internal getters */
  uint32_t getDenmRepetitionDuration() { return m_internals.repetitionDuration; }
  uint32_t getDenmRepetitionInterval() { return m_internals.repetitionInterval; }

  /* Container setters */
  void setDenmMgmtData_asn_types(denDataManagement management) {m_management = management;}
  void setDenmSituationData_asn_types(denDataSituation situation) {m_situation = situation; m_situation_set=true;}
  void setDenmLocationData_asn_types(denDataLocation location) {m_location = location; m_location_set=true;}
  void setDenmAlacarteData_asn_types(denDataAlacarte alacarte) {m_alacarte = alacarte; m_alacarte_set=true;}

  /* Object integrity check */
  bool isDenDataRight();

  void denDataFree();

private:
  INTEGER_t asnTimeConvert(long time);

  denDataInternals m_internals;
  denDataHeader m_header;
  denDataManagement m_management;

  bool m_situation_set=false;
  denDataSituation m_situation;

  bool m_location_set=false;
  denDataLocation m_location;

  bool m_alacarte_set=false;
  denDataAlacarte m_alacarte;
};

#endif // DENDATA_H
