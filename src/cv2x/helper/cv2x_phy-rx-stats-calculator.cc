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
 * modified by: Marco Miozzo <mmiozzo@cttc.es>
 *        Convert cv2x_MacStatsCalculator in cv2x_PhyRxStatsCalculator
 *        Convert NistMacStatsCalculator in NistPhyRxStatsCalculator
 * Modified by: NIST
 */

#include "cv2x_phy-rx-stats-calculator.h"
#include "ns3/string.h"
#include <ns3/simulator.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_PhyRxStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED (cv2x_PhyRxStatsCalculator);

cv2x_PhyRxStatsCalculator::cv2x_PhyRxStatsCalculator ()
  : m_dlRxFirstWrite (true),
    m_ulRxFirstWrite (true),
    m_slRxFirstWrite (true),
    m_slPscchRxFirstWrite (true)
{
  NS_LOG_FUNCTION (this);

}

cv2x_PhyRxStatsCalculator::~cv2x_PhyRxStatsCalculator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
cv2x_PhyRxStatsCalculator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_PhyRxStatsCalculator")
    .SetParent<cv2x_LteStatsCalculator> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_PhyRxStatsCalculator> ()
    .AddAttribute ("DlRxOutputFilename",
                   "Name of the file where the downlink results will be saved.",
                   StringValue ("DlRxPhyStats.txt"),
                   MakeStringAccessor (&cv2x_PhyRxStatsCalculator::SetDlRxOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("UlRxOutputFilename",
                   "Name of the file where the uplink results will be saved.",
                   StringValue ("UlRxPhyStats.txt"),
                   MakeStringAccessor (&cv2x_PhyRxStatsCalculator::SetUlRxOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("SlRxOutputFilename",
                   "Name of the file where the sidelink results will be saved.",
                   StringValue ("SlRxPhyStats.txt"),
                   MakeStringAccessor (&cv2x_PhyRxStatsCalculator::SetSlRxOutputFilename),
                   MakeStringChecker ())
    .AddAttribute ("SlPscchRxOutputFilename",
                   "Name of the file where the sidelink PSCHCH results will be saved.",
                   StringValue ("SlPscchRxPhyStats.txt"),
                   MakeStringAccessor (&cv2x_PhyRxStatsCalculator::SetSlPscchRxOutputFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

void
cv2x_PhyRxStatsCalculator::SetUlRxOutputFilename (std::string outputFilename)
{
  cv2x_LteStatsCalculator::SetUlOutputFilename (outputFilename);
}

std::string
cv2x_PhyRxStatsCalculator::GetUlRxOutputFilename (void)
{
  return cv2x_LteStatsCalculator::GetUlOutputFilename ();
}

void
cv2x_PhyRxStatsCalculator::SetDlRxOutputFilename (std::string outputFilename)
{
  cv2x_LteStatsCalculator::SetDlOutputFilename (outputFilename);
}

std::string
cv2x_PhyRxStatsCalculator::GetDlRxOutputFilename (void)
{
  return cv2x_LteStatsCalculator::GetDlOutputFilename ();
}

void
cv2x_PhyRxStatsCalculator::SetSlRxOutputFilename (std::string outputFilename)
{
  cv2x_LteStatsCalculator::SetSlOutputFilename (outputFilename);
}

std::string
cv2x_PhyRxStatsCalculator::GetSlRxOutputFilename (void)
{
  return cv2x_LteStatsCalculator::GetSlOutputFilename ();
}

void
cv2x_PhyRxStatsCalculator::SetSlPscchRxOutputFilename (std::string outputFilename)
{
  cv2x_LteStatsCalculator::SetSlPscchOutputFilename (outputFilename);
}

std::string
cv2x_PhyRxStatsCalculator::GetSlPscchRxOutputFilename (void)
{
  return cv2x_LteStatsCalculator::GetSlPscchOutputFilename ();
}

void
cv2x_PhyRxStatsCalculator::DlPhyReception (cv2x_PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (this << params.m_cellId << params.m_imsi << params.m_timestamp << params.m_rnti << params.m_layer << params.m_mcs << params.m_size << params.m_rv << params.m_ndi << params.m_correctness);
  NS_LOG_INFO ("Write DL Rx Phy Stats in " << GetDlRxOutputFilename ().c_str ());

  std::ofstream outFile;
  if ( m_dlRxFirstWrite == true )
    {
      outFile.open (GetDlRxOutputFilename ().c_str ());
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetDlRxOutputFilename ().c_str ());
          return;
        }
      m_dlRxFirstWrite = false;
      outFile << "% time\tcellId\tIMSI\tRNTI\ttxMode\tlayer\tmcs\tsize\trv\tndi\tcorrect\tccId";
      outFile << std::endl;
    }
  else
    {
      outFile.open (GetDlRxOutputFilename ().c_str (),  std::ios_base::app);
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetDlRxOutputFilename ().c_str ());
          return;
        }
    }

//   outFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t";
  outFile << params.m_timestamp << "\t";
  outFile << (uint32_t) params.m_cellId << "\t";
  outFile << params.m_imsi << "\t";
  outFile << params.m_rnti << "\t";
  outFile << (uint32_t) params.m_txMode << "\t";
  outFile << (uint32_t) params.m_layer << "\t";
  outFile << (uint32_t) params.m_mcs << "\t";
  outFile << params.m_size << "\t";
  outFile << (uint32_t) params.m_rv << "\t";
  outFile << (uint32_t) params.m_ndi << "\t";
  outFile << (uint32_t) params.m_correctness << "\t";
  outFile << (uint32_t) params.m_ccId << std::endl;
  outFile.close ();
}

void
cv2x_PhyRxStatsCalculator::UlPhyReception (cv2x_PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (this << params.m_cellId << params.m_imsi << params.m_timestamp << params.m_rnti << params.m_layer << params.m_mcs << params.m_size << params.m_rv << params.m_ndi << params.m_correctness);
  NS_LOG_INFO ("Write UL Rx Phy Stats in " << GetUlRxOutputFilename ().c_str ());

  std::ofstream outFile;
  if ( m_ulRxFirstWrite == true )
    {
      outFile.open (GetUlRxOutputFilename ().c_str ());
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetUlRxOutputFilename ().c_str ());
          return;
        }
      m_ulRxFirstWrite = false;
      outFile << "% time\tcellId\tIMSI\tRNTI\tlayer\tmcs\tsize\trv\tndi\tcorrect\tccId";
      outFile << std::endl;
    }
  else
    {
      outFile.open (GetUlRxOutputFilename ().c_str (),  std::ios_base::app);
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetUlRxOutputFilename ().c_str ());
          return;
        }
    }

