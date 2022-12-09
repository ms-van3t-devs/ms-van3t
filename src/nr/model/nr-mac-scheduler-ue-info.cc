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

#include "nr-mac-scheduler-ue-info.h"
#include <ns3/log.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerUeInfo");

NrMacSchedulerUeInfo::NrMacSchedulerUeInfo (uint16_t rnti, BeamConfId beamConfId, const GetRbPerRbgFn &fn) :
        m_rnti (rnti),
        m_beamConfId (beamConfId),
        m_getNumRbPerRbg (fn)
{
}

NrMacSchedulerUeInfo::~NrMacSchedulerUeInfo ()
{
}

uint32_t &
NrMacSchedulerUeInfo::GetDlRBG (const UePtr &ue)
{
  return ue->m_dlRBG;
}

uint32_t &
NrMacSchedulerUeInfo::GetUlRBG (const UePtr &ue)
{
  return ue->m_ulRBG;
}

uint8_t &
NrMacSchedulerUeInfo::GetDlSym (const UePtr &ue)
{
  return ue->m_dlSym;
}

uint8_t &
NrMacSchedulerUeInfo::GetUlSym (const UePtr &ue)
{
  return ue->m_ulSym;
}

uint8_t &
NrMacSchedulerUeInfo::GetDlMcs (const UePtr &ue, uint8_t stream)
{
  return ue->m_dlMcs.at (stream);
}

uint8_t &
NrMacSchedulerUeInfo::GetUlMcs (const UePtr &ue)
{
  return ue->m_ulMcs;
}

uint32_t &
NrMacSchedulerUeInfo::GetDlTBSPerStream (const UePtr &ue, uint8_t stream)
{
  return ue->m_dlTbSize.at (stream);
}

uint32_t
NrMacSchedulerUeInfo::GetDlTBS (const UePtr &ue)
{
  //if there are more than one streams we add the TbSizes
  //of all the streams. For example, when we need to satisfy
  //the bufQueueSize while allocating resources we should
  //add the TB sizes of all the streams.
  uint32_t tbSize = 0;
  for (const auto &it:ue->m_dlTbSize)
    {
      tbSize += it;
    }
  return tbSize;
}

uint32_t
NrMacSchedulerUeInfo::GetUlTBS (const UePtr &ue)
{
  return ue->m_ulTbSize;
}

std::unordered_map<uint8_t, LCGPtr> &
NrMacSchedulerUeInfo::GetDlLCG (const UePtr &ue)
{
  return ue->m_dlLCG;
}

std::unordered_map<uint8_t, LCGPtr> &
NrMacSchedulerUeInfo::GetUlLCG (const UePtr &ue)
{
  return ue->m_ulLCG;
}

NrMacHarqVector &
NrMacSchedulerUeInfo::GetDlHarqVector (const UePtr &ue)
{
  return ue->m_dlHarq;
}

NrMacHarqVector &
NrMacSchedulerUeInfo::GetUlHarqVector (const UePtr &ue)
{
  return ue->m_ulHarq;
}

void
NrMacSchedulerUeInfo::ResetDlSchedInfo ()
{
  m_dlMRBRetx = 0;
  m_dlRBG = 0;
  m_dlSym = 0;
  for (auto &it:m_dlTbSize)
    {
      it = 0;
    }
}

void
NrMacSchedulerUeInfo::ResetUlSchedInfo ()
{
  m_ulMRBRetx = 0;
  m_ulRBG = 0;
  m_ulSym = 0;
  m_ulTbSize = 0;
}


