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
*                         Sourjya Dutta <sdutta@nyu.edu>
*                         Russell Ford <russell.ford@nyu.edu>
*                         Menglei Zhang <menglei@nyu.edu>
*/


#ifndef SRC_NR_HELPER_NR_PHY_RX_TRACE_H_
#define SRC_NR_HELPER_NR_PHY_RX_TRACE_H_

#include <ns3/object.h>
#include <ns3/spectrum-value.h>
#include <ns3/nr-phy-mac-common.h>
#include <ns3/nr-control-messages.h>
#include <ns3/nr-spectrum-phy.h>
#include <ns3/spectrum-phy.h>
#include <fstream>
#include <iostream>

namespace ns3 {

class NrPhyRxTrace : public Object
{
public:
  NrPhyRxTrace ();
  virtual ~NrPhyRxTrace ();
  static TypeId GetTypeId (void);

  /**
   * \brief Set simTag that will be contatenated to
   * output file names
   * \param simTag string to be used as simulation tag
   */
  void SetSimTag (const std::string &simTag);

  /**
   * \brief Trace sink for DL Average SINR of DATA (in dB).
   * \param [in] phyStats NrPhyRxTrace object
   * \param [in] path context path
   * \param [in] cellId the cell ID
   * \param [in] rnti the RNTI
   * \param [in] avgSinr the average SINR
   * \param [in] bwpId the BWP ID
   * \param [in] streamId the stream ID
   */
  static void DlDataSinrCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                  uint16_t cellId, uint16_t rnti, double avgSinr, uint16_t bwpId, uint8_t streamId);

  /**
   * \brief Trace sink for DL Average SINR of CTRL (in dB).
   * \param [in] phyStats NrPhyRxTrace object
   * \param [in] path context path
   * \param [in] cellId the cell ID
   * \param [in] rnti the RNTI
   * \param [in] avgSinr the average SINR
   * \param [in] bwpId the BWP ID
   * \param [in] streamId the stream ID
   */
  static void DlCtrlSinrCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                 uint16_t cellId, uint16_t rnti, double avgSinr, uint16_t bwpId, uint8_t streamId);

  static void UlSinrTraceCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                   uint64_t imsi, SpectrumValue& sinr, SpectrumValue& power);
  static void ReportPacketCountUeCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                           UePhyPacketCountParameter param);
  static void ReportPacketCountEnbCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                            GnbPhyPacketCountParameter param);
  static void ReportDownLinkTBSize (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                    uint64_t imsi, uint64_t tbSize);
  static void RxPacketTraceUeCallback (Ptr<NrPhyRxTrace> phyStats, std::string path, RxPacketTraceParams param);
  static void RxPacketTraceEnbCallback (Ptr<NrPhyRxTrace> phyStats, std::string path, RxPacketTraceParams param);

  /**
   *  Trace sink for Enb Phy Received Control Messages.
   *
   * \param [in] frame Frame number
   * \param [in] subframe Subframe number
   * \param [in] slot number
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  static void RxedGnbPhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                          SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                          uint8_t bwpId, Ptr<const NrControlMessage> msg);

  /**
   *  Trace sink for Enb Phy Transmitted Control Messages.
   *
   * \param [in] frame Frame number
   * \param [in] subframe Subframe number
   * \param [in] slot number
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  static void TxedGnbPhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                          SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                          uint8_t bwpId, Ptr<const NrControlMessage> msg);

  /**
   *  Trace sink for Ue Phy Received Control Messages.
   *
   * \param [in] frame Frame number
   * \param [in] subframe Subframe number
   * \param [in] slot number
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  static void RxedUePhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                         SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                         uint8_t bwpId, Ptr<const NrControlMessage> msg);

  /**
   *  Trace sink for Ue Phy Transmitted Control Messages.
   *
   * \param [in] frame Frame number
   * \param [in] subframe Subframe number
   * \param [in] slot number
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  static void TxedUePhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                         SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                         uint8_t bwpId, Ptr<const NrControlMessage> msg);

  /**
   *  Trace sink for Ue Phy Received Control Messages.
   *
   * \param [in] frame Frame number
   * \param [in] subframe Subframe number
   * \param [in] slot number
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] harq Id
   * \param [in] k1 delay
   */
  static void RxedUePhyDlDciCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                      SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                      uint8_t bwpId, uint8_t harqId, uint32_t k1Delay);
  /**
   *  Trace sink for Ue Phy Received Control Messages.
   *
   * \param [in] frame Frame number
   * \param [in] subframe Subframe number
   * \param [in] slot number
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] harq Id
   * \param [in] k1 delay
   */
  static void TxedUePhyHarqFeedbackCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                             SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                             uint8_t bwpId, uint8_t harqId, uint32_t k1Delay);
  /**
   * \brief Trace sink for spectrum channel pathloss trace
   *
   * \param [in] phyStats Pointer to NrPhyRxTrace API
   * \param [in] path The context of the trace path
   * \param [in] txPhy The TX SpectrumPhy instance
   * \param [in] rxPhy The RX SpectrumPhy instance
   * \param [in] lossDb The loss value in dB
   */
  static void PathlossTraceCallback (Ptr<NrPhyRxTrace> phyStats,
                                     std::string path,
                                     Ptr<const SpectrumPhy> txPhy,
                                     Ptr<const SpectrumPhy> rxPhy,
                                     double lossDb);

