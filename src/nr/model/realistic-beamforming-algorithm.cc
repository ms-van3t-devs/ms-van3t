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

#include "realistic-beamforming-algorithm.h"
#include <ns3/double.h>
#include <ns3/angles.h>
#include <ns3/uinteger.h>
#include <ns3/mobility-model.h>
#include "nr-ue-phy.h"
#include "nr-gnb-phy.h"
#include "nr-gnb-net-device.h"
#include "nr-ue-net-device.h"
#include <ns3/nr-spectrum-value-helper.h>
#include "nr-mac-scheduler-ns3.h"
#include <ns3/random-variable-stream.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/node.h>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("RealisticBeamformingAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (RealisticBeamformingAlgorithm);

RealisticBeamformingAlgorithm::RealisticBeamformingAlgorithm ()
{
  m_normalRandomVariable = CreateObject<NormalRandomVariable> ();
  m_gnbDevice = nullptr;
  m_ueDevice = nullptr;
}

void
RealisticBeamformingAlgorithm::Install (const Ptr<NrGnbNetDevice>& gnbDevice,
                                        const Ptr<NrUeNetDevice>& ueDevice,
                                        const Ptr<NrSpectrumPhy> & gnbSpectrumPhy,
                                        const Ptr<NrSpectrumPhy>& ueSpectrumPhy,
                                        const Ptr<NrMacScheduler>& scheduler)
{
  NS_LOG_FUNCTION (this);
  m_gnbDevice = gnbDevice;
  m_ueDevice = ueDevice;
  m_gnbSpectrumPhy = gnbSpectrumPhy;
  m_ueSpectrumPhy = ueSpectrumPhy;
  m_scheduler = scheduler;
}

int64_t
RealisticBeamformingAlgorithm::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_normalRandomVariable->SetStream (stream);
  return 1;
}

RealisticBeamformingAlgorithm::~RealisticBeamformingAlgorithm()
{
}

TypeId
RealisticBeamformingAlgorithm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RealisticBeamformingAlgorithm")
                     .SetParent<Object> ()
                     .AddConstructor<RealisticBeamformingAlgorithm> ()
                     .AddAttribute ("BeamSearchAngleStep",
                                    "Angle step when searching for the best beam",
                                    DoubleValue (30),
                                    MakeDoubleAccessor (&RealisticBeamformingAlgorithm::SetBeamSearchAngleStep,
                                                        &RealisticBeamformingAlgorithm::GetBeamSearchAngleStep),
                                    MakeDoubleChecker<double> ())
                    .AddAttribute ("UseSnrSrs",
                                   "Denotes whether the SRS measurement will be SNR or SINR. If False"
                                   "SINR is used, if True the SNR",
                                   BooleanValue (true),
                                   MakeBooleanAccessor (&RealisticBeamformingAlgorithm::SetUseSnrSrs,
                                                        &RealisticBeamformingAlgorithm::UseSnrSrs),
                                   MakeBooleanChecker ());
  return tid;
}

uint8_t
RealisticBeamformingAlgorithm::GetSrsSymbolsPerSlot ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_UNLESS (m_scheduler, "Scheduler for the given ccId does not exist!");
  return (DynamicCast<NrMacSchedulerNs3>(m_scheduler))->GetSrsCtrlSyms();
}

RealisticBeamformingAlgorithm::TriggerEventConf
RealisticBeamformingAlgorithm::GetTriggerEventConf () const
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_UNLESS (m_gnbSpectrumPhy, "gNB spectrum PHY for the given ccId does not exist!");
  Ptr<RealisticBfManager> realBf = DynamicCast<RealisticBfManager> (m_gnbSpectrumPhy->GetBeamManager());
  NS_ABORT_MSG_UNLESS (realBf, "Realistic BF manager at gNB does not exist. Realistic BF manager at gNB should "
                      "be installed when using realistic BF algorithm.");

  TriggerEventConf conf;
  conf.event = realBf->GetTriggerEvent ();
  conf.updatePeriodicity = realBf->GetUpdatePeriodicity ();
  conf.updateDelay = realBf->GetUpdateDelay ();
  return conf;
}


