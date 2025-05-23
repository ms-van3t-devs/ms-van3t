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
 */

#ifndef CV2X_LTE_PHY_ERROR_MODEL_H
#define CV2X_LTE_PHY_ERROR_MODEL_H
#include <stdint.h>
#include <ns3/cv2x_lte-harq-phy.h>

namespace ns3 {

  const uint16_t XTABLE_SIZE = 3; //!<number of element in the X tables
  const uint16_t PUSCH_AWGN_SIZE = 33; //!<number of BLER values per HARQ transmission
  const uint16_t PSDCH_AWGN_SIZE = 23; //!<number of BLER values per HARQ transmission
  const uint16_t PSCCH_AWGN_SIZE = 38; //!<number of BLER values 
  const uint16_t PSBCH_AWGN_SIZE = 43; //!<number of BLER values

  /**
   * Structure to report transport block error rate  value and computed SINR
   */
  struct TbErrorStats_t
  {
    double tbler; //!< block error rate
    double sinr; //!< sinr value
  };

/**
  * This class contains functions to access the BLER model
  */
class cv2x_LtePhyErrorModel
{

public:

  /**
   * List of possible channels
   */
  enum cv2x_LtePhyChannel {
    PDCCH,
    PDSCH,
    PUCCH,
    PUSCH,
    PSCCH,
    PSSCH,
    PSDCH
  };

  /**
   * List of channel models. Note that not all models are available for each physical channel
   */
  enum LteFadingModel {
    AWGN
  };

  /**
   * List of transmission mode. Note that not all transmission modes are available for each physical channel or fading model
   */
  enum LteTxMode {
    SISO,
    SIMO,
    TxDiversity,
    SpatMultplex
  };

  /**
   * \brief Lookup the BLER for the given SINR
   * \param fadingChannel The channel to use
   * \param txmode The Transmission mode used
   * \param mcs The MCS of the TB
   * \param sinr The mean sinr of the TB
   * \param harqHistory The HARQ information
   */
  static TbErrorStats_t GetPsschBler (LteFadingModel fadingChannel, LteTxMode txmode, uint16_t mcs, double sinr, cv2x_HarqProcessInfoList_t harqHistory);

  /**
   * \brief Lookup the BLER for the given SINR
   * \param fadingChannel The channel to use
   * \param txmode The Transmission mode used
   * \param sinr The mean sinr of the TB
   * \param harqHistory The HARQ information
   */
  static TbErrorStats_t GetPsdchBler (LteFadingModel fadingChannel, LteTxMode txmode, double sinr, cv2x_HarqProcessInfoList_t harqHistory);

  /**
   * \brief Lookup the BLER for the given SINR
   * \param fadingChannel The channel to use
   * \param txmode The Transmission mode used
   * \param sinr The mean sinr of the TB
   */
  static TbErrorStats_t GetPscchBler (LteFadingModel fadingChannel, LteTxMode txmode, double sinr);

  /**
   * \brief Lookup the BLER for the given SINR
   * \param fadingChannel The channel to use
   * \param txmode The Transmission mode used
   * \param mcs The MCS of the TB
   * \param sinr The mean sinr of the TB
   * \param harqHistory The HARQ information
   */
  static TbErrorStats_t GetPuschBler (LteFadingModel fadingChannel, LteTxMode txmode, uint16_t mcs, double sinr, cv2x_HarqProcessInfoList_t harqHistory);

  /**
   * \brief Lookup the BLER for the given SINR
   * \param fadingChannel The channel to use
   * \param txmode The Transmission mode used
   * \param sinr The mean sinr of the TB
   */
  static TbErrorStats_t GetPsbchBler (LteFadingModel fadingChannel, LteTxMode txmode, double sinr);

 
  //TODO: as error models for other physical channels are added, add new functions. The signature should be the same

 private:
  /**
   * \brief Find the index of the data. Returns -1 if out of range.
   * \param mcs The MCS of the TB
   * \param harq The transmission number
   */
  static int16_t GetRowIndex (uint16_t mcs, uint8_t harq);
  
  /**
   * \brief Find the index of the column where the BLER is located. Returns -1 if out of range.
   */
  static int16_t GetColIndex (double val, double min, double max, double step);

  static double GetBlerValue (const double (*xtable)[XTABLE_SIZE], const double *ytable, const uint16_t ysize, uint16_t mcs, uint8_t harq, double sinr);

  static double GetSinrValue (const double (*xtable)[XTABLE_SIZE], const double *ytable, const uint16_t ysize, uint16_t mcs, uint8_t harq, double bler);


  /**
   * \brief Compute the SINR value given the index on the table
   */
  static double GetValueForIndex (uint16_t index, double min, double max, double step);

  static uint16_t GetBlerIndex (double bler, uint16_t row, const double *ytable, const uint16_t ysize);
  /**
   * \brief Generic function to compute the effective BLER and SINR
   */
  static TbErrorStats_t GetBler (const double (*xtable)[XTABLE_SIZE], const double *ytable, const uint16_t ysize, uint16_t mcs, uint8_t harq, double prevSinr, double newSinr);


}; //end class
} // namespace ns3
#endif /* LTE_PHY_ERROR_MODEL_H */
