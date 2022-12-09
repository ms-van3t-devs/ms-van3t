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

#include "nr-helper.h"
#include <ns3/lte-rrc-protocol-ideal.h>
#include <ns3/lte-rrc-protocol-real.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/lte-chunk-processor.h>
#include <ns3/epc-ue-nas.h>
#include <ns3/names.h>
#include <ns3/nr-rrc-protocol-ideal.h>
#include <ns3/nr-gnb-mac.h>
#include <ns3/nr-gnb-phy.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/nr-ue-mac.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-ch-access-manager.h>
#include <ns3/bandwidth-part-gnb.h>
#include <ns3/bwp-manager-gnb.h>
#include <ns3/bwp-manager-ue.h>
#include <ns3/nr-rrc-protocol-ideal.h>
#include <ns3/epc-helper.h>
#include <ns3/epc-enb-application.h>
#include <ns3/epc-x2.h>
#include <ns3/nr-phy-rx-trace.h>
#include <ns3/nr-mac-rx-trace.h>
#include "nr-bearer-stats-calculator.h"
#include <ns3/bandwidth-part-ue.h>
#include <ns3/beam-manager.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/three-gpp-channel-model.h>
#include <ns3/buildings-channel-condition-model.h>
#include <ns3/nr-mac-scheduler-tdma-rr.h>
#include <ns3/bwp-manager-algorithm.h>
#include <ns3/three-gpp-v2v-propagation-loss-model.h>
#include <ns3/three-gpp-v2v-channel-condition-model.h>
#include <ns3/uniform-planar-array.h>

#include <algorithm>

namespace ns3 {

/* ... */
NS_LOG_COMPONENT_DEFINE ("NrHelper");

NS_OBJECT_ENSURE_REGISTERED (NrHelper);

NrHelper::NrHelper (void)
{
  NS_LOG_FUNCTION (this);
  m_channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());
  m_gnbNetDeviceFactory.SetTypeId (NrGnbNetDevice::GetTypeId ());
  m_ueNetDeviceFactory.SetTypeId (NrUeNetDevice::GetTypeId ());
  m_ueMacFactory.SetTypeId (NrUeMac::GetTypeId ());
  m_gnbMacFactory.SetTypeId (NrGnbMac::GetTypeId ());
  m_ueSpectrumFactory.SetTypeId (NrSpectrumPhy::GetTypeId ());
  m_gnbSpectrumFactory.SetTypeId (NrSpectrumPhy::GetTypeId ());
  m_uePhyFactory.SetTypeId (NrUePhy::GetTypeId ());
  m_gnbPhyFactory.SetTypeId (NrGnbPhy::GetTypeId ());
  m_ueChannelAccessManagerFactory.SetTypeId (NrAlwaysOnAccessManager::GetTypeId ());
  m_gnbChannelAccessManagerFactory.SetTypeId (NrAlwaysOnAccessManager::GetTypeId ());
  m_schedFactory.SetTypeId (NrMacSchedulerTdmaRR::GetTypeId ());
  m_ueAntennaFactory.SetTypeId (UniformPlanarArray::GetTypeId ());
  m_gnbAntennaFactory.SetTypeId (UniformPlanarArray::GetTypeId ());
  m_gnbBwpManagerAlgoFactory.SetTypeId (BwpManagerAlgorithmStatic::GetTypeId ());
  m_ueBwpManagerAlgoFactory.SetTypeId (BwpManagerAlgorithmStatic::GetTypeId ());
  m_gnbUlAmcFactory.SetTypeId (NrAmc::GetTypeId ());
  m_gnbDlAmcFactory.SetTypeId (NrAmc::GetTypeId ());
  m_gnbBeamManagerFactory.SetTypeId (BeamManager::GetTypeId());
  m_ueBeamManagerFactory.SetTypeId (BeamManager::GetTypeId());
  m_spectrumPropagationFactory.SetTypeId (ThreeGppSpectrumPropagationLossModel::GetTypeId ());
  m_bwpManagerFactory.SetTypeId (BwpManagerUe::GetTypeId ());

  // Initialization that is there just because the user can configure attribute
  // through the helper methods without making it sad that no TypeId is set.
  // When the TypeId is changed, the user-set attribute will be maintained.
  m_pathlossModelFactory.SetTypeId (ThreeGppPropagationLossModel::GetTypeId ());
  m_channelConditionModelFactory.SetTypeId (ThreeGppChannelConditionModel::GetTypeId ());

  Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));

  m_phyStats = CreateObject<NrPhyRxTrace> ();
  m_macSchedStats = CreateObject <NrMacSchedulingStats> ();
}

NrHelper::~NrHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrHelper")
    .SetParent<Object> ()
    .AddConstructor<NrHelper> ()
    .AddAttribute ("HarqEnabled",
                   "Enable Hybrid ARQ",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrHelper::m_harqEnabled),
                   MakeBooleanChecker ())
    ;
  return tid;
}

typedef std::function<void (ObjectFactory *, ObjectFactory *)> InitPathLossFn;

static void
InitRma (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppRmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppRmaChannelConditionModel::GetTypeId ());
}

static void
InitRma_LoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppRmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (AlwaysLosChannelConditionModel::GetTypeId ());
}

static void
InitRma_nLoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppRmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (NeverLosChannelConditionModel::GetTypeId ());
}

static void
InitUma (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppUmaChannelConditionModel::GetTypeId ());
}

static void
InitUma_LoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (AlwaysLosChannelConditionModel::GetTypeId ());
}

static void
InitUma_nLoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (NeverLosChannelConditionModel::GetTypeId ());
}

static void
InitUmi (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppUmiStreetCanyonChannelConditionModel::GetTypeId ());
}

static void
InitUmi_LoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (AlwaysLosChannelConditionModel::GetTypeId ());
}

static void
InitUmi_nLoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (NeverLosChannelConditionModel::GetTypeId ());
}

static void
InitIndoorOpen (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppIndoorOpenOfficeChannelConditionModel::GetTypeId ());
}

static void
InitIndoorOpen_LoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (AlwaysLosChannelConditionModel::GetTypeId ());
}

static void
InitIndoorOpen_nLoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (NeverLosChannelConditionModel::GetTypeId ());
}

static void
InitIndoorMixed (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppIndoorMixedOfficeChannelConditionModel::GetTypeId ());
}

static void
InitIndoorMixed_LoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (AlwaysLosChannelConditionModel::GetTypeId ());
}

static void
InitIndoorMixed_nLoS (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (NeverLosChannelConditionModel::GetTypeId ());
}


static void
InitUmaBuildings (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (BuildingsChannelConditionModel::GetTypeId ());
}

static void
InitUmiBuildings (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (BuildingsChannelConditionModel::GetTypeId ());
}

static void
InitV2VHighway (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppV2vHighwayPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppV2vHighwayChannelConditionModel::GetTypeId ());
}

static void
InitV2VUrban (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppV2vUrbanPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppV2vUrbanChannelConditionModel::GetTypeId ());
}