void
RealisticBeamformingAlgorithm::SetBeamSearchAngleStep (double beamSearchAngleStep)
{
  m_beamSearchAngleStep = beamSearchAngleStep;
}

double
RealisticBeamformingAlgorithm::GetBeamSearchAngleStep () const
{
  return m_beamSearchAngleStep;
}

void
RealisticBeamformingAlgorithm::SetUseSnrSrs (bool v)
{
  m_useSnrSrs = v;
}

bool
RealisticBeamformingAlgorithm::UseSnrSrs () const
{
  return m_useSnrSrs;
}

void
RealisticBeamformingAlgorithm::NotifySrsSinrReport (uint16_t cellId, uint16_t rnti, double srsSinr)
{
  NS_LOG_FUNCTION (this);
  if (!m_useSnrSrs)
    {
      NotifySrsReport (cellId, rnti, srsSinr);
    }
}

void
RealisticBeamformingAlgorithm::NotifySrsSnrReport (uint16_t cellId, uint16_t rnti, double srsSnr)
{
  NS_LOG_FUNCTION (this);
  if (m_useSnrSrs)
    {
      NotifySrsReport (cellId, rnti, srsSnr);
    }
}

void
RealisticBeamformingAlgorithm::NotifySrsReport (uint16_t cellId, uint16_t rnti, double srsReport)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_UNLESS (m_ueDevice && m_gnbDevice, "Function install must be called to install gNB and UE pair, and set ccId.");

  //before anything check if RNTI corresponds to RNTI of UE of this algorithm instance
  if (m_ueDevice->GetRrc ()->GetRnti () != rnti)
    {
      NS_LOG_INFO ("Ignoring SRS report. Not for me. Report for RNTI:"<< rnti << ", and my RNTI is:"<< m_ueDevice->GetRrc ()->GetRnti ());
      return;
    }

  // update SRS symbols counter
  m_srsSymbolsCounter++;

  // update max SRS SINR/SNR, i.e., reset when m_srsSymbolsCounter == 1, otherwise do max
  m_maxSrsSinrPerSlot = (m_srsSymbolsCounter > 1) ? std::max(srsReport, m_maxSrsSinrPerSlot):srsReport;

  // if we reached the last SRS symbol, check whether some event should be triggered
  if (m_srsSymbolsCounter == GetSrsSymbolsPerSlot ())
    {
      // reset symbols per slot counter
      m_srsSymbolsCounter = 0;

      TriggerEventConf conf = GetTriggerEventConf ();

      if ( conf.event == RealisticBfManager::SRS_COUNT)
        {
          // it is time to update SRS periodicity counter
          m_srsPeriodicityCounter++;
          // reset or increase SRS periodicity counter
          if (m_srsPeriodicityCounter == conf.updatePeriodicity )
            {
              NS_LOG_INFO ("Update periodicity for updating BF reached. Time to trigger realistic BF helper callback.");
              // it is time to trigger helpers callback
              Simulator::ScheduleNow (&RealisticBeamformingAlgorithm::NotifyHelper, this);
              // and reset the counter
              m_srsPeriodicityCounter = 0;
            }
        }
      else if ( conf.event == RealisticBfManager::DELAYED_UPDATE)
        {
          NS_LOG_INFO ("Received all SRS symbols per current slot. Scheduler realistic BF helper callback");
          DelayedUpdateInfo dui;
          dui.updateTime = Simulator::Now () + conf.updateDelay;
          dui.srsSinr = m_maxSrsSinrPerSlot;  // SNR or SINR
          dui.channelMatrix = GetChannelMatrix ();
          m_delayedUpdateInfo.push (dui);
          // schedule delayed update
          Simulator::Schedule (conf.updateDelay, &RealisticBeamformingAlgorithm::NotifyHelper , this);
          Simulator::Schedule (conf.updateDelay + NanoSeconds (1), &RealisticBeamformingAlgorithm::RemoveUsedDelayedUpdateInfo , this);
          // we add 1 second just to be sure that will be removed after NotifyHelper, even if it is added into event sequence after, should be called after
        }
      else
        {
          NS_ABORT_MSG ("Unknown trigger event type.");
        }
    }
}

