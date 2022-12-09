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
*/

#include "lena-error-model.h"
#include <ns3/log.h>
#include <ns3/lte-amc.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LenaErrorModel");
NS_OBJECT_ENSURE_REGISTERED (LenaErrorModel);

LenaErrorModel::LenaErrorModel () : NrLteMiErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

LenaErrorModel::~LenaErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t
LenaErrorModel::GetPayloadSize ([[maybe_unused]] uint32_t usefulSC, uint8_t mcs, uint32_t rbNum, NrErrorModel::Mode mode) const
{
  NS_LOG_FUNCTION (this);

  // Since we call here LteAmc which assumes that for data is assigned 11 or 13 symbols
  // (11 DL and 13 UL).
  // In DL we will assign o DATA always 13 symbols in OFDMA, but in UL since we have
  // UL CTRL and at least 1 symbol for SRS, there will be 12 symbols for DATA.
  // We need to check before converting RBnum in symbols to RBnum in slots
  // whether the duration is 11, 12 or 13 to be able to convert it properly.
  // Make sure to configure the number of SRS symbols to be 1
  // e.g.: nrHelper->SetSchedulerAttribute ("SrsSymbols", UintegerValue (1));

  uint32_t lenaRBNum = 0;

  if (rbNum < 11)
    {
      NS_LOG_INFO ("Available resources are smaller than the minimum allowed in order to use the LENA AMC model.");
      return 0;
    }

  if (rbNum % 11 == 0)
    {
      lenaRBNum = rbNum/11;
    }
  else if (rbNum % 12 == 0)
    {
      lenaRBNum = rbNum/12;
    }
  else
    {
      lenaRBNum = rbNum/13;
    }

  static LteAmc lenaAmc;

  NS_LOG_DEBUG ("Asking LENA AMC to retrieve the TBS for MCS " << +mcs << " and RB " << lenaRBNum);

  if (mode == NrErrorModel::DL)
    {
      return (lenaAmc.GetDlTbSizeFromMcs (mcs, lenaRBNum) / 8);
    }
  else
    {
      return (lenaAmc.GetUlTbSizeFromMcs (mcs, lenaRBNum) / 8);
    }
}

TypeId
LenaErrorModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LenaErrorModel")
    .SetParent<NrLteMiErrorModel> ()
    .AddConstructor<LenaErrorModel> ()
  ;
  return tid;
}

TypeId
LenaErrorModel::GetInstanceTypeId() const
{
  return LenaErrorModel::GetTypeId ();
}

}
