/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *                        Sourjya Dutta <sdutta@nyu.edu>
 *                        Russell Ford <russell.ford@nyu.edu>
 *                        Menglei Zhang <menglei@nyu.edu>
 */



#include <ns3/log.h>
#include "nr-phy-rx-trace.h"
#include <ns3/simulator.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-gnb-net-device.h>
#include <stdio.h>
#include <ns3/string.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrPhyRxTrace");

NS_OBJECT_ENSURE_REGISTERED (NrPhyRxTrace);

std::ofstream NrPhyRxTrace::m_dlDataSinrFile;
std::string NrPhyRxTrace::m_dlDataSinrFileName;

std::ofstream NrPhyRxTrace::m_dlCtrlSinrFile;
std::string NrPhyRxTrace::m_dlCtrlSinrFileName;

std::ofstream NrPhyRxTrace::m_rxPacketTraceFile;
std::string NrPhyRxTrace::m_rxPacketTraceFilename;
std::string NrPhyRxTrace::m_simTag;

std::ofstream NrPhyRxTrace::m_rxedGnbPhyCtrlMsgsFile;
std::string NrPhyRxTrace::m_rxedGnbPhyCtrlMsgsFileName;
std::ofstream NrPhyRxTrace::m_txedGnbPhyCtrlMsgsFile;
std::string NrPhyRxTrace::m_txedGnbPhyCtrlMsgsFileName;

std::ofstream NrPhyRxTrace::m_rxedUePhyCtrlMsgsFile;
std::string NrPhyRxTrace::m_rxedUePhyCtrlMsgsFileName;
std::ofstream NrPhyRxTrace::m_txedUePhyCtrlMsgsFile;
std::string NrPhyRxTrace::m_txedUePhyCtrlMsgsFileName;
std::ofstream NrPhyRxTrace::m_rxedUePhyDlDciFile;
std::string NrPhyRxTrace::m_rxedUePhyDlDciFileName;

std::ofstream NrPhyRxTrace::m_dlPathlossFile;
std::string NrPhyRxTrace::m_dlPathlossFileName;
std::ofstream NrPhyRxTrace::m_ulPathlossFile;
std::string NrPhyRxTrace::m_ulPathlossFileName;


NrPhyRxTrace::NrPhyRxTrace ()
{
}

NrPhyRxTrace::~NrPhyRxTrace ()
{
  if (m_dlDataSinrFile.is_open ())
    {
      m_dlDataSinrFile.close ();
    }

  if (m_dlCtrlSinrFile.is_open ())
    {
      m_dlCtrlSinrFile.close ();
    }

  if (m_rxPacketTraceFile.is_open ())
    {
      m_rxPacketTraceFile.close ();
    }

  if (m_rxedGnbPhyCtrlMsgsFile.is_open ())
    {
      m_rxedGnbPhyCtrlMsgsFile.close ();
    }

  if (m_txedGnbPhyCtrlMsgsFile.is_open ())
    {
      m_txedGnbPhyCtrlMsgsFile.close ();
    }

  if (m_rxedUePhyCtrlMsgsFile.is_open ())
    {
      m_rxedUePhyCtrlMsgsFile.close ();
    }

  if (m_txedUePhyCtrlMsgsFile.is_open ())
    {
      m_txedUePhyCtrlMsgsFile.close ();
    }

  if (m_rxedUePhyDlDciFile.is_open ())
    {
      m_rxedUePhyDlDciFile.close ();
    }

  if (m_dlPathlossFile.is_open ())
    {
      m_dlPathlossFile.close ();
    }

  if (m_ulPathlossFile.is_open ())
    {
      m_ulPathlossFile.close ();
    }
}

TypeId
NrPhyRxTrace::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrPhyRxTrace")
    .SetParent<Object> ()
    .AddConstructor<NrPhyRxTrace> ()
    .AddAttribute ("SimTag",
                   "simulation tag that will be concatenated to output file names"
                   "in order to distinguish them, for example: RxPacketTrace-${SimTag}.out. ",
                   StringValue (""),
                   MakeStringAccessor (&NrPhyRxTrace::SetSimTag),
                   MakeStringChecker ())
  ;
  return tid;
}