void
RealisticBeamformingAlgorithm::RemoveUsedDelayedUpdateInfo ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_delayedUpdateInfo.size(), " No elements in m_delayedUpdateInfo queue.");
  m_delayedUpdateInfo.pop (); // we can now delete this first element
}

void
RealisticBeamformingAlgorithm::NotifyHelper ()
{
  NS_LOG_FUNCTION (this);
  m_helperCallback (m_gnbDevice, m_ueDevice, m_gnbSpectrumPhy, m_ueSpectrumPhy);
}

void
RealisticBeamformingAlgorithm::SetTriggerCallback (RealisticBfHelperCallback callback)
{
  m_helperCallback = callback;
}

Ptr<const MatrixBasedChannelModel::ChannelMatrix>
RealisticBeamformingAlgorithm::GetChannelMatrix () const
{
  NS_LOG_FUNCTION (this);
  Ptr<SpectrumChannel> gnbSpectrumChannel = m_gnbSpectrumPhy->GetSpectrumChannel (); // SpectrumChannel should be const.. but need to change ns-3-dev

  Ptr<SpectrumChannel> ueSpectrumChannel = m_ueSpectrumPhy->GetSpectrumChannel ();

  Ptr<PhasedArraySpectrumPropagationLossModel> gnbThreeGppSpectrumPropModel = gnbSpectrumChannel->GetPhasedArraySpectrumPropagationLossModel ();
  Ptr<ThreeGppSpectrumPropagationLossModel> threeGppSplm = DynamicCast<ThreeGppSpectrumPropagationLossModel>(gnbThreeGppSpectrumPropModel);
  Ptr<MatrixBasedChannelModel> matrixBasedChannelModel = threeGppSplm -> GetChannelModel();
  Ptr<ThreeGppChannelModel> channelModel = DynamicCast<ThreeGppChannelModel>(matrixBasedChannelModel);

  NS_ASSERT (channelModel!=nullptr);

  Ptr<const MatrixBasedChannelModel::ChannelMatrix> originalChannelMatrix = channelModel-> GetChannel (m_gnbDevice->GetNode()->GetObject<MobilityModel>(),
                                                                                                       m_ueDevice->GetNode()->GetObject<MobilityModel>(),
                                                                                                       m_gnbSpectrumPhy->GetAntenna ()->GetObject <PhasedArrayModel>(),
                                                                                                       m_ueSpectrumPhy->GetAntenna ()->GetObject <PhasedArrayModel>());

  Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrixCopy = Copy <const MatrixBasedChannelModel::ChannelMatrix> (originalChannelMatrix);
  return channelMatrixCopy;
}

