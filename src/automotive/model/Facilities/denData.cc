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

#include "denData.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/Setter.hpp"
#include "ns3/Encoding.hpp"
#include "ns3/SetOf.hpp"
#include "ns3/SequenceOf.hpp"


namespace ns3 {

denData::denData()
{
  denDataAlacarte alacarte = {};
  // m_management optional fields initialization
  m_management.termination = DENDataItem<long>(false);
  m_management.relevanceDistance = DENDataItem<long>(false);
  m_management.relevanceTrafficDirection = DENDataItem<long>(false);
  m_management.validityDuration = DENDataItem<long>(false);
  m_management.transmissionInterval = DENDataItem<long>(false);

  // m_situation optional fields initialization
  m_situation = DENDataItem<denDataSituation>();

  // m_location optional fields initialization
  m_location = DENDataItem<denDataLocation>();


  // m_alacarte optional fields initialization
  m_alacarte = DENDataItem<denDataAlacarte>();

  // Set m_internals isMandatorySet to false during the object creation
  m_internals.isMandatorySet=false;
  // Initialize the repetition parameters to 0
  m_internals.repetitionDuration=0;
  m_internals.repetitionInterval=0;
}

ActionID_t
denData::getDenmActionID()
{
  ActionID retval;
  retval.originatingStationId = m_management.stationID;
  retval.sequenceNumber = m_management.sequenceNumber;
  return retval;
}
void
denData::setDenmMandatoryFields (long detectionTime_ms, double latReference_deg, double longReference_deg)
{
  m_management.detectionTime = detectionTime_ms;
  m_management.latitude = (long) latReference_deg;
  m_management.longitude = (long) longReference_deg;
  m_management.altitude.setValue (AltitudeValue_unavailable);
  m_management.altitude.setConfidence (AltitudeConfidence_unavailable);
  m_management.posConfidenceEllipse.semiMajorConfidence = SemiAxisLength_unavailable;
  m_management.posConfidenceEllipse.semiMinorConfidence = SemiAxisLength_unavailable;
  m_management.posConfidenceEllipse.semiMajorOrientation = HeadingValue_unavailable;

  m_internals.isMandatorySet=true;
}

void
denData::setDenmMandatoryFields (long detectionTime_ms, double latReference_deg, double longReference_deg, double altitude_m)
{

  setDenmMandatoryFields (detectionTime_ms,latReference_deg,longReference_deg);
  m_management.altitude.setValue (altitude_m);
}

void
denData::setDenmMandatoryFields_asn_types (TimestampIts_t detectionTime, ReferencePosition_t eventPosition)
{
  asn_INTEGER2long (&detectionTime,&m_management.detectionTime);
  m_management.longitude = (long) eventPosition.longitude;
  m_management.latitude = (long) eventPosition.latitude;
  m_management.posConfidenceEllipse.semiMajorConfidence = eventPosition.positionConfidenceEllipse.semiMajorConfidence;
  m_management.posConfidenceEllipse.semiMinorConfidence = eventPosition.positionConfidenceEllipse.semiMinorConfidence;
  m_management.posConfidenceEllipse.semiMajorOrientation = eventPosition.positionConfidenceEllipse.semiMajorOrientation;
  m_management.altitude.setValue (eventPosition.altitude.altitudeValue);
  m_management.altitude.setConfidence (eventPosition.altitude.altitudeConfidence);

  m_internals.isMandatorySet=true;
}

void
denData::setDenmMandatoryFields (unsigned long originatingStationID, long sequenceNumber, long detectionTime_ms, double latReference_deg, double longReference_deg)
{
  setDenmMandatoryFields (detectionTime_ms,latReference_deg,longReference_deg);
  m_management.stationID = originatingStationID;
  m_management.sequenceNumber = sequenceNumber;
}

void
denData::setDenmMandatoryFields (unsigned long originatingStationID, long sequenceNumber, long detectionTime_ms, double latReference_deg, double longReference_deg, double altitude_m)
{
  setDenmMandatoryFields (detectionTime_ms,latReference_deg,longReference_deg, altitude_m);
  m_management.stationID = originatingStationID;
  m_management.sequenceNumber = sequenceNumber;

}

void
denData::setDenmMandatoryFields_asn_types (ActionID_t actionID, TimestampIts_t detectionTime, ReferencePosition_t eventPosition)
{
  setDenmMandatoryFields_asn_types(detectionTime,eventPosition);
  m_management.stationID = (long) actionID.originatingStationId;
  m_management.sequenceNumber = (long) actionID.sequenceNumber;
}

void
denData::setDenmHeader(long messageID, long protocolVersion, unsigned long stationID)
{
  m_header.messageID=messageID;
  m_header.protocolVersion=protocolVersion;
  m_header.stationID=stationID;
}

void
denData::setDenmActionID(DEN_ActionID_t actionID)
{
  m_management.stationID = actionID.originatingStationID;
  m_management.sequenceNumber = actionID.sequenceNumber;
}

int
denData::setValidityDuration (long validityDuration_s)
{

  if(validityDuration_s<0 || validityDuration_s>86400)
    {
      return -2;
    }

  m_management.validityDuration = DENDataItem<long> (validityDuration_s);

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

}