//   outFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t";
  outFile << params.m_timestamp << "\t";
  outFile << (uint32_t) params.m_cellId << "\t";
  outFile << params.m_imsi << "\t";
  outFile << params.m_rnti << "\t";
  outFile << (uint32_t) params.m_layer << "\t";
  outFile << (uint32_t) params.m_mcs << "\t";
  outFile << params.m_size << "\t";
  outFile << (uint32_t) params.m_rv << "\t";
  outFile << (uint32_t) params.m_ndi << "\t";
  outFile << (uint32_t) params.m_correctness << "\t";
  outFile << (uint32_t) params.m_ccId <<std::endl;
  outFile.close ();
}

void
cv2x_PhyRxStatsCalculator::SlPhyReception (cv2x_PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (this << params.m_cellId << params.m_imsi << params.m_timestamp << params.m_rnti << params.m_layer << params.m_mcs << params.m_size << params.m_rv << params.m_ndi << params.m_correctness);
  NS_LOG_INFO ("Write SL Rx Phy Stats in " << GetSlRxOutputFilename ().c_str ());

  std::ofstream outFile;
  if ( m_slRxFirstWrite == true )
    {
      outFile.open (GetSlRxOutputFilename ().c_str ());
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetSlRxOutputFilename ().c_str ());
          return;
        }
      m_slRxFirstWrite = false;

      outFile.open (GetSlRxOutputFilename ().c_str (),  std::ios_base::app);
          NS_LOG_ERROR ("Can't open file " << GetSlRxOutputFilename ().c_str ());
    }
}

void
cv2x_PhyRxStatsCalculator::SlPscchReception (cv2x_PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (this << params.m_cellId << params.m_imsi << params.m_timestamp << params.m_rnti << params.m_layer << params.m_mcs << params.m_size << params.m_rv << params.m_ndi << params.m_correctness);
  NS_LOG_INFO ("Write SL Rx PSCCH Stats in " << GetSlPscchRxOutputFilename ().c_str ());

  std::ofstream outFile;
  if ( m_slPscchRxFirstWrite == true )
    {
      outFile.open (GetSlPscchRxOutputFilename ().c_str ());
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetSlPscchRxOutputFilename ().c_str ());
          return;
        }
      m_slPscchRxFirstWrite = false;
      outFile << "% time\tcellId\tIMSI\tRNTI\tlayer\tcorrect";
      //outFile << "% time\tcellId\tIMSI\tRNTI\tlayer\tmcs\tsize\trbStart\trbLen\tcorrect";

      outFile << std::endl;
    }
  else
    {
      outFile.open (GetSlPscchRxOutputFilename ().c_str (),  std::ios_base::app);
      if (!outFile.is_open ())
        {
          NS_LOG_ERROR ("Can't open file " << GetSlPscchRxOutputFilename ().c_str ());
          return;
        }
    }

