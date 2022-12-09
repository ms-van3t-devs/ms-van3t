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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);
#include "nr-mac-scheduler-cqi-management.h"
#include <ns3/nr-spectrum-value-helper.h>
#include "nr-amc.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerCQIManagement");

void
NrMacSchedulerCQIManagement::DlSBCQIReported ([[maybe_unused]]const DlCqiInfo &info,
                                              [[maybe_unused]] const std::shared_ptr<NrMacSchedulerUeInfo>&ueInfo) const
{
  NS_LOG_INFO (this);
  // TODO
}

void
NrMacSchedulerCQIManagement::UlSBCQIReported (uint32_t expirationTime, [[maybe_unused]] uint32_t tbs,
                                              const NrMacSchedSapProvider::SchedUlCqiInfoReqParameters& params,
                                              const std::shared_ptr<NrMacSchedulerUeInfo> &ueInfo,
                                              const std::vector<uint8_t> &rbgMask,
                                              uint32_t numRbPerRbg,
                                              const Ptr<const SpectrumModel> &model) const
{
  NS_LOG_INFO (this);
  NS_ASSERT (rbgMask.size () > 0);

  NS_LOG_INFO ("Computing SB CQI for UE " << ueInfo->m_rnti);

  ueInfo->m_ulCqi.m_sinr = params.m_ulCqi.m_sinr;
  ueInfo->m_ulCqi.m_cqiType = NrMacSchedulerUeInfo::CqiInfo::SB;
  ueInfo->m_ulCqi.m_timer = expirationTime;

  std::vector<int> rbAssignment (params.m_ulCqi.m_sinr.size (), 0);

  for (uint32_t i = 0; i < rbgMask.size (); ++i)
    {
      if (rbgMask.at (i) == 1)
        {
          for (uint32_t k = 0; k < numRbPerRbg; ++k)
            {
              rbAssignment[i * numRbPerRbg + k] = 1;
            }
        }
    }

  SpectrumValue specVals (model);
  Values::iterator specIt = specVals.ValuesBegin ();

  std::stringstream out;

  for (uint32_t ichunk = 0; ichunk < model->GetNumBands (); ichunk++)
    {
      NS_ASSERT (specIt != specVals.ValuesEnd ());
      if (rbAssignment[ichunk] == 1)
        {
          *specIt = ueInfo->m_ulCqi.m_sinr.at (ichunk);
          out << ueInfo->m_ulCqi.m_sinr.at (ichunk) << " ";
        }
      else
        {
          out << "0.0 ";
          *specIt = 0.0;
        }

      specIt++;
    }

  NS_LOG_INFO ("Values of SINR to pass to the AMC: " << out.str ());

  // MCS updated inside the function; crappy API... but we can't fix everything
  ueInfo->m_ulCqi.m_cqi = GetAmcUl ()->CreateCqiFeedbackWbTdma (specVals, ueInfo->m_ulMcs);
  NS_LOG_DEBUG ("Calculated MCS for RNTI " << ueInfo->m_rnti << " is " << ueInfo->m_ulMcs);
}

void
NrMacSchedulerCQIManagement::InstallGetBwpIdFn (const std::function<uint16_t ()> &fn)
{
  NS_LOG_FUNCTION (this);
  m_getBwpId = fn;
}

void
NrMacSchedulerCQIManagement::InstallGetCellIdFn (const std::function<uint16_t ()> &fn)
{
  NS_LOG_FUNCTION (this);
  m_getCellId = fn;
}

void
NrMacSchedulerCQIManagement::InstallGetStartMcsDlFn (const std::function<uint8_t ()> &fn)
{
  NS_LOG_FUNCTION (this);
  m_getStartMcsDl = fn;
}

void
NrMacSchedulerCQIManagement::InstallGetStartMcsUlFn (const std::function<uint8_t ()> &fn)
{
  NS_LOG_FUNCTION (this);
  m_getStartMcsUl = fn;
}

void
NrMacSchedulerCQIManagement::InstallGetNrAmcDlFn (const std::function<Ptr<const NrAmc> ()> &fn)
{
  NS_LOG_FUNCTION (this);
  m_getAmcDl = fn;
}

void
NrMacSchedulerCQIManagement::InstallGetNrAmcUlFn (const std::function<Ptr<const NrAmc> ()> &fn)
{
  NS_LOG_FUNCTION (this);
  m_getAmcUl = fn;
}

