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
 * Modified by: Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *              Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 *              NIST
 */

#include "cv2x_mac-stats-calculator.h"
#include "ns3/string.h"
#include <ns3/simulator.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_MacStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED (cv2x_MacStatsCalculator);

cv2x_MacStatsCalculator::cv2x_MacStatsCalculator ()
  : m_dlFirstWrite (true),
    m_ulFirstWrite (true),
    m_slUeFirstWrite (true),
    m_slSchUeFirstWrite (true)
{
  NS_LOG_FUNCTION (this);

}

cv2x_MacStatsCalculator::~cv2x_MacStatsCalculator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
cv2x_MacStatsCalculator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_MacStatsCalculator")
    .SetParent<cv2x_LteStatsCalculator> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_MacStatsCalculator> ()
    .AddAttribute ("DlOutputFilename",
                   "Name of the file where the downlink results will be saved.",
                   StringValue ("DlMacStats.txt"),
                   MakeStringAccessor (&cv2x_MacStatsCalculator::SetDlOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("UlOutputFilename",
                   "Name of the file where the uplink results will be saved.",
                   StringValue ("UlMacStats.txt"),
                   MakeStringAccessor (&cv2x_MacStatsCalculator::SetUlOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("SlUeOutputFilename",
                   "Name of the file where the sidelink results will be saved.",
                   StringValue ("SlUeMacStats.txt"),
                   MakeStringAccessor (&cv2x_MacStatsCalculator::SetSlUeOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("SlSchUeOutputFilename",
                   "Name of the file where the sidelink results will be saved.",
                   StringValue ("SlSchUeMacStats.txt"),
                   MakeStringAccessor (&cv2x_MacStatsCalculator::SetSlSchUeOutputFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

void
cv2x_MacStatsCalculator::SetUlOutputFilename (std::string outputFilename)
{
  cv2x_LteStatsCalculator::SetUlOutputFilename (outputFilename);
}

std::string
cv2x_MacStatsCalculator::GetUlOutputFilename (void)
{
  return cv2x_LteStatsCalculator::GetUlOutputFilename ();
}

void
cv2x_MacStatsCalculator::SetDlOutputFilename (std::string outputFilename)
{
  cv2x_LteStatsCalculator::SetDlOutputFilename (outputFilename);
}

std::string
cv2x_MacStatsCalculator::GetDlOutputFilename (void)
{
  return cv2x_LteStatsCalculator::GetDlOutputFilename ();
}

void
cv2x_MacStatsCalculator::SetSlUeOutputFilename (std::string outputFilename)
{
  cv2x_LteStatsCalculator::SetSlUeOutputFilename (outputFilename);
}

std::string
cv2x_MacStatsCalculator::GetSlUeOutputFilename (void)
{
  return cv2x_LteStatsCalculator::GetSlUeOutputFilename ();
}

void
cv2x_MacStatsCalculator::SetSlSchUeOutputFilename (std::string outputFilename)
{
  cv2x_LteStatsCalculator::SetSlSchUeOutputFilename (outputFilename);
}

std::string
cv2x_MacStatsCalculator::GetSlSchUeOutputFilename (void)
{
  return cv2x_LteStatsCalculator::GetSlSchUeOutputFilename ();
}

void
cv2x_MacStatsCalculator::DlScheduling (uint16_t cellId, uint64_t imsi, cv2x_DlSchedulingCallbackInfo dlSchedulingCallbackInfo)
{
  NS_LOG_FUNCTION (this << cellId << imsi << dlSchedulingCallbackInfo.frameNo << dlSchedulingCallbackInfo.subframeNo <<
		  dlSchedulingCallbackInfo.rnti << (uint32_t) dlSchedulingCallbackInfo.mcsTb1 << dlSchedulingCallbackInfo.sizeTb1 << (uint32_t) dlSchedulingCallbackInfo.mcsTb2 << dlSchedulingCallbackInfo.sizeTb2);
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
      outFile << "% time\tcellId\tIMSI\tframe\tsframe\tRNTI\tmcsTb1\tsizeTb1\tmcsTb2\tsizeTb2\tccId";
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

  outFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t";
  outFile << (uint32_t) cellId << "\t";
  outFile << imsi << "\t";
  outFile << dlSchedulingCallbackInfo.frameNo << "\t";
  outFile << dlSchedulingCallbackInfo.subframeNo << "\t";
  outFile << dlSchedulingCallbackInfo.rnti << "\t";
  outFile << (uint32_t) dlSchedulingCallbackInfo.mcsTb1 << "\t";
  outFile << dlSchedulingCallbackInfo.sizeTb1 << "\t";
  outFile << (uint32_t) dlSchedulingCallbackInfo.mcsTb2 << "\t";
  outFile << dlSchedulingCallbackInfo.sizeTb2 << "\t";
  outFile << (uint32_t) dlSchedulingCallbackInfo.componentCarrierId << std::endl;
  outFile.close ();
}

void
cv2x_MacStatsCalculator::UlScheduling (uint16_t cellId, uint64_t imsi, uint32_t frameNo,
                                  uint32_t subframeNo, uint16_t rnti,uint8_t mcsTb, uint16_t size, uint8_t componentCarrierId)
{
  NS_LOG_FUNCTION (this << cellId << imsi << frameNo << subframeNo << rnti << (uint32_t) mcsTb << size);
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
      outFile << "% time\tcellId\tIMSI\tframe\tsframe\tRNTI\tmcs\tsize\tccId";
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

  outFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t";
  outFile << (uint32_t) cellId << "\t";
  outFile << imsi << "\t";
  outFile << frameNo << "\t";
  outFile << subframeNo << "\t";
  outFile << rnti << "\t";
  outFile << (uint32_t) mcsTb << "\t";
  outFile << size << "\t";
  outFile << (uint32_t) componentCarrierId << std::endl;
  outFile.close ();
}

void
cv2x_MacStatsCalculator::SlUeScheduling (cv2x_SlUeMacStatParameters params)
{
  NS_LOG_FUNCTION (this << params.m_cellId << params.m_imsi << params.m_frameNo << params.m_subframeNo << params.m_rnti << (uint32_t) params.m_mcs << params.m_pscchRi << params.m_pscchFrame1 << params.m_pscchSubframe1 << params.m_pscchFrame2 << params.m_pscchSubframe2 << params.m_psschTxStartRB << params.m_psschTxLengthRB << params.m_psschItrp);
  NS_LOG_INFO ("Write SL UE Mac Stats in " << GetSlUeOutputFilename ().c_str ());

  std::ofstream outFile;
  if ( m_slUeFirstWrite == true )
    {
      outFile.open (GetSlUeOutputFilename ().c_str ());
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetSlUeOutputFilename ().c_str ());
          return;
        }
      m_slUeFirstWrite = false;
      outFile << "% time\tcellId\tIMSI\tRNTI\tframe\tsframe\tpscchRi\tpscchF1\tpscchSF1\tpscchF2\tpscchSF2\tmcs\tTBS\tpsschRB\tpsschLen\tpssch_itrp";
      outFile << std::endl;
    }
  else
    {
      outFile.open (GetSlUeOutputFilename ().c_str (),  std::ios_base::app);
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetSlUeOutputFilename ().c_str ());
          return;
        }
    }

  outFile << (uint32_t) params.m_timestamp << "\t";
  outFile << (uint32_t) params.m_cellId << "\t";
  outFile << params.m_imsi << "\t";
  outFile << params.m_rnti << "\t";
  outFile << params.m_frameNo << "\t";
  outFile << params.m_subframeNo << "\t";
  outFile << params.m_pscchRi << "\t";
  outFile << params.m_pscchFrame1 << "\t";
  outFile << params.m_pscchSubframe1 << "\t";
  outFile << params.m_pscchFrame2 << "\t";
  outFile << params.m_pscchSubframe2 << "\t";
  outFile << (uint32_t) params.m_mcs << "\t";
  outFile << params.m_tbSize << "\t";
  outFile << params.m_psschTxStartRB << "\t";
  outFile << params.m_psschTxLengthRB << "\t";
  outFile << params.m_psschItrp << std::endl;
  outFile.close ();
}

void
cv2x_MacStatsCalculator::SlSharedChUeScheduling (cv2x_SlUeMacStatParameters params)
{
  NS_LOG_FUNCTION (this << params.m_cellId << params.m_imsi << params.m_rnti << params.m_frameNo << params.m_subframeNo << (uint32_t) params.m_mcs << params.m_tbSize << params.m_psschTxStartRB << params.m_psschTxLengthRB);
  NS_LOG_INFO ("Write SL Shared Channel UE Mac Stats in " << GetSlSchUeOutputFilename ().c_str ());

  std::ofstream outFile;
  if ( m_slSchUeFirstWrite == true )
    {
      outFile.open (GetSlSchUeOutputFilename ().c_str ());
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetSlSchUeOutputFilename ().c_str ());
          return;
        }
      m_slSchUeFirstWrite = false;
      outFile << "% time\tcellId\tIMSI\tRNTI\tSlPframe\tSlPsframe\tSchStartframe\tSchSsframe\tframe\tsframe\tmcs\tTBS\tpsschRB\tpsschLen";
      outFile << std::endl;
    }
  else
    {
      outFile.open (GetSlSchUeOutputFilename ().c_str (),  std::ios_base::app);
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetSlSchUeOutputFilename ().c_str ());
          return;
        }
    }

  outFile << (uint32_t) params.m_timestamp << "\t";
  outFile << (uint32_t) params.m_cellId << "\t";
  outFile << params.m_imsi << "\t";
  outFile << params.m_rnti << "\t";
  outFile << params.m_frameNo << "\t";
  outFile << params.m_subframeNo << "\t";
  outFile << params.m_psschFrameStart << "\t";
  outFile << params.m_psschSubframeStart << "\t";
  outFile << params.m_psschFrame << "\t";
  outFile << params.m_psschSubframe << "\t";
  outFile << (uint32_t) params.m_mcs << "\t";
  outFile << params.m_tbSize << "\t";
  outFile << params.m_psschTxStartRB << "\t";
  outFile << params.m_psschTxLengthRB << std::endl;
  outFile.close ();
}