void
NrPhyRxTrace::SetSimTag (const std::string &simTag)
{
  m_simTag = simTag;
}

void
NrPhyRxTrace::DlDataSinrCallback ([[maybe_unused]]Ptr<NrPhyRxTrace> phyStats, [[maybe_unused]] std::string path,
                                  uint16_t cellId, uint16_t rnti, double avgSinr, uint16_t bwpId, uint8_t streamId)
{
  NS_LOG_INFO ("UE" << rnti << "of " << cellId << " over bwp ID " << bwpId << "->Generate RsrpSinrTrace");
  if (!m_dlDataSinrFile.is_open ())
      {
        std::ostringstream oss;
        oss << "DlDataSinr" << m_simTag.c_str () << ".txt";
        m_dlDataSinrFileName = oss.str ();
        m_dlDataSinrFile.open (m_dlDataSinrFileName.c_str ());

        m_dlDataSinrFile << "Time" << "\t" << "CellId" << "\t" << "RNTI" << "\t" << "BWPId"
                       << "\t" << "StreamId" << "\t" << "SINR(dB)" << std::endl;

        if (!m_dlDataSinrFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_dlDataSinrFile << Simulator::Now ().GetSeconds () <<
                    "\t" << cellId << "\t" << rnti << "\t" << bwpId <<
                    "\t" << +streamId << "\t" << 10 * log10 (avgSinr) << std::endl;
}


void
NrPhyRxTrace::DlCtrlSinrCallback ([[maybe_unused]] Ptr<NrPhyRxTrace> phyStats, [[maybe_unused]] std::string path,
                                  uint16_t cellId, uint16_t rnti, double avgSinr, uint16_t bwpId, uint8_t streamId)
{
  NS_LOG_INFO ("UE" << rnti << "of " << cellId << " over bwp ID " << bwpId << "->Generate RsrpSinrTrace");

  if (!m_dlCtrlSinrFile.is_open ())
      {
        std::ostringstream oss;
        oss << "DlCtrlSinr" << m_simTag.c_str () << ".txt";
        m_dlCtrlSinrFileName = oss.str ();
        m_dlCtrlSinrFile.open (m_dlCtrlSinrFileName.c_str ());

        m_dlCtrlSinrFile << "Time" << "\t" << "CellId" << "\t" << "RNTI" << "\t" << "BWPId"
                       << "\t" << "StreamId" << "\t" << "SINR(dB)" << std::endl;

        if (!m_dlCtrlSinrFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_dlCtrlSinrFile << Simulator::Now ().GetSeconds () <<
                    "\t" << cellId << "\t" << rnti << "\t" << bwpId <<
                    "\t" << +streamId << "\t" << 10 * log10 (avgSinr) << std::endl;
}

void
NrPhyRxTrace::UlSinrTraceCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                       uint64_t imsi, SpectrumValue& sinr, SpectrumValue& power)
{
  NS_LOG_INFO ("UE" << imsi << "->Generate UlSinrTrace");
  uint64_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  sprintf (fname, "UE_%llu_UL_SINR_dB.txt", (long long unsigned ) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = sinr.ValuesBegin ();
  while (it != sinr.ValuesEnd ())
    {
      //fprintf(log_file, "%d\t%d\t%f\t \n", tti_count/2, rb_count, 10*log10(*it));
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned )tti_count / 8 + 1, (long long unsigned )tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
  //phyStats->ReportInterferenceTrace (imsi, sinr);
  //phyStats->ReportPowerTrace (imsi, power);
}

void
NrPhyRxTrace::RxedGnbPhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                              SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                              uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  if (!m_rxedGnbPhyCtrlMsgsFile.is_open ())
      {
        std::ostringstream oss;
        oss << "RxedGnbPhyCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        m_rxedGnbPhyCtrlMsgsFileName = oss.str ();
        m_rxedGnbPhyCtrlMsgsFile.open (m_rxedGnbPhyCtrlMsgsFileName.c_str ());

        m_rxedGnbPhyCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" <<
                                    "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                    "\t" << "nodeId" << "\t" << "RNTI" <<
                                    "\t" << "bwpId" << "\t" << "MsgType" << std::endl;

        if (!m_rxedGnbPhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedGnbPhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                              "\t" << "ENB PHY Rxed" << "\t" << sfn.GetFrame () <<
                              "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                              "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                              "\t" << nodeId << "\t" << rnti <<
                              "\t" << static_cast<uint32_t> (bwpId) << "\t";


  if (msg->GetMessageType () == NrControlMessage::DL_CQI)
    {
      m_rxedGnbPhyCtrlMsgsFile << "DL_CQI";
    }
  else if (msg->GetMessageType () == NrControlMessage::SR)
    {
      m_rxedGnbPhyCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == NrControlMessage::BSR)
    {
      m_rxedGnbPhyCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == NrControlMessage::RACH_PREAMBLE)
    {
      m_rxedGnbPhyCtrlMsgsFile << "RACH_PREAMBLE";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_HARQ)
    {
      m_rxedGnbPhyCtrlMsgsFile << "DL_HARQ";
    }
  else if (msg->GetMessageType () == NrControlMessage::SRS)
    {
      m_rxedGnbPhyCtrlMsgsFile << "SRS";
    }
  else
    {
      m_rxedGnbPhyCtrlMsgsFile << "Other";
    }
  m_rxedGnbPhyCtrlMsgsFile << std::endl;
}

void
NrPhyRxTrace::TxedGnbPhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                              SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                              uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  if (!m_txedGnbPhyCtrlMsgsFile.is_open ())
      {
        std::ostringstream oss;
        oss << "TxedGnbPhyCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        m_txedGnbPhyCtrlMsgsFileName = oss.str ();
        m_txedGnbPhyCtrlMsgsFile.open (m_txedGnbPhyCtrlMsgsFileName.c_str ());

        m_txedGnbPhyCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                    "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                    "\t" << "nodeId" << "\t" << "RNTI"<<
                                    "\t" << "bwpId" << "\t" << "MsgType" << std::endl;

        if (!m_txedGnbPhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedGnbPhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                              "\t" << "ENB PHY Txed" << "\t" << sfn.GetFrame () <<
                              "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                              "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                              "\t" << nodeId << "\t" << rnti <<
                              "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::MIB)
    {
      m_txedGnbPhyCtrlMsgsFile << "MIB";
    }
  else if (msg->GetMessageType () == NrControlMessage::SIB1)
    {
      m_txedGnbPhyCtrlMsgsFile << "SIB1";
    }
  else if (msg->GetMessageType () == NrControlMessage::RAR)
    {
      m_txedGnbPhyCtrlMsgsFile << "RAR";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_DCI)
    {
      m_txedGnbPhyCtrlMsgsFile << "DL_DCI";
    }
  else if (msg->GetMessageType () == NrControlMessage::UL_DCI)
    {
      m_txedGnbPhyCtrlMsgsFile << "UL_UCI";
    }
  else
    {
      m_txedGnbPhyCtrlMsgsFile << "Other";
    }
  m_txedGnbPhyCtrlMsgsFile << std::endl;
}

void
NrPhyRxTrace::RxedUePhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                             SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                             uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  if (!m_rxedUePhyCtrlMsgsFile.is_open ())
      {
        std::ostringstream oss;
        oss << "RxedUePhyCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        m_rxedUePhyCtrlMsgsFileName = oss.str ();
        m_rxedUePhyCtrlMsgsFile.open (m_rxedUePhyCtrlMsgsFileName.c_str ());

        m_rxedUePhyCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                   "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                   "\t" << "nodeId" << "\t" << "RNTI" <<
                                   "\t" << "bwpId" << "\t" << "MsgType" << std::endl;

        if (!m_rxedUePhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                             "\t" << "UE  PHY Rxed" << "\t" << sfn.GetFrame ()<<
                             "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                             "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                             "\t" << nodeId << "\t" << rnti <<
                             "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::UL_DCI)
    {
      m_rxedUePhyCtrlMsgsFile << "UL_DCI";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_DCI)
    {
      m_rxedUePhyCtrlMsgsFile << "DL_DCI";
    }
  else if (msg->GetMessageType () == NrControlMessage::MIB)
    {
      m_rxedUePhyCtrlMsgsFile << "MIB";
    }
  else if (msg->GetMessageType () == NrControlMessage::SIB1)
    {
      m_rxedUePhyCtrlMsgsFile << "SIB1";
    }
  else if (msg->GetMessageType () == NrControlMessage::RAR)
    {
      m_rxedUePhyCtrlMsgsFile << "RAR";
    }
  else
    {
      m_rxedUePhyCtrlMsgsFile << "Other";
    }
  m_rxedUePhyCtrlMsgsFile << std::endl;
}

void
NrPhyRxTrace::TxedUePhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                             SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                             uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  if (!m_txedUePhyCtrlMsgsFile.is_open ())
      {
        std::ostringstream oss;
        oss << "TxedUePhyCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        m_txedUePhyCtrlMsgsFileName = oss.str ();
        m_txedUePhyCtrlMsgsFile.open (m_txedUePhyCtrlMsgsFileName.c_str ());

        m_txedUePhyCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                   "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                   "\t" << "nodeId" <<
                                   "\t" << "RNTI" << "\t" << "bwpId" <<
                                   "\t" << "MsgType" << std::endl;

        if (!m_txedUePhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedUePhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                             "\t" << "UE  PHY Txed" << "\t" << sfn.GetFrame () <<
                             "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                             "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                             "\t" << nodeId << "\t" << rnti <<
                             "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::RACH_PREAMBLE)
    {
      m_txedUePhyCtrlMsgsFile << "RACH_PREAMBLE";
    }
  else if (msg->GetMessageType () == NrControlMessage::SR)
    {
      m_txedUePhyCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == NrControlMessage::BSR)
    {
      m_txedUePhyCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_CQI)
    {
      m_txedUePhyCtrlMsgsFile << "DL_CQI";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_HARQ)
    {
      m_txedUePhyCtrlMsgsFile << "DL_HARQ";
    }
  else if (msg->GetMessageType () == NrControlMessage::SRS)
    {
      m_txedUePhyCtrlMsgsFile << "SRS";
    }
  else
    {
      m_txedUePhyCtrlMsgsFile << "Other";
    }
  m_txedUePhyCtrlMsgsFile << std::endl;
}

void
NrPhyRxTrace::RxedUePhyDlDciCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                          SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                          uint8_t bwpId, uint8_t harqId, uint32_t k1Delay)
{
  if (!m_rxedUePhyDlDciFile.is_open ())
      {
        std::ostringstream oss;
        oss << "RxedUePhyDlDciTrace" << m_simTag.c_str () << ".txt";
        m_rxedUePhyDlDciFileName = oss.str ();
        m_rxedUePhyDlDciFile.open (m_rxedUePhyDlDciFileName.c_str ());

        m_rxedUePhyDlDciFile << "Time" << "\t" << "Entity"  << "\t" << "Frame" <<
                                "\t" << "SF" << "\t" << "Slot" << "\t" <<
                                "nodeId" << "\t" << "RNTI" << "\t" << "bwpId" <<
                                "\t" << "Harq ID" << "\t" << "K1 Delay" << std::endl;

        if (!m_rxedUePhyDlDciFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyDlDciFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                          "\t" << "DL DCI Rxed" << "\t" << sfn.GetFrame () <<
                          "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                          "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                          "\t" << nodeId << "\t" << rnti << "\t" <<
                          static_cast<uint32_t> (bwpId) << "\t" <<
                          static_cast<uint32_t> (harqId) << "\t" <<
                          k1Delay << std::endl;
}

void
NrPhyRxTrace::TxedUePhyHarqFeedbackCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                             SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                             uint8_t bwpId, uint8_t harqId, uint32_t k1Delay)
{
  if (!m_rxedUePhyDlDciFile.is_open ())
      {
        std::ostringstream oss;
        oss << "RxedUePhyDlDciTrace" << m_simTag.c_str () << ".txt";
        m_rxedUePhyDlDciFileName = oss.str ();
        m_rxedUePhyDlDciFile.open (m_rxedUePhyDlDciFileName.c_str ());

        m_rxedUePhyDlDciFile << "Time" << "\t" << "Entity"  << "\t" << "Frame" <<
                                "\t" << "SF" << "\t" << "Slot" << "\t" <<
                                "nodeId" << "\t" << "RNTI" << "\t" << "bwpId" <<
                                "\t" << "Harq ID" << "\t" << "K1 Delay" << std::endl;

        if (!m_rxedUePhyDlDciFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyDlDciFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                          "\t" << "HARQ FD Txed" << "\t" << sfn.GetFrame () <<
                          "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                          "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                          "\t" << nodeId << "\t" << rnti << "\t" <<
                          static_cast<uint32_t> (bwpId) << "\t" <<
                          static_cast<uint32_t> (harqId) << "\t" <<
                          k1Delay << std::endl;
}

void
NrPhyRxTrace::ReportInterferenceTrace (uint64_t imsi, SpectrumValue& sinr)
{
  uint64_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  sprintf (fname, "UE_%llu_SINR_dB.txt", (long long unsigned ) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = sinr.ValuesBegin ();
  while (it != sinr.ValuesEnd ())
    {
      //fprintf(log_file, "%d\t%d\t%f\t \n", tti_count/2, rb_count, 10*log10(*it));
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned) tti_count / 8 + 1, (long long unsigned) tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
}

void
NrPhyRxTrace::ReportPowerTrace (uint64_t imsi, SpectrumValue& power)
{

  uint32_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  printf (fname, "UE_%llu_ReceivedPower_dB.txt", (long long unsigned) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = power.ValuesBegin ();
  while (it != power.ValuesEnd ())
    {
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned) tti_count / 8 + 1,(long long unsigned) tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
}

void
NrPhyRxTrace::ReportPacketCountUeCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                               UePhyPacketCountParameter param)
{
  phyStats->ReportPacketCountUe (param);
}
void
NrPhyRxTrace::ReportPacketCountEnbCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                                GnbPhyPacketCountParameter param)
{
  phyStats->ReportPacketCountEnb (param);
}

void
NrPhyRxTrace::ReportDownLinkTBSize (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                        uint64_t imsi, uint64_t tbSize)
{
  phyStats->ReportDLTbSize (imsi, tbSize);
}



void
NrPhyRxTrace::ReportPacketCountUe (UePhyPacketCountParameter param)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"UE_%llu_Packet_Trace.txt", (long long unsigned) param.m_imsi);
  log_file = fopen (fname, "a");
  if (param.m_isTx)
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, param.m_noBytes, 0);
    }
  else
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, 0, param.m_noBytes);
    }

  fflush (log_file);
  fclose (log_file);

}

void
NrPhyRxTrace::ReportPacketCountEnb (GnbPhyPacketCountParameter param)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"BS_%llu_Packet_Trace.txt",(long long unsigned) param.m_cellId);
  log_file = fopen (fname, "a");
  if (param.m_isTx)
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, param.m_noBytes, 0);
    }
  else
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, 0, param.m_noBytes);
    }

  fflush (log_file);
  fclose (log_file);
}

