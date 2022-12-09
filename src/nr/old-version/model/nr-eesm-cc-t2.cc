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
#include "nr-eesm-cc-t2.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrEesmCcT2");
NS_OBJECT_ENSURE_REGISTERED (NrEesmCcT2);

NrEesmCcT2::NrEesmCcT2()
{

}

NrEesmCcT2::~NrEesmCcT2()
{

}

TypeId
NrEesmCcT2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrEesmCcT2")
    .SetParent<NrEesmCc> ()
    .AddConstructor<NrEesmCcT2> ()
  ;
  return tid;
}

const std::vector<double> *
NrEesmCcT2::GetBetaTable() const
{
  return m_t2.m_betaTable;
}

const std::vector<double> *
NrEesmCcT2::GetMcsEcrTable() const
{
  return m_t2.m_mcsEcrTable;
}

const NrEesmErrorModel::SimulatedBlerFromSINR *
NrEesmCcT2::GetSimulatedBlerFromSINR() const
{
  return m_t2.m_simulatedBlerFromSINR;
}

const std::vector<uint8_t> *
NrEesmCcT2::GetMcsMTable() const
{
  return m_t2.m_mcsMTable;
}

const std::vector<double> *
NrEesmCcT2::GetSpectralEfficiencyForMcs() const
{
  return m_t2.m_spectralEfficiencyForMcs;
}

const std::vector<double> *
NrEesmCcT2::GetSpectralEfficiencyForCqi() const
{
  return m_t2.m_spectralEfficiencyForCqi;
}

} // namespace ns3
