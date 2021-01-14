/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.

 * We would appreciate acknowledgement if the software is used.

 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 *
 * Modified by: NIST
 * 
 * Modified by: CNI
 */

#ifndef CV2X_SL_RESOURCE_POOL_FACTORY_H
#define CV2X_SL_RESOURCE_POOL_FACTORY_H

#include "cv2x_lte-rrc-sap.h"
#include "cv2x_sl-pool-factory.h"

namespace ns3 {

  /** Class to configure and generate resource pools */
class cv2x_SlResourcePoolFactory : public cv2x_SlPoolFactory
{
public:
  cv2x_SlResourcePoolFactory ();

  /**
   * Creates a new resource pool based on the factory configuration
   * \return The new resource pool
   */
  cv2x_LteRrcSap::SlCommResourcePool CreatePool ();

  /**
   * Sets the cyclic prefix length for the control channel
   * \param cpLen The cyclic prefix length
   */
  void SetControlCpLen (std::string cpLen);

  /** 
   * Set the sidelink period duration
   * \param period The sidelink period
   */
  void SetControlPeriod (std::string period);

  /**
   * Sets the number of PRBs in  the control channel (from the start up and the end down)
   * \param prbNum The number of PRBs
   */
  void SetControlPrbNum (int prbNum);

  /**
   * Sets the index of the PRB where the control channel starts
   * \param prbStart The index of the PRB
   */
  void SetControlPrbStart (int prbStart);

  /**
   * Sets the index of the PRB where the control channel ends
   * \param prbEnd The index of the PRB
   */
  void SetControlPrbEnd (int prbEnd);

  /**
   * Sets the offset (in subframe) for the control channel
   * \param offset the number of subframes from the begining of the sidelink period
   */
  void SetControlOffset (int offset);

  /**
   * Sets the bitmap identifying which subframes belong to the control channel
   * \param value The bitmap identifying which subframes belong to the control channel
   */
  void SetControlBitmap (uint64_t value);

  /**
   * Sets the cyclic prefix for the data channel
   * \param cpLen The cyclic prefix
   */
  void SetDataCpLen (std::string cpLen);

  /**
   * Sets the frequency hopping parameters
   * \param hoppingParameters The frequency hopping parameters
   */
  void SetDataHoppingParameters (int hoppingParameters);

  /**
   * Sets the bands for frequency hoping
   * \param subbands The bands for frequency hopping
   */
  void SetDataHoppingSubbands (std::string subbands);

  /**
   * Sets the RB offset for frequency hopping
   * \param rbOffset The RB offset to use
   */
  void SetDataHoppingOffset (int rbOffset);

  // UE Selected Parameters
  /** 
   * Indicates if the configuration is for a UE selected pool
   * \param ueSelected true if the connfiguration is for a UE selected pool 
   */
  void SetHaveUeSelectedResourceConfig (bool ueSelected);

  /**
   * Indicates if the configuration contains the TRP subset information
   * \param haveTrptSubset True if the configuration contain the TRP subset information
   */
  void SetHaveTrptSubset (bool haveTrptSubset);

  /**
   * Sets the TRP subset value
   * \param value The TRP subset value
   */
  void SetTrptSubset (int value);

  /**
   * Sets the number of PRBs for the shared channel (from the start up and end down)
   * \param prbNum The number of PRBs for the shared channel
   */
  void SetDataPrbNum (int prbNum);

  /**
   * Sets the index of the PRB where the shared channel starts
   * \param prbStart the index of the PRB where the shared channel starts
   */
  void SetDataPrbStart (int prbStart);

  /**
   * Sets the index of the PRB where the shared channel ends
   * \param prbEnd the index of the PRB where the shared channel ends
   */
  void SetDataPrbEnd (int prbEnd);

  /**
   * Sets the offset of the shared channel (from the start of the period)
   * \param offset The offset (in subframe)
   */
  void SetDataOffset (int offset);

  /**
   * Sets the bitmap identifying the subframes to use for the shared channel
   * \param value The bitmap value
   */
  void SetDataBitmap (int value);

  // Tx Parameters
  /**
   * Indicates if the configuration provides transmission parameters to use
   * \param txParam True if the configuration provides transmission parameters to use
   */ 
  void SetHaveTxParameters (bool txParam);
  
  /**
   * Sets the alpha parameter for the power control algorithm in the control channel
   * \param alpha The alpha parameter for the power control algorithm
   */
  void SetControlTxAlpha (std::string alpha);

  /**
   * Sets the p0 parameter for the power control algorithm in the control channel
   * \param p0 The p0 parameter for the power control algorithm
   */  
  void SetControlTxP0 (int p0);

  /**
   * Sets the alpha parameter for the power control algorithm in the shared channel
   * \param alpha The alpha parameter for the power control algorithm
   */
  void SetDataTxAlpha (std::string alpha);

  /**
   * Sets the p0 parameter for the power control algorithm in the shared channel
   * \param p0 The p0 parameter for the power control algorithm
   */  
  void SetDataTxP0 (int p0);

private:
  cv2x_LteRrcSap::SlCommResourcePool m_pool;

protected:
  // Control
  std::string m_scCpLen; //!< cyclic prefix for the control channel
  std::string m_period; //!< sidelink period duration
  int8_t m_scPrbNum; //!< number of PRBs in the control channel
  int8_t m_scPrbStart; //!< index of the PRB where the control channel starts
  int8_t m_scPrbEnd; //!< index of the PRB where the control channel ends
  int16_t m_scOffset; //!< offset when the control channel starts (subframe)
  //int m_scBitmapSize;
  int64_t m_scBitmapValue; //!< the bitmap specifying which subframe to use in the control channel

  // Data
  std::string m_dataCpLen; //!< cyclic prefix for the shared channel
  int8_t m_hoppingParameters; //!< frequency hopping for the shared channel
  std::string m_subbands; //!< number of subbands
  int8_t m_rbOffset; //!< offset in the PRBs

  // UE Selected Parameters
  bool m_ueSelected; //!< indicates if the pool if for UE selected mode
  bool m_haveTrptSubset; //!< indicates if the TRP subset is defined
  //int m_trptSubsetSize; 
  int64_t m_trptSubsetValue; //!< value for the TRP subset
  int8_t m_dataPrbNum; //!< number of PRBs in the shared channel
  int8_t m_dataPrbStart; //!< index of the PRB where the shared channel starts
  int8_t m_dataPrbEnd; //!< index of the PRB where the shared channel ends
  int16_t m_dataOffset; //!< offset when the shared channel starts (subframe)
  //int m_dataBitmapSize;
  int64_t m_dataBitmapValue; //!< bitmap specifying which subframe to use in the shared channel

  // Tx Parameters
  bool m_txParam; //!< indicates if the transmission parameters are defined
  std::string m_txAlpha; //!< the alpha parameter for power control in the control channel
  int16_t m_txP0; //!< the p0 parameter for the power control in the control channel
  std::string m_dataAlpha; //!< the alpha parameter for power control in the shared channel
  int16_t m_dataP0; //!< the p0 parameter for the power control in the shared channel

};
}

#endif
