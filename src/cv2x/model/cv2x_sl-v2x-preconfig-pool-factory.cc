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
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#include "cv2x_sl-v2x-preconfig-pool-factory.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_SlV2xPreconfigPoolFactory");

cv2x_SlV2xPreconfigPoolFactory::cv2x_SlV2xPreconfigPoolFactory ()
{
  m_ueSelected = true;
  m_adjacencyPscchPssch = true; 
  m_slSubframe = std::bitset<20>(0xFFFFF);
  m_sizeSubchannel = 10;
  m_numSubchannel = 5;
  m_startRbSubchannel = 0;
  m_startRbPscchPool = 0; 
  m_dataP0 = -4;
  m_dataAlpha = 0.9;

  NS_LOG_FUNCTION (this);
}

cv2x_LteRrcSap::SlV2xPreconfigCommPool
cv2x_SlV2xPreconfigPoolFactory::CreatePool ()
{
  m_pool.slSubframeV2x.bitmap = std::bitset<20>(m_slSubframe); 

  m_pool.adjacencyPscchPssch.adjacency = m_adjacencyPscchPssch; 

  if (m_adjacencyPscchPssch == true) {
    switch (m_sizeSubchannel)
    {
      case 5:
      case 6:
      case 10:
      case 20:
      case 25:
      case 50:
      case 75:
      case 100:
        m_pool.sizeSubchannel = cv2x_LteRrcSap::SizeSubchannelFromInt (m_sizeSubchannel);
        break;
      case 4:
      case 8:
      case 9:
      case 12:
      case 16:
      case 18:
      case 30:
      case 48:
      case 72:
      case 96:
      default:
        NS_FATAL_ERROR ("UNSUPPORTED SUBCHANNEL SIZE FOR ADJACENT MODE: " << m_sizeSubchannel);
        break;
    }
  }
  else {
    switch (m_sizeSubchannel)
    {
      case 4:
      case 5:
      case 6:
      case 8:
      case 9:
      case 10:
      case 12:
      case 16:
      case 18:
      case 20:
      case 30:
      case 48:
      case 72:
      case 96:
        m_pool.sizeSubchannel = cv2x_LteRrcSap::SizeSubchannelFromInt (m_sizeSubchannel);
        break;
      case 25:
      case 50:
      case 75:
      case 100:
      default:
        NS_FATAL_ERROR ("UNSUPPORTED SUBCHANNEL SIZE FOR NON-ADJACENT MODE: " << m_sizeSubchannel);
        break;
    }
  }
  
  /*
  if (m_sizeSubchannel == "n4" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n4;}
  else if (m_sizeSubchannel == "n5"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n5;}
  else if (m_sizeSubchannel == "n6"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n6;}
  else if (m_sizeSubchannel == "n8" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n8;}
  else if (m_sizeSubchannel == "n9" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n9;}
  else if (m_sizeSubchannel == "n10"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n10;}
  else if (m_sizeSubchannel == "n12" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n12;}
  else if (m_sizeSubchannel == "n15"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n15;}
  else if (m_sizeSubchannel == "n16" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n16;}
  else if (m_sizeSubchannel == "n18" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n18;}
  else if (m_sizeSubchannel == "n20"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n20;}
  else if (m_sizeSubchannel == "n25" && m_adjacencyPscchPssch == true){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n25;}
  else if (m_sizeSubchannel == "n30" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n30;}
  else if (m_sizeSubchannel == "n48" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n48;}
  else if (m_sizeSubchannel == "n50" && m_adjacencyPscchPssch == true){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n50;}
  else if (m_sizeSubchannel == "n72" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n72;}
  else if (m_sizeSubchannel == "n75" && m_adjacencyPscchPssch == true){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n75;}
  else if (m_sizeSubchannel == "n96" && m_adjacencyPscchPssch == false){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n96;}  
  else if (m_sizeSubchannel == "n100" && m_adjacencyPscchPssch == true){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::n100;}
  else if (m_sizeSubchannel == "spare13"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare13;}
  else if (m_sizeSubchannel == "spare12"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare12;}
  else if (m_sizeSubchannel == "spare11"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare11;}
  else if (m_sizeSubchannel == "spare10"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare10;}
  else if (m_sizeSubchannel == "spare9"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare9;}
  else if (m_sizeSubchannel == "spare8"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare8;}
  else if (m_sizeSubchannel == "spare7"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare7;}
  else if (m_sizeSubchannel == "spare6"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare6;}
  else if (m_sizeSubchannel == "spare5"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare5;}
  else if (m_sizeSubchannel == "spare4"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare4;}
  else if (m_sizeSubchannel == "spare3"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare3;}
  else if (m_sizeSubchannel == "spare2"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare2;}
  else if (m_sizeSubchannel == "spare1"){m_pool.sizeSubchannel.size = cv2x_LteRrcSap::SizeSubchannel::spare1;}
  else {NS_FATAL_ERROR ("UNSUPPORTED SUBCHANNEL SIZE");}
  */

  m_pool.numSubchannel = cv2x_LteRrcSap::cv2x_NumSubchannelFromInt (m_numSubchannel);
  /*
  if (m_numSubchannel == "n1"){m_pool.numSubchannel.num = cv2x_LteRrcSap::cv2x_NumSubchannel::n1;}
  else if (m_numSubchannel == "n3"){m_pool.numSubchannel.num = cv2x_LteRrcSap::cv2x_NumSubchannel::n3;}
  else if (m_numSubchannel == "n5"){m_pool.numSubchannel.num = cv2x_LteRrcSap::cv2x_NumSubchannel::n5;}
  else if (m_numSubchannel == "n8"){m_pool.numSubchannel.num = cv2x_LteRrcSap::cv2x_NumSubchannel::n8;}
  else if (m_numSubchannel == "n10"){m_pool.numSubchannel.num = cv2x_LteRrcSap::cv2x_NumSubchannel::n10;}
  else if (m_numSubchannel == "n15"){m_pool.numSubchannel.num = cv2x_LteRrcSap::cv2x_NumSubchannel::n15;}
  else if (m_numSubchannel == "n20"){m_pool.numSubchannel.num = cv2x_LteRrcSap::cv2x_NumSubchannel::n20;}
  else if (m_numSubchannel == "spare1"){m_pool.numSubchannel.num = cv2x_LteRrcSap::cv2x_NumSubchannel::spare1;}
  else {NS_FATAL_ERROR ("UNSUPPORTED SUBCHANNEL NUMBER");}
  */

  m_pool.startRbSubchannel.startRb = m_startRbSubchannel; 


  m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::AlphaFromDouble (m_dataAlpha);
  /*
  if (m_dataAlpha == "al0") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al0;}
  else if (m_dataAlpha == "al04") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al04;}
  else if (m_dataAlpha == "al05") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al05;}
  else if (m_dataAlpha == "al06") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al06;}
  else if (m_dataAlpha == "al07") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al07;}
  else if (m_dataAlpha == "al08") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al08;}
  else if (m_dataAlpha == "al09") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al09;}
  else if (m_dataAlpha == "al1") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al1;}
  else {NS_FATAL_ERROR ("UNSUPPORTED DATA TX ALPHA");}
  */
  
  m_pool.dataTxParameters.p0 = m_dataP0;

  m_pool.startRbPscchPool.startRb = m_startRbPscchPool;
  
/*
  // Optional parameter 
  m_pool.slOffsetIndicator.offset = m_slOffsetIndicator;
  m_pool.zoneId.id = m_zoneId; 
  m_pool.threshSrssiCbr.thresh = m_threshSrssiCbr;
  m_pool.cbrPsschTxConfigList =  
  m_pool.resourceSelectionConfigP2x = ... 
  m_pool.restrictResourceReservationPeriod = m_restrictResourceReservationPeriod; 
*/

  return m_pool; 
}

} // namespace ns3
