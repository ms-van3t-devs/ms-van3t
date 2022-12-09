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
#include "nr-eesm-cc-t1.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrEesmCcT1");
NS_OBJECT_ENSURE_REGISTERED (NrEesmCcT1);

NrEesmCcT1::NrEesmCcT1()
{

}

NrEesmCcT1::~NrEesmCcT1()
{

}

TypeId
NrEesmCcT1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrEesmCcT1")
    .SetParent<NrEesmCc> ()
    .AddConstructor<NrEesmCcT1> ()
  ;
  return tid;
}

const std::vector<double> *
NrEesmCcT1::GetBetaTable() const
{
  return m_t1.m_betaTable;
}

const std::vector<double> *
NrEesmCcT1::GetMcsEcrTable() const
{
  return m_t1.m_mcsEcrTable;
}

const NrEesmErrorModel::SimulatedBlerFromSINR *
NrEesmCcT1::GetSimulatedBlerFromSINR() const
{
  return m_t1.m_simulatedBlerFromSINR;
}

const std::vector<uint8_t> *
NrEesmCcT1::GetMcsMTable() const
{
  return m_t1.m_mcsMTable;
}

const std::vector<double> *
NrEesmCcT1::GetSpectralEfficiencyForMcs() const
{
  return m_t1.m_spectralEfficiencyForMcs;
}

const std::vector<double> *
NrEesmCcT1::GetSpectralEfficiencyForCqi() const
{
  return m_t1.m_spectralEfficiencyForCqi;
}

} // namespace ns3
