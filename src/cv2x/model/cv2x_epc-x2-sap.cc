/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "ns3/cv2x_epc-x2-sap.h"

namespace ns3 {


cv2x_EpcX2Sap::~cv2x_EpcX2Sap ()
{
}

cv2x_EpcX2Sap::ErabToBeSetupItem::ErabToBeSetupItem () :
  erabLevelQosParameters (cv2x_EpsBearer (cv2x_EpsBearer::GBR_CONV_VOICE))
{
}

cv2x_EpcX2SapProvider::~cv2x_EpcX2SapProvider ()
{
}

cv2x_EpcX2SapUser::~cv2x_EpcX2SapUser ()
{
}

} // namespace ns3
