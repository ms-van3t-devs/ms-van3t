#include "denData.h"

denData::denData()
{
  // m_management optional fields initialization
  m_management.validityDuration=NULL;
  m_management.transmissionInterval=NULL;
  m_management.termination=NULL;
  m_management.relevanceDistance=NULL;
  m_management.relevanceTrafficDirection=NULL;

  memset(&m_management.eventPosition,0,sizeof(ReferencePosition_t));
  memset(&m_management.detectionTime,0,sizeof(TimestampIts_t));
  memset(&m_management.actionID,0,sizeof(ActionID_t));
  memset(&m_management.referenceTime,0,sizeof(TimestampIts_t));

  // m_situation optional fields initialization
  m_situation.linkedCause=NULL;
  m_situation.eventHistory=NULL;

  memset(&m_situation.informationQuality,0,sizeof(InformationQuality_t));
  memset(&m_situation.eventType,0,sizeof(CauseCode_t));

  // m_location optional fields initialization
  m_location.eventSpeed=NULL;
  m_location.eventPositionHeading=NULL;
  m_location.roadType=NULL;

  memset(&m_location.traces,0,sizeof(Traces_t));

  // m_alacarte optional fields initialization
  m_alacarte.lanePosition=NULL;
  m_alacarte.impactReduction=NULL;
  m_alacarte.externalTemperature=NULL;
  m_alacarte.roadWorks=NULL;
  m_alacarte.positioningSolution=NULL;
  m_alacarte.stationaryVehicle=NULL;

  // Set m_internals isMandatorySet to false during the object creation
  m_internals.isMandatorySet=false;

  // Initialize the repetition parameters to 0
  m_internals.repetitionDuration=0;
  m_internals.repetitionInterval=0;
}

void
denData::setDenmMandatoryFields (long detectionTime_ms, double latReference_deg, double longReference_deg)
{
  m_management.detectionTime = asnTimeConvert(detectionTime_ms);
  m_management.eventPosition.latitude = (Latitude_t) (latReference_deg);
  m_management.eventPosition.longitude = (Longitude_t) (longReference_deg);
  m_management.eventPosition.altitude.altitudeValue = AltitudeValue_unavailable;
  m_management.eventPosition.altitude.altitudeConfidence = AltitudeConfidence_unavailable;

  m_internals.isMandatorySet=true;
}

void
denData::setDenmMandatoryFields (long detectionTime_ms, double latReference_deg, double longReference_deg, double altitude_m)
{
  setDenmMandatoryFields (detectionTime_ms,latReference_deg,longReference_deg);
  m_management.eventPosition.altitude.altitudeValue = altitude_m;
}

void
denData::setDenmMandatoryFields_asn_types (TimestampIts_t detectionTime, ReferencePosition_t eventPosition)
{
  m_management.detectionTime = detectionTime;
  m_management.eventPosition = eventPosition;

  m_internals.isMandatorySet=true;
}

void
denData::setDenmMandatoryFields (long originatingStationID, long sequenceNumber, long detectionTime_ms, double latReference_deg, double longReference_deg)
{
  setDenmMandatoryFields (detectionTime_ms,latReference_deg,longReference_deg);
  m_management.actionID.originatingStationID = (StationID_t) originatingStationID;
  m_management.actionID.sequenceNumber = (SequenceNumber_t) sequenceNumber;
}

void
denData::setDenmMandatoryFields (long originatingStationID, long sequenceNumber, long detectionTime_ms, double latReference_deg, double longReference_deg, double altitude_m)
{
  setDenmMandatoryFields (detectionTime_ms,latReference_deg,longReference_deg, altitude_m);
  m_management.actionID.originatingStationID = (StationID_t) originatingStationID;
  m_management.actionID.sequenceNumber = (SequenceNumber_t) sequenceNumber;

}

void
denData::setDenmMandatoryFields_asn_types (ActionID_t actionID, TimestampIts_t detectionTime, ReferencePosition_t eventPosition)
{
  setDenmMandatoryFields_asn_types(detectionTime,eventPosition);
  m_management.actionID = actionID;
}

int
denData::setValidityDuration (long validityDuration_s)
{
  if(m_management.validityDuration) free(m_management.validityDuration);

  if(validityDuration_s<0 || validityDuration_s>86400)
    {
      return -2;
    }

  m_management.validityDuration=(ValidityDuration_t*) calloc(1, sizeof(ValidityDuration_t));

  if(m_management.validityDuration==NULL)
    {
      return -1;
    }

  *m_management.validityDuration=validityDuration_s;

  return 1;
}

INTEGER_t
denData::asnTimeConvert (long time)
{
  INTEGER_t value;
  memset(&value,0,sizeof(value));
  asn_long2INTEGER (&value, time);
  return value;
}

/* Integrity check method */
bool
denData::isDenDataRight()
{
  if(m_internals.isMandatorySet==false)
    {
      return false;
    }
  return true;
}

void
denData::denDataFree()
{
  if(m_management.detectionTime.buf) free(m_management.detectionTime.buf);
  if(m_management.validityDuration) free(m_management.validityDuration);
}