private:
  void ReportInterferenceTrace (uint64_t imsi, SpectrumValue& sinr);
  void ReportPowerTrace (uint64_t imsi, SpectrumValue& power);
  void ReportPacketCountUe (UePhyPacketCountParameter param);
  void ReportPacketCountEnb (GnbPhyPacketCountParameter param);
  void ReportDLTbSize (uint64_t imsi, uint64_t tbSize);
  /**
   * \brief Write DL pathloss values in a file
   *
   * \param [in] txNrSpectrumPhy The TX NrSpectrumPhy instance
   * \param [in] rxNrSpectrumPhy The RX NrSpectrumPhy instance
   * \param [in] lossDb The loss value in dB
   */
  void WriteDlPathlossTrace (Ptr<NrSpectrumPhy> txNrSpectrumPhy,
                             Ptr<NrSpectrumPhy> rxNrSpectrumPhy,
                             double lossDb);
  /**
   * \brief Write UL pathloss values in a file
   *
   * \param [in] txNrSpectrumPhy The TX NrSpectrumPhy instance
   * \param [in] rxNrSpectrumPhy The RX NrSpectrumPhy instance
   * \param [in] lossDb The loss value in dB
   */
  void WriteUlPathlossTrace (Ptr<NrSpectrumPhy> txNrSpectrumPhy,
                             Ptr<NrSpectrumPhy> rxNrSpectrumPhy,
                             double lossDb);


  static std::string m_simTag;   //!< The `SimTag` attribute.

  static std::ofstream m_dlDataSinrFile;
  static std::string m_dlDataSinrFileName;

  static std::ofstream m_dlCtrlSinrFile;
  static std::string m_dlCtrlSinrFileName;

  static std::ofstream m_rxPacketTraceFile;
  static std::string m_rxPacketTraceFilename;

  static std::ofstream m_rxedGnbPhyCtrlMsgsFile;
  static std::string m_rxedGnbPhyCtrlMsgsFileName;
  static std::ofstream m_txedGnbPhyCtrlMsgsFile;
  static std::string m_txedGnbPhyCtrlMsgsFileName;

  static std::ofstream m_rxedUePhyCtrlMsgsFile;
  static std::string m_rxedUePhyCtrlMsgsFileName;
  static std::ofstream m_txedUePhyCtrlMsgsFile;
  static std::string m_txedUePhyCtrlMsgsFileName;
  static std::ofstream m_rxedUePhyDlDciFile;
  static std::string m_rxedUePhyDlDciFileName;
  static std::ofstream m_dlPathlossFile;
  static std::string m_dlPathlossFileName;
  static std::ofstream m_ulPathlossFile;
  static std::string m_ulPathlossFileName;
};

} /* namespace ns3 */

#endif /* SRC_NR_HELPER_NR_PHY_RX_TRACE_H_ */