void
cv2x_MacStatsCalculator::DlSchedulingCallback (Ptr<cv2x_MacStatsCalculator> macStats, std::string path, cv2x_DlSchedulingCallbackInfo dlSchedulingCallbackInfo)
{
  NS_LOG_FUNCTION (macStats << path);
  uint64_t imsi = 0;
  std::ostringstream pathAndRnti;
  std::string pathEnb  = path.substr (0, path.find ("/cv2x_ComponentCarrierMap"));
  pathAndRnti << pathEnb << "/cv2x_LteEnbRrc/UeMap/" << dlSchedulingCallbackInfo.rnti;
  if (macStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      imsi = macStats->GetImsiPath (pathAndRnti.str ());
    }
  else
    {
      imsi = FindImsiFromEnbRlcPath (pathAndRnti.str ());
      macStats->SetImsiPath (pathAndRnti.str (), imsi);
    }
  uint16_t cellId = 0;
  if (macStats->ExistsCellIdPath (pathAndRnti.str ()) == true)
    {
      cellId = macStats->GetCellIdPath (pathAndRnti.str ());
    }
  else
    {
      cellId = FindCellIdFromEnbRlcPath (pathAndRnti.str ());
      macStats->SetCellIdPath (pathAndRnti.str (), cellId);
    }

  macStats->DlScheduling (cellId, imsi, dlSchedulingCallbackInfo);
}