void
NrHelper::InitializeOperationBand (OperationBandInfo *band, uint8_t flags)
{
  NS_LOG_FUNCTION (this);

  static std::unordered_map<BandwidthPartInfo::Scenario, InitPathLossFn, std::hash<int>> initLookupTable
  {
    {BandwidthPartInfo::RMa, std::bind (&InitRma, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::RMa_LoS, std::bind (&InitRma_LoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::RMa_nLoS, std::bind (&InitRma_nLoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMa, std::bind (&InitUma, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMa_LoS, std::bind (&InitUma_LoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMa_nLoS, std::bind (&InitUma_nLoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMi_StreetCanyon, std::bind (&InitUmi, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMi_StreetCanyon_LoS, std::bind (&InitUmi_LoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMi_StreetCanyon_nLoS, std::bind (&InitUmi_nLoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::InH_OfficeOpen, std::bind (&InitIndoorOpen, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::InH_OfficeOpen_LoS, std::bind (&InitIndoorOpen_LoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::InH_OfficeOpen_nLoS, std::bind (&InitIndoorOpen_nLoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::InH_OfficeMixed, std::bind (&InitIndoorMixed, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::InH_OfficeMixed_LoS, std::bind (&InitIndoorMixed_LoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::InH_OfficeMixed_nLoS, std::bind (&InitIndoorMixed_nLoS, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMa_Buildings, std::bind (&InitUmaBuildings, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMi_Buildings, std::bind (&InitUmiBuildings, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::V2V_Highway, std::bind (&InitV2VHighway, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::V2V_Urban, std::bind (&InitV2VUrban, std::placeholders::_1, std::placeholders::_2)},
  };

  // Iterate over all CCs, and instantiate the channel and propagation model
  for (const auto & cc : band->m_cc)
    {
      for (const auto & bwp : cc->m_bwp)
        {
          // Initialize the type ID of the factories by calling the relevant
          // static function defined above and stored inside the lookup table
          initLookupTable.at (bwp->m_scenario) (&m_pathlossModelFactory, &m_channelConditionModelFactory);

          auto channelConditionModel  = m_channelConditionModelFactory.Create<ChannelConditionModel>();

          if (bwp->m_propagation == nullptr && flags & INIT_PROPAGATION)
            {

              bwp->m_propagation = m_pathlossModelFactory.Create <ThreeGppPropagationLossModel> ();
              bwp->m_propagation->SetAttributeFailSafe ("Frequency", DoubleValue (bwp->m_centralFrequency));
              DynamicCast<ThreeGppPropagationLossModel> (bwp->m_propagation)->SetChannelConditionModel (channelConditionModel);
            }

          if (bwp->m_3gppChannel == nullptr && flags & INIT_FADING)
            {
              bwp->m_3gppChannel = m_spectrumPropagationFactory.Create<ThreeGppSpectrumPropagationLossModel>();
              DynamicCast<ThreeGppSpectrumPropagationLossModel> (bwp->m_3gppChannel)->SetChannelModelAttribute ("Frequency", DoubleValue (bwp->m_centralFrequency));
              DynamicCast<ThreeGppSpectrumPropagationLossModel> (bwp->m_3gppChannel)->SetChannelModelAttribute ("Scenario", StringValue (bwp->GetScenario ()));
              DynamicCast<ThreeGppSpectrumPropagationLossModel> (bwp->m_3gppChannel)->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (channelConditionModel));
            }

          if (bwp->m_channel == nullptr && flags & INIT_CHANNEL)
            {
              bwp->m_channel = m_channelFactory.Create<SpectrumChannel> ();
              bwp->m_channel->AddPropagationLossModel (bwp->m_propagation);
              bwp->m_channel->AddPhasedArraySpectrumPropagationLossModel (bwp->m_3gppChannel);
            }
        }
    }
}

uint32_t
NrHelper::GetNumberBwp (const Ptr<const NetDevice> &gnbDevice)
{
  NS_LOG_FUNCTION (gnbDevice);
  Ptr<const NrGnbNetDevice> netDevice = DynamicCast<const NrGnbNetDevice> (gnbDevice);
  if (netDevice == nullptr)
    {
      return 0;
    }
  return netDevice->GetCcMapSize ();
}

Ptr<NrGnbPhy>
NrHelper::GetGnbPhy (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex)
{
  NS_LOG_FUNCTION (gnbDevice << bwpIndex);
  NS_ASSERT(bwpIndex < UINT8_MAX);
  Ptr<NrGnbNetDevice> netDevice = DynamicCast<NrGnbNetDevice> (gnbDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }
  return netDevice->GetPhy (static_cast<uint8_t> (bwpIndex));
}

Ptr<NrGnbMac>
NrHelper::GetGnbMac (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex)
{
  NS_LOG_FUNCTION (gnbDevice << bwpIndex);
  NS_ASSERT(bwpIndex < UINT8_MAX);
  Ptr<NrGnbNetDevice> netDevice = DynamicCast<NrGnbNetDevice> (gnbDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }
  return netDevice->GetMac (static_cast<uint8_t> (bwpIndex));
}

Ptr<NrUeMac>
NrHelper::GetUeMac(const Ptr<NetDevice> &ueDevice, uint32_t bwpIndex)
{
  NS_LOG_FUNCTION (ueDevice << bwpIndex);
  NS_ASSERT(bwpIndex < UINT8_MAX);
  Ptr<NrUeNetDevice> netDevice = DynamicCast<NrUeNetDevice> (ueDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }
  return netDevice->GetMac (static_cast<uint8_t> (bwpIndex));
}

Ptr<NrUePhy>
NrHelper::GetUePhy(const Ptr<NetDevice> &ueDevice, uint32_t bwpIndex)
{
  NS_LOG_FUNCTION (ueDevice << bwpIndex);
  NS_ASSERT(bwpIndex < UINT8_MAX);
  Ptr<NrUeNetDevice> netDevice = DynamicCast<NrUeNetDevice> (ueDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }
  return netDevice->GetPhy (static_cast<uint8_t> (bwpIndex));
}

Ptr<BwpManagerGnb>
NrHelper::GetBwpManagerGnb(const Ptr<NetDevice> &gnbDevice)
{
  NS_LOG_FUNCTION (gnbDevice);

  Ptr<NrGnbNetDevice> netDevice = DynamicCast<NrGnbNetDevice> (gnbDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }

  return netDevice->GetBwpManager ();
}

Ptr<BwpManagerUe>
NrHelper::GetBwpManagerUe(const Ptr<NetDevice> &ueDevice)
{
  NS_LOG_FUNCTION (ueDevice);

  Ptr<NrUeNetDevice> netDevice = DynamicCast<NrUeNetDevice> (ueDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }

  return netDevice->GetBwpManager ();
}

Ptr<NrMacScheduler>
NrHelper::GetScheduler(const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex)
{
  NS_LOG_FUNCTION (gnbDevice << bwpIndex);

  Ptr<NrGnbNetDevice> netDevice = DynamicCast<NrGnbNetDevice> (gnbDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }

  return netDevice->GetScheduler (bwpIndex);
}

void
NrHelper::SetHarqEnabled (bool harqEnabled)
{
  m_harqEnabled = harqEnabled;
}

bool
NrHelper::GetHarqEnabled ()
{
  return m_harqEnabled;
}

void
NrHelper::SetSnrTest (bool snrTest)
{
  m_snrTest = snrTest;
}

bool
NrHelper::GetSnrTest ()
{
  return m_snrTest;
}

NetDeviceContainer
NrHelper::InstallUeDevice (const NodeContainer &c,
                               const std::vector<std::reference_wrapper<BandwidthPartInfoPtr> > &allBwps,
                               uint8_t numberOfStreams)
{
  NS_LOG_FUNCTION (this);
  Initialize ();    // Run DoInitialize (), if necessary
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleUeDevice (node, allBwps, numberOfStreams);
      device->SetAddress (Mac48Address::Allocate ());
      devices.Add (device);
    }
  return devices;

}

NetDeviceContainer
NrHelper::InstallGnbDevice (const NodeContainer & c,
                                const std::vector<std::reference_wrapper<BandwidthPartInfoPtr> > allBwps,
                                uint8_t numberOfStreams)
{
  NS_LOG_FUNCTION (this);
  Initialize ();    // Run DoInitialize (), if necessary
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleGnbDevice (node, allBwps, numberOfStreams);
      device->SetAddress (Mac48Address::Allocate ());
      devices.Add (device);
    }
  return devices;
}

Ptr<NrUeMac>
NrHelper::CreateUeMac () const
{
  NS_LOG_FUNCTION (this);
  Ptr<NrUeMac> mac = m_ueMacFactory.Create <NrUeMac> ();
  return mac;
}

Ptr<NrUePhy>
NrHelper::CreateUePhy (const Ptr<Node> &n, const std::unique_ptr<BandwidthPartInfo> &bwp,
                           const Ptr<NrUeNetDevice> &dev,
                           const NrSpectrumPhy::NrPhyRxCtrlEndOkCallback &phyRxCtrlCallback,
                           uint8_t numberOfStreams)
{
  NS_LOG_FUNCTION (this);

  Ptr<NrUePhy> phy = m_uePhyFactory.Create <NrUePhy> ();

  NS_ASSERT (bwp->m_channel != nullptr);

  DoubleValue frequency;
  bool res = bwp->m_propagation->GetAttributeFailSafe ("Frequency", frequency);
  NS_ASSERT_MSG (res, "Propagation model without Frequency attribute");
  phy->InstallCentralFrequency (frequency.Get ());

  phy->ScheduleStartEventLoop (n->GetId (), 0, 0, 0);

  // connect CAM and PHY
  Ptr<NrChAccessManager> cam = DynamicCast<NrChAccessManager> (m_ueChannelAccessManagerFactory.Create ());
  phy->SetCam (cam);
  // set device
  phy->SetDevice (dev);

  Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
  NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling NrHelper::InstallUeDevice ()");

  for (uint8_t streamIndex = 0 ; streamIndex < numberOfStreams; streamIndex++)
    {
      Ptr<NrSpectrumPhy> channelPhy = m_ueSpectrumFactory.Create <NrSpectrumPhy> (); // Create NrSpectrumPhy per stream
      channelPhy->SetStreamId (streamIndex);

      Ptr<NrHarqPhy> harq = Create<NrHarqPhy> (); // Create HARQ instance per stream
      channelPhy->InstallHarqPhyModule (harq);

      channelPhy->SetDevice (dev); // each NrSpectrumPhy should have a pointer to device

      Ptr<UniformPlanarArray> antenna = m_ueAntennaFactory.Create <UniformPlanarArray> (); // Create antenna per stream
      channelPhy->SetAntenna (antenna);

      if (streamIndex == 0)
        {
          cam->SetNrSpectrumPhy (channelPhy); // TODO currently we connect CAM only to the first stream
        }

      Ptr<LteChunkProcessor> pData = Create<LteChunkProcessor> ();
      pData->AddCallback (MakeCallback (&NrSpectrumPhy::GenerateDlCqiReport, channelPhy));
      pData->AddCallback (MakeCallback (&NrSpectrumPhy::UpdateSinrPerceived, channelPhy));
      channelPhy->AddDataSinrChunkProcessor (pData);

      Ptr<LteChunkProcessor> pRs = Create<LteChunkProcessor> ();
      pRs->AddCallback (MakeCallback (&NrSpectrumPhy::ReportRsReceivedPower, channelPhy));
      channelPhy->AddRsPowerChunkProcessor (pRs);

      Ptr<LteChunkProcessor> pSinr = Create<LteChunkProcessor> ();
      pSinr->AddCallback (MakeCallback (&NrSpectrumPhy::ReportDlCtrlSinr, channelPhy));
      channelPhy->AddDlCtrlSinrChunkProcessor (pSinr);

      channelPhy->SetChannel (bwp->m_channel);
      channelPhy->InstallPhy (phy);
      channelPhy->SetMobility (mm);
      channelPhy->SetPhyRxDataEndOkCallback (MakeCallback (&NrUePhy::PhyDataPacketReceived, phy));
      channelPhy->SetPhyRxCtrlEndOkCallback (phyRxCtrlCallback);

      Ptr<BeamManager> beamManager = m_ueBeamManagerFactory.Create<BeamManager>();
      beamManager->Configure (antenna);
      channelPhy->SetBeamManager (beamManager);
      phy->InstallSpectrumPhy (channelPhy);
    }
  return phy;
}

Ptr<NetDevice>
NrHelper::InstallSingleUeDevice (const Ptr<Node> &n,
                                     const std::vector<std::reference_wrapper<BandwidthPartInfoPtr> > allBwps,
                                     uint8_t numberOfStreams)
{
  NS_LOG_FUNCTION (this);

  Ptr<NrUeNetDevice> dev = m_ueNetDeviceFactory.Create<NrUeNetDevice> ();
  dev->SetNode (n);

  std::map<uint8_t, Ptr<BandwidthPartUe> > ueCcMap;

  // Create, for each ue, its bandwidth parts
  for (uint32_t bwpId = 0; bwpId < allBwps.size (); ++bwpId)
    {
      Ptr <BandwidthPartUe> cc =  CreateObject<BandwidthPartUe> ();
      double bwInKhz = allBwps[bwpId].get()->m_channelBandwidth / 1000.0;
      NS_ABORT_MSG_IF (bwInKhz/100.0 > 65535.0, "A bandwidth of " << bwInKhz/100.0 << " kHz cannot be represented");
      cc->SetUlBandwidth (static_cast<uint16_t> (bwInKhz / 100));
      cc->SetDlBandwidth (static_cast<uint16_t> (bwInKhz / 100));
      cc->SetDlEarfcn (0); // Used for nothing..
      cc->SetUlEarfcn (0); // Used for nothing..

      auto mac = CreateUeMac ();
      cc->SetMac (mac);

      auto phy = CreateUePhy (n, allBwps[bwpId].get(), dev, std::bind (&NrUeNetDevice::RouteIngoingCtrlMsgs, dev,
                                         std::placeholders::_1, bwpId), numberOfStreams);

      if (m_harqEnabled)
        {
          phy->SetPhyDlHarqFeedbackCallback (MakeCallback (&NrUeNetDevice::EnqueueDlHarqFeedback, dev));
        }

      phy->SetBwpId (bwpId);
      cc->SetPhy (phy);

      if (bwpId == 0)
        {
          cc->SetAsPrimary (true);
        }
      else
        {
          cc->SetAsPrimary (false);
        }

      ueCcMap.insert (std::make_pair (bwpId, cc));
    }

  Ptr<LteUeComponentCarrierManager> ccmUe = DynamicCast<LteUeComponentCarrierManager> (m_bwpManagerFactory.Create ());
  DynamicCast<BwpManagerUe> (ccmUe)->SetBwpManagerAlgorithm (m_ueBwpManagerAlgoFactory.Create <BwpManagerAlgorithm> ());

  Ptr<LteUeRrc> rrc = CreateObject<LteUeRrc> ();
  rrc->m_numberOfComponentCarriers = ueCcMap.size ();
  // run intializeSap to create the proper number of sap provider/users
  rrc->InitializeSap ();
  rrc->SetLteMacSapProvider (ccmUe->GetLteMacSapProvider ());
  // setting ComponentCarrierManager SAP
  rrc->SetLteCcmRrcSapProvider (ccmUe->GetLteCcmRrcSapProvider ());
  ccmUe->SetLteCcmRrcSapUser (rrc->GetLteCcmRrcSapUser ());
  ccmUe->SetNumberOfComponentCarriers (ueCcMap.size ());

  bool useIdealRrc = true;
  if (useIdealRrc)
    {
      Ptr<nrUeRrcProtocolIdeal> rrcProtocol = CreateObject<nrUeRrcProtocolIdeal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetLteUeRrcSapProvider (rrc->GetLteUeRrcSapProvider ());
      rrc->SetLteUeRrcSapUser (rrcProtocol->GetLteUeRrcSapUser ());
    }
  else
    {
      Ptr<LteUeRrcProtocolReal> rrcProtocol = CreateObject<LteUeRrcProtocolReal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetLteUeRrcSapProvider (rrc->GetLteUeRrcSapProvider ());
      rrc->SetLteUeRrcSapUser (rrcProtocol->GetLteUeRrcSapUser ());
    }

  if (m_epcHelper != nullptr)
    {
      rrc->SetUseRlcSm (false);
    }
  else
    {
      rrc->SetUseRlcSm (true);
    }
  Ptr<EpcUeNas> nas = CreateObject<EpcUeNas> ();

  nas->SetAsSapProvider (rrc->GetAsSapProvider ());
  nas->SetDevice (dev);
  nas->SetForwardUpCallback (MakeCallback (&NrUeNetDevice::Receive, dev));

  rrc->SetAsSapUser (nas->GetAsSapUser ());

  for (auto it = ueCcMap.begin (); it != ueCcMap.end (); ++it)
    {
      rrc->SetLteUeCmacSapProvider (it->second->GetMac ()->GetUeCmacSapProvider (), it->first);
      it->second->GetMac ()->SetUeCmacSapUser (rrc->GetLteUeCmacSapUser (it->first));

      it->second->GetPhy ()->SetUeCphySapUser (rrc->GetLteUeCphySapUser ());
      rrc->SetLteUeCphySapProvider (it->second->GetPhy ()->GetUeCphySapProvider (), it->first);

      it->second->GetPhy ()->SetPhySapUser (it->second->GetMac ()->GetPhySapUser ());
      it->second->GetMac ()->SetPhySapProvider (it->second->GetPhy ()->GetPhySapProvider ());

      bool ccmTest = ccmUe->SetComponentCarrierMacSapProviders (it->first,
                                                                it->second->GetMac ()->GetUeMacSapProvider ());

      if (ccmTest == false)
        {
          NS_FATAL_ERROR ("Error in SetComponentCarrierMacSapProviders");
        }
    }

  NS_ABORT_MSG_IF (m_imsiCounter >= 0xFFFFFFFF, "max num UEs exceeded");
  uint64_t imsi = ++m_imsiCounter;

  dev->SetAttribute ("Imsi", UintegerValue (imsi));
  dev->SetCcMap (ueCcMap);
  dev->SetAttribute ("nrUeRrc", PointerValue (rrc));
  dev->SetAttribute ("EpcUeNas", PointerValue (nas));
  dev->SetAttribute ("LteUeComponentCarrierManager", PointerValue (ccmUe));

  n->AddDevice (dev);


  if (m_epcHelper != nullptr)
    {
      m_epcHelper->AddUe (dev, dev->GetImsi ());
    }

  dev->Initialize ();

  return dev;
}

Ptr<NrGnbPhy>
NrHelper::CreateGnbPhy (const Ptr<Node> &n, const std::unique_ptr<BandwidthPartInfo> &bwp,
                           const Ptr<NrGnbNetDevice> &dev,
                           const NrSpectrumPhy::NrPhyRxCtrlEndOkCallback &phyEndCtrlCallback,
                           uint8_t numberOfStreams)
{
  NS_LOG_FUNCTION (this);

  Ptr<NrGnbPhy> phy = m_gnbPhyFactory.Create <NrGnbPhy> ();

  DoubleValue frequency;
  bool res = bwp->m_propagation->GetAttributeFailSafe ("Frequency", frequency);
  NS_ASSERT_MSG (res, "Propagation model without Frequency attribute");
  phy->InstallCentralFrequency (frequency.Get ());

  phy->ScheduleStartEventLoop (n->GetId (), 0, 0, 0);

  // PHY <--> CAM
  Ptr<NrChAccessManager> cam = DynamicCast<NrChAccessManager> (m_gnbChannelAccessManagerFactory.Create ());
  phy->SetCam (cam);
  phy->SetDevice (dev);

  Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
  NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling NrHelper::InstallEnbDevice ()");

  for (uint8_t streamIndex = 0 ; streamIndex < numberOfStreams; streamIndex++)
    {
      Ptr<NrSpectrumPhy> channelPhy = m_gnbSpectrumFactory.Create <NrSpectrumPhy> ();
      Ptr<UniformPlanarArray> antenna = m_gnbAntennaFactory.Create <UniformPlanarArray> ();
      channelPhy->SetAntenna (antenna);

      if (streamIndex == 0)
        {
          // TODO currently we set to CAM only the first spectrum phy, this feature needs to be further extended
          cam->SetNrSpectrumPhy (channelPhy);
        }

      channelPhy->InstallHarqPhyModule (Create<NrHarqPhy> ()); // there should be one HARQ instance per NrSpectrumPhy
      channelPhy->SetDevice (dev); // each NrSpectrumPhy should have a pointer to device
      channelPhy->SetChannel (bwp->m_channel); // each NrSpectrumPhy needs to have a pointer to the SpectrumChannel object of the corresponding spectrum part
      channelPhy->InstallPhy (phy); // each NrSpectrumPhy should have a pointer to its NrPhy device, in this case NrGnbPhy
      channelPhy->SetStreamId (streamIndex);

      Ptr<LteChunkProcessor> pData = Create<LteChunkProcessor> (); // create pData chunk processor per NrSpectrumPhy
      Ptr<LteChunkProcessor> pSrs = Create<LteChunkProcessor> ();  // create pSrs per processor per NrSpectrumPhy
      if (!m_snrTest)
        {
           pData->AddCallback (MakeCallback (&NrSpectrumPhy::GenerateDataCqiReport, channelPhy)); // connect DATA chunk processor that will call GenerateDataCqiReport function
           pData->AddCallback (MakeCallback (&NrSpectrumPhy::UpdateSinrPerceived, channelPhy));  // connect DATA chunk processor that will call UpdateSinrPerceived function
           pSrs->AddCallback (MakeCallback (&NrSpectrumPhy::UpdateSrsSinrPerceived, channelPhy)); // connect SRS chunk processor that will call UpdateSrsSinrPerceived function
        }
      channelPhy->AddDataSinrChunkProcessor (pData); // set DATA chunk processor to NrSpectrumPhy
      channelPhy->AddSrsSinrChunkProcessor (pSrs);  // set SRS chunk processor to NrSpectrumPhy
      channelPhy->SetMobility (mm); // set mobility model to this NrSpectrumPhy
      channelPhy->SetPhyRxDataEndOkCallback (MakeCallback (&NrGnbPhy::PhyDataPacketReceived, phy)) ; // connect PhyRxDataEndOk callback
      channelPhy->SetPhyRxCtrlEndOkCallback (phyEndCtrlCallback); //connect PhyRxCtrlEndOk callback
      channelPhy->SetPhyUlHarqFeedbackCallback (MakeCallback (&NrGnbPhy::ReportUlHarqFeedback, phy)); // PhyUlHarqFeedback callback


      Ptr<BeamManager> beamManager = m_gnbBeamManagerFactory.Create<BeamManager>();
      beamManager->Configure (antenna);
      channelPhy->SetBeamManager (beamManager);
      phy->InstallSpectrumPhy (channelPhy); // finally let know phy that there is this spectrum phy

    }

  return phy;
}

Ptr<NrGnbMac>
NrHelper::CreateGnbMac ()
{
  NS_LOG_FUNCTION (this);

  Ptr<NrGnbMac> mac = m_gnbMacFactory.Create <NrGnbMac> ();
  return mac;
}

Ptr<NrMacScheduler>
NrHelper::CreateGnbSched ()
{
  NS_LOG_FUNCTION (this);

  auto sched = m_schedFactory.Create <NrMacSchedulerNs3> ();
  auto dlAmc = m_gnbDlAmcFactory.Create <NrAmc> ();
  auto ulAmc = m_gnbUlAmcFactory.Create <NrAmc> ();

  sched->InstallDlAmc (dlAmc);
  sched->InstallUlAmc (ulAmc);

  return sched;
}

Ptr<NetDevice>
NrHelper::InstallSingleGnbDevice (const Ptr<Node> &n,
                                      const std::vector<std::reference_wrapper<BandwidthPartInfoPtr> > allBwps,
                                      uint8_t numberOfStreams)
{
  NS_ABORT_MSG_IF (m_cellIdCounter == 65535, "max num gNBs exceeded");

  Ptr<NrGnbNetDevice> dev = m_gnbNetDeviceFactory.Create<NrGnbNetDevice> ();

  NS_LOG_DEBUG ("Creating gNB, cellId = " << m_cellIdCounter);
  uint16_t cellId = m_cellIdCounter++;

  dev->SetCellId (cellId);
  dev->SetNode (n);

  // create component carrier map for this gNB device
  std::map<uint8_t,Ptr<BandwidthPartGnb> > ccMap;

  for (uint32_t bwpId = 0; bwpId < allBwps.size (); ++bwpId)
    {
      NS_LOG_DEBUG ("Creating BandwidthPart, id = " << bwpId);
      Ptr <BandwidthPartGnb> cc =  CreateObject<BandwidthPartGnb> ();
      double bwInKhz = allBwps[bwpId].get()->m_channelBandwidth / 1000.0;
      NS_ABORT_MSG_IF (bwInKhz/100.0 > 65535.0, "A bandwidth of " << bwInKhz/100.0 << " kHz cannot be represented");

      cc->SetUlBandwidth (static_cast<uint16_t> (bwInKhz / 100));
      cc->SetDlBandwidth (static_cast<uint16_t> (bwInKhz / 100));
      cc->SetDlEarfcn (0); // Argh... handover not working
      cc->SetUlEarfcn (0); // Argh... handover not working
      cc->SetCellId (m_cellIdCounter++);

      auto phy = CreateGnbPhy (n, allBwps[bwpId].get(), dev,
                               std::bind (&NrGnbNetDevice::RouteIngoingCtrlMsgs,
                                          dev, std::placeholders::_1, bwpId), numberOfStreams);
      phy->SetBwpId (bwpId);
      cc->SetPhy (phy);

      auto mac = CreateGnbMac ();
      cc->SetMac (mac);
      phy->GetCam ()->SetNrGnbMac (mac);

      auto sched = CreateGnbSched ();
      cc->SetNrMacScheduler (sched);

      if (bwpId == 0)
        {
          cc->SetAsPrimary (true);
        }
      else
        {
          cc->SetAsPrimary (false);
        }

      ccMap.insert (std::make_pair (bwpId, cc));
    }

  Ptr<LteEnbRrc> rrc = CreateObject<LteEnbRrc> ();
  Ptr<LteEnbComponentCarrierManager> ccmEnbManager = DynamicCast<LteEnbComponentCarrierManager> (CreateObject<BwpManagerGnb> ());
  DynamicCast<BwpManagerGnb> (ccmEnbManager)->SetBwpManagerAlgorithm (m_gnbBwpManagerAlgoFactory.Create <BwpManagerAlgorithm> ());

  // Convert Enb carrier map to only PhyConf map
  // we want to make RRC to be generic, to be able to work with any type of carriers, not only strictly LTE carriers
  std::map < uint8_t, Ptr<ComponentCarrierBaseStation> > ccPhyConfMap;
  for (const auto &i : ccMap)
    {
      Ptr<ComponentCarrierBaseStation> c = i.second;
      ccPhyConfMap.insert (std::make_pair (i.first,c));
    }

  //ComponentCarrierManager SAP
  rrc->SetLteCcmRrcSapProvider (ccmEnbManager->GetLteCcmRrcSapProvider ());
  ccmEnbManager->SetLteCcmRrcSapUser (rrc->GetLteCcmRrcSapUser ());
  // Set number of component carriers. Note: eNB CCM would also set the
  // number of component carriers in eNB RRC

  ccmEnbManager->SetNumberOfComponentCarriers (ccMap.size ());
  rrc->ConfigureCarriers (ccPhyConfMap);

  //nr module currently uses only RRC ideal mode
  bool useIdealRrc = true;

  if (useIdealRrc)
    {
      Ptr<NrGnbRrcProtocolIdeal> rrcProtocol = CreateObject<NrGnbRrcProtocolIdeal> ();
      rrcProtocol->SetLteEnbRrcSapProvider (rrc->GetLteEnbRrcSapProvider ());
      rrc->SetLteEnbRrcSapUser (rrcProtocol->GetLteEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
    }
  else
    {
      Ptr<LteEnbRrcProtocolReal> rrcProtocol = CreateObject<LteEnbRrcProtocolReal> ();
      rrcProtocol->SetLteEnbRrcSapProvider (rrc->GetLteEnbRrcSapProvider ());
      rrc->SetLteEnbRrcSapUser (rrcProtocol->GetLteEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
    }

  if (m_epcHelper != nullptr)
    {
      EnumValue epsBearerToRlcMapping;
      rrc->GetAttribute ("EpsBearerToRlcMapping", epsBearerToRlcMapping);
      // it does not make sense to use RLC/SM when also using the EPC
      if (epsBearerToRlcMapping.Get () == LteEnbRrc::RLC_SM_ALWAYS)
        {
          rrc->SetAttribute ("EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_UM_ALWAYS));
        }
    }

  // This RRC attribute is used to connect each new RLC instance with the MAC layer
  // (for function such as TransmitPdu, ReportBufferStatusReport).
  // Since in this new architecture, the component carrier manager acts a proxy, it
  // will have its own LteMacSapProvider interface, RLC will see it as through original MAC
  // interface LteMacSapProvider, but the function call will go now through LteEnbComponentCarrierManager
  // instance that needs to implement functions of this interface, and its task will be to
  // forward these calls to the specific MAC of some of the instances of component carriers. This
  // decision will depend on the specific implementation of the component carrier manager.
  rrc->SetLteMacSapProvider (ccmEnbManager->GetLteMacSapProvider ());
  rrc->SetForwardUpCallback (MakeCallback (&NrGnbNetDevice::Receive, dev));

  for (auto it = ccMap.begin (); it != ccMap.end (); ++it)
    {
      it->second->GetPhy ()->SetEnbCphySapUser (rrc->GetLteEnbCphySapUser (it->first));
      rrc->SetLteEnbCphySapProvider (it->second->GetPhy ()->GetEnbCphySapProvider (), it->first);

      rrc->SetLteEnbCmacSapProvider (it->second->GetMac ()->GetEnbCmacSapProvider (),it->first );
      it->second->GetMac ()->SetEnbCmacSapUser (rrc->GetLteEnbCmacSapUser (it->first));

      // PHY <--> MAC SAP
      it->second->GetPhy ()->SetPhySapUser (it->second->GetMac ()->GetPhySapUser ());
      it->second->GetMac ()->SetPhySapProvider (it->second->GetPhy ()->GetPhySapProvider ());
      // PHY <--> MAC SAP END

      //Scheduler SAP
      it->second->GetMac ()->SetNrMacSchedSapProvider (it->second->GetScheduler ()->GetMacSchedSapProvider ());
      it->second->GetMac ()->SetNrMacCschedSapProvider (it->second->GetScheduler ()->GetMacCschedSapProvider ());

      it->second->GetScheduler ()->SetMacSchedSapUser (it->second->GetMac ()->GetNrMacSchedSapUser ());
      it->second->GetScheduler ()->SetMacCschedSapUser (it->second->GetMac ()->GetNrMacCschedSapUser ());
      // Scheduler SAP END

      it->second->GetMac ()->SetLteCcmMacSapUser (ccmEnbManager->GetLteCcmMacSapUser ());
      ccmEnbManager->SetCcmMacSapProviders (it->first, it->second->GetMac ()->GetLteCcmMacSapProvider ());

      // insert the pointer to the LteMacSapProvider interface of the MAC layer of the specific component carrier
      ccmEnbManager->SetMacSapProvider (it->first, it->second->GetMac ()->GetMacSapProvider ());
    }


  dev->SetAttribute ("LteEnbComponentCarrierManager", PointerValue (ccmEnbManager));
  dev->SetCcMap (ccMap);
  dev->SetAttribute ("LteEnbRrc", PointerValue (rrc));
  dev->Initialize ();

  n->AddDevice (dev);

  if (m_epcHelper != nullptr)
    {
      NS_LOG_INFO ("adding this eNB to the EPC");
      m_epcHelper->AddEnb (n, dev, dev->GetCellIds ());
      Ptr<EpcEnbApplication> enbApp = n->GetApplication (0)->GetObject<EpcEnbApplication> ();
      NS_ASSERT_MSG (enbApp != nullptr, "cannot retrieve EpcEnbApplication");

      // S1 SAPs
      rrc->SetS1SapProvider (enbApp->GetS1SapProvider ());
      enbApp->SetS1SapUser (rrc->GetS1SapUser ());

      // X2 SAPs
      Ptr<EpcX2> x2 = n->GetObject<EpcX2> ();
      x2->SetEpcX2SapUser (rrc->GetEpcX2SapUser ());
      rrc->SetEpcX2SapProvider (x2->GetEpcX2SapProvider ());
    }

  return dev;
}

void
NrHelper::AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);

  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); i++)
    {
      AttachToClosestEnb (*i, enbDevices);
    }
}

void
NrHelper::AttachToClosestEnb (Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (enbDevices.GetN () > 0, "empty enb device container");
  Vector uepos = ueDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  double minDistance = std::numeric_limits<double>::infinity ();
  Ptr<NetDevice> closestEnbDevice;
  for (NetDeviceContainer::Iterator i = enbDevices.Begin (); i != enbDevices.End (); ++i)
    {
      Vector enbpos = (*i)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
      double distance = CalculateDistance (uepos, enbpos);
      if (distance < minDistance)
        {
          minDistance = distance;
          closestEnbDevice = *i;
        }
    }
  NS_ASSERT (closestEnbDevice != 0);

  AttachToEnb (ueDevice, closestEnbDevice);
}


void
NrHelper::AttachToEnb (const Ptr<NetDevice> &ueDevice,
                           const Ptr<NetDevice> &gnbDevice)
{
  Ptr<NrGnbNetDevice> enbNetDev = gnbDevice->GetObject<NrGnbNetDevice> ();
  Ptr<NrUeNetDevice> ueNetDev = ueDevice->GetObject<NrUeNetDevice> ();

  NS_ABORT_IF (enbNetDev == nullptr || ueNetDev == nullptr);

  for (uint32_t i = 0; i < enbNetDev->GetCcMapSize (); ++i)
    {
      enbNetDev->GetPhy(i)->RegisterUe (ueNetDev->GetImsi (), ueNetDev);
      ueNetDev->GetPhy (i)->RegisterToEnb (enbNetDev->GetBwpId (i));
      ueNetDev->GetPhy (i)->SetDlAmc (
            DynamicCast<NrMacSchedulerNs3> (enbNetDev->GetScheduler (i))->GetDlAmc ());
      ueNetDev->GetPhy (i)->SetDlCtrlSyms (enbNetDev->GetMac(i)->GetDlCtrlSyms ());
      ueNetDev->GetPhy (i)->SetUlCtrlSyms (enbNetDev->GetMac(i)->GetUlCtrlSyms ());
      ueNetDev->GetPhy (i)->SetNumRbPerRbg (enbNetDev->GetMac(i)->GetNumRbPerRbg());
      ueNetDev->GetPhy (i)->SetRbOverhead (enbNetDev->GetPhy (i)->GetRbOverhead ());
      ueNetDev->GetPhy (i)->SetSymbolsPerSlot (enbNetDev->GetPhy (i)->GetSymbolsPerSlot ());
      ueNetDev->GetPhy (i)->SetNumerology (enbNetDev->GetPhy(i)->GetNumerology ());
      ueNetDev->GetPhy (i)->SetPattern (enbNetDev->GetPhy (i)->GetPattern ());
      Ptr<EpcUeNas> ueNas = ueNetDev->GetNas ();
      ueNas->Connect (enbNetDev->GetBwpId (i), enbNetDev->GetEarfcn (i));
    }

  if (m_epcHelper)
    {
      // activate default EPS bearer
      m_epcHelper->ActivateEpsBearer (ueDevice, ueNetDev->GetImsi (), EpcTft::Default (), EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
    }

  // tricks needed for the simplified LTE-only simulations
  //if (m_epcHelper == 0)
  //{
  ueNetDev->SetTargetEnb (enbNetDev);
  //}

  if (m_beamformingHelper)
    {
      m_beamformingHelper->AddBeamformingTask (enbNetDev, ueNetDev);
    }
}



uint8_t
NrHelper::ActivateDedicatedEpsBearer (NetDeviceContainer ueDevices, EpsBearer bearer, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      uint8_t bearerId = ActivateDedicatedEpsBearer (*i, bearer, tft);
      return bearerId;
    }
  return 0;
}


uint8_t
NrHelper::ActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "dedicated EPS bearers cannot be set up when the EPC is not used");

  uint64_t imsi = ueDevice->GetObject<NrUeNetDevice> ()->GetImsi ();
  uint8_t bearerId = m_epcHelper->ActivateEpsBearer (ueDevice, imsi, tft, bearer);
  return bearerId;
}

void
NrHelper::DeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice,Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);
  NS_ASSERT_MSG (m_epcHelper != nullptr, "Dedicated EPS bearers cannot be de-activated when the EPC is not used");
  NS_ASSERT_MSG (bearerId != 1, "Default bearer cannot be de-activated until and unless and UE is released");

  DoDeActivateDedicatedEpsBearer (ueDevice, enbDevice, bearerId);
}

void
NrHelper::SetUeMacAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueMacFactory.Set (n, v);
}

void
NrHelper::SetGnbMacAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbMacFactory.Set (n, v);
}

void
NrHelper::SetGnbSpectrumAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbSpectrumFactory.Set (n, v);
}

void
NrHelper::SetUeSpectrumAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueSpectrumFactory.Set (n, v);
}

void
NrHelper::SetUeChannelAccessManagerAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueChannelAccessManagerFactory.Set (n, v);
}

void
NrHelper::SetGnbChannelAccessManagerAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbChannelAccessManagerFactory.Set (n, v);
}

void
NrHelper::SetSchedulerAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_schedFactory.Set (n, v);
}

void
NrHelper::SetUePhyAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_uePhyFactory.Set (n, v);
}

void
NrHelper::SetGnbPhyAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbPhyFactory.Set (n, v);
}

void
NrHelper::SetUeAntennaAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueAntennaFactory.Set (n, v);
}