void
NrPhyRxTrace::ReportDLTbSize (uint64_t imsi, uint64_t tbSize)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"UE_%llu_Tb_Size.txt", (long long unsigned) imsi);
  log_file = fopen (fname, "a");

  fprintf (log_file, "%llu \t %llu\n", (long long unsigned )Now ().GetMicroSeconds (), (long long unsigned )tbSize);
  fprintf (log_file, "%lld \t %llu \n",(long long int) Now ().GetMicroSeconds (), (long long unsigned) tbSize);
  fflush (log_file);
  fclose (log_file);
}

void
NrPhyRxTrace::RxPacketTraceUeCallback (Ptr<NrPhyRxTrace> phyStats, std::string path, RxPacketTraceParams params)
{
  if (!m_rxPacketTraceFile.is_open ())
    {
      std::ostringstream oss;
      oss << "RxPacketTrace" << m_simTag.c_str() << ".txt";
      m_rxPacketTraceFilename = oss.str ();
      m_rxPacketTraceFile.open (m_rxPacketTraceFilename.c_str ());

      m_rxPacketTraceFile << "Time" << "\t" << "direction" << "\t" <<
                             "frame" << "\t" << "subF" << "\t" << "slot" <<
                             "\t" << "1stSym" << "\t" << "nSymbol" <<
                             "\t" << "cellId" << "\t" << "bwpId" <<
                             "\t" << "streamId" << "\t" << "rnti" <<
                             "\t" << "tbSize" << "\t" << "mcs" <<
                             "\t" << "rv" << "\t" << "SINR(dB)" << "\t" << "CQI" <<
                             "\t" << "corrupt" << "\t" << "TBler" << std::endl;

      if (!m_rxPacketTraceFile.is_open ())
        {
          NS_FATAL_ERROR ("Could not open tracefile");
        }
    }

  m_rxPacketTraceFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                         "\t" << "DL" <<
                         "\t" << params.m_frameNum <<
                         "\t" << (unsigned)params.m_subframeNum <<
                         "\t" << (unsigned)params.m_slotNum <<
                         "\t" << (unsigned)params.m_symStart <<
                         "\t" << (unsigned)params.m_numSym <<
                         "\t" << params.m_cellId <<
                         "\t" << (unsigned)params.m_bwpId <<
                         "\t" << static_cast<uint16_t> (params.m_streamId) <<
                         "\t" << params.m_rnti <<
                         "\t" << params.m_tbSize <<
                         "\t" << (unsigned)params.m_mcs <<
                         "\t" << (unsigned)params.m_rv <<
                         "\t" << 10 * log10 (params.m_sinr) <<
                         "\t" << (unsigned)params.m_cqi <<
                         "\t" << params.m_corrupt <<
                         "\t" << params.m_tbler << std::endl;

  if (params.m_corrupt)
    {
      NS_LOG_DEBUG ("DL TB error\t" << params.m_frameNum <<
                    "\t" << (unsigned)params.m_subframeNum <<
                    "\t" << (unsigned)params.m_slotNum <<
                    "\t" << (unsigned)params.m_symStart <<
                    "\t" << (unsigned)params.m_numSym <<
                    "\t" << params.m_rnti <<
                    "\t" << params.m_tbSize <<
                    "\t" << (unsigned)params.m_mcs <<
                    "\t" << (unsigned)params.m_rv <<
                    "\t" << params.m_sinr <<
                    "\t" << (unsigned)params.m_cqi <<
                    "\t" << params.m_tbler <<
                    "\t" << params.m_corrupt <<
                    "\t" << (unsigned)params.m_bwpId);
    }
}
void
NrPhyRxTrace::RxPacketTraceEnbCallback (Ptr<NrPhyRxTrace> phyStats, std::string path, RxPacketTraceParams params)
{
  if (!m_rxPacketTraceFile.is_open ())
    {
      std::ostringstream oss;
      oss << "RxPacketTrace" << m_simTag.c_str () << ".txt";
      m_rxPacketTraceFilename = oss.str ();
      m_rxPacketTraceFile.open (m_rxPacketTraceFilename.c_str ());

      m_rxPacketTraceFile << "Time" << "\t" << "direction" << "\t" <<
                             "frame" << "\t" << "subF" << "\t" << "slot" <<
                             "\t" << "1stSym" << "\t" << "nSymbol" <<
                             "\t" << "cellId" << "\t" << "bwpId" <<
                             "\t" << "streamId" << "\t" << "rnti" <<
                             "\t" << "tbSize" << "\t" << "mcs" <<
                             "\t" << "rv" << "\t" << "SINR(dB)" <<
                             "\t" << "corrupt" << "\t" << "TBler" << std::endl;

      if (!m_rxPacketTraceFile.is_open ())
        {
          NS_FATAL_ERROR ("Could not open tracefile");
        }
    }
  m_rxPacketTraceFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                         "\t" << "UL" <<
                         "\t" << params.m_frameNum <<
                         "\t" << (unsigned)params.m_subframeNum <<
                         "\t" << (unsigned)params.m_slotNum <<
                         "\t" << (unsigned)params.m_symStart <<
                         "\t" << (unsigned)params.m_numSym <<
                         "\t" << params.m_cellId <<
                         "\t" << (unsigned)params.m_bwpId <<
                         "\t" << static_cast<uint16_t> (params.m_streamId) <<
                         "\t" << params.m_rnti <<
                         "\t" << params.m_tbSize <<
                         "\t" << (unsigned)params.m_mcs <<
                         "\t" << (unsigned)params.m_rv <<
                         "\t" << 10 * log10 (params.m_sinr) <<
                         "\t" << params.m_corrupt <<
                         "\t" << params.m_tbler << std::endl;

  if (params.m_corrupt)
    {
      NS_LOG_DEBUG ("UL TB error\t" << params.m_frameNum <<
                    "\t" << (unsigned)params.m_subframeNum <<
                    "\t" << (unsigned)params.m_slotNum <<
                    "\t" << (unsigned)params.m_symStart <<
                    "\t" << (unsigned)params.m_numSym <<
                    "\t" << params.m_rnti <<
                    "\t" << params.m_tbSize <<
                    "\t" << (unsigned)params.m_mcs <<
                    "\t" << (unsigned)params.m_rv <<
                    "\t" << params.m_sinr <<
                    "\t" << params.m_tbler <<
                    "\t" << params.m_corrupt <<
                    "\t" << params.m_sinrMin <<
                    "\t" << params.m_bwpId);
    }
}

