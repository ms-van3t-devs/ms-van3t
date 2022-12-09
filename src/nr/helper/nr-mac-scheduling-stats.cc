/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * This API is derived from MacStatsCalculator of LTE module
 */

#include "ns3/string.h"
#include <ns3/simulator.h>
#include <ns3/log.h>
#include "nr-mac-scheduling-stats.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulingStats");

NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulingStats);

NrMacSchedulingStats::NrMacSchedulingStats ()
  : m_dlFirstWrite (true),
    m_ulFirstWrite (true)
{
  NS_LOG_FUNCTION (this);

}

NrMacSchedulingStats::~NrMacSchedulingStats ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrMacSchedulingStats::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulingStats")
    .SetParent<NrStatsCalculator> ()
    .SetGroupName ("nr")
    .AddConstructor<NrMacSchedulingStats> ()
    .AddAttribute ("DlOutputFilename",
                   "Name of the file where the downlink results will be saved.",
                   StringValue ("NrDlMacStats.txt"),
                   MakeStringAccessor (&NrMacSchedulingStats::SetDlOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("UlOutputFilename",
                   "Name of the file where the uplink results will be saved.",
                   StringValue ("NrUlMacStats.txt"),
                   MakeStringAccessor (&NrMacSchedulingStats::SetUlOutputFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

void
NrMacSchedulingStats::SetUlOutputFilename (std::string outputFilename)
{
  NrStatsCalculator::SetUlOutputFilename (outputFilename);
}

std::string
NrMacSchedulingStats::GetUlOutputFilename (void)
{
  return NrStatsCalculator::GetUlOutputFilename ();
}

void
NrMacSchedulingStats::SetDlOutputFilename (std::string outputFilename)
{
  NrStatsCalculator::SetDlOutputFilename (outputFilename);
}

std::string
NrMacSchedulingStats::GetDlOutputFilename (void)
{
  return NrStatsCalculator::GetDlOutputFilename ();
}

void
NrMacSchedulingStats::DlScheduling (uint16_t cellId, uint64_t imsi, const NrSchedulingCallbackInfo &traceInfo)
{
  NS_LOG_FUNCTION (this << cellId << imsi << traceInfo.m_frameNum << traceInfo.m_subframeNum <<
                   traceInfo.m_rnti << (uint32_t) traceInfo.m_mcs << traceInfo.m_tbSize);
  NS_LOG_INFO ("Write DL Mac Stats in " << GetDlOutputFilename ().c_str ());

  std::ofstream outFile;
  if ( m_dlFirstWrite == true )
    {
      outFile.open (GetDlOutputFilename ().c_str ());
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetDlOutputFilename ().c_str ());
          return;
        }
      m_dlFirstWrite = false;
      outFile << "% time(s)\tcellId\tbwpId\tIMSI\tRNTI\tframe\tsframe\tslot\tsymStart\tnumSym\tstream\tharqId\tndi\trv\tmcs\ttbSize";
      outFile << std::endl;
    }
  else
    {
      outFile.open (GetDlOutputFilename ().c_str (),  std::ios_base::app);
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetDlOutputFilename ().c_str ());
          return;
        }
    }

  outFile << Simulator::Now ().GetSeconds () << "\t";
  outFile << (uint32_t) cellId << "\t";
  outFile << (uint32_t) traceInfo.m_bwpId  << "\t";
  outFile << imsi << "\t";
  outFile << traceInfo.m_rnti << "\t";
  outFile << traceInfo.m_frameNum << "\t";
  outFile << (uint32_t)traceInfo.m_subframeNum << "\t";
  outFile << traceInfo.m_slotNum << "\t";
  outFile << (uint32_t)traceInfo.m_symStart << "\t";
  outFile << (uint32_t)traceInfo.m_numSym << "\t";
  outFile << (uint32_t) traceInfo.m_streamId << "\t";
  outFile << (uint32_t) traceInfo.m_harqId << "\t";
  outFile << (uint32_t) traceInfo.m_ndi << "\t";
  outFile << (uint32_t) traceInfo.m_rv << "\t";
  outFile << (uint32_t) traceInfo.m_mcs << "\t";
  outFile << traceInfo.m_tbSize << std::endl;
  outFile.close ();
}