void
NrHelper::SetGnbAntennaAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbAntennaFactory.Set (n, v);
}

void
NrHelper::SetUeChannelAccessManagerTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_ueChannelAccessManagerFactory.SetTypeId (typeId);
}

void
NrHelper::SetGnbChannelAccessManagerTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_gnbChannelAccessManagerFactory.SetTypeId (typeId);
}

void
NrHelper::SetSchedulerTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_schedFactory.SetTypeId (typeId);
}

void
NrHelper::SetUeBwpManagerAlgorithmTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_ueBwpManagerAlgoFactory.SetTypeId (typeId);
}

void
NrHelper::SetPhasedArraySpectrumPropagationLossModelTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_spectrumPropagationFactory.SetTypeId(typeId);
}

void
NrHelper::SetUeBwpManagerAlgorithmAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueBwpManagerAlgoFactory.Set (n, v);
}

void
NrHelper::SetPhasedArraySpectrumPropagationLossModelAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_spectrumPropagationFactory.Set (n, v);
}

void
NrHelper::SetChannelConditionModelAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_channelConditionModelFactory.Set (n, v);
}

void
NrHelper::SetPathlossAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_pathlossModelFactory.Set (n, v);
}

void
NrHelper::SetGnbDlAmcAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbDlAmcFactory.Set (n, v);
}

