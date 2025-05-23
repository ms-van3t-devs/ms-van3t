/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
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
 *
 * Author: Jaume Nin <jnin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 */

#include "cv2x_radio-bearer-stats-calculator.h"
#include "ns3/string.h"
#include "ns3/nstime.h"
#include <ns3/log.h>
#include <vector>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_RadioBearerStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED ( cv2x_RadioBearerStatsCalculator);

cv2x_RadioBearerStatsCalculator::cv2x_RadioBearerStatsCalculator ()
  : m_firstWrite (true),
    m_pendingOutput (false), 
    m_protocolType ("RLC")
{
  NS_LOG_FUNCTION (this);
}

cv2x_RadioBearerStatsCalculator::cv2x_RadioBearerStatsCalculator (std::string protocolType)
  : m_firstWrite (true),
    m_pendingOutput (false)
{
  NS_LOG_FUNCTION (this);
  m_protocolType = protocolType;
}

cv2x_RadioBearerStatsCalculator::~cv2x_RadioBearerStatsCalculator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
cv2x_RadioBearerStatsCalculator::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::cv2x_RadioBearerStatsCalculator")
    .SetParent<cv2x_LteStatsCalculator> ().AddConstructor<cv2x_RadioBearerStatsCalculator> ()
    .SetGroupName("Lte")
    .AddAttribute ("StartTime", "Start time of the on going epoch.", 
                   TimeValue (Seconds (0.)),
                   MakeTimeAccessor (&cv2x_RadioBearerStatsCalculator::SetStartTime,
                                     &cv2x_RadioBearerStatsCalculator::GetStartTime), 
                   MakeTimeChecker ())
    .AddAttribute ("EpochDuration", "Epoch duration.", 
                   TimeValue (Seconds (0.25)), 
                   MakeTimeAccessor (&cv2x_RadioBearerStatsCalculator::GetEpoch,
                                     &cv2x_RadioBearerStatsCalculator::SetEpoch), 
                   MakeTimeChecker ())
    .AddAttribute ("DlRlcOutputFilename",
                   "Name of the file where the downlink results will be saved.",
                   StringValue ("DlRlcStats.txt"),
                   MakeStringAccessor (&cv2x_LteStatsCalculator::SetDlOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("UlRlcOutputFilename",
                   "Name of the file where the uplink results will be saved.",
                   StringValue ("UlRlcStats.txt"),
                   MakeStringAccessor (&cv2x_LteStatsCalculator::SetUlOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("DlPdcpOutputFilename",
                   "Name of the file where the downlink results will be saved.",
                   StringValue ("DlPdcpStats.txt"),
                   MakeStringAccessor (&cv2x_RadioBearerStatsCalculator::SetDlPdcpOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("UlPdcpOutputFilename",
                   "Name of the file where the uplink results will be saved.",
                   StringValue ("UlPdcpStats.txt"),
                   MakeStringAccessor (&cv2x_RadioBearerStatsCalculator::SetUlPdcpOutputFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

void
cv2x_RadioBearerStatsCalculator::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_pendingOutput)
    {
      ShowResults ();
    }
}

void 
cv2x_RadioBearerStatsCalculator::SetStartTime (Time t)
{
  m_startTime = t;
  RescheduleEndEpoch ();
}

Time 
cv2x_RadioBearerStatsCalculator::GetStartTime () const
{
  return m_startTime;
}

void 
cv2x_RadioBearerStatsCalculator::SetEpoch (Time e)
{
  m_epochDuration = e;
  RescheduleEndEpoch ();
}

Time 
cv2x_RadioBearerStatsCalculator::GetEpoch () const
{
  return m_epochDuration;  
}

void
cv2x_RadioBearerStatsCalculator::UlTxPdu (uint16_t cellId, uint64_t imsi, uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  NS_LOG_FUNCTION (this << "UlTxPDU" << cellId << imsi << rnti << (uint32_t) lcid << packetSize);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  if (Simulator::Now () >= m_startTime)
    {
      m_ulCellId[p] = cellId;
      m_flowId[p] = cv2x_LteFlowId_t (rnti, lcid);
      m_ulTxPackets[p]++;
      m_ulTxData[p] += packetSize;
    }
  m_pendingOutput = true;
}

void
cv2x_RadioBearerStatsCalculator::DlTxPdu (uint16_t cellId, uint64_t imsi, uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  NS_LOG_FUNCTION (this << "DlTxPDU" << cellId << imsi << rnti << (uint32_t) lcid << packetSize);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  if (Simulator::Now () >= m_startTime)
    {
      m_dlCellId[p] = cellId;
      m_flowId[p] = cv2x_LteFlowId_t (rnti, lcid);
      m_dlTxPackets[p]++;
      m_dlTxData[p] += packetSize;
    }
  m_pendingOutput = true;
}

void
cv2x_RadioBearerStatsCalculator::UlRxPdu (uint16_t cellId, uint64_t imsi, uint16_t rnti, uint8_t lcid, uint32_t packetSize,
                                     uint64_t delay)
{
  NS_LOG_FUNCTION (this << "UlRxPDU" << cellId << imsi << rnti << (uint32_t) lcid << packetSize << delay);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  if (Simulator::Now () >= m_startTime)
    {
      m_ulCellId[p] = cellId;
      m_ulRxPackets[p]++;
      m_ulRxData[p] += packetSize;

      cv2x_Uint64StatsMap::iterator it = m_ulDelay.find (p);
      if (it == m_ulDelay.end ())
        {
          NS_LOG_DEBUG (this << " Creating UL stats calculators for IMSI " << p.m_imsi << " and LCID " << (uint32_t) p.m_lcId);
          m_ulDelay[p] = CreateObject<MinMaxAvgTotalCalculator<uint64_t> > ();
          m_ulPduSize[p] = CreateObject<MinMaxAvgTotalCalculator<uint32_t> > ();
        }
      m_ulDelay[p]->Update (delay);
      m_ulPduSize[p]->Update (packetSize);
    }
  m_pendingOutput = true;
}

void
cv2x_RadioBearerStatsCalculator::DlRxPdu (uint16_t cellId, uint64_t imsi, uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint64_t delay)
{
  NS_LOG_FUNCTION (this << "DlRxPDU" << cellId << imsi << rnti << (uint32_t) lcid << packetSize << delay);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  if (Simulator::Now () >= m_startTime)
    {
      m_dlCellId[p] = cellId;
      m_dlRxPackets[p]++;
      m_dlRxData[p] += packetSize;

      cv2x_Uint64StatsMap::iterator it = m_dlDelay.find (p);
      if (it == m_dlDelay.end ())
        {
          NS_LOG_DEBUG (this << " Creating DL stats calculators for IMSI " << p.m_imsi << " and LCID " << (uint32_t) p.m_lcId);
          m_dlDelay[p] = CreateObject<MinMaxAvgTotalCalculator<uint64_t> > ();
          m_dlPduSize[p] = CreateObject<MinMaxAvgTotalCalculator<uint32_t> > ();
        }
      m_dlDelay[p]->Update (delay);
      m_dlPduSize[p]->Update (packetSize);
    }
  m_pendingOutput = true;
}

void
cv2x_RadioBearerStatsCalculator::ShowResults (void)
{

  NS_LOG_FUNCTION (this << GetUlOutputFilename ().c_str () << GetDlOutputFilename ().c_str ());
  NS_LOG_INFO ("Write Rlc Stats in " << GetUlOutputFilename ().c_str () << " and in " << GetDlOutputFilename ().c_str ());

  std::ofstream ulOutFile;
  std::ofstream dlOutFile;

  if (m_firstWrite == true)
    {
      ulOutFile.open (GetUlOutputFilename ().c_str ());
      if (!ulOutFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetUlOutputFilename ().c_str ());
          return;
        }

      dlOutFile.open (GetDlOutputFilename ().c_str ());
      if (!dlOutFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetDlOutputFilename ().c_str ());
          return;
        }
      m_firstWrite = false;
      ulOutFile << "% start\tend\tCellId\tIMSI\tRNTI\tLCID\tnTxPDUs\tTxBytes\tnRxPDUs\tRxBytes\t";
      ulOutFile << "delay\tstdDev\tmin\tmax\t";
      ulOutFile << "PduSize\tstdDev\tmin\tmax";
      ulOutFile << std::endl;
      dlOutFile << "% start\tend\tCellId\tIMSI\tRNTI\tLCID\tnTxPDUs\tTxBytes\tnRxPDUs\tRxBytes\t";
      dlOutFile << "delay\tstdDev\tmin\tmax\t";
      dlOutFile << "PduSize\tstdDev\tmin\tmax";
      dlOutFile << std::endl;
    }
  else
    {
      ulOutFile.open (GetUlOutputFilename ().c_str (), std::ios_base::app);
      if (!ulOutFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetUlOutputFilename ().c_str ());
          return;
        }

      dlOutFile.open (GetDlOutputFilename ().c_str (), std::ios_base::app);
      if (!dlOutFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetDlOutputFilename ().c_str ());
          return;
        }
    }

  WriteUlResults (ulOutFile);
  WriteDlResults (dlOutFile);
  m_pendingOutput = false;

}

void
cv2x_RadioBearerStatsCalculator::WriteUlResults (std::ofstream& outFile)
{
  NS_LOG_FUNCTION (this);

  // Get the unique IMSI/LCID pairs list
  std::vector < cv2x_ImsiLcidPair_t > pairVector;
  for (cv2x_Uint32Map::iterator it = m_ulTxPackets.begin (); it != m_ulTxPackets.end (); ++it)
    {
      if (find (pairVector.begin (), pairVector.end (), (*it).first) == pairVector.end ())
        {
          pairVector.push_back ((*it).first);
        }
    }

  for (cv2x_Uint32Map::iterator it = m_ulRxPackets.begin (); it != m_ulRxPackets.end (); ++it)
    {
      if (find (pairVector.begin (), pairVector.end (), (*it).first) == pairVector.end ())
        {
          pairVector.push_back ((*it).first);
        }
    }

  Time endTime = m_startTime + m_epochDuration;
  for (std::vector<cv2x_ImsiLcidPair_t>::iterator it = pairVector.begin (); it != pairVector.end (); ++it)
    {
      cv2x_ImsiLcidPair_t p = *it;
      cv2x_FlowIdMap::const_iterator flowIdIt = m_flowId.find (p);
      // \TODO Temporary workaround until traces are connected correctly in cv2x_LteEnbRrc and cv2x_LteUeRrc
      if (flowIdIt == m_flowId.end ()) continue;
//       NS_ASSERT_MSG (flowIdIt != m_flowId.end (),
//                      "FlowId (imsi " << p.m_imsi << " lcid " << (uint32_t) p.m_lcId << ") is missing");
      cv2x_LteFlowId_t flowId = flowIdIt->second;
      NS_ASSERT_MSG (flowId.m_lcId == p.m_lcId, "lcid mismatch");

      outFile << m_startTime.GetNanoSeconds () / 1.0e9 << "\t";
      outFile << endTime.GetNanoSeconds () / 1.0e9 << "\t";
      outFile << GetUlCellId (p.m_imsi, p.m_lcId) << "\t";
      outFile << p.m_imsi << "\t";
      outFile << flowId.m_rnti << "\t";
      outFile << (uint32_t) flowId.m_lcId << "\t";
      outFile << GetUlTxPackets (p.m_imsi, p.m_lcId) << "\t";
      outFile << GetUlTxData (p.m_imsi, p.m_lcId) << "\t";
      outFile << GetUlRxPackets (p.m_imsi, p.m_lcId) << "\t";
      outFile << GetUlRxData (p.m_imsi, p.m_lcId) << "\t";
      std::vector<double> stats = GetUlDelayStats (p.m_imsi, p.m_lcId);
      for (std::vector<double>::iterator it = stats.begin (); it != stats.end (); ++it)
        {
          outFile << (*it) * 1e-9 << "\t";
        }
      stats = GetUlPduSizeStats (p.m_imsi, p.m_lcId);
      for (std::vector<double>::iterator it = stats.begin (); it != stats.end (); ++it)
        {
          outFile << (*it) << "\t";
        }
      outFile << std::endl;
    }

  outFile.close ();
}

void
cv2x_RadioBearerStatsCalculator::WriteDlResults (std::ofstream& outFile)
{
  NS_LOG_FUNCTION (this);

  // Get the unique IMSI/LCID pairs list
  std::vector < cv2x_ImsiLcidPair_t > pairVector;
  for (cv2x_Uint32Map::iterator it = m_dlTxPackets.begin (); it != m_dlTxPackets.end (); ++it)
    {
      if (find (pairVector.begin (), pairVector.end (), (*it).first) == pairVector.end ())
        {
          pairVector.push_back ((*it).first);
        }
    }

  for (cv2x_Uint32Map::iterator it = m_dlRxPackets.begin (); it != m_dlRxPackets.end (); ++it)
    {
      if (find (pairVector.begin (), pairVector.end (), (*it).first) == pairVector.end ())
        {
          pairVector.push_back ((*it).first);
        }
    }

  Time endTime = m_startTime + m_epochDuration;
  for (std::vector<cv2x_ImsiLcidPair_t>::iterator pair = pairVector.begin (); pair != pairVector.end (); ++pair)
    {
      cv2x_ImsiLcidPair_t p = *pair;
      cv2x_FlowIdMap::const_iterator flowIdIt = m_flowId.find (p);
      // \TODO Temporary workaround until traces are connected correctly in cv2x_LteEnbRrc and cv2x_LteUeRrc
      if (flowIdIt == m_flowId.end ()) continue;
//       NS_ASSERT_MSG (flowIdIt != m_flowId.end (),
//                      "FlowId (imsi " << p.m_imsi << " lcid " << (uint32_t) p.m_lcId << ") is missing");
      cv2x_LteFlowId_t flowId = flowIdIt->second;
      NS_ASSERT_MSG (flowId.m_lcId == p.m_lcId, "lcid mismatch");

      outFile << m_startTime.GetNanoSeconds () / 1.0e9 << "\t";
      outFile << endTime.GetNanoSeconds () / 1.0e9 << "\t";
      outFile << GetDlCellId (p.m_imsi, p.m_lcId) << "\t";
      outFile << p.m_imsi << "\t";
      outFile << flowId.m_rnti << "\t";
      outFile << (uint32_t) flowId.m_lcId << "\t";
      outFile << GetDlTxPackets (p.m_imsi, p.m_lcId) << "\t";
      outFile << GetDlTxData (p.m_imsi, p.m_lcId) << "\t";
      outFile << GetDlRxPackets (p.m_imsi, p.m_lcId) << "\t";
      outFile << GetDlRxData (p.m_imsi, p.m_lcId) << "\t";
      std::vector<double> stats = GetDlDelayStats (p.m_imsi, p.m_lcId);
      for (std::vector<double>::iterator it = stats.begin (); it != stats.end (); ++it)
        {
          outFile << (*it) * 1e-9 << "\t";
        }
      stats = GetDlPduSizeStats (p.m_imsi, p.m_lcId);
      for (std::vector<double>::iterator it = stats.begin (); it != stats.end (); ++it)
        {
          outFile << (*it) << "\t";
        }
      outFile << std::endl;
    }

  outFile.close ();
}

void
cv2x_RadioBearerStatsCalculator::ResetResults (void)
{
  NS_LOG_FUNCTION (this);

  m_ulTxPackets.erase (m_ulTxPackets.begin (), m_ulTxPackets.end ());
  m_ulRxPackets.erase (m_ulRxPackets.begin (), m_ulRxPackets.end ());
  m_ulRxData.erase (m_ulRxData.begin (), m_ulRxData.end ());
  m_ulTxData.erase (m_ulTxData.begin (), m_ulTxData.end ());
  m_ulDelay.erase (m_ulDelay.begin (), m_ulDelay.end ());
  m_ulPduSize.erase (m_ulPduSize.begin (), m_ulPduSize.end ());

  m_dlTxPackets.erase (m_dlTxPackets.begin (), m_dlTxPackets.end ());
  m_dlRxPackets.erase (m_dlRxPackets.begin (), m_dlRxPackets.end ());
  m_dlRxData.erase (m_dlRxData.begin (), m_dlRxData.end ());
  m_dlTxData.erase (m_dlTxData.begin (), m_dlTxData.end ());
  m_dlDelay.erase (m_dlDelay.begin (), m_dlDelay.end ());
  m_dlPduSize.erase (m_dlPduSize.begin (), m_dlPduSize.end ());
}

void
cv2x_RadioBearerStatsCalculator::RescheduleEndEpoch (void)
{
  NS_LOG_FUNCTION (this);
  m_endEpochEvent.Cancel ();
  NS_ASSERT (Simulator::Now ().GetMilliSeconds () == 0); // below event time assumes this
  m_endEpochEvent = Simulator::Schedule (m_startTime + m_epochDuration, &cv2x_RadioBearerStatsCalculator::EndEpoch, this);
}

void
cv2x_RadioBearerStatsCalculator::EndEpoch (void)
{
  NS_LOG_FUNCTION (this);
  ShowResults ();
  ResetResults ();
  m_startTime += m_epochDuration;
  m_endEpochEvent = Simulator::Schedule (m_epochDuration, &cv2x_RadioBearerStatsCalculator::EndEpoch, this);
}

uint32_t
cv2x_RadioBearerStatsCalculator::GetUlTxPackets (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_ulTxPackets[p];
}

uint32_t
cv2x_RadioBearerStatsCalculator::GetUlRxPackets (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_ulRxPackets[p];
}

uint64_t
cv2x_RadioBearerStatsCalculator::GetUlTxData (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_ulTxData[p];
}

uint64_t
cv2x_RadioBearerStatsCalculator::GetUlRxData (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_ulRxData[p];
}

double
cv2x_RadioBearerStatsCalculator::GetUlDelay (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  cv2x_Uint64StatsMap::iterator it = m_ulDelay.find (p);
  if (it == m_ulDelay.end ())
    {
      NS_LOG_ERROR ("UL delay for " << imsi << " - " << (uint16_t) lcid << " not found");
      return 0;

    }
  return m_ulDelay[p]->getMean ();
}

std::vector<double>
cv2x_RadioBearerStatsCalculator::GetUlDelayStats (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  std::vector<double> stats;
  cv2x_Uint64StatsMap::iterator it = m_ulDelay.find (p);
  if (it == m_ulDelay.end ())
    {
      stats.push_back (0.0);
      stats.push_back (0.0);
      stats.push_back (0.0);
      stats.push_back (0.0);
      return stats;

    }
  stats.push_back (m_ulDelay[p]->getMean ());
  stats.push_back (m_ulDelay[p]->getStddev ());
  stats.push_back (m_ulDelay[p]->getMin ());
  stats.push_back (m_ulDelay[p]->getMax ());
  return stats;
}

std::vector<double>
cv2x_RadioBearerStatsCalculator::GetUlPduSizeStats (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  std::vector<double> stats;
  cv2x_Uint32StatsMap::iterator it = m_ulPduSize.find (p);
  if (it == m_ulPduSize.end ())
    {
      stats.push_back (0.0);
      stats.push_back (0.0);
      stats.push_back (0.0);
      stats.push_back (0.0);
      return stats;

    }
  stats.push_back (m_ulPduSize[p]->getMean ());
  stats.push_back (m_ulPduSize[p]->getStddev ());
  stats.push_back (m_ulPduSize[p]->getMin ());
  stats.push_back (m_ulPduSize[p]->getMax ());
  return stats;
}

uint32_t
cv2x_RadioBearerStatsCalculator::GetDlTxPackets (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_dlTxPackets[p];
}

uint32_t
cv2x_RadioBearerStatsCalculator::GetDlRxPackets (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_dlRxPackets[p];
}

uint64_t
cv2x_RadioBearerStatsCalculator::GetDlTxData (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_dlTxData[p];
}

uint64_t
cv2x_RadioBearerStatsCalculator::GetDlRxData (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_dlRxData[p];
}

uint32_t
cv2x_RadioBearerStatsCalculator::GetUlCellId (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_ulCellId[p];
}

uint32_t
cv2x_RadioBearerStatsCalculator::GetDlCellId (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  return m_dlCellId[p];
}

double
cv2x_RadioBearerStatsCalculator::GetDlDelay (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  cv2x_Uint64StatsMap::iterator it = m_dlDelay.find (p);
  if (it == m_dlDelay.end ())
    {
      NS_LOG_ERROR ("DL delay for " << imsi << " not found");
      return 0;
    }
  return m_dlDelay[p]->getMean ();
}

std::vector<double>
cv2x_RadioBearerStatsCalculator::GetDlDelayStats (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  std::vector<double> stats;
  cv2x_Uint64StatsMap::iterator it = m_dlDelay.find (p);
  if (it == m_dlDelay.end ())
    {
      stats.push_back (0.0);
      stats.push_back (0.0);
      stats.push_back (0.0);
      stats.push_back (0.0);
      return stats;

    }
  stats.push_back (m_dlDelay[p]->getMean ());
  stats.push_back (m_dlDelay[p]->getStddev ());
  stats.push_back (m_dlDelay[p]->getMin ());
  stats.push_back (m_dlDelay[p]->getMax ());
  return stats;
}

std::vector<double>
cv2x_RadioBearerStatsCalculator::GetDlPduSizeStats (uint64_t imsi, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << imsi << (uint16_t) lcid);
  cv2x_ImsiLcidPair_t p (imsi, lcid);
  std::vector<double> stats;
  cv2x_Uint32StatsMap::iterator it = m_dlPduSize.find (p);
  if (it == m_dlPduSize.end ())
    {
      stats.push_back (0.0);
      stats.push_back (0.0);
      stats.push_back (0.0);
      stats.push_back (0.0);
      return stats;

    }
  stats.push_back (m_dlPduSize[p]->getMean ());
  stats.push_back (m_dlPduSize[p]->getStddev ());
  stats.push_back (m_dlPduSize[p]->getMin ());
  stats.push_back (m_dlPduSize[p]->getMax ());
  return stats;
}

std::string
cv2x_RadioBearerStatsCalculator::GetUlOutputFilename (void)
{
  if (m_protocolType == "RLC")
    {
      return cv2x_LteStatsCalculator::GetUlOutputFilename ();
    }
  else
    {
      return GetUlPdcpOutputFilename ();
    }
}

std::string
cv2x_RadioBearerStatsCalculator::GetDlOutputFilename (void)
{
  if (m_protocolType == "RLC")
    {
      return cv2x_LteStatsCalculator::GetDlOutputFilename ();
    }
  else
    {
      return GetDlPdcpOutputFilename ();
    }
}

void
cv2x_RadioBearerStatsCalculator::SetUlPdcpOutputFilename (std::string outputFilename)
{
  m_ulPdcpOutputFilename = outputFilename;
}

std::string
cv2x_RadioBearerStatsCalculator::GetUlPdcpOutputFilename (void)
{
  return m_ulPdcpOutputFilename;
}
void
cv2x_RadioBearerStatsCalculator::SetDlPdcpOutputFilename (std::string outputFilename)
{
  m_dlPdcpOutputFilename = outputFilename;
}

std::string
cv2x_RadioBearerStatsCalculator::GetDlPdcpOutputFilename (void)
{
  return m_dlPdcpOutputFilename;
}

} // namespace ns3