BeamformingVectorPair
RealisticBeamformingAlgorithm::GetBeamformingVectors ()
{
  NS_ABORT_MSG_IF (m_gnbSpectrumPhy == nullptr || m_ueSpectrumPhy == nullptr, "Something went wrong, gnb or UE PHY layer not set.");
  double distance = m_gnbSpectrumPhy->GetMobility ()->GetDistanceFrom (m_ueSpectrumPhy->GetMobility());
  NS_ABORT_MSG_IF (distance == 0, "Beamforming method cannot be performed between two devices that are placed in the same position.");

  double max = 0, maxTxTheta = 0, maxRxTheta = 0;
  uint16_t maxTxSector = 0, maxRxSector = 0;
  complexVector_t maxTxW, maxRxW;

  UintegerValue uintValue;
  m_gnbSpectrumPhy->GetAntenna ()->GetAttribute ("NumRows", uintValue);
  uint32_t gnbNumRows = static_cast<uint32_t> (uintValue.Get ());
  m_ueSpectrumPhy->GetAntenna ()->GetAttribute ("NumRows", uintValue);
  uint32_t ueNumRows = static_cast<uint32_t> (uintValue.Get ());

  TriggerEventConf conf = GetTriggerEventConf ();
  double srsSinr = 0;
  Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix = nullptr;

  if (conf.event == RealisticBfManager::DELAYED_UPDATE)
    {
      NS_ASSERT (m_delayedUpdateInfo.size ());
      DelayedUpdateInfo dui = m_delayedUpdateInfo.front ();
      NS_ABORT_MSG_UNLESS (dui.updateTime == Simulator::Now (), "Current time should be equal to the updateTime from the DelayedUpdateInfo structure."); // sanity check that we are using correct dui
      srsSinr = dui.srsSinr;
      channelMatrix = dui.channelMatrix;
    }
  else
    {
      srsSinr = m_maxSrsSinrPerSlot;
      channelMatrix = GetChannelMatrix ();
    }

  for (double gnbTheta = 60; gnbTheta < 121; gnbTheta = gnbTheta + m_beamSearchAngleStep)
    {
      for (uint16_t gnbSector = 0; gnbSector <= gnbNumRows; gnbSector++)
        {
          NS_ASSERT(gnbSector < UINT16_MAX);
          m_gnbSpectrumPhy->GetBeamManager()->SetSector (gnbSector, gnbTheta);
          complexVector_t gnbW = m_gnbSpectrumPhy->GetBeamManager ()->GetCurrentBeamformingVector ();

          for (double ueTheta = 60; ueTheta < 121; ueTheta = static_cast<uint16_t> (ueTheta + m_beamSearchAngleStep))
            {
              for (uint16_t ueSector = 0; ueSector <= ueNumRows; ueSector++)
                {
                  NS_ASSERT(ueSector < UINT16_MAX);
                  m_ueSpectrumPhy->GetBeamManager ()->SetSector (ueSector, ueTheta);
                  complexVector_t ueW = m_ueSpectrumPhy->GetBeamManager ()->GetCurrentBeamformingVector ();

                  NS_ABORT_MSG_IF (gnbW.size()==0 || ueW.size()==0,
                                   "Beamforming vectors must be initialized in order to calculate the long term matrix.");

                  const UniformPlanarArray::ComplexVector estimatedLongTermComponent = GetEstimatedLongTermComponent (channelMatrix, gnbW, ueW,
                                                                                                                      m_gnbSpectrumPhy->GetObject<MobilityModel>(),
                                                                                                                      m_ueSpectrumPhy->GetObject<MobilityModel>(),
                                                                                                                      srsSinr,
                                                                                                                      m_gnbSpectrumPhy->GetAntenna ()->GetObject<PhasedArrayModel>(),
                                                                                                                      m_ueSpectrumPhy->GetAntenna ()->GetObject<PhasedArrayModel>()
                                                                                                                      );

                  double estimatedLongTermMetric = CalculateTheEstimatedLongTermMetric (estimatedLongTermComponent);

                  NS_LOG_LOGIC (" Estimated long term metric value: "<< estimatedLongTermMetric <<
                                " gnb theta " << gnbTheta <<
                                " ue theta " << ueTheta <<
                                " gnb sector " << (M_PI *  static_cast<double> (gnbSector) / static_cast<double> (gnbNumRows) - 0.5 * M_PI) / (M_PI) * 180 <<
                                " ue sector " << (M_PI * static_cast<double> (ueSector) / static_cast<double> (ueNumRows) - 0.5 * M_PI) / (M_PI) * 180);

                  if (max < estimatedLongTermMetric)
                    {
                      max = estimatedLongTermMetric;
                      maxTxSector = gnbSector;
                      maxRxSector = ueSector;
                      maxTxTheta = gnbTheta;
                      maxRxTheta = ueTheta;
                      maxTxW = gnbW;
                      maxRxW = ueW;
                    }
                }
            }
        }
    }

  BeamformingVectorPair bfPair = std::make_pair (BeamformingVector (std::make_pair(maxTxW, BeamId (maxTxSector, maxTxTheta))),
                                                 BeamformingVector (std::make_pair (maxRxW, BeamId (maxRxSector, maxRxTheta))));
  NS_LOG_DEBUG ("Beamforming vectors for gNB with node id: "<< m_gnbSpectrumPhy->GetMobility()->GetObject<Node>()->GetId () <<
                " and UE with node id: " << m_ueSpectrumPhy->GetMobility()->GetObject<Node>()->GetId () <<
                " txTheta " << maxTxTheta <<
                " rxTheta " << maxRxTheta <<
                " tx sector " << (M_PI * static_cast<double> (maxTxSector) / static_cast<double> (gnbNumRows) - 0.5 * M_PI) / (M_PI) * 180 <<
                " rx sector " << (M_PI * static_cast<double> (maxRxSector) / static_cast<double> (ueNumRows) - 0.5 * M_PI) / (M_PI) * 180);

 return bfPair;
}