void
NrHelper::SetGnbUlAmcAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbUlAmcFactory.Set (n, v);
}

void
NrHelper::SetGnbBeamManagerAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbBeamManagerFactory.Set (n, v);
}

void
NrHelper::SetGnbBeamManagerTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_gnbBeamManagerFactory.SetTypeId (typeId);
}

void
NrHelper::SetUlErrorModel(const std::string &errorModelTypeId)
{
  NS_LOG_FUNCTION (this);

  SetGnbUlAmcAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName (errorModelTypeId)));
  SetGnbSpectrumAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModelTypeId)));
}

void
NrHelper::SetDlErrorModel(const std::string &errorModelTypeId)
{
  NS_LOG_FUNCTION (this);

  SetGnbDlAmcAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName (errorModelTypeId)));
  SetUeSpectrumAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModelTypeId)));
}


int64_t
NrHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<NrGnbNetDevice> nrGnb = DynamicCast<NrGnbNetDevice> (netDevice);
      if (nrGnb)
        {
          for (uint32_t bwp = 0; bwp < nrGnb->GetCcMapSize (); bwp++)
            {
              currentStream += nrGnb->GetScheduler (bwp)->AssignStreams (currentStream);
              for (uint8_t streamIndex = 0; streamIndex < nrGnb->GetPhy (bwp)->GetNumberOfStreams (); streamIndex++)
                {
                  currentStream += nrGnb->GetPhy (bwp)->GetSpectrumPhy (streamIndex)->AssignStreams (currentStream);
                  currentStream += DoAssignStreamsToChannelObjects (nrGnb->GetPhy (bwp)->GetSpectrumPhy (streamIndex), currentStream);
                }
            }
        }

      Ptr<NrUeNetDevice> nrUe = DynamicCast<NrUeNetDevice> (netDevice);
      if (nrUe)
        {
          for (uint32_t bwp = 0; bwp < nrUe->GetCcMapSize (); bwp++)
            {
              currentStream += nrUe->GetMac (bwp)->AssignStreams (currentStream);
              for (uint8_t streamIndex = 0; streamIndex < nrUe->GetPhy (bwp)->GetNumberOfStreams (); streamIndex++)
                {
                  currentStream += nrUe->GetPhy (bwp)->GetSpectrumPhy (streamIndex)->AssignStreams (currentStream);
                  currentStream += DoAssignStreamsToChannelObjects (nrUe->GetPhy (bwp)->GetSpectrumPhy (streamIndex), currentStream);
                }
            }
        }
    }

  return (currentStream - stream);
}

