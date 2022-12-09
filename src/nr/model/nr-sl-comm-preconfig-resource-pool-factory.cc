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

#include "nr-sl-comm-preconfig-resource-pool-factory.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlCommPreconfigResourcePoolFactory");

NrSlCommPreconfigResourcePoolFactory::NrSlCommPreconfigResourcePoolFactory ()
{
  NS_LOG_FUNCTION (this);
  m_setupReleasePscch = "SETUP";
  m_slTimeResourcePscch = 1;
  m_slFreqResourcePscch = 10;
  m_slSubchannelSize = 50;
  m_slSensingWindow = 100;
  m_slSelectionWindow = 5;
  m_slResourceReservePeriodList = {0, 10, 20, 50, 100, 150, 200, 250, 300, 350, 400, 500, 550, 600, 750, 1000};
  m_slTimeResource = {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1};
  m_slMaxNumPerReserve = 2;
}

NrSlCommPreconfigResourcePoolFactory::~NrSlCommPreconfigResourcePoolFactory ()
{
  NS_LOG_FUNCTION (this);
}
const LteRrcSap::SlResourcePoolNr
NrSlCommPreconfigResourcePoolFactory::CreatePool ()
{
  NS_LOG_FUNCTION (this);
  if (m_setupReleasePscch == "SETUP")
    {
      m_pool.slPscchConfig.setupRelease = LteRrcSap::SlPscchConfig::SETUP;
    }
  else if (m_setupReleasePscch == "RELEASE")
    {
      NS_FATAL_ERROR ("Releasing of PSCCH resources is not supported");
    }
  else
    {
      NS_FATAL_ERROR ("Invalid setuprelease option : " << m_setupReleasePscch << " for SlPscchConfig");
    }

  switch (m_slTimeResourcePscch)
    {
    case 1:
      m_pool.slPscchConfig.slTimeResourcePscch.resources = LteRrcSap::SlTimeResourcePscch::N1;
      break;
    case 2:
      m_pool.slPscchConfig.slTimeResourcePscch.resources = LteRrcSap::SlTimeResourcePscch::N2;
      break;
    case 3:
      m_pool.slPscchConfig.slTimeResourcePscch.resources = LteRrcSap::SlTimeResourcePscch::N3;
      break;
    default:
      NS_FATAL_ERROR ("Invalid number of symbols : " << m_slTimeResourcePscch << " chosen for SL PSCCH");
    }

  switch (m_slFreqResourcePscch)
    {
    case 10:
      m_pool.slPscchConfig.slFreqResourcePscch.resources = LteRrcSap::SlFreqResourcePscch::N10;
      break;
    case 12:
      m_pool.slPscchConfig.slFreqResourcePscch.resources = LteRrcSap::SlFreqResourcePscch::N12;
      break;
    case 15:
      m_pool.slPscchConfig.slFreqResourcePscch.resources = LteRrcSap::SlFreqResourcePscch::N15;
      break;
    case 20:
      m_pool.slPscchConfig.slFreqResourcePscch.resources = LteRrcSap::SlFreqResourcePscch::N20;
      break;
    case 25:
      m_pool.slPscchConfig.slFreqResourcePscch.resources = LteRrcSap::SlFreqResourcePscch::N25;
      break;
    default:
      NS_FATAL_ERROR ("Invalid number of RBs : " << m_slFreqResourcePscch << " chosen for SL PSCCH");
    }

  switch (m_slSubchannelSize)
    {
    case 10:
      m_pool.slSubchannelSize.numPrbs = LteRrcSap::SlSubchannelSize::N10;
      break;
    case 15:
      m_pool.slSubchannelSize.numPrbs = LteRrcSap::SlSubchannelSize::N15;
      break;
    case 20:
      m_pool.slSubchannelSize.numPrbs = LteRrcSap::SlSubchannelSize::N20;
      break;
    case 25:
      m_pool.slSubchannelSize.numPrbs = LteRrcSap::SlSubchannelSize::N25;
      break;
    case 50:
      m_pool.slSubchannelSize.numPrbs = LteRrcSap::SlSubchannelSize::N50;
      break;
    case 75:
      m_pool.slSubchannelSize.numPrbs = LteRrcSap::SlSubchannelSize::N75;
      break;
    case 100:
      m_pool.slSubchannelSize.numPrbs = LteRrcSap::SlSubchannelSize::N100;
      break;
    default:
      NS_FATAL_ERROR ("Invalid subchannel size in RBs : " << m_slSubchannelSize);
    }

  switch (m_slSensingWindow)
    {
    case 100:
      m_pool.slUeSelectedConfigRp.slSensingWindow.windSize = LteRrcSap::SlSensingWindow::MS100;
      break;
    case 1100:
      m_pool.slUeSelectedConfigRp.slSensingWindow.windSize = LteRrcSap::SlSensingWindow::MS1100;
      break;
    default:
      NS_FATAL_ERROR ("Invalid sensing window size : " << m_slSensingWindow);
    }

  switch (m_slSelectionWindow)
    {
    case 1:
      m_pool.slUeSelectedConfigRp.slSelectionWindow.windSize = LteRrcSap::SlSelectionWindow::N1;
      break;
    case 5:
      m_pool.slUeSelectedConfigRp.slSelectionWindow.windSize = LteRrcSap::SlSelectionWindow::N5;
      break;
    case 10:
      m_pool.slUeSelectedConfigRp.slSelectionWindow.windSize = LteRrcSap::SlSelectionWindow::N10;
      break;
    case 20:
      m_pool.slUeSelectedConfigRp.slSelectionWindow.windSize = LteRrcSap::SlSelectionWindow::N20;
      break;
    default:
      NS_FATAL_ERROR ("Invalid selection window size : " << m_slSelectionWindow);
    }

  uint16_t index = 0;
  LteRrcSap::SlResourceReservePeriod slReserved;
  for (const auto i : m_slResourceReservePeriodList)
    {
      switch (i)
        {
        case 0:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS0;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 10:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS10;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 20:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS20;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 30:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS30;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 40:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS40;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 50:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS50;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 60:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS60;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 70:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS70;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 80:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS80;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 90:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS90;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 100:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS100;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 150:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS150;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 200:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS200;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 250:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS250;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 300:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS300;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 350:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS350;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 400:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS400;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 450:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS450;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 500:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS500;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 550:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS550;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 600:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS600;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 650:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS650;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 700:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS700;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 750:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS750;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 800:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS800;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 850:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS850;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 900:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS900;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 950:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS950;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        case 1000:
          slReserved.period = LteRrcSap::SlResourceReservePeriod::MS1000;
          m_pool.slUeSelectedConfigRp.slResourceReservePeriodList [index] = slReserved;
          ++index;
          break;
        default:
          NS_FATAL_ERROR ("Invalid sidelink reservation period : " << i << " used");
        }
    }

  switch (m_slMaxNumPerReserve)
    {
    case 1:
      m_pool.slUeSelectedConfigRp.slMaxNumPerReserve.maxNumPerRes = LteRrcSap::SlMaxNumPerReserve::N1;
      break;
    case 2:
      m_pool.slUeSelectedConfigRp.slMaxNumPerReserve.maxNumPerRes = LteRrcSap::SlMaxNumPerReserve::N2;
      break;
    case 3:
      m_pool.slUeSelectedConfigRp.slMaxNumPerReserve.maxNumPerRes = LteRrcSap::SlMaxNumPerReserve::N3;
      break;
    default:
      NS_FATAL_ERROR ("Invalid sidelink value " << m_slMaxNumPerReserve << " used for number SlMaxNumPerReserve");
    }

  m_pool.slTimeResource = m_slTimeResource;

  return m_pool;
}

} // namespace ns3

