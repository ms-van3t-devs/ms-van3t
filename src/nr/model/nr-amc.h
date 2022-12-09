/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#ifndef NR_AMC_H
#define NR_AMC_H

#include <ns3/nr-phy-mac-common.h>
#include <ns3/nr-error-model.h>

namespace ns3 {

/**
 * \ingroup error-models
 * \brief Adaptive Modulation and Coding class for the NR module
 *
 * The class has two option to calculate the CQI feedback (which is the MCS to use
 * in the future transmissions): the "ShannonModel" or the "ErrorModel" model,
 * which uses the output of an error model to find the optimal MCS.
 *
 * Please note that it is necessary, even when using the ShannonModel, to correctly
 * configure the ErrorModel type, which must be the same as the one set in the
 * NrSpectrumPhy class.
 *
 * \section nr_amc_conf Configuration
 *
 * The attributes of this class can be configured through the helper methods
 * NrHelper::SetGnbDlAmcAttribute() and NrHelper::SetGnbUlAmcAttribute()
 * for what regards the GNB side (DL or UL). It is important to note that the
 * UE gets a pointer to the GNB AMC to which is connected to.
 *
 * \todo Pass NrAmc parameters through RRC, and don't pass pointers to AMC
 * between GNB and UE
 */
class NrAmc : public Object
{
public:
  /**
   * \brief GetTypeId
   * \return the TypeId of the Object
   */
  static TypeId GetTypeId (void);

  /**
   * \brief GetInstanceTypeId
   * \return the instance typeid
   */
  TypeId GetInstanceTypeId () const override;

  /**
   * \brief NrAmc constructor
   */
  NrAmc ();

  /**
   * \brief ~NrAmc deconstructor
   */
  virtual ~NrAmc () override;

  /**
   * \brief Set the object to be in "DL" mode.
   *
   * In this mode, all the requests made to the underlying error model
   * will be done keeping in consideration that the requests refers to
   * DL transmissions.
   */
  void SetDlMode ();

  /**
   * \brief Set the object to be in "UL" mode.
   *
   * In this mode, all the requests made to the underlying error model
   * will be done keeping in consideration that the requests refers to
   * UL transmissions.
   */
  void SetUlMode ();

  /**
   * \brief Valid types of the model used to create a cqi feedback
   *
   * \see CreateCqiFeedbackWbTdma
   */
  enum AmcModel
  {
    ShannonModel, //!< Shannon based model (very conservative)
    ErrorModel    //!< Error Model version (can use different error models, see NrErrorModel)
  };

  /**
   * \brief Get the MCS value from a CQI value
   * \param cqi the CQI
   * \return the MCS that corresponds to that CQI (it depends on the error model)
   */
  uint8_t GetMcsFromCqi (uint8_t cqi) const;

  /**
   * \return The number of reference subcarriers per resource block
   */
  uint8_t GetNumRefScPerRb () const;

  /**
   * \brief Set the the number of subcarriers carrying reference signals per resource block
   *
   * By default it is fixed at 1. For LTE, it should be 4.
   *
   * \param nref the number of reference subcarriers
   *
   */
  void SetNumRefScPerRb (uint8_t nref);

  /**
   * \brief Create a CQI/MCS wideband feedback from a SINR values
   *
   * For CQI creation, a CSI reference resource equal to all RBs in
   * which the gNB/UE has transmitted power, and from which the SINR can be
   * measured, during 1 OFDM symbol, is assumed.
   *
   * \param sinr the sinr values
   * \param mcsWb The calculated MCS
   * \return The calculated CQI
   */
  uint8_t CreateCqiFeedbackWbTdma (const SpectrumValue& sinr, uint8_t &mcsWb) const;

  /**
   * \brief Get CQI from a SpectralEfficiency value
   * \param s spectral efficiency
   * \return the CQI (depends on the Error Model)
   */
  uint8_t GetCqiFromSpectralEfficiency (double s) const;

  /**
   * \brief Get MCS from a SpectralEfficiency value
   * \param s spectral efficiency
   * \return the MCS (depends on the Error Model)
   */
  uint8_t GetMcsFromSpectralEfficiency (double s) const;

 /**
  * \brief Get the maximum MCS (depends on the underlying error model)
  * \return the maximum MCS
  */
  uint32_t GetMaxMcs () const;

  /**
   * \brief Set the AMC model type
   * \param m the AMC model
   */
  void SetAmcModel (AmcModel m);
  /**
   * \brief Get the AMC model type
   * \return the AMC model type
   */
  AmcModel GetAmcModel () const;

  /**
   * \brief Set Error model type
   * \param type the Error model type
   */
  void SetErrorModelType (const TypeId &type);
  /**
   * \brief Get the error model type
   * \return the error model type
   */
  TypeId GetErrorModelType () const;

  /**
   * \brief Calculate the TransportBlock size (in bytes) giving the MCS and the number of RB assigned
   *
   * It depends on the error model and the "mode" configured with SetMode().
   * Please note that this function expects in input the RB, not the RBG of the transmission.
   *
   * \param mcs the MCS of the transmission
   * \param nprb The number of physical resource blocks used in the transmission
   * \return the TBS in bytes
   */
  uint32_t CalculateTbSize (uint8_t mcs, uint32_t nprb) const;

  /**
   * \brief Calculate the Payload Size (in bytes) from MCS and the number of RB
   * \param mcs MCS of the transmission
   * \param nprb Number of Physical Resource Blocks (not RBG)
   * \return the payload size in bytes
   */
  uint32_t GetPayloadSize (uint8_t mcs, uint32_t nprb) const;
private:
  /**
   * \brief Get the requested BER in assigning MCS (Shannon-bound model)
   * \return BER
   */
  double GetBer () const;

private:
  AmcModel m_amcModel;             //!< Type of the CQI feedback model
  Ptr<NrErrorModel> m_errorModel;  //!< Pointer to an instance of ErrorModel
  TypeId m_errorModelType;         //!< Type of the error model
  uint8_t m_numRefScPerRb {1};     //!< number of reference subcarriers per RB
  NrErrorModel::Mode m_emMode {NrErrorModel::DL}; //!< Error model mode
  static const unsigned int m_crcLen = 24 / 8; //!< CRC length (in bytes)
};

} // end namespace ns3

#endif /* NR_AMC_H */
