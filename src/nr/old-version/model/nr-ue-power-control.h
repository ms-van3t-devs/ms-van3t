/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */


#ifndef NR_UE_POWER_CONTROL_H
#define NR_UE_POWER_CONTROL_H

#include <ns3/ptr.h>
#include <ns3/traced-callback.h>
#include <ns3/object.h>
#include <vector>


namespace ns3 {

/**
 * \brief This class implements NR Uplink Power Control functionality.
 * Can operate in two different modes: following specification TS 36.213
 * that is used in LTE, LAA, etc; or  following specification TS 38.213
 * for New Radio technology.
 *
 * NrUePowerControl entity is responsible for calculating total
 * power that will be used to transmit PUSCH, PUCCH and SRS.
 *
 * NrUePowerControl functionality is inspired by LteUePowerControl,
 * but it does not inherits it because almost all of its functions needed to be
 * overidden and extended in order to support different specifications, i.e., to be
 * compatible with both LTE and NR standard (e.g. support different numerologies).
 * And also to support power control for PUCCH.
 *
 * NrUePowerControl computes the TX power based on pre-configured
 * parameters and current measurements, such as path loss.
 * NrUePhy should pass the RSRP to NrUePowerControl, while
 * referenceSignalPower is configurable by attribute system.
 * NrUePowerControl uses latter values to calculate path loss.
 * When closed loop power control is being used NrUePhy should also
 * pass TPC values to NrUePowerControl.
 *
 * Specification that are used to implement uplink power control feature are
 * the latest available specifications for LTE and NR:
 * 1) ETSI TS 136 213 V14.2.0 (2017-04)
 * 2) ETSI TS 138 213 V15.6.0 (2019-07)
 *
 */

class NrUePhy;

class NrUePowerControl : public Object
{
public:

  /**
   * Power control supports two technical specifications:
   * 1) TS 36.213, for LTE, LAA, etc.
   * 1) TS 38.213, for New Radio (NR)
   *
   */
  enum TechnicalSpec {
     TS_36_213,
     TS_38_213,
  };

  NrUePowerControl ();
  /**
   * \brief Constructor that sets a pointer to its NrUePhy instance owner.
   * This is necessary in order to obtain information such as numerology
   * that is used in calculation of tranmit power.
   */
  NrUePowerControl (const Ptr<NrUePhy>& nrUePhy);

  /**
   * \brief Destructor
   */
  virtual ~NrUePowerControl ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  // inherited from Object
  virtual void DoInitialize (void);
  virtual void DoDispose (void);


  // set functions

  /*
   * \brief Sets technical specification according to which will
   * be calculated power
   * \param value technical specification to be used
   */
  void SetTechnicalSpec (NrUePowerControl::TechnicalSpec value);
  /**
   * \brief Sets whether Closed loop model will be active.
   * \param value If true Closed loop mode will be used, otherwise Open Loop.
   */
  void SetClosedLoop (bool value);
  /**
   * \brief Sets whether the accumulation mode will be used or not.
   * \param value If true TPC accumulation mode will be active,
   * otherwise absolute mode will be active
   */
  void SetAccumulationEnabled (bool value);
  /**
   * \brief Sets alpha parameter for uplink power control calculation.
   * \param value the value of Alpha parameter
   */
  void SetAlpha (double value);
  /**
   * \brief Set PC maximum function
   * \param value the PC maximum value
   */
  void SetPcmax (double value);
  /**
   * \brief Set PC minimum function
   * \param value the PC minimum value
   */
  void SetPcmin (double pcmin);
  /**
   * \brief Sets KPusch
   * \param value KPUSCH value to be used in PUSCH transmit power
   */
  void SetKPusch (uint16_t value);
  /**
   * \brief Sets KPucch
   * \param value KPUCCH value to be used in PUSCH transmit power
   */
  void SetK0Pucch (uint16_t value);
  /**
   * \brief Sets whether the device for which is configure this
   *  uplink power control algorithm is for device that is
   *  bandwidth reduced low complexity device or coverage enhanced (BL/CE)
   *  device
   * \param value an indicator telling whether device is BL/CE or not
   */
  void SetBlCe (bool value);
  /**
   * \brief Sets P0 SRS parameter for calculation of SRS power control
   * \param value P0 SRS value to be set
   */
  void SetP0Srs (double value);
  /**
   * \brief Sets Delta TF power adjustment component for
   * PUSCH power control calculation
   * \param value Delta TF power adjustment value
   */
  void SetDeltaTF (double value);
  /**
   * \brief Sets Delta TF transmission power adjustment component for
   * PUCCH power control calculation
   * \param value power adjustment value
   */
  void SetDeltaTFControl (double value);
  /**
   * \brief Sets delta_f_pucch value needed for calculation of
   * PUCCH power control. It is provided by higher layers
   * through deltaF-PUCCH-f0 for PUCCH format 0,
   * deltaF-PUCCH-f1 for PUCCH format 1,
   * deltaF-PUCCH-f2 for PUCCH format 2,
   * deltaF-PUCCH-f3 for PUCCH format 3, and
   * deltaF-PUCCH-f4 for PUCCH format 4.
   * \param value delta_F_Pucch value to be set
   */
  void SetDeltaFPucch (double value);
  /**
   * \brief Set PO nominal PUCCH value
   * \param value the value to set
   */
  void SetPoNominalPucch (int16_t value);
  /**
  * \brief Set PO PUCCH value
  * \param value the value to set
  */
  void SetPoUePucch (int16_t value);
  /**
   * \brief Set Po nominal Pusch
   * \param value the Po nominal Pusch
   */
  void SetPoNominalPusch (int16_t value);
  /**
   * \brief Set Po Ue Pusch
   * \param value the Po Ue Pusch
   */
  void SetPoUePusch (int16_t value);
  /**
   * \brief Set transmit power function
   * \param value the transmit power value
   */
  void SetTxPower (double value);
  /**
   * \brief Configure reference signal power (dBm) function
   * \param value the reference signal power
   */
  void ConfigureReferenceSignalPower (double value);
  /**
   * \brief Set RSRP function
   * \param value the RSRP (dBm) value to set
   */
  void SetRsrp (double value);
  /**
   * \brief Set RSRP function
   * \param rsrpFilterCoefficient value. Determines the strength of
   * smoothing effect induced by layer 3 filtering of RSRP
   * used for uplink power control in all attached UE.
   * If equals to 0, no layer 3 filtering is applicable.
   */
  void SetRsrpFilterCoefficient (uint8_t rsrpFilterCoefficient);