double
RealisticBeamformingAlgorithm::CalculateTheEstimatedLongTermMetric (const UniformPlanarArray::ComplexVector& longTermComponent) const
{
  NS_LOG_FUNCTION (this);

  double totalSum = 0;
  for (std::complex<double> c:longTermComponent)
    {
      totalSum += c.imag () * c.imag () + c.real () * c.real ();
    }
  return totalSum;
}


UniformPlanarArray::ComplexVector
RealisticBeamformingAlgorithm::GetEstimatedLongTermComponent (const Ptr<const MatrixBasedChannelModel::ChannelMatrix>& channelMatrix,
                                                              const UniformPlanarArray::ComplexVector &aW,
                                                              const UniformPlanarArray::ComplexVector &bW,
                                                              Ptr<const MobilityModel> a,
                                                              Ptr<const MobilityModel> b,
                                                              double srsSinr,
                                                              Ptr<const PhasedArrayModel> aArray,
                                                              Ptr<const PhasedArrayModel> bArray
) const
{
  NS_LOG_FUNCTION (this);

  // check if the channel matrix was generated considering a as the s-node and
  // b as the u-node or viceversa
  UniformPlanarArray::ComplexVector sW, uW;
  if (!channelMatrix->IsReverse (aArray->GetId (), bArray->GetId ()))
    {
      sW = aW;
      uW = bW;
    }
  else
    {
      sW = bW;
      uW = aW;
    }

  uint16_t sAntenna = static_cast<uint16_t> (sW.size ());
  uint16_t uAntenna = static_cast<uint16_t> (uW.size ());

  NS_LOG_DEBUG ("Calculate the estimation of the long term component with sAntenna: " << sAntenna << " uAntenna: " << uAntenna);
  UniformPlanarArray::ComplexVector estimatedlongTerm;

  NS_ABORT_IF (srsSinr == 0);

  double varError = 1 / (srsSinr); // SINR the SINR from UL SRS reception
  uint8_t numCluster = static_cast<uint8_t> (channelMatrix->m_channel[0][0].size ());

  for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
    {
      std::complex<double> txSum (0,0);
      for (uint16_t sIndex = 0; sIndex < sAntenna; sIndex++)
        {
          std::complex<double> rxSum (0,0);
          for (uint16_t uIndex = 0; uIndex < uAntenna; uIndex++)
            {
              //error is generated from the normal random variable with mean 0 and  variance varError*sqrt(1/2) for real/imaginary parts
              std::complex<double> error = std::complex <double> (m_normalRandomVariable->GetValue (0, sqrt (0.5) * varError),
                                                                  m_normalRandomVariable->GetValue (0, sqrt (0.5) * varError)) ;
              std::complex<double> hEstimate = channelMatrix->m_channel [uIndex][sIndex][cIndex] + error;
              rxSum += uW[uIndex] * (hEstimate);
            }
          txSum = txSum + sW[sIndex] * rxSum;
        }
      estimatedlongTerm.push_back (txSum);
    }
  return estimatedlongTerm;
}

} // end of namespace ns-3
