/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
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
 * Modified by: Marco Miozzo <mmiozzo@cttc.es> convert to
 *               cv2x_LteSpectrumSignalParametersDlCtrlFrame framework
 */


#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/simulator.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/cv2x_lte-spectrum-signal-parameters.h>
#include <ns3/antenna-model.h>

#include "cv2x_rem-spectrum-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_RemSpectrumPhy");

NS_OBJECT_ENSURE_REGISTERED (cv2x_RemSpectrumPhy);

cv2x_RemSpectrumPhy::cv2x_RemSpectrumPhy ()
  : m_mobility (0),
    m_referenceSignalPower (0),
    m_sumPower (0),
    m_active (true),
    m_useDataChannel (false),
    m_rbId (-1)
{
  NS_LOG_FUNCTION (this);
}



cv2x_RemSpectrumPhy::~cv2x_RemSpectrumPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_RemSpectrumPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_mobility = 0;
  SpectrumPhy::DoDispose ();
}

TypeId
cv2x_RemSpectrumPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_RemSpectrumPhy")
    .SetParent<SpectrumPhy> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_RemSpectrumPhy> ()
  ;
  return tid;
}



void
cv2x_RemSpectrumPhy::SetChannel (Ptr<SpectrumChannel> c)
{
  // this is a no-op, cv2x_RemSpectrumPhy does not transmit hence it does not need a reference to the channel
}

void
cv2x_RemSpectrumPhy::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);
  m_mobility = m;
}

void
cv2x_RemSpectrumPhy::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  // this is a no-op, cv2x_RemSpectrumPhy does not handle any data hence it does not support the use of a NetDevice
}

Ptr<MobilityModel>
cv2x_RemSpectrumPhy::GetMobility ()
{
  return m_mobility;
}

Ptr<NetDevice>
cv2x_RemSpectrumPhy::GetDevice () const
{
  return 0;
}

Ptr<const SpectrumModel>
cv2x_RemSpectrumPhy::GetRxSpectrumModel () const
{
  return m_rxSpectrumModel;
}

Ptr<AntennaModel>
cv2x_RemSpectrumPhy::GetRxAntenna ()
{
  return 0;
}


void
cv2x_RemSpectrumPhy::StartRx (Ptr<SpectrumSignalParameters> params)
{
  NS_LOG_FUNCTION ( this << params);

  if (m_active)
    {
      if (m_useDataChannel)
        {
          Ptr<cv2x_LteSpectrumSignalParametersDataFrame> lteDlDataRxParams = DynamicCast<cv2x_LteSpectrumSignalParametersDataFrame> (params);
          if (lteDlDataRxParams != 0)
            {
              NS_LOG_DEBUG ("StartRx data");
              double power = 0;
              if (m_rbId >= 0)
                {
                  power = (*(params->psd))[m_rbId] * 180000;
                }
              else
                {
                  power = Integral (*(params->psd));
                }

              m_sumPower += power;
              if (power > m_referenceSignalPower)
                {
                  m_referenceSignalPower = power;
                }
            }
        }
      else
        {
          Ptr<cv2x_LteSpectrumSignalParametersDlCtrlFrame> lteDlCtrlRxParams = DynamicCast<cv2x_LteSpectrumSignalParametersDlCtrlFrame> (params);
          if (lteDlCtrlRxParams != 0)
            {
              NS_LOG_DEBUG ("StartRx control");
              double power = 0;
              if (m_rbId >= 0)
                {
                  power = (*(params->psd))[m_rbId] * 180000;
                }
              else
                {
                  power = Integral (*(params->psd));
                }

              m_sumPower += power;
              if (power > m_referenceSignalPower)
                {
                  m_referenceSignalPower = power;
                }
            }
        }
    }
}

void
cv2x_RemSpectrumPhy::SetRxSpectrumModel (Ptr<const SpectrumModel> m)
{
  NS_LOG_FUNCTION (this << m);
  m_rxSpectrumModel = m;
}

double
cv2x_RemSpectrumPhy::GetSinr (double noisePower)
{
  return m_referenceSignalPower / (m_sumPower - m_referenceSignalPower + noisePower);
}

void
cv2x_RemSpectrumPhy::Deactivate ()
{
  m_active = false;
}

bool
cv2x_RemSpectrumPhy::IsActive ()
{
  return m_active;
}

void
cv2x_RemSpectrumPhy::Reset ()
{
  m_referenceSignalPower = 0;
  m_sumPower = 0;
}

void
cv2x_RemSpectrumPhy::SetUseDataChannel (bool value)
{
  m_useDataChannel = value;
}

void
cv2x_RemSpectrumPhy::SetRbId (int32_t rbId)
{
  m_rbId = rbId;
}


} // namespace ns3
