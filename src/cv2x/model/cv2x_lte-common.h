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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 * Modified by: NIST (D2D)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */
#ifndef CV2X_LTE_COMMON_H
#define CV2X_LTE_COMMON_H

#include "ns3/uinteger.h"
#include <cmath>

// see 36.213 section 8
#define UL_PUSCH_TTIS_DELAY 4

#define HARQ_PERIOD 7

namespace ns3 {


/// LteFlowId structure
struct cv2x_LteFlowId_t
{
  uint16_t  m_rnti; ///< RNTI
  uint8_t   m_lcId; ///< LCID

public:
  cv2x_LteFlowId_t ();
  /**
   * Constructor
   *
   * \param a RNTI
   * \param b LCID
   */
  cv2x_LteFlowId_t (const uint16_t a, const uint8_t b);

  friend bool operator == (const cv2x_LteFlowId_t &a, const cv2x_LteFlowId_t &b);
  friend bool operator < (const cv2x_LteFlowId_t &a, const cv2x_LteFlowId_t &b);
};

/// ImsiLcidPair structure
struct cv2x_ImsiLcidPair_t
{
  uint64_t  m_imsi; ///< IMSI
  uint8_t   m_lcId; ///< LCID

public:
  cv2x_ImsiLcidPair_t ();
  /**
   * Constructor
   *
   * \param a first pair
   * \param b second pair
   */
  cv2x_ImsiLcidPair_t (const uint64_t a, const uint8_t b);

  friend bool operator == (const cv2x_ImsiLcidPair_t &a, const cv2x_ImsiLcidPair_t &b);
  friend bool operator < (const cv2x_ImsiLcidPair_t &a, const cv2x_ImsiLcidPair_t &b);
};

/**
* \brief Parameters for configuring the UE 
*/
struct cv2x_LteUeConfig_t
{
  /**
  * RNTI
  */
  uint16_t  m_rnti;
  /**
  * When false means that the message is intended for configuring a new UE
  */
  bool      m_reconfigureFlag;
  /**
  * Transmission mode [1..7] (i.e., SISO, MIMO, etc.)
  */
  uint8_t   m_transmissionMode;
  /**
  * Srs Configuration index for UE specific SRS, see section 8.2 of TS 36.213
  */
  uint16_t  m_srsConfigurationIndex;

public:
  cv2x_LteUeConfig_t ();

  /**
   * Equality operator
   *
   * \param a lhs
   * \param b rhs
   * \returns true if "equal"
   */
  friend bool operator == (const cv2x_LteUeConfig_t &a, const cv2x_LteUeConfig_t &b);
  /**
   * Less than operator
   *
   * \param a lhs
   * \param b rhs
   * \returns true if "less than"
   */
  friend bool operator < (const cv2x_LteUeConfig_t &a, const cv2x_LteUeConfig_t &b);
};

/// cv2x_LteFfConverter class
class cv2x_LteFfConverter
{

public:
  /**
   * Convert from double to fixed point S11.3 notation
   * 
   * \param val double value
   * \returns fixed point S11.3 value
   */
  static uint16_t double2fpS11dot3 (double val);
  /**
   * Convert from fixed point S11.3 notation to double
   *
   * \param val fixed point S11.3 value
   * \returns double value
   */ 
  static double fpS11dot3toDouble (uint16_t val);
  /**
   * Get minimum fixed point S11.3 value
   *
   * \returns double value for the minimum fixed point S11.3 value
   */
  static double getMinFpS11dot3Value ();

  //static const double MIN_FP_S11DOT3_VALUE = -4096;

};

/// cv2x_BufferSizeLevelBsr class
class cv2x_BufferSizeLevelBsr
{

public:
  /**
   * Convert BSR ID to buffer size
   *
   * \param val BSR ID
   * \returns buffer size
   */
  static uint32_t BsrId2BufferSize (uint8_t val);
  /**
   * Convert Buffer size to BSR ID
   *
   * \param val buffer size
   * \returns BSR ID
   */
  static uint8_t BufferSize2BsrId (uint32_t val);

