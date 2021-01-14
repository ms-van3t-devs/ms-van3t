/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 CTTC
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by:
 *          Marco Miozzo <mmiozzo@cttc.es> (add data and ctrl diversity)
 *          NIST (D2D)
 */

#ifndef CV2X_LTE_SPECTRUM_SIGNAL_PARAMETERS_H
#define CV2X_LTE_SPECTRUM_SIGNAL_PARAMETERS_H


#include <ns3/spectrum-signal-parameters.h>

namespace ns3 {

class PacketBurst;
class cv2x_LteControlMessage;


/**
 * \ingroup lte
 *
 * Signal parameters for Lte
 */
struct cv2x_LteSpectrumSignalParameters : public SpectrumSignalParameters
{

  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();

  /**
   * default constructor
   */
  cv2x_LteSpectrumSignalParameters ();

  /**
   * copy constructor
   * \param p the cv2x_LteSpectrumSignalParameters to copy
   */
  cv2x_LteSpectrumSignalParameters (const cv2x_LteSpectrumSignalParameters& p);

  /**
   * The packet burst being transmitted with this signal
   */
  Ptr<PacketBurst> packetBurst;
};


/**
* \ingroup lte
*
* Signal parameters for Lte Data Frame (PDSCH), and eventually after some 
* control messages through other control channel embedded in PDSCH
* (i.e. PBCH)
*/
struct cv2x_LteSpectrumSignalParametersDataFrame : public SpectrumSignalParameters
{
  
  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();
  
  /**
  * default constructor
  */
  cv2x_LteSpectrumSignalParametersDataFrame ();
  
  /**
  * copy constructor
  * \param p the cv2x_LteSpectrumSignalParametersDataFrame to copy
  */
  cv2x_LteSpectrumSignalParametersDataFrame (const cv2x_LteSpectrumSignalParametersDataFrame& p);
  
  /**
  * The packet burst being transmitted with this signal
  */
  Ptr<PacketBurst> packetBurst;
  
  std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList; ///< the control message list
  
  uint16_t cellId; ///< cell ID
};


/**
* \ingroup lte
*
* Signal parameters for Lte DL Ctrl Frame (RS, PCFICH and PDCCH)
*/
struct cv2x_LteSpectrumSignalParametersDlCtrlFrame : public SpectrumSignalParameters
{
  
  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();
  
  /**
  * default constructor
  */
  cv2x_LteSpectrumSignalParametersDlCtrlFrame ();
  
  /**
  * copy constructor
  * \param p the cv2x_LteSpectrumSignalParametersDlCtrlFrame to copy
  */
  cv2x_LteSpectrumSignalParametersDlCtrlFrame (const cv2x_LteSpectrumSignalParametersDlCtrlFrame& p);


  std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList; ///< control message list
  
  uint16_t cellId; ///< cell ID
  bool pss; ///< primary synchronization signal
};



/**
* \ingroup lte
*
* Signal parameters for Lte SRS Frame
*/
struct cv2x_LteSpectrumSignalParametersUlSrsFrame : public SpectrumSignalParameters
{
  
  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();
  
  /**
  * default constructor
  */
  cv2x_LteSpectrumSignalParametersUlSrsFrame ();
  
  /**
  * copy constructor
  * \param p the cv2x_LteSpectrumSignalParametersUlSrsFrame to copy
  */
  cv2x_LteSpectrumSignalParametersUlSrsFrame (const cv2x_LteSpectrumSignalParametersUlSrsFrame& p);
  
  uint16_t cellId; ///< cell ID
};


/**
* \ingroup lte
*
* Signal parameters for Lte SL Frame (PSCCH and PSSCH)
*/
struct cv2x_LteSpectrumSignalParametersSlFrame : public SpectrumSignalParameters
{
  
  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();
  
  /**
  * default constructor
  */
  cv2x_LteSpectrumSignalParametersSlFrame ();
  
  /**
  * copy constructor
  */
  cv2x_LteSpectrumSignalParametersSlFrame (const cv2x_LteSpectrumSignalParametersSlFrame& p);


  /**
  * The packet burst being transmitted with this signal
  */
  Ptr<PacketBurst> packetBurst;

  /**
   * The control messages being sent (for sidelink, there should only be 1)
   */
  std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList;
  
  uint32_t nodeId;
  uint8_t groupId;

  /**
   * The Sidelink synchronization signal identifier of the transmitting UE
   */
  uint64_t slssId;
  
};

}  // namespace ns3


#endif /* CV2X_LTE_SPECTRUM_SIGNAL_PARAMETERS_H */