void
cv2x_MacStatsCalculator::UlSchedulingCallback (Ptr<cv2x_MacStatsCalculator> macStats, std::string path,
                      uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                      uint8_t mcs, uint16_t size, uint8_t componentCarrierId)
{
  NS_LOG_FUNCTION (macStats << path);

  uint64_t imsi = 0;
  std::ostringstream pathAndRnti;
  std::string pathEnb  = path.substr (0, path.find ("/cv2x_ComponentCarrierMap"));
  pathAndRnti << pathEnb << "/cv2x_LteEnbRrc/UeMap/" << rnti;
  if (macStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      imsi = macStats->GetImsiPath (pathAndRnti.str ());
    }
  else
    {
      imsi = FindImsiFromEnbRlcPath (pathAndRnti.str ());
      macStats->SetImsiPath (pathAndRnti.str (), imsi);
    }
  uint16_t cellId = 0;
  if (macStats->ExistsCellIdPath (pathAndRnti.str ()) == true)
    {
      cellId = macStats->GetCellIdPath (pathAndRnti.str ());
    }
  else
    {
      cellId = FindCellIdFromEnbRlcPath (pathAndRnti.str ());
      macStats->SetCellIdPath (pathAndRnti.str (), cellId);
    }

  macStats->UlScheduling (cellId, imsi, frameNo, subframeNo, rnti, mcs, size, componentCarrierId);
}