//   outFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t";
  outFile << params.m_timestamp << "\t";
  outFile << (uint32_t) params.m_cellId << "\t";
  outFile << params.m_imsi << "\t";
  outFile << params.m_rnti << "\t";
  outFile << (uint32_t) params.m_layer << "\t";
  //outFile << (uint32_t) params.m_mcs << "\t";
  //outFile << params.m_size << "\t";
  //outFile << (uint32_t) params.m_rv << "\t"; // This is used for the rbStart
  //outFile << (uint32_t) params.m_ndi << "\t";// This is used for the rbLen
  outFile << (uint32_t) params.m_correctness << std::endl;
  outFile.close ();
}

void
cv2x_PhyRxStatsCalculator::DlPhyReceptionCallback (Ptr<cv2x_PhyRxStatsCalculator> phyRxStats,
                      std::string path, cv2x_PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (phyRxStats << path);
  uint64_t imsi = 0;
  std::ostringstream pathAndRnti;
  pathAndRnti << path << "/" << params.m_rnti;
  std::string pathUePhy  = path.substr (0, path.find ("/cv2x_ComponentCarrierMapUe"));
  if (phyRxStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      imsi = phyRxStats->GetImsiPath (pathAndRnti.str ());
    }
  else
    {
      imsi = FindImsiFromLteNetDevice (pathUePhy);
      phyRxStats->SetImsiPath (pathAndRnti.str (), imsi);
    }

  params.m_imsi = imsi;
  phyRxStats->DlPhyReception (params);
}

void
cv2x_PhyRxStatsCalculator::UlPhyReceptionCallback (Ptr<cv2x_PhyRxStatsCalculator> phyRxStats,
                      std::string path, cv2x_PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (phyRxStats << path);
  uint64_t imsi = 0;
  std::ostringstream pathAndRnti;
  std::string pathEnb  = path.substr (0, path.find ("/cv2x_ComponentCarrierMap"));
  pathAndRnti << pathEnb << "/cv2x_LteEnbRrc/UeMap/" << params.m_rnti;
  if (phyRxStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      imsi = phyRxStats->GetImsiPath (pathAndRnti.str ());
    }
  else
    {
      imsi = FindImsiFromEnbRlcPath (pathAndRnti.str ());
      phyRxStats->SetImsiPath (pathAndRnti.str (), imsi);
    }

  params.m_imsi = imsi;
  phyRxStats->UlPhyReception (params);
}

void
cv2x_PhyRxStatsCalculator::SlPhyReceptionCallback (Ptr<cv2x_PhyRxStatsCalculator> phyRxStats,
                      std::string path, cv2x_PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (phyRxStats << path);
  uint64_t imsi = 0;
  std::ostringstream pathAndRnti;
  pathAndRnti << path << "/" << params.m_rnti;
  if (phyRxStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      imsi = phyRxStats->GetImsiPath (pathAndRnti.str ());
    }
  else
    {
      imsi = FindImsiForUe (path, params.m_rnti);
      phyRxStats->SetImsiPath (pathAndRnti.str (), imsi);
    }

  params.m_imsi = imsi;
  phyRxStats->SlPhyReception (params);
}

void
cv2x_PhyRxStatsCalculator::SlPscchReceptionCallback (Ptr<cv2x_PhyRxStatsCalculator> phyRxStats,
                      std::string path, cv2x_PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (phyRxStats << path);
  uint64_t imsi = 0;
  std::ostringstream pathAndRnti;
  pathAndRnti << path << "/" << params.m_rnti;
  if (phyRxStats->ExistsImsiPath (pathAndRnti.str ()) == true)
    {
      imsi = phyRxStats->GetImsiPath (pathAndRnti.str ());
    }
  else
    {
      imsi = FindImsiForUe (path, params.m_rnti);
      phyRxStats->SetImsiPath (pathAndRnti.str (), imsi);
    }

  params.m_imsi = imsi;
  phyRxStats->SlPscchReception (params);
}

} // namespace ns3