void
NrPhyRxTrace::PathlossTraceCallback (Ptr<NrPhyRxTrace> phyStats,
                                     std::string path,
                                     Ptr<const SpectrumPhy> txPhy,
                                     Ptr<const SpectrumPhy> rxPhy,
                                     double lossDb)
{
  Ptr<NrSpectrumPhy> txNrSpectrumPhy = txPhy->GetObject <NrSpectrumPhy> ();
  Ptr<NrSpectrumPhy> rxNrSpectrumPhy = rxPhy->GetObject <NrSpectrumPhy> ();
  if (DynamicCast<NrGnbNetDevice> (txNrSpectrumPhy->GetDevice ()) != nullptr)
    {
      // All the gNBs are on the same channel (especially in TDD), therefore,
      // to log real DL reception we need to make sure that the RX spectrum
      // belongs to an UE.
      if (DynamicCast<NrUeNetDevice> (rxNrSpectrumPhy->GetDevice ()) != nullptr)
        {
          uint16_t txCellId = txNrSpectrumPhy->GetDevice ()->GetObject<NrGnbNetDevice> ()->GetCellId ();
          uint16_t rxCellId = rxNrSpectrumPhy->GetDevice ()->GetObject<NrUeNetDevice> ()->GetCellId ();
          uint16_t txBwpId = txNrSpectrumPhy->GetBwpId ();
          uint16_t rxBwpId = rxNrSpectrumPhy->GetBwpId ();
          uint16_t txStreamId = txNrSpectrumPhy->GetStreamId ();
          uint16_t rxStreamId = rxNrSpectrumPhy->GetStreamId ();

          if (txCellId == rxCellId && txBwpId == rxBwpId && txStreamId == rxStreamId)
            {
              // We multiply loss values with -1 to get the notion of loss
              // instead of a gain.
              phyStats->WriteDlPathlossTrace (txNrSpectrumPhy, rxNrSpectrumPhy, lossDb * -1);
            }
        }
    }
  else
    {
      // All the UEs are on the same channel (especially in TDD), therefore,
      // to log real UL reception we need to make sure that the RX spectrum
      // belongs to a gNB.
      if (DynamicCast<NrGnbNetDevice> (rxNrSpectrumPhy->GetDevice ()) != nullptr)
        {
          uint16_t txCellId = txNrSpectrumPhy->GetDevice ()->GetObject<NrUeNetDevice> ()->GetCellId ();
          uint16_t rxCellId = rxNrSpectrumPhy->GetDevice ()->GetObject<NrGnbNetDevice> ()->GetCellId ();
          uint16_t txBwpId = txNrSpectrumPhy->GetBwpId ();
          uint16_t rxBwpId = rxNrSpectrumPhy->GetBwpId ();
          uint16_t txStreamId = txNrSpectrumPhy->GetStreamId ();
          uint16_t rxStreamId = rxNrSpectrumPhy->GetStreamId ();
          if (txCellId == rxCellId && txBwpId == rxBwpId && txStreamId == rxStreamId)
            {
              // We multiply loss values with -1 to get the notion of loss
              // instead of a gain.
              phyStats->WriteUlPathlossTrace (txNrSpectrumPhy, rxNrSpectrumPhy, lossDb * -1);
            }
        }
    }
}