void
cv2x_MacStatsCalculator::SlUeSchedulingCallback (Ptr<cv2x_MacStatsCalculator> macStats, std::string path, cv2x_SlUeMacStatParameters params)
  // uint32_t frameNo, uint32_t subframeNo, uint16_t rnti, uint8_t mcs, uint16_t pscch_ri, uint16_t pssch_rb, uint16_t pssch_length, uint16_t pssch_itrp)
{
  NS_LOG_FUNCTION (macStats << path);

  std::ostringstream pathAndRnti;
  pathAndRnti << path << "/" << params.m_rnti;
  if (macStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      NS_LOG_LOGIC("Existing IMSI path. Getting IMSI..."); 
      params.m_imsi = macStats->GetImsiPath (pathAndRnti.str ());
      NS_LOG_LOGIC("IMSI= " << params.m_imsi);
    }
  else
    {
      NS_LOG_LOGIC("NON-existing IMSI path. Finding IMSI from UE cv2x_LteNetDevice...");
      std::string ueNetDevicePath = path.substr (0, path.find("/cv2x_LteUeMac"));
      params.m_imsi = FindImsiFromLteNetDevice (ueNetDevicePath);
      NS_LOG_LOGIC("Found IMSI= " << params.m_imsi);
      macStats->SetImsiPath (pathAndRnti.str (), params.m_imsi);
    }
  params.m_cellId = 0;
 /* if (macStats->ExistsCellIdPath (pathAndRnti.str ()) == true)
    {
      cellId = macStats->GetCellIdPath (pathAndRnti.str ());
    }
  else
    {
      cellId = FindCellIdFromEnbMac (path, rnti);
      macStats->SetCellIdPath (pathAndRnti.str (), cellId);
    }
    */

  macStats->SlUeScheduling (params);
}

void
cv2x_MacStatsCalculator::SlSharedChUeSchedulingCallback (Ptr<cv2x_MacStatsCalculator> macStats, std::string path, cv2x_SlUeMacStatParameters params)
{
  NS_LOG_FUNCTION (macStats << path);

  std::ostringstream pathAndRnti;
  pathAndRnti << path << "/" << params.m_rnti;
  if (macStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      NS_LOG_LOGIC("Existing IMSI path. Getting IMSI..."); 
      params.m_imsi = macStats->GetImsiPath (pathAndRnti.str ());
      NS_LOG_LOGIC("IMSI= " << params.m_imsi);
    }
  else
    {
      NS_LOG_LOGIC("NON-existing IMSI path. Finding IMSI from UE cv2x_LteNetDevice...");
      std::string ueNetDevicePath = path.substr (0, path.find("/cv2x_LteUeMac"));
      params.m_imsi = FindImsiFromLteNetDevice (ueNetDevicePath);
      NS_LOG_LOGIC("Found IMSI= " << params.m_imsi);
      macStats->SetImsiPath (pathAndRnti.str (), params.m_imsi);
    }
  params.m_cellId = 0;
 /* if (macStats->ExistsCellIdPath (pathAndRnti.str ()) == true)
    {
      cellId = macStats->GetCellIdPath (pathAndRnti.str ());
    }
  else
    {
      cellId = FindCellIdFromEnbMac (path, rnti);
      macStats->SetCellIdPath (pathAndRnti.str (), cellId);
    }
    */

  macStats->SlSharedChUeScheduling (params);
}


} // namespace ns3
