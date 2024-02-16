#ifndef DENDATA_H
#define DENDATA_H

#include "asn_utils.h"
#include "ns3/DENM.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/BitString.hpp"
#include "ns3/AdverseWeatherCondition-AdhesionSubCauseCode.h"
#include "ns3/AdverseWeatherCondition-ExtremeWeatherConditionSubCauseCode.h"
#include "ns3/AdverseWeatherCondition-PrecipitationSubCauseCode.h"
#include "ns3/AdverseWeatherCondition-VisibilitySubCauseCode.h"
#include <cstring>

typedef struct CauseCode sCauseCode_t;
typedef struct EventHistory sEventHistory_t;

typedef struct Speed sSpeed_t;
typedef struct Heading sHeading_t;

typedef struct ImpactReductionContainer sImpactReductionContainer_t;
typedef struct RoadWorksContainerExtended sRoadWorksContainerExtended_t;
typedef struct StationaryVehicleContainer sStationaryVehicleContainer_t;

#define DEN_DEFAULT_VALIDITY_S 600
namespace ns3 {

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
    unsigned long stationID;
  } denDataHeader;

  typedef struct _management {
    //ActionID_t actionID;
    unsigned long stationID;
    long sequenceNumber;
    long detectionTime;
    long referenceTime;
    DENDataItem<long> termination;
    //ReferencePosition_t eventPosition;
    long longitude;
    long latitude;
    DENValueConfidence<long,long> altitude;
    DEN_PosConfidenceEllipse_t posConfidenceEllipse;
    DENDataItem<long> relevanceDistance;
    DENDataItem<long> relevanceTrafficDirection;
    DENDataItem<long> validityDuration;
    DENDataItem<long> transmissionInterval;
    // StationType_t stationType; // Defined during the creation of the DEN Basic service object
  } denDataManagement;

  typedef struct _situation {
      long informationQuality;
      long causeCode;
      long subCauseCode;
      DENDataItem<long> linkedCauseCode;
      DENDataItem<long> linkedSubCauseCode;
      DENDataItem<std::vector<DEN_EventPoint_t>> eventHistory;
  } denDataSituation;

  typedef struct _location {
      DENDataItem<DENValueConfidence<long,long>> eventSpeed;
      DENDataItem<DENValueConfidence<long,long>> eventPositionHeading;
      std::vector<std::vector<DEN_PathPoint_t>> traces;
      DENDataItem<long> roadType;
  } denDataLocation;

  typedef struct _alacarte {
      DENDataItem<long> lanePosition;
      DENDataItem<DEN_ImpactReductionContainer_t> impactReduction;
      DENDataItem<long> externalTemperature;
      DENDataItem<DEN_RoadWorksContainerExtended_t> roadWorks;
      DENDataItem<long> positioningSolution;
      DENDataItem<DEN_StationaryVehicleContainer_t> stationaryVehicle;
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
  void setDenmHeader(long messageID, long protocolVersion, unsigned long stationID);
  void setDenmMessageID(long messageID){m_header.messageID=messageID;}
  void setDenmProtocolVersion(long protocolVersion){m_header.protocolVersion=protocolVersion;}
  void setDenmStationID(unsigned long stationID){m_header.stationID=stationID;}

  /* AppDENM_update mandatory setters */
  /* AppDENM_terminate mandatory setters */
  void setDenmMandatoryFields(unsigned long originatingStationID, long sequenceNumber, long detectionTime_ms, double latReference_deg, double longReference_deg);
  void setDenmMandatoryFields(unsigned long originatingStationID, long sequenceNumber, long detectionTime_ms, double latReference_deg, double longReference_deg, double altitude_m);
  void setDenmMandatoryFields_asn_types(ActionID_t actionID, TimestampIts_t detectionTime, ReferencePosition_t eventPosition);

  /* receiveDENM setters */
  void setDenmActionID(DEN_ActionID_t actionID);

  /* Optional information setters */

  /* Header getters */
  long getDenmHeaderMessageID() {return m_header.messageID;}
  long getDenmHeaderProtocolVersion() {return m_header.protocolVersion;}
  long getDenmHeaderStationID() {return m_header.stationID;}

  /* Container getters */
  denDataHeader getDenmHeader_asn_types() {return m_header;}
  denDataManagement getDenmMgmtData_asn_types() {return m_management;}
  DENDataItem<denDataSituation> getDenmSituationData_asn_types() {return m_situation;}
  DENDataItem<denDataLocation> getDenmLocationData_asn_types() {return m_location;}
  DENDataItem<denDataAlacarte> getDenmAlacarteData_asn_types() {return m_alacarte;}

  bool isDenmSituationDataSet() {return m_situation.isAvailable ();}
  bool isDenmLocationDataSet() {return m_location.isAvailable ();}
  bool isDenmAlacarteDataSet() {return m_alacarte.isAvailable ();}

  long getDenmMgmtDetectionTime() {return m_management.detectionTime;}

  long getDenmMgmtValidityDuration() {if(m_management.validityDuration.isAvailable())return m_management.validityDuration.getData ();
                                     else return DEN_DEFAULT_VALIDITY_S;}

  long getDenmMgmtReferenceTime() {return m_management.validityDuration.getData ();}

  long getDenmMgmtLatitude() {return m_management.latitude;}
  long getDenmMgmtLongitude() {return m_management.longitude;}
  long getDenmMgmtAltitude() {return m_management.altitude.getValue ();}

  ActionID_t getDenmActionID();

  /* Internals setters */
  // Units are milliseconds
  void setDenmRepetition(uint32_t repetitionDuration,uint32_t repetitionInterval){m_internals.repetitionInterval=repetitionInterval; m_internals.repetitionDuration = repetitionDuration;}
  void setDenmRepetitionInterval(uint32_t repetitionInterval){m_internals.repetitionInterval=repetitionInterval;}
  void setDenmRepetitionDuration(uint32_t repetitionDuration){m_internals.repetitionDuration=repetitionDuration;}
  int setValidityDuration(long validityDuration_s);

  /* Internal getters */
  uint32_t getDenmRepetitionDuration() { return m_internals.repetitionDuration; }
  uint32_t getDenmRepetitionInterval() { return m_internals.repetitionInterval; }

  /* Container setters */
  void setDenmMgmtData_asn_types(denDataManagement management) {m_management = management;}
  void setDenmSituationData_asn_types(denDataSituation situation) {m_situation.setData (situation);}
  void setDenmLocationData_asn_types(denDataLocation location) {m_location.setData (location);}
  void setDenmAlacarteData_asn_types(denDataAlacarte alacarte) {m_alacarte.setData(alacarte);}

  /* Object integrity check */
  bool isDenDataRight();

  void denDataFree();

private:
  INTEGER_t asnTimeConvert(long time);

  denDataInternals m_internals;
  denDataHeader m_header;
  denDataManagement m_management;

  DENDataItem<denDataSituation> m_situation;

  DENDataItem<denDataLocation> m_location;

  DENDataItem<denDataAlacarte> m_alacarte;
};
}
#endif // DENDATA_H
