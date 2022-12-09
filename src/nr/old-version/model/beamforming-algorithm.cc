/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "beamforming-algorithm.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("BeamformingAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (BeamformingAlgorithm);

BeamformingAlgorithm::BeamformingAlgorithm ()
{
}

BeamformingAlgorithm::~BeamformingAlgorithm()
{
}

TypeId
BeamformingAlgorithm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BeamformingAlgorithm")
                      .SetParent<Object> ()
  ;

  return tid;
}

} // end of ns3 namespace
