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
 */

#include "ns3/log.h"
#include "cv2x_lte-test-ue-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteTestUePhy");
 
NS_OBJECT_ENSURE_REGISTERED (cv2x_LteTestUePhy);

cv2x_LteTestUePhy::cv2x_LteTestUePhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

cv2x_LteTestUePhy::cv2x_LteTestUePhy (Ptr<cv2x_LteSpectrumPhy> dlPhy, Ptr<cv2x_LteSpectrumPhy> ulPhy)
  : cv2x_LtePhy (dlPhy, ulPhy)
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteTestUePhy::~cv2x_LteTestUePhy ()
{
}

void
cv2x_LteTestUePhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  cv2x_LtePhy::DoDispose ();
}

TypeId
cv2x_LteTestUePhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteTestUePhy")
    .SetParent<cv2x_LtePhy> ()
    .AddConstructor<cv2x_LteTestUePhy> ()
  ;
  return tid;
}

void
cv2x_LteTestUePhy::DoSendMacPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
}

Ptr<SpectrumValue>
cv2x_LteTestUePhy::CreateTxPowerSpectralDensity ()
{
  NS_LOG_FUNCTION (this);
  Ptr<SpectrumValue> psd;

  return psd;
}

void
cv2x_LteTestUePhy::GenerateCtrlCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);

  // Store calculated SINR, it will be retrieved at the end of the test
  m_sinr = sinr;
}

void
cv2x_LteTestUePhy::GenerateDataCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);
  
  // Store calculated SINR, it will be retrieved at the end of the test
  m_sinr = sinr;
}

void
cv2x_LteTestUePhy::ReportRsReceivedPower (const SpectrumValue& power)
{
  NS_LOG_FUNCTION (this);
  // Not used by the cv2x_LteTestUePhy
}

void
cv2x_LteTestUePhy::ReportInterference (const SpectrumValue& interf)
{
  NS_LOG_FUNCTION (this);
  // Not used by the cv2x_LteTestUePhy
}

void
cv2x_LteTestUePhy::ReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
}

SpectrumValue
cv2x_LteTestUePhy::GetSinr ()
{
  NS_LOG_FUNCTION (this);

  return m_sinr;
}


} // namespace ns3