  // Get functions
  /**
   * \brief Implements calculation of PUSCH
   * power control according to TS 36.213 and TS 38.213.
   * \param rbNum number of RBs used for PUSCH
   */
  double GetPuschTxPower (std::size_t rbNum);
  /**
   * \brief Implements calculation of PUCCH
   * power control according to TS 36.213 and TS 38.213.
   * \param rbNum number of RBs used for PUCCH
   */
  double GetPucchTxPower (std::size_t rbNum);
  /**
   * \brief Implements calculation of SRS
   * power control according to TS 36.213 and TS 38.213.
   * \param rbNum number of RBs used for SRS
   */
  double GetSrsTxPower (std::size_t rbNum);
  /**
   * \brief Function that is called by NrUePhy
   * to notify NrUePowerControl algorithm
   * that TPC command was received by gNB
   * \param tpc the TPC command
   */
  void ReportTpcPusch (uint8_t tpc);
  /**
   * \brief Function that is called by NrUePhy
   * to notify NrUePowerControl algorithm
   * that TPC command for PUCCH was received by gNB
   * \param tpc the TPC command
   */
  void ReportTpcPucch (uint8_t tpc);

  /**
   * TracedCallback signature for uplink transmit power.
   *
   * \param [in] cellId Cell identifier.
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] power The current TX power.
   */
  typedef void (* TxPowerTracedCallback)
    (uint16_t cellId, uint16_t rnti, double power);

  /**
   * \brief Sets some information for easier logging
   * \param cellId cell ID
   * \param rnti UE RNTI
   */
  void SetLoggingInfo (uint16_t cellId, uint16_t rnti);

private:

  /*
    * \brief Implements conversion from TPC
    * command to absolute delta value. Follows both,
    * TS 36.213 and TS 38.213 specification for PUSCH.
    * In 36.213 table is Table 5.1.1.1-2. and
    * in 38.213 the table is Table 7.1.1-1.
    * \param tpc TPC command value from 0 to 3
    */
   int8_t GetAbsoluteDelta (uint8_t tpc) const;
   /*
    * \brief Implements conversion from TPC
    * command to accumulated delta value. Follows both,
    * TS 36.213 and TS 38.213 specification for PUSCH
    * and PUCCH. In 36.213 tables are Table 5.1.1.1-2.
    * and Table 5.1.2.1-1;
    * while in 38.213 tables are:
    * Table 7.1.1-1 and Table 7.2.1-1.
    * \param tpc TPC command value from 0 to 3
    */
   int8_t GetAccumulatedDelta (uint8_t tpc) const;
   /**
    * \brief Calculates fc value for PUSCH power control
    * according to TS 38.213 7.2.1 formulas.
    */
   void UpdateFc ();
   /**
    * \brief Calculate gc value for PUCCH power control
    * according to TS 38.213 7.2.1 formulas.
    */
   void UpdateGc ();
   /**
    * \brief Calculates PUSCH transmit power
    * according TS 38.213 7.1.1 formulas
    * \param rbNum number of RBs
    */
  double CalculatePuschTxPowerNr (std::size_t rbNum);
   /**
    * \brief Calculates PUCCH transmit power
    * according TS 38.213 7.2.1 formulas
    * \param rbNum number of RBs
    */
  double CalculatePucchTxPowerNr (std::size_t rbNum);
   /**
    * \brief Calculates SRS transmit power
    * \param rbNum number of RBs
    */
  double CalculateSrsTxPowerNr (std::size_t rbNum);