int64_t
NrHelper::DoAssignStreamsToChannelObjects (Ptr<NrSpectrumPhy> phy, int64_t currentStream)
{
  Ptr<ThreeGppPropagationLossModel> propagationLossModel = DynamicCast<ThreeGppPropagationLossModel> (phy->GetSpectrumChannel ()->GetPropagationLossModel ());
  NS_ASSERT (propagationLossModel != nullptr);

  int64_t initialStream = currentStream;

  if (std::find (m_channelObjectsWithAssignedStreams.begin(),
                 m_channelObjectsWithAssignedStreams.end (),
                 propagationLossModel) == m_channelObjectsWithAssignedStreams.end ())
    {
      currentStream += propagationLossModel->AssignStreams (currentStream);
      m_channelObjectsWithAssignedStreams.push_back (propagationLossModel);
    }

  Ptr<ChannelConditionModel> channelConditionModel = propagationLossModel->GetChannelConditionModel();

  if (std::find (m_channelObjectsWithAssignedStreams.begin(),
                 m_channelObjectsWithAssignedStreams.end (),
                 channelConditionModel) == m_channelObjectsWithAssignedStreams.end ())
    {
      currentStream += channelConditionModel->AssignStreams (currentStream);
      m_channelObjectsWithAssignedStreams.push_back (channelConditionModel);
    }

  Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (phy->GetSpectrumChannel ()->GetPhasedArraySpectrumPropagationLossModel ());

  if (spectrumLossModel)
    {
      if (std::find (m_channelObjectsWithAssignedStreams.begin(),
                     m_channelObjectsWithAssignedStreams.end (),
                     spectrumLossModel) == m_channelObjectsWithAssignedStreams.end ())
        {
          Ptr <ThreeGppChannelModel> channel = DynamicCast<ThreeGppChannelModel> (spectrumLossModel->GetChannelModel());
          currentStream += channel->AssignStreams (currentStream);
          m_channelObjectsWithAssignedStreams.push_back (channel);
        }
    }

  return currentStream - initialStream;
}