void
NrMacSchedulerUeInfo::UpdateDlMetric (const Ptr<const NrAmc> &amc)
{
  if (m_dlRBG == 0)
    {
      for (auto &it:m_dlTbSize)
        {
          it = 0;
        }
    }
  else
    {
      switch (m_dlCqi.m_ri)
      {
        case 1:
          if (m_dlMcs.size () == 1)
            {
              //the UE supports only one stream, i.e., max 1 stream
              NS_ABORT_MSG_IF (m_dlMcs.at (0) == 255, "DL MCS " << +m_dlMcs.at (0) << " is invalid");
              m_dlTbSize.at (0) = amc->CalculateTbSize (m_dlMcs.at (0), m_dlRBG * GetNumRbPerRbg ());
            }
          else
            {
              //this else is to cover the case when UE switched from 2 streams
              //to 1. In that case, scheduler needs to choose one stream with
              //highest CQI. Therefore, we need to correctly read the MCS from
              //the right index in the MCS vector and write a valid TB size at
              //the right index of the TB size vector. REMEMBER: Index of these
              //vectors directly maps to the stream index.
              std::vector<uint8_t>::iterator it;
              it = std::max_element (m_dlCqi.m_wbCqi.begin (), m_dlCqi.m_wbCqi.end ());
              uint8_t maxCqiIndex = std::distance (m_dlCqi.m_wbCqi.begin(), it);
              NS_LOG_DEBUG ("Max CQI " << static_cast <uint16_t> (*it) << " at index : " <<  maxCqiIndex);
              for (uint32_t stream = 0; stream < m_dlCqi.m_wbCqi.size (); stream++)
                {
                  if (stream == maxCqiIndex)
                    {
                      uint8_t mcs = m_dlMcs.at (stream);
                      NS_ASSERT_MSG (mcs != UINT8_MAX, "Invalid MCS " << +mcs
                                     << " for CQI " << +m_dlCqi.m_wbCqi.at (stream)
                                     << " for stream " << stream);
                      NS_LOG_DEBUG ("Switching from 2 streams to 1. Using " <<
                                    stream << " that has the maximum MCS : " <<
                                    +mcs);
                      m_dlTbSize.at (stream) = amc->CalculateTbSize (mcs, m_dlRBG * GetNumRbPerRbg ());
                    }
                  else
                    {
                      m_dlTbSize.at (stream) = 0;
                    }
                }
            }
          break;
        case 2:
          NS_ASSERT_MSG (m_dlMcs.at (0) != UINT8_MAX, "Invalid MCS " << +m_dlMcs.at (0)
                                               << " for CQI " << +m_dlCqi.m_wbCqi.at (0)
                                               << " for stream 0");
          NS_ASSERT_MSG (m_dlMcs.at (1) != UINT8_MAX, "Invalid MCS " << +m_dlMcs.at (1)
                                               << " for CQI " << +m_dlCqi.m_wbCqi.at (1)
                                               << " for stream 1");
          NS_ABORT_MSG_IF (m_dlMcs.size () < 2,"No MCS computed to be used for the second stream");

          m_dlTbSize.at (0) = amc->CalculateTbSize (m_dlMcs.at (0), m_dlRBG * GetNumRbPerRbg ());

          //we have the MCS to be used for the 2nd stream
          m_dlTbSize.at (1) = amc->CalculateTbSize (m_dlMcs.at (1), m_dlRBG * GetNumRbPerRbg ());
          break;
        default:
          NS_FATAL_ERROR ("Rank indicator value of " << +m_dlCqi.m_ri << " is not supported");
      }

    }
}

void
NrMacSchedulerUeInfo::ResetDlMetric ()
{
  for (auto &it:m_dlTbSize)
    {
      it = 0;
    }
}


void
NrMacSchedulerUeInfo::UpdateUlMetric (const Ptr<const NrAmc> &amc)
{
  if (m_ulRBG == 0)
    {
      m_ulTbSize = 0;
    }
  else
    {
      m_ulTbSize = amc->CalculateTbSize (m_ulMcs, m_ulRBG * GetNumRbPerRbg ());
    }
}

void
NrMacSchedulerUeInfo::ResetUlMetric ()
{
  m_ulTbSize = 0;
}

uint32_t
NrMacSchedulerUeInfo::GetNumRbPerRbg () const
{
  return m_getNumRbPerRbg ();
}


} // namespace ns3