  // general attributes
  bool m_closedLoop {true};                     //!< is closed loop
  bool m_accumulationEnabled {true};            //!< accumulation enabled
  TechnicalSpec m_technicalSpec;                //!< Technical specification to be used for transmit power calculations
  double m_Pcmax {0};                           //!< PC maximum
  double m_Pcmin {0};                           //!< PC minimum
  double m_referenceSignalPower {30};           //!< reference signal power in dBm
  double m_alpha {0};                           //!< alpha value, current code supports only a single parameter set configuration
  bool m_blCe {false};                          /*!< Indicator whether the power control is applied to bandwidth reduced low complexity or coverage enhanced devices.
                                                       When set to true means that this power control is applied to bandwidth reduced,
                                                       low complexity or coverage enhanced device. By default this attribute is set to false.
                                                       Default BL/CE mode is CEModeB.*/
  double m_P_0_SRS {0.0};                       //!< P_0_SRS parameter for calculation of SRS power control


  // PUSCH attributes
  int16_t m_PoNominalPusch {0};                 //!< PO nominal PUSCH, current code supports only a single parameter set configuration
  int16_t m_PoUePusch {0};                      //!< PO US PUSCH, current code supports only a single parameter set configuration
  uint16_t m_k_PUSCH {0};                       //!< One of the principal parameters for the calculation of the PUSCH pc accumulation state m_fc
  double m_deltaTF {0};                         //!< PUSCH transmission power adjustment component for UL BWP of carrier of primary cell

  // PUCCH attributes
  int16_t m_PoNominalPucch {0};                 //!< PO nominal PUCCH, current code supports only a single parameter set configuration
  int16_t m_PoUePucch {0};                      //!< PO US PUCCH, current code supports only a single parameter set configuration
  uint16_t m_k_PUCCH {0};                       //!< One of the principal parameters for the calculation of the PUCCH pc accumulation state m_gc
  double m_delta_F_Pucch {0.0};                 //!< Delta F_PUCCH to calculate 38.213 7.2.1 formula for PUCCH transmit power
  double m_deltaTF_control {0.0};               //!< PUCCH transmission power adjustment component for UL BWP of carrier of primary cell

  // other variables
  double m_curPuschTxPower {10};               //!< current PUSCH transmit power
  double m_curPucchTxPower {10};               //!< current PUCCH transmit power
  double m_curSrsTxPower {10};                 //!< current SRS transmit power
  bool m_rsrpSet {false};                       //!< is RSRP set?
  double m_rsrp {-40};                         //!< RSRP value in dBm
  int16_t m_PsrsOffset;                         //!< PSRS offset
  double m_pathLoss {100};                      //!< path loss value in dB
  std::vector <int8_t> m_deltaPucch;           //!< vector that saves TPC command accumulated values for PUCCH transmit power calculation
  std::vector <int8_t> m_deltaPusch;            //!< vector that saves TPC command accumulated values for PUSCH transmit power calculation
  double m_fc {0.0};                            //!< FC
  double m_gc {0.0};                            //!< Is the current PUCCH power control adjustment state. This variable is used for calculation of PUCCH transmit power.
  double m_hc {0.0};                            //!< Is the current SRS power control adjustment state. This variable is used for calculation of SRS transmit power.

  //another attributes needed for function calls
  Ptr<NrUePhy> m_nrUePhy;                       //!< NrUePhy instance owner

  /**
  * The `RsrpFilterCoefficient` attribute. Determines the strength of
  * smoothing effect induced by layer 3 filtering of RSRP in all attached UE.
  * If equals to 0, no layer 3 filtering is applicable.
  */
  uint8_t m_pcRsrpFilterCoefficient {4};
  
  uint16_t m_cellId = 0; //!< cell ID that will be used for logging purposes
  uint16_t m_rnti = 0; //!< RNTI that will be used for logging purposes
  
  
  /**
   * Trace information regarding Uplink TxPower
   * uint16_t cellId, uint16_t rnti, double txPower
   */
  TracedCallback<uint16_t, uint16_t, double> m_reportPuschTxPower;
  /**
   * Trace information regarding Uplink TxPower
   * uint16_t cellId, uint16_t rnti, double txPower
   */
  TracedCallback<uint16_t, uint16_t, double> m_reportPucchTxPower;
  /**
   * Trace information regarding Uplink TxPower
   * uint16_t cellId, uint16_t rnti, double txPower
   */
  TracedCallback<uint16_t, uint16_t, double> m_reportSrsTxPower;
};

}

#endif /* NR_UE_POWER_CONTROL_H */