void
NrHelper::SetGnbBwpManagerAlgorithmTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_gnbBwpManagerAlgoFactory.SetTypeId (typeId);
}

void NrHelper::SetGnbBwpManagerAlgorithmAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbBwpManagerAlgoFactory.Set (n, v);
}

void
NrHelper::DoDeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);

  //Extract IMSI and rnti
  uint64_t imsi = ueDevice->GetObject<NrUeNetDevice> ()->GetImsi ();
  uint16_t rnti = ueDevice->GetObject<NrUeNetDevice> ()->GetRrc ()->GetRnti ();


  Ptr<LteEnbRrc> enbRrc = enbDevice->GetObject<NrGnbNetDevice> ()->GetRrc ();

  enbRrc->DoSendReleaseDataRadioBearer (imsi,rnti,bearerId);
}


void
NrHelper::SetEpcHelper (Ptr<EpcHelper> epcHelper)
{
  m_epcHelper = epcHelper;
}

void
NrHelper::SetBeamformingHelper (Ptr<BeamformingHelperBase> beamformingHelper)
{
  m_beamformingHelper = beamformingHelper;
  m_beamformingHelper->Initialize();
}

class NrDrbActivator : public SimpleRefCount<NrDrbActivator>
{
public:
  NrDrbActivator (Ptr<NetDevice> ueDevice, EpsBearer bearer);
  static void ActivateCallback (Ptr<NrDrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti);
  void ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti);