void
NrMacSchedulerCQIManagement::DlWBCQIReported (const DlCqiInfo &info,
                                                  const std::shared_ptr<NrMacSchedulerUeInfo> &ueInfo,
                                                  uint32_t expirationTime, int8_t maxDlMcs) const
{
  NS_LOG_INFO (this);

  ueInfo->m_dlCqi.m_cqiType = NrMacSchedulerUeInfo::DlCqiInfo::WB;
  ueInfo->m_dlCqi.m_timer = expirationTime;
  ueInfo->m_dlCqi.m_ri = info.m_ri;
  ueInfo->m_dlCqi.m_wbCqi.resize (info.m_wbCqi.size ());
  ueInfo->m_dlMcs.resize (info.m_wbCqi.size ());
  //TODO following code limits the number of streams
  //2. In future, we should try to lift this
  //limit.
  if (info.m_ri == 2 || info.m_wbCqi.size () == 2)
    {
      if (ueInfo->m_dlTbSize.size () == 1)
        {
          //scheduling first time the TB of the second stream
          ueInfo->m_dlTbSize.push_back (0);
        }
    }
  for (uint8_t stream = 0; stream < info.m_wbCqi.size (); stream++)
    {
      if (info.m_wbCqi.at (stream) > 0)
        {
          ueInfo->m_dlCqi.m_wbCqi.at (stream) = (info.m_wbCqi.at (stream));
          uint8_t mcs = std::min(static_cast<uint8_t> (GetAmcDl()->GetMcsFromCqi (info.m_wbCqi.at (stream))), static_cast<uint8_t> (maxDlMcs));
          ueInfo->m_dlMcs.at (stream) = mcs;
          NS_LOG_INFO ("Calculated MCS for UE " << ueInfo->m_rnti
                       << " stream index " << static_cast<uint16_t> (stream)
                       << " MCS " << static_cast<uint32_t> (ueInfo->m_dlMcs.at (stream)));

          NS_LOG_INFO ("Updated WB CQI of UE " << ueInfo->m_rnti
                       << " stream index " << static_cast<uint16_t> (stream)
                       << " CQI " << static_cast<uint16_t> (ueInfo->m_dlCqi.m_wbCqi.at (stream))
                       << ". It will expire in " << ueInfo->m_dlCqi.m_timer << " slots.");
        }
      else
        {
          ueInfo->m_dlCqi.m_wbCqi.at (stream) = 0;
          //TODO
          //ueInfo->m_dlMcs.at (stream) = UINT8_MAX;
          //I am not happy to assign MCS 0 for CQI 0, however, this was the
          //original behavior of the NR code before MIMO implementation.
          //It was computing the MCS the way this code is doing for cqi > 0.
          //I need to adapt at the old way so all the old test written
          //based on this assumption could pass. I would like to update those
          //tests one day but today is not that day.
          ueInfo->m_dlMcs.at (stream) = 0;
          NS_LOG_INFO ("Not updated WB CQI of UE " << ueInfo->m_rnti
                       << " stream index " << static_cast<uint16_t> (stream)
                       << " CQI " << static_cast<uint16_t> (ueInfo->m_dlCqi.m_wbCqi.at (stream)));
        }
    }
}

void
NrMacSchedulerCQIManagement::RefreshDlCqiMaps (const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > &ueMap) const
{
  NS_LOG_FUNCTION (this);

  for (const auto &itUe : ueMap)
    {
      const std::shared_ptr<NrMacSchedulerUeInfo>&ue = itUe.second;

      if (ue->m_dlCqi.m_timer == 0)
        {
          ue->m_dlCqi.m_cqiType = NrMacSchedulerUeInfo::DlCqiInfo::WB;
          for (uint8_t stream = 0; stream < ue->m_dlCqi.m_wbCqi.size (); stream++)
            {
              ue->m_dlCqi.m_wbCqi.at (stream) = 1; // lowest value for trying a transmission
              ue->m_dlMcs.at (stream) = GetStartMcsDl ();
            }
        }
      else
        {
          ue->m_dlCqi.m_timer -= 1;
        }
    }
}

void
NrMacSchedulerCQIManagement::RefreshUlCqiMaps (const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > &ueMap) const
{
  NS_LOG_FUNCTION (this);

  for (const auto &itUe : ueMap)
    {
      const std::shared_ptr<NrMacSchedulerUeInfo>&ue = itUe.second;

      if (ue->m_ulCqi.m_timer == 0)
        {
          ue->m_ulCqi.m_cqi = 1; // lowest value for trying a transmission
          ue->m_ulCqi.m_cqiType = NrMacSchedulerUeInfo::CqiInfo::WB;
          ue->m_ulMcs = GetStartMcsUl ();
        }
      else
        {
          ue->m_ulCqi.m_timer -= 1;
        }
    }
}

uint16_t
NrMacSchedulerCQIManagement::GetBwpId () const
{
  return m_getBwpId ();
}

uint16_t
NrMacSchedulerCQIManagement::GetCellId () const
{
  return m_getCellId ();
}

uint8_t
NrMacSchedulerCQIManagement::GetStartMcsDl() const
{
  return m_getStartMcsDl ();
}

uint8_t
NrMacSchedulerCQIManagement::GetStartMcsUl() const
{
  return m_getStartMcsUl ();
}

Ptr<const NrAmc>
NrMacSchedulerCQIManagement::GetAmcDl() const
{
  return m_getAmcDl ();
}

Ptr<const NrAmc>
NrMacSchedulerCQIManagement::GetAmcUl() const
{
  return m_getAmcUl ();
}

} // namespace ns3