  static int  m_bufferSizeLevelBsr[64]; ///< buffer size level BSR

};

/// cv2x_TransmissionModesLayers class
class cv2x_TransmissionModesLayers
{
public:
  /**
   * Transmit mode 2 layer number
   * \param txMode the transmit mode
   * \returns the layer 2 number
   */
  static uint8_t TxMode2LayerNum (uint8_t txMode);
};


/// cv2x_PhyTransmissionStatParameters structure
struct cv2x_PhyTransmissionStatParameters
{
  int64_t  m_timestamp; ///< in millisecond
  uint16_t m_cellId;  ///< Cell ID of the attached Enb
  uint64_t m_imsi;    ///< IMSI of the scheduled UE
  uint16_t m_rnti;    ///< C-RNTI scheduled
  uint8_t  m_txMode;  ///< the transmission Mode
  uint8_t  m_layer;   ///< the layer (cw) of the transmission
  uint8_t  m_mcs;     ///< MCS for transport block
  uint16_t m_size;    ///< Size of transport block
  uint8_t  m_rv;      ///< the redundancy version (HARQ)
  uint8_t  m_ndi;     ///< new data indicator flag
  uint8_t  m_ccId;    ///< component carrier id
  
  /**
   *  TracedCallback signature.
   *
   * \param [in] params Value of the PhyTransmissionionStatParameters.
   * \todo The argument should be passed by const reference, since it's large.
   */
  typedef void (* TracedCallback)(const cv2x_PhyTransmissionStatParameters params);
  
};


/// cv2x_PhyReceptionStatParameters structure
struct cv2x_PhyReceptionStatParameters
{
  int64_t  m_timestamp;    ///< in millisecond
  uint16_t m_cellId;       ///< Cell ID of the attached Enb
  uint64_t m_imsi;         ///< IMSI of the scheduled UE
  uint16_t m_rnti;         ///< C-RNTI scheduled
  uint8_t  m_txMode;       ///< the transmission Mode
  uint8_t  m_layer;        ///< the layer (cw) of the transmission
  uint8_t  m_mcs;          ///< MCS for transport block
  uint16_t m_size;         ///< Size of transport block
  uint8_t  m_rv;           ///< the redundancy version (HARQ)
  uint8_t  m_ndi;          ///< new data indicator flag
  uint8_t  m_correctness;  ///< correctness of the TB received
  uint8_t  m_ccId;        ///< component carrier id
  //Aziza
  double m_sinrPerRb; ///< linear average SINR per RB

  /**
   *  TracedCallback signature.
   *
   * \param [in] params Value of the cv2x_PhyReceptionStatParameters.
   * \todo The argument should be passed by const reference, since it's large.
   */
  typedef void (* TracedCallback)(const cv2x_PhyReceptionStatParameters params);
  
};

struct cv2x_SlUeMacStatParameters
{
  int64_t  m_timestamp;    ///< in millisecond
  uint16_t m_cellId;       ///< Cell ID of the attached Enb
  uint64_t m_imsi;         ///< IMSI of the scheduled UE
  uint16_t m_rnti;         ///< C-RNTI scheduled
  uint32_t m_frameNo;       ///< the frame number
  uint32_t m_subframeNo;   ///< the subframe number
  uint8_t  m_mcs;          ///< MCS for transport block
  uint16_t m_tbSize;       ///< Size of transport block in bytes
  uint16_t m_pscchRi;      ///< PSCCH resource index
  //uint16_t m_pscchTx1;     ///< PSCCH 1st transmission sf
  //uint16_t m_pscchTx2;     ///< PSCCH 2nd transmission sf
  uint32_t m_pscchFrame1;    ///< PSCCH 1st transmission frame number
  uint32_t m_pscchSubframe1; ///< PSCCH 1st transmission subframe number
  uint32_t m_pscchFrame2;    ///< PSCCH 2st transmission frame number
  uint32_t m_pscchSubframe2; ///< PSCCH 2st transmission subframe number
  uint16_t m_psschTxStartRB;   ///< PSSCH transmission start RB
  uint16_t m_psschTxLengthRB;  ///< PSSCH transmission length RB
  uint16_t m_psschItrp;   ///< PSSCH I_TRP
  uint32_t m_psschFrame;    ///< PSSCH frame used to transmitt
  uint32_t m_psschSubframe;    ///< PSSCH subframe used to transmitt
  uint32_t m_psschFrameStart;    ///< PSSCH frame start
  uint32_t m_psschSubframeStart;    ///< PSSCH frame start

