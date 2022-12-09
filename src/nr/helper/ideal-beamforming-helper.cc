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

#include "ideal-beamforming-helper.h"
#include <ns3/object-factory.h>
#include <ns3/ideal-beamforming-algorithm.h>
#include <ns3/log.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-gnb-phy.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/beam-manager.h>
#include <ns3/vector.h>
#include <ns3/nr-spectrum-phy.h>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("IdealBeamformingHelper");
NS_OBJECT_ENSURE_REGISTERED (IdealBeamformingHelper);


IdealBeamformingHelper::IdealBeamformingHelper ()
{
  NS_LOG_FUNCTION (this);
}

IdealBeamformingHelper::~IdealBeamformingHelper ()
{
  NS_LOG_FUNCTION (this);
  m_beamformingAlgorithm = nullptr;
}

void
IdealBeamformingHelper::DoInitialize ()
{
  m_beamformingTimer = Simulator::Schedule (m_beamformingPeriodicity,
                                              &IdealBeamformingHelper::ExpireBeamformingTimer, this);
  BeamformingHelperBase::DoInitialize ();
}

TypeId
IdealBeamformingHelper::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::IdealBeamformingHelper")
      .SetParent<BeamformingHelperBase> ()
      .AddConstructor<IdealBeamformingHelper> ()
      .AddAttribute ("BeamformingMethod",
                     "Type of the ideal beamforming method in the case that it is enabled, by default is \"cell scan\" method.",
                      TypeIdValue (CellScanBeamforming::GetTypeId ()),
                      MakeTypeIdAccessor (&IdealBeamformingHelper::SetBeamformingMethod),
                      MakeTypeIdChecker ())
      .AddAttribute ("BeamformingPeriodicity",
                     "Interval between consecutive beamforming method executions. If set to 0 it will not be updated.",
                      TimeValue (MilliSeconds (100)),
                      MakeTimeAccessor (&IdealBeamformingHelper::SetPeriodicity,
                                        &IdealBeamformingHelper::GetPeriodicity),
                      MakeTimeChecker())
      ;
    return tid;
}

void
IdealBeamformingHelper::AddBeamformingTask (const Ptr<NrGnbNetDevice>& gnbDev,
                                            const Ptr<NrUeNetDevice>& ueDev)
{
  NS_LOG_FUNCTION (this);

  if (!m_beamformingAlgorithm)
    {
      m_beamformingAlgorithm = m_algorithmFactory.Create<IdealBeamformingAlgorithm> ();
    }

  for (uint8_t ccId = 0; ccId < gnbDev->GetCcMapSize () ; ccId++)
     {
       uint8_t gnbAntennaArrays = gnbDev->GetPhy (ccId)->GetNumberOfStreams ();
       uint8_t ueAntennaArrays = ueDev->GetPhy (ccId)->GetNumberOfStreams ();
       uint8_t arrays = std::min (gnbAntennaArrays, ueAntennaArrays);
       NS_ASSERT (arrays);
       //TODO add assert to check if they are of the same polarization
       for (uint8_t arrayIndex = 0; arrayIndex < arrays; arrayIndex++)
         {
           Ptr<NrSpectrumPhy> gnbSpectrumPhy = gnbDev->GetPhy (ccId)->GetSpectrumPhy (arrayIndex);
           Ptr<NrSpectrumPhy> ueSpectrumPhy = ueDev->GetPhy (ccId)->GetSpectrumPhy (arrayIndex);

           m_spectrumPhyPairToDevicePair [std::make_pair (gnbSpectrumPhy, ueSpectrumPhy)] = std::make_pair (gnbDev, ueDev);

           RunTask (gnbDev, ueDev, gnbSpectrumPhy, ueSpectrumPhy);
         }

     }
}

void
IdealBeamformingHelper::Run () const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Running the beamforming method. There are :" <<
               m_spectrumPhyPairToDevicePair.size()<<" tasks.");

  for (const auto& task : m_spectrumPhyPairToDevicePair)
    {
      RunTask (task.second.first, task.second.second, task.first.first, task.first.second);
    }
}

BeamformingVectorPair
IdealBeamformingHelper::GetBeamformingVectors (const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
                                               const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const
{
  NS_LOG_FUNCTION (this);
  return m_beamformingAlgorithm->GetBeamformingVectors (gnbSpectrumPhy, ueSpectrumPhy);
}

void
IdealBeamformingHelper::SetBeamformingMethod (const TypeId &beamformingMethod)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (beamformingMethod.IsChildOf (IdealBeamformingAlgorithm::GetTypeId ()));
  m_algorithmFactory.SetTypeId (beamformingMethod);
}

void
IdealBeamformingHelper::ExpireBeamformingTimer ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Beamforming timer expired; programming a beamforming");

  Run (); //Run beamforming tasks
  m_beamformingTimer.Cancel (); // Cancel any previous beamforming event
  m_beamformingTimer = Simulator::Schedule (m_beamformingPeriodicity,
                                            &IdealBeamformingHelper::ExpireBeamformingTimer, this);
}

void
IdealBeamformingHelper::SetPeriodicity (const Time &v)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF ( v == MilliSeconds (0), "Periodicity must be greater than 0 ms.");
  m_beamformingPeriodicity = v;
}

Time
IdealBeamformingHelper::GetPeriodicity () const
{
  NS_LOG_FUNCTION (this);
  return m_beamformingPeriodicity;
}


}
