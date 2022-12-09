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

#include <ns3/nr-point-to-point-epc-helper.h>
#include <ns3/log.h>
#include <ns3/object.h>
#include <ns3/node-container.h>
#include <ns3/net-device-container.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/epc-x2.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/lte-enb-rrc.h>
#include <ns3/epc-ue-nas.h>
#include <ns3/lte-sl-tft.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrPointToPointEpcHelper");

NS_OBJECT_ENSURE_REGISTERED (NrPointToPointEpcHelper);

NrPointToPointEpcHelper::NrPointToPointEpcHelper () : PointToPointEpcHelper ()
{
  NS_LOG_FUNCTION (this);
}

NrPointToPointEpcHelper::~NrPointToPointEpcHelper ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrPointToPointEpcHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrPointToPointEpcHelper")
    .SetParent<PointToPointEpcHelper> ()
    .SetGroupName ("nr")
    .AddConstructor<NrPointToPointEpcHelper> ()
  ;
  return tid;
}

void
NrPointToPointEpcHelper::DoAddX2Interface (const Ptr<EpcX2> &gnb1X2, const Ptr<NetDevice> &gnb1NetDev,
                                           const Ipv4Address &gnb1X2Address,
                                           const Ptr<EpcX2> &gnb2X2, const Ptr<NetDevice> &gnb2NetDev,
                                           const Ipv4Address &gnb2X2Address) const
{
  NS_LOG_FUNCTION (this);

  Ptr<NrGnbNetDevice> gnb1NetDevice = gnb1NetDev->GetObject<NrGnbNetDevice> ();
  Ptr<NrGnbNetDevice> gnb2NetDevice = gnb2NetDev->GetObject<NrGnbNetDevice> ();
  uint16_t gnb1CellId = gnb1NetDevice->GetCellId ();
  uint16_t gnb2CellId = gnb2NetDevice->GetCellId ();

  NS_ABORT_IF (gnb1NetDevice == nullptr);
  NS_ABORT_IF (gnb2NetDevice == nullptr);

  NS_LOG_LOGIC ("NrGnbNetDevice #1 = " << gnb1NetDev << " - CellId = " << gnb1CellId);
  NS_LOG_LOGIC ("NrGnbNetDevice #2 = " << gnb2NetDev << " - CellId = " << gnb2CellId);

  gnb1X2->AddX2Interface (gnb1CellId, gnb1X2Address, gnb2CellId, gnb2X2Address);
  gnb2X2->AddX2Interface (gnb2CellId, gnb2X2Address, gnb1CellId, gnb1X2Address);

  gnb1NetDevice->GetRrc ()->AddX2Neighbour (gnb2CellId);
  gnb2NetDevice->GetRrc ()->AddX2Neighbour (gnb1CellId);
}

void
NrPointToPointEpcHelper::DoActivateEpsBearerForUe (const Ptr<NetDevice> &ueDevice,
                                                   const Ptr<EpcTft> &tft,
                                                   const EpsBearer &bearer) const
{
  Ptr<NrUeNetDevice> ueNetDevice = ueDevice->GetObject<NrUeNetDevice> ();
  if (ueNetDevice)
    {
      Simulator::ScheduleNow (&EpcUeNas::ActivateEpsBearer, ueNetDevice->GetNas (), bearer, tft);
    }
  else
    {
      NS_FATAL_ERROR ("What kind of device are you trying to pass to the NR helper?");
    }
}

void
NrPointToPointEpcHelper::ActivateNrSlBearerForUe (const Ptr<NetDevice> &ueDevice, const Ptr<LteSlTft> &slTft) const
{
  Ptr<NrUeNetDevice> nrUeNetDevice = ueDevice->GetObject<NrUeNetDevice> ();
  if (nrUeNetDevice)
    {
      Simulator::ScheduleNow (&EpcUeNas::ActivateNrSlBearer, nrUeNetDevice->GetNas (), slTft);
    }
  else
    {
      NS_FATAL_ERROR ("What kind of device are you trying to pass to the NR helper?");
    }
}


} // namespace ns3