private:
  bool m_active;
  Ptr<NetDevice> m_ueDevice;
  EpsBearer m_bearer;
  uint64_t m_imsi;
};

NrDrbActivator::NrDrbActivator (Ptr<NetDevice> ueDevice, EpsBearer bearer)
  : m_active (false),
  m_ueDevice (ueDevice),
  m_bearer (bearer),
  m_imsi (m_ueDevice->GetObject< NrUeNetDevice> ()->GetImsi ())
{
}

void
NrDrbActivator::ActivateCallback (Ptr<NrDrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (a << context << imsi << cellId << rnti);
  a->ActivateDrb (imsi, cellId, rnti);
}

void
NrDrbActivator::ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{

  NS_LOG_FUNCTION (this << imsi << cellId << rnti << m_active);
  if ((!m_active) && (imsi == m_imsi))
    {
      Ptr<LteUeRrc> ueRrc = m_ueDevice->GetObject<NrUeNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetState () == LteUeRrc::CONNECTED_NORMALLY);
      uint16_t rnti = ueRrc->GetRnti ();
      Ptr<const NrGnbNetDevice> enbLteDevice = m_ueDevice->GetObject<NrUeNetDevice> ()->GetTargetEnb ();
      Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetObject<NrGnbNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetCellId () == enbLteDevice->GetCellId ());
      Ptr<UeManager> ueManager = enbRrc->GetUeManager (rnti);
      NS_ASSERT (ueManager->GetState () == UeManager::CONNECTED_NORMALLY
                 || ueManager->GetState () == UeManager::CONNECTION_RECONFIGURATION);
      EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params;
      params.rnti = rnti;
      params.bearer = m_bearer;
      params.bearerId = 0;
      params.gtpTeid = 0;   // don't care
      enbRrc->GetS1SapUser ()->DataRadioBearerSetupRequest (params);
      m_active = true;
    }
}
void
NrHelper::ActivateDataRadioBearer (NetDeviceContainer ueDevices, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      ActivateDataRadioBearer (*i, bearer);
    }
}
void
NrHelper::ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice);
  NS_ASSERT_MSG (m_epcHelper == 0, "this method must not be used when the EPC is being used");

  // Normally it is the EPC that takes care of activating DRBs
  // when the UE gets connected. When the EPC is not used, we achieve
  // the same behavior by hooking a dedicated DRB activation function
  // to the Enb RRC Connection Established trace source


  Ptr<const NrGnbNetDevice> enbnrDevice = ueDevice->GetObject<NrUeNetDevice> ()->GetTargetEnb ();

  std::ostringstream path;
  path << "/NodeList/" << enbnrDevice->GetNode ()->GetId ()
       << "/DeviceList/" << enbnrDevice->GetIfIndex ()
       << "/LteEnbRrc/ConnectionEstablished";
  Ptr<NrDrbActivator> arg = Create<NrDrbActivator> (ueDevice, bearer);
  Config::Connect (path.str (), MakeBoundCallback (&NrDrbActivator::ActivateCallback, arg));
}