  /**
   *  TracedCallback signature.
   *
   * \param [in] params Value of the cv2x_SlUeMacStatParameters.
   */
  typedef void (* TracedCallback)(const cv2x_SlUeMacStatParameters params);
  
};

struct cv2x_SlUeMacStatParametersV2x
{
  int64_t  m_timestamp;    ///< in millisecond
  uint16_t m_cellId;       ///< Cell ID of the attached Enb
  uint64_t m_imsi;         ///< IMSI of the scheduled UE
  uint16_t m_rnti;         ///< C-RNTI scheduled
  uint32_t m_frameNo;      ///< the frame number
  uint32_t m_subframeNo;   ///< the subframe number
  uint8_t  m_mcs;          ///< MCS for transport block
  uint16_t m_tbSize;       ///< Size of transport block in bytes
  uint8_t m_resPscch;       ///< Resource of PSCCH 
  uint32_t m_txFrame;      
  uint32_t m_txSubframe;
  uint16_t m_psschTxStartRB;   ///< PSSCH transmission start RB
  uint16_t m_psschTxLengthRB;  ///< PSSCH transmission length RB

  /**
   *  TracedCallback signature.
   *
   * \param [in] params Value of the cv2x_SlUeMacStatParameters.
   */
  typedef void (* TracedCallback)(const cv2x_SlUeMacStatParametersV2x params);
  
};


/// cv2x_DlSchedulingCallbackInfo structure
struct cv2x_DlSchedulingCallbackInfo
{
  uint32_t frameNo; ///< frame number
  uint32_t subframeNo; ///< subframe number
  uint16_t rnti; ///< RNTI
  uint8_t  mcsTb1; ///< MCS TB1
  uint16_t sizeTb1; ///< size TB1
  uint8_t  mcsTb2; ///< MCS TB2
  uint16_t sizeTb2; ///< size TB2
  uint8_t  componentCarrierId; ///< component carrier ID
}; 

/**
 * Implements the E-UTRA measurement mappings defined in  3GPP TS
 * 36.133 section 9.1 E-UTRAN measurements 
 * 
 */
class cv2x_EutranMeasurementMapping
{
public:
  /** 
   * converts an RSRP range to dBm as per 
   * 3GPP TS 36.133 section 9.1.4 RSRP Measurement Report Mapping
   *
   * \param range the RSRP range value
   * 
   * \return the corresponding RSRP value in dBm
   */
  static double RsrpRange2Dbm (uint8_t range);

  /** 
   * convert an RSRP value in dBm to the corresponding range as per 
   * 3GPP TS 36.133 section 9.1.4 RSRP Measurement Report Mapping
   * 
   * \param dbm the RSRP value in dBm
   * 
   * \return the corresponding range
   */
  static uint8_t Dbm2RsrpRange (double dbm);

  /** 
   * converts an RSRQ range to dB as per 
   * 3GPP TS 36.133 section 9.1.7 RSRQ Measurement Report Mapping
   *
   * \param range the RSRQ range value
   * 
   * \return the corresponding RSRQ value in dB
   */
  static double RsrqRange2Db (uint8_t range);

