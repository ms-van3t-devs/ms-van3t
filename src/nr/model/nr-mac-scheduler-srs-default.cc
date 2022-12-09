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
#include "nr-mac-scheduler-srs-default.h"

#include <ns3/uinteger.h>

#include <algorithm>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerSrsDefault");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerSrsDefault);

std::vector<uint32_t>
NrMacSchedulerSrsDefault::StandardPeriodicity = {
  2, 4, 5, 8, 10, 16, 20, 32, 40, 64, 80, 160, 320, 640, 1280, 2560
};

NrMacSchedulerSrsDefault::NrMacSchedulerSrsDefault ()
{
  m_random = CreateObject<UniformRandomVariable> ();
}

int64_t
NrMacSchedulerSrsDefault::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 1;
}

NrMacSchedulerSrsDefault::~NrMacSchedulerSrsDefault ()
{

}

TypeId
NrMacSchedulerSrsDefault::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerSrsDefault")
    .SetParent<Object> ()
    .AddConstructor<NrMacSchedulerSrsDefault> ()
    .SetGroupName ("nr")
    .AddAttribute ("StartingPeriodicity",
                   "Starting value for the periodicity",
                   UintegerValue (80),
                   MakeUintegerAccessor (&NrMacSchedulerSrsDefault::SetStartingPeriodicity,
                                         &NrMacSchedulerSrsDefault::GetStartingPeriodicity),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

NrMacSchedulerSrs::SrsPeriodicityAndOffset
NrMacSchedulerSrsDefault::AddUe ()
{
  NS_LOG_FUNCTION (this);
  SrsPeriodicityAndOffset ret;

  if (m_availableOffsetValues.size () == 0)
    {
      return ret; // ret will be invalid
    }

  ret.m_offset = m_availableOffsetValues.back ();
  ret.m_periodicity = m_periodicity;
  ret.m_isValid = true;

  m_availableOffsetValues.pop_back();
  return ret;
}

void
NrMacSchedulerSrsDefault::RemoveUe (uint32_t offset)
{
  NS_LOG_FUNCTION (this);
  m_availableOffsetValues.push_back (offset); // Offset will be reused as soon as possible
}

bool
NrMacSchedulerSrsDefault::IncreasePeriodicity (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap)
{
  NS_LOG_FUNCTION (this);

  m_availableOffsetValues.clear ();
  auto it = std::upper_bound (StandardPeriodicity.begin (), StandardPeriodicity.end (),
                              m_periodicity);
  if (it == StandardPeriodicity.end ())
    {
      return false;
    }

  SetStartingPeriodicity (*it);
  ReassignSrsValue (ueMap);

  return true;
}

bool
NrMacSchedulerSrsDefault::DecreasePeriodicity (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap)
{
  NS_LOG_FUNCTION (this);

  m_availableOffsetValues.clear ();
  auto it = std::lower_bound (StandardPeriodicity.begin (), StandardPeriodicity.end (),
                              m_periodicity);
  if (it == StandardPeriodicity.end ())
    {
      return false;
    }

  SetStartingPeriodicity (*it);
  ReassignSrsValue (ueMap);
  return true;
}

void
NrMacSchedulerSrsDefault::SetStartingPeriodicity (uint32_t start)
{
  NS_ABORT_MSG_IF (m_availableOffsetValues.size () != 0,
                   "We already started giving offset to UEs, you cannot alter the periodicity");

  if (std::find (StandardPeriodicity.begin(), StandardPeriodicity.end (), start) == StandardPeriodicity.end ())
    {
      NS_FATAL_ERROR ("You cannot use " << start <<
                      " as periodicity; please use a standard value like "
                      "2, 4, 5, 8, 10, 16, 20, 32, 40, 64, 80, 160, 320, 640, 1280, 2560"
                      " (or write your own algorithm)");
    }

  m_periodicity = start;
  m_availableOffsetValues.resize (m_periodicity);

  // Fill the available values
  for (uint32_t i = 0; i < m_periodicity; ++i)
    {
      m_availableOffsetValues[i] = i;
    }

  // Shuffle the available values, so it contains the element in a random order
  for (auto i = m_availableOffsetValues.size () - 1; i > 0; --i)
    {
      auto j = m_random->GetInteger (0, i);
      std::swap (m_availableOffsetValues[i], m_availableOffsetValues[j]);
    }
}

uint32_t
NrMacSchedulerSrsDefault::GetStartingPeriodicity () const
{
  return m_periodicity;
}

void
NrMacSchedulerSrsDefault::ReassignSrsValue (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap)
{
  NS_LOG_FUNCTION (this);

  for (auto & ue : *ueMap)
    {
      auto srs = AddUe ();

      NS_ASSERT (srs.m_isValid);

      ue.second->m_srsPeriodicity = srs.m_periodicity;
      ue.second->m_srsOffset = srs.m_offset;
    }
}

} // namespace ns3