void
NrHelper::EnableTraces (void)
{
  EnableDlDataPhyTraces ();
  EnableDlCtrlPhyTraces ();
  EnableUlPhyTraces ();
  //EnableEnbPacketCountTrace ();
  //EnableUePacketCountTrace ();
  //EnableTransportBlockTrace ();
  EnableRlcSimpleTraces ();
  EnableRlcE2eTraces ();
  EnablePdcpSimpleTraces ();
  EnablePdcpE2eTraces ();
  EnableGnbPhyCtrlMsgsTraces ();
  EnableUePhyCtrlMsgsTraces ();
  EnableGnbMacCtrlMsgsTraces ();
  EnableUeMacCtrlMsgsTraces ();
  EnableDlMacSchedTraces ();
  EnableUlMacSchedTraces ();
  EnablePathlossTraces ();
}

Ptr<NrPhyRxTrace>
NrHelper::GetPhyRxTrace (void)
{
  return m_phyStats;
}

void
NrHelper::EnableDlDataPhyTraces (void)
{
  //NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/DlDataSinr",
                   MakeBoundCallback (&NrPhyRxTrace::DlDataSinrCallback, m_phyStats));

  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/NrSpectrumPhyList/*/RxPacketTraceUe",
                   MakeBoundCallback (&NrPhyRxTrace::RxPacketTraceUeCallback, m_phyStats));
}


void
NrHelper::EnableDlCtrlPhyTraces (void)
{
  //NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/DlCtrlSinr",
                   MakeBoundCallback (&NrPhyRxTrace::DlCtrlSinrCallback, m_phyStats));
}

void
NrHelper::EnableGnbPhyCtrlMsgsTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbPhy/GnbPhyRxedCtrlMsgsTrace",
                   MakeBoundCallback (&NrPhyRxTrace::RxedGnbPhyCtrlMsgsCallback, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbPhy/GnbPhyTxedCtrlMsgsTrace",
                   MakeBoundCallback (&NrPhyRxTrace::TxedGnbPhyCtrlMsgsCallback, m_phyStats));
}

void
NrHelper::EnableGnbMacCtrlMsgsTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbMac/GnbMacRxedCtrlMsgsTrace",
                   MakeBoundCallback (&NrMacRxTrace::RxedGnbMacCtrlMsgsCallback, m_macStats));

  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbMac/GnbMacTxedCtrlMsgsTrace",
                   MakeBoundCallback (&NrMacRxTrace::TxedGnbMacCtrlMsgsCallback, m_macStats));
}

void
NrHelper::EnableUePhyCtrlMsgsTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/UePhyRxedCtrlMsgsTrace",
                   MakeBoundCallback (&NrPhyRxTrace::RxedUePhyCtrlMsgsCallback, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/UePhyTxedCtrlMsgsTrace",
                   MakeBoundCallback (&NrPhyRxTrace::TxedUePhyCtrlMsgsCallback, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/UePhyRxedDlDciTrace",
                   MakeBoundCallback (&NrPhyRxTrace::RxedUePhyDlDciCallback, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/UePhyTxedHarqFeedbackTrace",
                   MakeBoundCallback (&NrPhyRxTrace::TxedUePhyHarqFeedbackCallback, m_phyStats));
}

void
NrHelper::EnableUeMacCtrlMsgsTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUeMac/UeMacRxedCtrlMsgsTrace",
                   MakeBoundCallback (&NrMacRxTrace::RxedUeMacCtrlMsgsCallback, m_macStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUeMac/UeMacTxedCtrlMsgsTrace",
                   MakeBoundCallback (&NrMacRxTrace::TxedUeMacCtrlMsgsCallback, m_macStats));
}

void
NrHelper::EnableUlPhyTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbPhy/NrSpectrumPhyList/*/RxPacketTraceEnb",
                   MakeBoundCallback (&NrPhyRxTrace::RxPacketTraceEnbCallback, m_phyStats));
}

void
NrHelper::EnableGnbPacketCountTrace ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbPhy/NrSpectrumPhyList/*/ReportEnbTxRxPacketCount",
                   MakeBoundCallback (&NrPhyRxTrace::ReportPacketCountEnbCallback, m_phyStats));

}

void
NrHelper::EnableUePacketCountTrace ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/NrSpectrumPhyList/*/ReportUeTxRxPacketCount",
                   MakeBoundCallback (&NrPhyRxTrace::ReportPacketCountUeCallback, m_phyStats));

}

void
NrHelper::EnableTransportBlockTrace ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/ReportDownlinkTbSize",
                   MakeBoundCallback (&NrPhyRxTrace::ReportDownLinkTBSize, m_phyStats));
}


void
NrHelper::EnableRlcSimpleTraces (void)
{
  Ptr<NrBearerStatsSimple> rlcStats = CreateObject<NrBearerStatsSimple> ("RLC");
  m_radioBearerStatsConnectorSimpleTraces.EnableRlcStats (rlcStats);
}

void
NrHelper::EnablePdcpSimpleTraces (void)
{
  Ptr<NrBearerStatsSimple> pdcpStats = CreateObject<NrBearerStatsSimple> ("PDCP");
  m_radioBearerStatsConnectorSimpleTraces.EnablePdcpStats (pdcpStats);
}

void
NrHelper::EnableRlcE2eTraces (void)
{
  Ptr<NrBearerStatsCalculator> rlcStats = CreateObject<NrBearerStatsCalculator> ("RLC");
  m_radioBearerStatsConnectorCalculator.EnableRlcStats (rlcStats);
}

void
NrHelper::EnablePdcpE2eTraces (void)
{
  Ptr<NrBearerStatsCalculator> pdcpStats = CreateObject<NrBearerStatsCalculator> ("PDCP");
  m_radioBearerStatsConnectorCalculator.EnablePdcpStats (pdcpStats);
}



Ptr<NrBearerStatsCalculator>
NrHelper::GetRlcStatsCalculator (void)
{
  return DynamicCast<NrBearerStatsCalculator> (m_radioBearerStatsConnectorCalculator.GetRlcStats ());
}


Ptr<NrBearerStatsCalculator>
NrHelper::GetPdcpStatsCalculator (void)
{
  return DynamicCast<NrBearerStatsCalculator> (m_radioBearerStatsConnectorCalculator.GetPdcpStats ());
}

void
NrHelper::EnableDlMacSchedTraces ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbMac/DlScheduling",
                   MakeBoundCallback (&NrMacSchedulingStats::DlSchedulingCallback, m_macSchedStats));
}

void
NrHelper::EnableUlMacSchedTraces ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbMac/UlScheduling",
                   MakeBoundCallback (&NrMacSchedulingStats::UlSchedulingCallback, m_macSchedStats));
}

void
NrHelper::EnablePathlossTraces ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/ChannelList/*/$ns3::SpectrumChannel/PathLoss",
                   MakeBoundCallback (&NrPhyRxTrace::PathlossTraceCallback, m_phyStats));

}

void
NrHelper::SetBwpManagerTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_bwpManagerFactory.SetTypeId (typeId);
}

} // namespace ns3