void
NrMacSchedulingStats::UlScheduling (uint16_t cellId, uint64_t imsi, const NrSchedulingCallbackInfo &traceInfo)
{
  NS_LOG_FUNCTION (this << cellId << imsi << traceInfo.m_frameNum << traceInfo.m_subframeNum
                        << traceInfo.m_rnti << (uint32_t) traceInfo.m_mcs << traceInfo.m_tbSize);
  NS_LOG_INFO ("Write UL Mac Stats in " << GetUlOutputFilename ().c_str ());

  std::ofstream outFile;
  if ( m_ulFirstWrite == true )
    {
      outFile.open (GetUlOutputFilename ().c_str ());
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetUlOutputFilename ().c_str ());
          return;
        }
      m_ulFirstWrite = false;
      outFile << "% time(s)\tcellId\tbwpId\tIMSI\tRNTI\tframe\tsframe\tslot\tsymStart\tnumSym\tstream\tharqId\tndi\trv\tmcs\ttbSize";
      outFile << std::endl;
    }
  else
    {
      outFile.open (GetUlOutputFilename ().c_str (),  std::ios_base::app);
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetUlOutputFilename ().c_str ());
          return;
        }
    }

  outFile << Simulator::Now ().GetSeconds () << "\t";
  outFile << (uint32_t) cellId << "\t";
  outFile << (uint32_t) traceInfo.m_bwpId  << "\t";
  outFile << imsi << "\t";
  outFile << traceInfo.m_rnti << "\t";
  outFile << traceInfo.m_frameNum << "\t";
  outFile << (uint32_t)traceInfo.m_subframeNum << "\t";
  outFile << traceInfo.m_slotNum << "\t";
  outFile << (uint32_t)traceInfo.m_symStart << "\t";
  outFile << (uint32_t)traceInfo.m_numSym << "\t";
  outFile << (uint32_t) traceInfo.m_streamId << "\t";
  outFile << (uint32_t) traceInfo.m_harqId << "\t";
  outFile << (uint32_t) traceInfo.m_ndi << "\t";
  outFile << (uint32_t) traceInfo.m_rv << "\t";
  outFile << (uint32_t) traceInfo.m_mcs << "\t";
  outFile << traceInfo.m_tbSize << std::endl;
  outFile.close ();
}

void
NrMacSchedulingStats::DlSchedulingCallback (Ptr<NrMacSchedulingStats> macStats, std::string path, NrSchedulingCallbackInfo traceInfo)
{
  NS_LOG_FUNCTION (macStats << path);
  uint64_t imsi = 0;
  std::ostringstream pathAndRnti;
  std::string pathGnb  = path.substr (0, path.find ("/BandwidthPartMap"));
  pathAndRnti << pathGnb << "/LteEnbRrc/UeMap/" << traceInfo.m_rnti;
  if (macStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      imsi = macStats->GetImsiPath (pathAndRnti.str ());
    }
  else
    {
      imsi = FindImsiFromGnbRlcPath (pathAndRnti.str ());
      macStats->SetImsiPath (pathAndRnti.str (), imsi);
    }
  uint16_t cellId = 0;
  if (macStats->ExistsCellIdPath (pathAndRnti.str ()) == true)
    {
      cellId = macStats->GetCellIdPath (pathAndRnti.str ());
    }
  else
    {
      cellId = FindCellIdFromGnbRlcPath (pathAndRnti.str ());
      macStats->SetCellIdPath (pathAndRnti.str (), cellId);
    }

  macStats->DlScheduling (cellId, imsi, traceInfo);
}

void
NrMacSchedulingStats::UlSchedulingCallback (Ptr<NrMacSchedulingStats> macStats,
                                            std::string path, NrSchedulingCallbackInfo traceInfo)
{
  NS_LOG_FUNCTION (macStats << path);

  uint64_t imsi = 0;
  std::ostringstream pathAndRnti;
  std::string pathGnb  = path.substr (0, path.find ("/BandwidthPartMap"));
  pathAndRnti << pathGnb << "/LteEnbRrc/UeMap/" << traceInfo.m_rnti;
  if (macStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      imsi = macStats->GetImsiPath (pathAndRnti.str ());
    }
  else
    {
      imsi = FindImsiFromGnbRlcPath (pathAndRnti.str ());
      macStats->SetImsiPath (pathAndRnti.str (), imsi);
    }
  uint16_t cellId = 0;
  if (macStats->ExistsCellIdPath (pathAndRnti.str ()) == true)
    {
      cellId = macStats->GetCellIdPath (pathAndRnti.str ());
    }
  else
    {
      cellId = FindCellIdFromGnbRlcPath (pathAndRnti.str ());
      macStats->SetCellIdPath (pathAndRnti.str (), cellId);
    }

  macStats->UlScheduling (cellId, imsi, traceInfo);
}


} // namespace ns3