void
NrPhyRxTrace::WriteDlPathlossTrace (Ptr<NrSpectrumPhy> txNrSpectrumPhy,
                                    Ptr<NrSpectrumPhy> rxNrSpectrumPhy,
                                    double lossDb)
{
  if (!m_dlPathlossFile.is_open ())
      {
        std::ostringstream oss;
        oss << "DlPathlossTrace" << m_simTag.c_str () << ".txt";
        m_dlPathlossFileName = oss.str ();
        m_dlPathlossFile.open (m_dlPathlossFileName.c_str ());

        m_dlPathlossFile << "Time(sec)" << "\t" << "CellId" << "\t"
                         << "BwpId" << "\t"  << "txStreamId "<< "\t"
                         << "IMSI" << "\t" << "rxStreamId" << "\t"
                         << "pathLoss(dB)" << std::endl;

        if (!m_dlPathlossFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open DL pathloss tracefile");
          }
      }

  m_dlPathlossFile << Simulator::Now ().GetSeconds () << "\t"
                   << txNrSpectrumPhy->GetDevice ()->GetObject<NrGnbNetDevice> ()->GetCellId () << "\t"
                   << txNrSpectrumPhy->GetBwpId () << "\t"
                   << +txNrSpectrumPhy->GetStreamId () << "\t"
                   << rxNrSpectrumPhy->GetDevice ()->GetObject<NrUeNetDevice> ()->GetImsi () << "\t"
                   << +rxNrSpectrumPhy->GetStreamId () << "\t"
                   << lossDb << std::endl;

}

