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
 */

#include "cv2x_sl-preconfig-pool-factory.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_SlPreconfigPoolFactory");

cv2x_SlPreconfigPoolFactory::cv2x_SlPreconfigPoolFactory ()
{
  m_scCpLen = "NORMAL";
  m_period = "sf40";
  m_scPrbNum = 22;
  m_scPrbStart = 0;
  m_scPrbEnd = 49;
  m_scOffset = 0;
  //m_scBitmapSize(40),
  m_scBitmapValue = 0xFF00000000;
  m_dataCpLen = "NORMAL";
  m_hoppingParameters = 0;
  m_subbands = "ns4";
  m_rbOffset = 0;
  m_ueSelected = true;
  m_haveTrptSubset = true;
  //m_trptSubsetSize(3),
  m_trptSubsetValue = 0x7;
  m_dataPrbNum = 25;
  m_dataPrbStart = 0;
  m_dataPrbEnd = 49;
  m_dataOffset = 8;
  //m_dataBitmapSize(40),
  m_dataBitmapValue = 0xFFFFFFFFFF;
  m_txParam = true;
  m_txAlpha = "al09";
  m_txP0 = -40;
  m_dataAlpha = "al09";
  m_dataP0 = -4;
  NS_LOG_FUNCTION (this);
}

cv2x_LteRrcSap::SlPreconfigCommPool
cv2x_SlPreconfigPoolFactory::CreatePool ()
{
  // Control
  //pool.scCpLen.cplen = cv2x_LteRrcSap::SlCpLen::NORMAL;
  if (m_scCpLen == "NORMAL") {m_pool.scCpLen.cplen = cv2x_LteRrcSap::SlCpLen::NORMAL;}
  else if (m_scCpLen == "EXTENDED") {m_pool.scCpLen.cplen = cv2x_LteRrcSap::SlCpLen::EXTENDED;}
  else {NS_FATAL_ERROR ("UNSUPPORTED CONTROL CP LENGTH");}
  //pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf40;
  if (m_period == "sf40") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf40;}
  else if (m_period == "sf60") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf60;}
  else if (m_period == "sf70") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf70;}
  else if (m_period == "sf80") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf80;}
  else if (m_period == "sf120") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf120;}
  else if (m_period == "sf140") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf140;}
  else if (m_period == "sf160") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf160;}
  else if (m_period == "sf240") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf240;}
  else if (m_period == "sf280") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf280;}
  else if (m_period == "sf320") {m_pool.scPeriod.period = cv2x_LteRrcSap::SlPeriodComm::sf320;}
  else {NS_FATAL_ERROR ("UNSUPPORTED CONTROL PERIOD LENGTH");}
  m_pool.scTfResourceConfig.prbNum = m_scPrbNum;
  m_pool.scTfResourceConfig.prbStart = m_scPrbStart;
  m_pool.scTfResourceConfig.prbEnd = m_scPrbEnd;
  m_pool.scTfResourceConfig.offsetIndicator.offset = m_scOffset;
  m_pool.scTfResourceConfig.subframeBitmap.bitmap = std::bitset<40> (m_scBitmapValue);//(0xFF00000000);
  //data
  //pool.dataCpLen.cplen = cv2x_LteRrcSap::SlCpLen::NORMAL;
  if (m_dataCpLen == "NORMAL") {m_pool.dataCpLen.cplen = cv2x_LteRrcSap::SlCpLen::NORMAL;}
  else if (m_dataCpLen == "EXTENDED") {m_pool.dataCpLen.cplen = cv2x_LteRrcSap::SlCpLen::EXTENDED;}
  else {NS_FATAL_ERROR ("UNSUPPORTED CONTROL CP LENGTH");}
  m_pool.dataHoppingConfig.hoppingParameters = 0;
  //pool.dataHoppingConfig.numSubbands = cv2x_LteRrcSap::SlHoppingConfigComm::ns4;
  if (m_subbands == "ns1") {m_pool.dataHoppingConfig.numSubbands = cv2x_LteRrcSap::SlHoppingConfigComm::ns1;}
  else if (m_subbands == "ns2") {m_pool.dataHoppingConfig.numSubbands = cv2x_LteRrcSap::SlHoppingConfigComm::ns2;}
  else if (m_subbands == "ns4") {m_pool.dataHoppingConfig.numSubbands = cv2x_LteRrcSap::SlHoppingConfigComm::ns4;}
  else {NS_FATAL_ERROR ("UNSUPPORTED NUMBER OF SUBBANDS");}
  m_pool.dataHoppingConfig.rbOffset = 0;
  //UE selected parameters
  m_pool.trptSubset.subset = std::bitset<3> (m_trptSubsetValue);
  m_pool.dataTfResourceConfig.prbNum = m_dataPrbNum;
  m_pool.dataTfResourceConfig.prbStart = m_dataPrbStart;
  m_pool.dataTfResourceConfig.prbEnd = m_dataPrbEnd;
  m_pool.dataTfResourceConfig.offsetIndicator.offset = m_dataOffset;
  m_pool.dataTfResourceConfig.subframeBitmap.bitmap = std::bitset<40> (m_dataBitmapValue);

  if (m_txAlpha == "al0") {m_pool.scTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al0;}
  else if (m_txAlpha == "al04") {m_pool.scTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al04;}
  else if (m_txAlpha == "al05") {m_pool.scTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al05;}
  else if (m_txAlpha == "al06") {m_pool.scTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al06;}
  else if (m_txAlpha == "al07") {m_pool.scTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al07;}
  else if (m_txAlpha == "al08") {m_pool.scTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al08;}
  else if (m_txAlpha == "al09") {m_pool.scTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al09;}
  else if (m_txAlpha == "al1") {m_pool.scTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al1;}
  else {NS_FATAL_ERROR ("UNSUPPORTED CONTROL TX ALPHA");}
  m_pool.scTxParameters.p0 = m_txP0;
  //pool.txParameters.dataTxParameters.alpha = m_dataAlpha;
  if (m_dataAlpha == "al0") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al0;}
  else if (m_dataAlpha == "al04") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al04;}
  else if (m_dataAlpha == "al05") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al05;}
  else if (m_dataAlpha == "al06") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al06;}
  else if (m_dataAlpha == "al07") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al07;}
  else if (m_dataAlpha == "al08") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al08;}
  else if (m_dataAlpha == "al09") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al09;}
  else if (m_dataAlpha == "al1") {m_pool.dataTxParameters.alpha = cv2x_LteRrcSap::SlTxParameters::al1;}
  else {NS_FATAL_ERROR ("UNSUPPORTED DATA TX ALPHA");}
  m_pool.dataTxParameters.p0 = m_dataP0;
  return m_pool;
}

} // namespace ns3