  /** 
   * convert an RSRQ value in dB to the corresponding range as per 
   * 3GPP TS 36.133 section 9.1.7 RSRQ Measurement Report Mapping
   * 
   * \param db the RSRQ value in dB
   * 
   * \return the corresponding range
   */
  static uint8_t Db2RsrqRange (double db);

  /** 
   * Quantize an RSRP value according to the measurement mapping of TS 36.133
   * 
   * \param v RSRP value in dBm
   * 
   * \return the quantized RSRP value in dBm
   */
  static double QuantizeRsrp (double v);

  /** 
   * Quantize an RSRQ value according to the measurement mapping of TS 36.133
   * 
   * \param v RSRQ value in dB
   * 
   * \return the quantized RSRQ value in dB
   */
  static double QuantizeRsrq (double v);

  /**
   * \brief Returns the actual value of a hysteresis parameter.
   * \param hysteresisIeValue IE value of hysteresis
   * \return actual value in dB, which is IE value * 0.5 dB
   *
   * As per section 6.3.5 of 3GPP TS 36.331.
   *
   * The allowed values for hysteresis IE value are between 0 and 30.
   *
   * \sa cv2x_LteRrcSap::ReportConfigEutra
   */
  static double IeValue2ActualHysteresis (uint8_t hysteresisIeValue);

  /**
   * \brief Returns the IE value of hysteresis.
   * \param hysteresisDb actual hysteresis value in dB
   * \return IE value of hysteresis in dB, which is actual value * 2, rounded
   *         to the nearest integer
   *
   * The allowed values for hysteresis are between 0 and 15 dB.
   *
   * \sa cv2x_LteRrcSap::ReportConfigEutra
   */
  static uint8_t ActualHysteresis2IeValue (double hysteresisDb);

  /**
   * \brief Returns the actual value of an a3-Offset parameter.
   * \param a3OffsetIeValue IE value of a3-Offset
   * \return actual value in dB, which is IE value * 0.5 dB
   *
   * As per section 6.3.5 of 3GPP TS 36.331.
   *
   * The allowed values for a3-Offset IE value are between -30 and 30.
   *
   * \sa cv2x_LteRrcSap::ReportConfigEutra
   */
  static double IeValue2ActualA3Offset (int8_t a3OffsetIeValue);

  /**
   * \brief Returns the IE value of a3-Offset.
   * \param a3OffsetDb actual A3 Offset value in dB
   * \return IE value of a3-Offset in dB, which is actual value * 2, rounded
   *         to the nearest integer
   *
   * The allowed values for A3 Offset are between -15 and 15 dB.
   *
   * \sa cv2x_LteRrcSap::ReportConfigEutra
   */
  static int8_t ActualA3Offset2IeValue (double a3OffsetDb);

  /**
   * \brief Returns the actual value of an Q-RxLevMin parameter.
   * \param qRxLevMinIeValue IE value of Q-RxLevMin
   * \return Q-RxLevMin actual value in dBm, which is IE value * 2 dBm
   *
   * As per section 6.3.4 of 3GPP TS 36.331.
   *
   * \sa cv2x_LteRrcSap::CellSelectionInfo
   */
  static double IeValue2ActualQRxLevMin (int8_t qRxLevMinIeValue);

  /**
   * \brief Returns the actual value of an Q-QualMin parameter.
   * \param qQualMinIeValue IE value of Q-QualMin
   * \return Q-QualMin actual value in dB, which is IE value dB
   *
   * As per section 6.3.4 of 3GPP TS 36.331.
   *
   * \sa cv2x_LteRrcSap::CellSelectionInfo
   */
  static double IeValue2ActualQQualMin (int8_t qQualMinIeValue);

}; // end of class cv2x_EutranMeasurementMapping

}; // namespace ns3


#endif /* CV2X_LTE_COMMON_H_ */