void
NrPhyRxTrace::WriteUlPathlossTrace (Ptr<NrSpectrumPhy> txNrSpectrumPhy,
                                    Ptr<NrSpectrumPhy> rxNrSpectrumPhy,
                                    double lossDb)
{
  if (!m_ulPathlossFile.is_open ())
      {
        std::ostringstream oss;
        oss << "UlPathlossTrace" << m_simTag.c_str () << ".txt";
        m_ulPathlossFileName = oss.str ();
        m_ulPathlossFile.open (m_ulPathlossFileName.c_str ());

        m_ulPathlossFile << "Time(sec)" << "\t" << "CellId" << "\t"
                         << "BwpId" << "\t"  << "txStreamId "<< "\t"
                         << "IMSI" << "\t" << "rxStreamId" << "\t"
                         << "pathLoss(dB)" << std::endl;

        if (!m_ulPathlossFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open UL pathloss tracefile");
          }
      }

  m_ulPathlossFile << Simulator::Now ().GetSeconds () << "\t"
                   << txNrSpectrumPhy->GetDevice ()->GetObject<NrUeNetDevice> ()->GetCellId () << "\t"
                   << txNrSpectrumPhy->GetBwpId () << "\t"
                   << +txNrSpectrumPhy->GetStreamId () << "\t"
                   << txNrSpectrumPhy->GetDevice ()->GetObject<NrUeNetDevice> ()->GetImsi () << "\t"
                   << +rxNrSpectrumPhy->GetStreamId () << "\t"
                   << lossDb << std::endl;
}

} /* namespace ns3 */
