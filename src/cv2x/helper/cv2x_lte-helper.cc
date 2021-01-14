/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Nicola Baldo <nbaldo@cttc.es> (re-wrote from scratch this helper)
 *          Giuseppe Piro <g.piro@poliba.it> (parts of the PHY & channel  creation & configuration copied from the GSoC 2011 code)
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 *          NIST (D2D)
 *          Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *          Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#include "cv2x_lte-helper.h"
#include <ns3/string.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/cv2x_lte-enb-rrc.h>
#include <ns3/cv2x_epc-ue-nas.h>
#include <ns3/cv2x_epc-enb-application.h>
#include <ns3/cv2x_lte-ue-rrc.h>
#include <ns3/cv2x_lte-ue-mac.h>
#include <ns3/cv2x_lte-enb-mac.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/cv2x_lte-ue-phy.h>
#include <ns3/cv2x_lte-spectrum-phy.h>
#include <ns3/cv2x_lte-chunk-processor.h>
#include <ns3/cv2x_lte-sl-chunk-processor.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/friis-spectrum-propagation-loss.h>
#include <ns3/cv2x_trace-fading-loss-model.h>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/cv2x_lte-ue-net-device.h>
#include <ns3/cv2x_ff-mac-scheduler.h>
#include <ns3/cv2x_lte-ffr-algorithm.h>
#include <ns3/cv2x_lte-handover-algorithm.h>
#include <ns3/cv2x_lte-enb-component-carrier-manager.h>
#include <ns3/cv2x_lte-ue-component-carrier-manager.h>
#include <ns3/cv2x_lte-anr.h>
#include <ns3/cv2x_lte-rlc.h>
#include <ns3/cv2x_lte-rlc-um.h>
#include <ns3/cv2x_lte-rlc-am.h>
#include <ns3/cv2x_epc-enb-s1-sap.h>
#include <ns3/cv2x_lte-rrc-protocol-ideal.h>
#include <ns3/cv2x_lte-rrc-protocol-real.h>
#include <ns3/cv2x_mac-stats-calculator.h>
#include <ns3/cv2x_phy-stats-calculator.h>
#include <ns3/cv2x_phy-tx-stats-calculator.h>
#include <ns3/cv2x_phy-rx-stats-calculator.h>
#include <ns3/cv2x_epc-helper.h>
#include <iostream>
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/cv2x_lte-spectrum-value-helper.h>
#include <ns3/cv2x_epc-x2.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>
#include <cfloat>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteHelper");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteHelper);

cv2x_LteHelper::cv2x_LteHelper (void)
  : m_fadingStreamsAssigned (false),
    m_imsiCounter (0),
    m_cellIdCounter {1},
    m_EnbEnablePhyLayer (true)
{
  NS_LOG_FUNCTION (this);
  m_enbNetDeviceFactory.SetTypeId (cv2x_LteEnbNetDevice::GetTypeId ());
  m_enbAntennaModelFactory.SetTypeId (IsotropicAntennaModel::GetTypeId ());
  m_ueNetDeviceFactory.SetTypeId (cv2x_LteUeNetDevice::GetTypeId ());
  m_ueAntennaModelFactory.SetTypeId (IsotropicAntennaModel::GetTypeId ());
  m_channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());
}

void 
cv2x_LteHelper::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  ChannelModelInitialization ();
  m_phyStats = CreateObject<cv2x_PhyStatsCalculator> ();
  m_phyTxStats = CreateObject<cv2x_PhyTxStatsCalculator> ();
  m_phyRxStats = CreateObject<cv2x_PhyRxStatsCalculator> ();
  m_macStats = CreateObject<cv2x_MacStatsCalculator> ();
  Object::DoInitialize ();

}

cv2x_LteHelper::~cv2x_LteHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId cv2x_LteHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_LteHelper")
    .SetParent<Object> ()
    .AddConstructor<cv2x_LteHelper> ()
    .AddAttribute ("Scheduler",
                   "The type of scheduler to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::cv2x_FfMacScheduler.",
                   StringValue ("ns3::cv2x_PfFfMacScheduler"),
                   MakeStringAccessor (&cv2x_LteHelper::SetSchedulerType,
                                       &cv2x_LteHelper::GetSchedulerType),
                   MakeStringChecker ())
    .AddAttribute ("UlScheduler",    
                   "The type of UL scheduler to be used for UEs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::cv2x_FfMacScheduler.",
                   StringValue ("ns3::cv2x_RrFfMacScheduler"),
                   MakeStringAccessor (&cv2x_LteHelper::SetUlSchedulerType,
                                       &cv2x_LteHelper::GetUlSchedulerType),
                   MakeStringChecker ())
    .AddAttribute ("FfrAlgorithm",
                   "The type of FFR algorithm to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::cv2x_LteFfrAlgorithm.",
                   StringValue ("ns3::cv2x_LteFrNoOpAlgorithm"),
                   MakeStringAccessor (&cv2x_LteHelper::SetFfrAlgorithmType,
                                       &cv2x_LteHelper::GetFfrAlgorithmType),
                   MakeStringChecker ())
    .AddAttribute ("HandoverAlgorithm",
                   "The type of handover algorithm to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::cv2x_LteHandoverAlgorithm.",
                   StringValue ("ns3::cv2x_NoOpHandoverAlgorithm"),
                   MakeStringAccessor (&cv2x_LteHelper::SetHandoverAlgorithmType,
                                       &cv2x_LteHelper::GetHandoverAlgorithmType),
                   MakeStringChecker ())
    .AddAttribute ("PathlossModel",
                   "The type of pathloss model to be used. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::PropagationLossModel.",
                   TypeIdValue (FriisPropagationLossModel::GetTypeId ()),
                   MakeTypeIdAccessor (&cv2x_LteHelper::SetPathlossModelType),
                   MakeTypeIdChecker ())
    .AddAttribute ("FadingModel",
                   "The type of fading model to be used."
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::SpectrumPropagationLossModel."
                   "If the type is set to an empty string, no fading model is used.",
                   StringValue (""),
                   MakeStringAccessor (&cv2x_LteHelper::SetFadingModel),
                   MakeStringChecker ())
    .AddAttribute ("UseIdealRrc",
                   "If true, LteRrcProtocolIdeal will be used for RRC signaling. "
                   "If false, LteRrcProtocolReal will be used.",
                   BooleanValue (true), 
                   MakeBooleanAccessor (&cv2x_LteHelper::m_useIdealRrc),
                   MakeBooleanChecker ())
    .AddAttribute ("AnrEnabled",
                   "Activate or deactivate Automatic Neighbour Relation function",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_LteHelper::m_isAnrEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("UsePdschForCqiGeneration",
                   "If true, DL-CQI will be calculated from PDCCH as signal and PDSCH as interference "
                   "If false, DL-CQI will be calculated from PDCCH as signal and PDCCH as interference  ",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_LteHelper::m_usePdschForCqiGeneration),
                   MakeBooleanChecker ())
    .AddAttribute ("UseSidelink",
                   "If true, UEs will be able to receive sidelink communication. "
                   "If false, sidelink communication will not be possible.",
                   BooleanValue (false), 
                   MakeBooleanAccessor (&cv2x_LteHelper::m_useSidelink),
                   MakeBooleanChecker ())
    .AddAttribute ("UseDiscovery",
                   "If true, UEs will be able to do discovery. "
                   "If false, discovery will not be possible.",
                   BooleanValue (false), 
                   MakeBooleanAccessor (&cv2x_LteHelper::m_useDiscovery),
                   MakeBooleanChecker ())
    .AddAttribute ("EnbEnablePhyLayer",
                   "If true, normal eNB operation (default). "
                   "If false, eNB physical layer disable.",
                   BooleanValue (true), 
                   MakeBooleanAccessor (&cv2x_LteHelper::m_EnbEnablePhyLayer),
                   MakeBooleanChecker ())
    .AddAttribute ("UseSameUlDlPropagationCondition",
                   "If true, same conditions for both UL and DL"
                   "If false, different instances of the pathloss model",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_LteHelper::m_sameUlDlPropagationCondition),
                   MakeBooleanChecker ())
    .AddAttribute ("EnbComponentCarrierManager",
                   "The type of Component Carrier Manager to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting ns3::cv2x_LteEnbComponentCarrierManager.",
                   StringValue ("ns3::cv2x_NoOpComponentCarrierManager"),
                   MakeStringAccessor (&cv2x_LteHelper::SetEnbComponentCarrierManagerType,
                                       &cv2x_LteHelper::GetEnbComponentCarrierManagerType),
                   MakeStringChecker ())
    .AddAttribute ("UeComponentCarrierManager",
                   "The type of Component Carrier Manager to be used for UEs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting ns3::cv2x_LteUeComponentCarrierManager.",
                   StringValue ("ns3::cv2x_SimpleUeComponentCarrierManager"),
                   MakeStringAccessor (&cv2x_LteHelper::SetUeComponentCarrierManagerType,
                                       &cv2x_LteHelper::GetUeComponentCarrierManagerType),
                   MakeStringChecker ())
    .AddAttribute ("UseCa",
                   "If true, Carrier Aggregation feature is enabled and a valid Component Carrier Map is expected."
                   "If false, single carrier simulation.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_LteHelper::m_useCa),
                   MakeBooleanChecker ())
    .AddAttribute ("NumberOfComponentCarriers",
                   "Set the number of Component carrier to use "
                   "If it is more than one and m_useCa is false, it will raise an error ",
                   UintegerValue (1),
                   MakeUintegerAccessor (&cv2x_LteHelper::m_noOfCcs),
                   MakeUintegerChecker<uint16_t> (MIN_NO_CC, MAX_NO_CC))
  ;
  return tid;
}

void
cv2x_LteHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_downlinkChannel = 0;
  m_uplinkChannel = 0;
  m_componentCarrierPhyParams.clear();
  Object::DoDispose ();
}

void
cv2x_LteHelper::EnableNewEnbPhy ()
{
  NS_LOG_FUNCTION (this);
  m_EnbEnablePhyLayer = true;
}

void
cv2x_LteHelper::DisableNewEnbPhy ()
{
  NS_LOG_FUNCTION (this);
  m_EnbEnablePhyLayer = false;
}

Ptr<SpectrumChannel>
cv2x_LteHelper::GetUplinkSpectrumChannel (void) const
{
  return m_uplinkChannel;
}

Ptr<SpectrumChannel>
cv2x_LteHelper::GetDownlinkSpectrumChannel (void) const
{
  return m_downlinkChannel;
}

void
cv2x_LteHelper::ChannelModelInitialization (void)
{
  // Channel Object (i.e. Ptr<SpectrumChannel>) are within a vector
  // PathLossModel Objects are vectors --> in InstallSingleEnb we will set the frequency
  NS_LOG_FUNCTION (this << m_noOfCcs);

  m_downlinkChannel = m_channelFactory.Create<SpectrumChannel> ();
  m_uplinkChannel = m_channelFactory.Create<SpectrumChannel> ();

  m_downlinkPathlossModel = m_pathlossModelFactory.Create ();
  Ptr<SpectrumPropagationLossModel> dlSplm = m_downlinkPathlossModel->GetObject<SpectrumPropagationLossModel> ();
  if (dlSplm != 0)
    {
      NS_LOG_LOGIC (this << " using a SpectrumPropagationLossModel in DL");
      m_downlinkChannel->AddSpectrumPropagationLossModel (dlSplm);
    }
  else
    {
      NS_LOG_LOGIC (this << " using a PropagationLossModel in DL");
      Ptr<PropagationLossModel> dlPlm = m_downlinkPathlossModel->GetObject<PropagationLossModel> ();
      NS_ASSERT_MSG (dlPlm != 0, " " << m_downlinkPathlossModel << " is neither PropagationLossModel nor SpectrumPropagationLossModel");
      m_downlinkChannel->AddPropagationLossModel (dlPlm);
    }

  // Nist
  if (m_sameUlDlPropagationCondition)
    {
      m_uplinkPathlossModel = m_downlinkPathlossModel;
    }
  else
    {
      m_uplinkPathlossModel = m_pathlossModelFactory.Create ();
    }
  //

  Ptr<SpectrumPropagationLossModel> ulSplm = m_uplinkPathlossModel->GetObject<SpectrumPropagationLossModel> ();
  if (ulSplm != 0)
    {
      NS_LOG_LOGIC (this << " using a SpectrumPropagationLossModel in UL");
      m_uplinkChannel->AddSpectrumPropagationLossModel (ulSplm);
    }
  else
    {
      NS_LOG_LOGIC (this << " using a PropagationLossModel in UL");
      Ptr<PropagationLossModel> ulPlm = m_uplinkPathlossModel->GetObject<PropagationLossModel> ();
      NS_ASSERT_MSG (ulPlm != 0, " " << m_uplinkPathlossModel << " is neither PropagationLossModel nor SpectrumPropagationLossModel");
      m_uplinkChannel->AddPropagationLossModel (ulPlm);
    }
  if (!m_fadingModelType.empty ())
    {
      m_fadingModule = m_fadingModelFactory.Create<SpectrumPropagationLossModel> ();
      m_fadingModule->Initialize ();
      m_downlinkChannel->AddSpectrumPropagationLossModel (m_fadingModule);
      m_uplinkChannel->AddSpectrumPropagationLossModel (m_fadingModule);
    }
}

void 
cv2x_LteHelper::SetEpcHelper (Ptr<cv2x_EpcHelper> h)
{
  NS_LOG_FUNCTION (this << h);
  m_epcHelper = h;
}

void 
cv2x_LteHelper::SetSchedulerType (std::string type) 
{
  NS_LOG_FUNCTION (this << type);

  if ( (type =="ns3::cv2x_PriorityFfMacScheduler") || ( type == "ns3::cv2x_PfFfMacScheduler") || ( type == "ns3::MtFfMacScheduler" )  || ( type == "ns3::cv2x_RrSlFfMacScheduler") || (type == "ns3::cv2x_Lte3GPPcalMacScheduler")) 
    {
      //this DL scheduler has a correspandant UL scheduler 
      SetUlSchedulerType (type);   
    }
  else if ( (type =="ns3::cv2x_FdMtFfMacScheduler") || (type == "ns3::cv2x_TdMtFfMacScheduler") || (type == "ns3::cv2x_FdBetFfMacScheduler")  || (type == "ns3::cv2x_TdBetFfMacScheduler")  || (type == "ns3::cv2x_TtaFfMacScheduler") || (type  == "ns3::cv2x_TdTbfqFfMacScheduler")  || (type == "ns3::cv2x_FdTbfqFfMacScheduler")   || (type == "ns3::cv2x_PssFfMacScheduler")  || (type == "ns3::cv2x_RrFfMacScheduler")) 
    {
      // these DL schedulers don't have a corresponding UE scheduler 
      SetUlSchedulerType ("ns3::cv2x_RrFfMacScheduler");
    }
  else
    {
      NS_FATAL_ERROR ("unknown Scheduler type " << type);
    }

  m_schedulerFactory = ObjectFactory ();
  m_schedulerFactory.SetTypeId (type);
}

void 
cv2x_LteHelper::SetUlSchedulerType (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  if ( (type == "ns3::cv2x_RrFfMacScheduler") || (type == "ns3::cv2x_PriorityFfMacScheduler") || (type == "ns3::cv2x_PfFfMacScheduler") || (type == "ns3::MtFfMacScheduler") || ( type == "ns3::cv2x_RrSlFfMacScheduler") || (type == "ns3::cv2x_Lte3GPPcalMacScheduler"))
   {
     m_UlschedulerFactory = ObjectFactory ();
     m_UlschedulerFactory.SetTypeId (type);
   }
  else
     NS_FATAL_ERROR ("unknown UE Scheduler type " << type << " ; The only schedulers implemented are : RR, MT, PF, Priority schedulers, FDM: All in from 3GPPcal.");
}

std::string
cv2x_LteHelper::GetSchedulerType () const
{
  return m_schedulerFactory.GetTypeId ().GetName ();
} 

std::string
cv2x_LteHelper::GetUlSchedulerType () const
{
  return m_UlschedulerFactory.GetTypeId ().GetName ();
} 

void 
cv2x_LteHelper::SetSchedulerAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_schedulerFactory.Set (n, v);
}

std::string
cv2x_LteHelper::GetFfrAlgorithmType () const
{
  return m_ffrAlgorithmFactory.GetTypeId ().GetName ();
}

void
cv2x_LteHelper::SetFfrAlgorithmType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_ffrAlgorithmFactory = ObjectFactory ();
  m_ffrAlgorithmFactory.SetTypeId (type);
}

void
cv2x_LteHelper::SetFfrAlgorithmAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_ffrAlgorithmFactory.Set (n, v);
}

std::string
cv2x_LteHelper::GetHandoverAlgorithmType () const
{
  return m_handoverAlgorithmFactory.GetTypeId ().GetName ();
}

void
cv2x_LteHelper::SetHandoverAlgorithmType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_handoverAlgorithmFactory = ObjectFactory ();
  m_handoverAlgorithmFactory.SetTypeId (type);
}

void
cv2x_LteHelper::SetHandoverAlgorithmAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_handoverAlgorithmFactory.Set (n, v);
}


std::string
cv2x_LteHelper::GetEnbComponentCarrierManagerType () const
{
  return m_enbComponentCarrierManagerFactory.GetTypeId ().GetName ();
}

void
cv2x_LteHelper::SetEnbComponentCarrierManagerType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_enbComponentCarrierManagerFactory = ObjectFactory ();
  m_enbComponentCarrierManagerFactory.SetTypeId (type);
}

void
cv2x_LteHelper::SetEnbComponentCarrierManagerAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_enbComponentCarrierManagerFactory.Set (n, v);
}

std::string
cv2x_LteHelper::GetUeComponentCarrierManagerType () const
{
  return m_ueComponentCarrierManagerFactory.GetTypeId ().GetName ();
}

void
cv2x_LteHelper::SetUeComponentCarrierManagerType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_ueComponentCarrierManagerFactory = ObjectFactory ();
  m_ueComponentCarrierManagerFactory.SetTypeId (type);
}

void
cv2x_LteHelper::SetUeComponentCarrierManagerAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_ueComponentCarrierManagerFactory.Set (n, v);
}

void
cv2x_LteHelper::SetPathlossModelType (TypeId type)
{
  NS_LOG_FUNCTION (this << type);
  m_pathlossModelFactory = ObjectFactory ();
  m_pathlossModelFactory.SetTypeId (type);
}

void 
cv2x_LteHelper::SetPathlossModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_pathlossModelFactory.Set (n, v);
}

void
cv2x_LteHelper::SetEnbDeviceAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_enbNetDeviceFactory.Set (n, v);
}


void 
cv2x_LteHelper::SetEnbAntennaModelType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_enbAntennaModelFactory.SetTypeId (type);
}

void 
cv2x_LteHelper::SetEnbAntennaModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_enbAntennaModelFactory.Set (n, v);
}

void
cv2x_LteHelper::SetUeDeviceAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueNetDeviceFactory.Set (n, v);
}

void 
cv2x_LteHelper::SetUeAntennaModelType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_ueAntennaModelFactory.SetTypeId (type);
}

void 
cv2x_LteHelper::SetUeAntennaModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueAntennaModelFactory.Set (n, v);
}

void 
cv2x_LteHelper::SetFadingModel (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  m_fadingModelType = type;
  if (!type.empty ())
    {
      m_fadingModelFactory = ObjectFactory ();
      m_fadingModelFactory.SetTypeId (type);
    }
}

void 
cv2x_LteHelper::SetFadingModelAttribute (std::string n, const AttributeValue &v)
{
  m_fadingModelFactory.Set (n, v);
}

void 
cv2x_LteHelper::SetSpectrumChannelType (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  m_channelFactory.SetTypeId (type);
}

void 
cv2x_LteHelper::SetSpectrumChannelAttribute (std::string n, const AttributeValue &v)
{
  m_channelFactory.Set (n, v);
}

NetDeviceContainer
cv2x_LteHelper::InstallEnbDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  Initialize ();  // will run DoInitialize () if necessary
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleEnbDevice (node);
      devices.Add (device);
    }
  return devices;
}

NetDeviceContainer
cv2x_LteHelper::InstallUeDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleUeDevice (node);
      devices.Add (device);
    }
  return devices;
}


Ptr<NetDevice>
cv2x_LteHelper::InstallSingleEnbDevice (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this << n);
  uint16_t cellId = m_cellIdCounter; // \todo Remove, eNB has no cell ID

  Ptr<cv2x_LteEnbNetDevice> dev = m_enbNetDeviceFactory.Create<cv2x_LteEnbNetDevice> ();
  Ptr<cv2x_LteHandoverAlgorithm> handoverAlgorithm = m_handoverAlgorithmFactory.Create<cv2x_LteHandoverAlgorithm> ();

  NS_ABORT_MSG_IF (m_componentCarrierPhyParams.size() != 0, "CC map is not clean");
  DoComponentCarrierConfigure (dev->GetUlEarfcn (), dev->GetDlEarfcn (),
                               dev->GetUlBandwidth (), dev->GetDlBandwidth ());
  NS_ABORT_MSG_IF (m_componentCarrierPhyParams.size() != m_noOfCcs,
                   "CC map size (" << m_componentCarrierPhyParams.size () <<
                   ") must be equal to number of carriers (" <<
                   m_noOfCcs << ")");

  // create component carrier map for this eNb device
  std::map<uint8_t,Ptr<cv2x_ComponentCarrierEnb> > ccMap;
  for (std::map<uint8_t, cv2x_ComponentCarrier >::iterator it = m_componentCarrierPhyParams.begin ();
       it != m_componentCarrierPhyParams.end ();
       ++it)
    {
      Ptr <cv2x_ComponentCarrierEnb> cc = CreateObject<cv2x_ComponentCarrierEnb> ();
      cc->SetUlBandwidth (it->second.GetUlBandwidth ());
      cc->SetDlBandwidth (it->second.GetDlBandwidth ());
      cc->SetDlEarfcn (it->second.GetDlEarfcn ());
      cc->SetUlEarfcn (it->second.GetUlEarfcn ());
      cc->SetAsPrimary (it->second.IsPrimary ());
      NS_ABORT_MSG_IF (m_cellIdCounter == 65535, "max num cells exceeded");
      cc->SetCellId (m_cellIdCounter++);
      ccMap [it->first] =  cc;
    }
  // CC map is not needed anymore
  m_componentCarrierPhyParams.clear ();

  NS_ABORT_MSG_IF (m_useCa && ccMap.size()<2, "You have to either specify carriers or disable carrier aggregation");
  NS_ASSERT (ccMap.size () == m_noOfCcs);

  for (std::map<uint8_t,Ptr<cv2x_ComponentCarrierEnb> >::iterator it = ccMap.begin (); it != ccMap.end (); ++it)
    {
      NS_LOG_DEBUG (this << "component carrier map size " << (uint16_t) ccMap.size ());
      Ptr<cv2x_LteSpectrumPhy> dlPhy = CreateObject<cv2x_LteSpectrumPhy> ();
      Ptr<cv2x_LteSpectrumPhy> ulPhy = CreateObject<cv2x_LteSpectrumPhy> ();
      Ptr<cv2x_LteEnbPhy> phy = CreateObject<cv2x_LteEnbPhy> (dlPhy, ulPhy, m_EnbEnablePhyLayer);

      Ptr<cv2x_LteHarqPhy> harq = Create<cv2x_LteHarqPhy> ();
      dlPhy->SetHarqPhyModule (harq);
      ulPhy->SetHarqPhyModule (harq);
      phy->SetHarqPhyModule (harq);

      Ptr<cv2x_LteChunkProcessor> pCtrl = Create<cv2x_LteChunkProcessor> ();
      pCtrl->AddCallback (MakeCallback (&cv2x_LteEnbPhy::GenerateCtrlCqiReport, phy));
      ulPhy->AddCtrlSinrChunkProcessor (pCtrl);   // for evaluating SRS UL-CQI

      Ptr<cv2x_LteChunkProcessor> pData = Create<cv2x_LteChunkProcessor> ();
      pData->AddCallback (MakeCallback (&cv2x_LteEnbPhy::GenerateDataCqiReport, phy));
      pData->AddCallback (MakeCallback (&cv2x_LteSpectrumPhy::UpdateSinrPerceived, ulPhy));
      ulPhy->AddDataSinrChunkProcessor (pData);   // for evaluating PUSCH UL-CQI

      Ptr<cv2x_LteChunkProcessor> pInterf = Create<cv2x_LteChunkProcessor> ();
      pInterf->AddCallback (MakeCallback (&cv2x_LteEnbPhy::ReportInterference, phy));
      ulPhy->AddInterferenceDataChunkProcessor (pInterf);   // for interference power tracing

      dlPhy->SetChannel (m_downlinkChannel);
      ulPhy->SetChannel (m_uplinkChannel);

      Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
      NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling cv2x_LteHelper::InstallEnbDevice ()");
      dlPhy->SetMobility (mm);
      ulPhy->SetMobility (mm);

      Ptr<AntennaModel> antenna = (m_enbAntennaModelFactory.Create ())->GetObject<AntennaModel> ();
      NS_ASSERT_MSG (antenna, "error in creating the AntennaModel object");
      dlPhy->SetAntenna (antenna);
      ulPhy->SetAntenna (antenna);

      Ptr<cv2x_LteEnbMac> mac = CreateObject<cv2x_LteEnbMac> ();
      Ptr<cv2x_FfMacScheduler> sched = m_schedulerFactory.Create<cv2x_FfMacScheduler> ();
      Ptr<cv2x_LteFfrAlgorithm> ffrAlgorithm = m_ffrAlgorithmFactory.Create<cv2x_LteFfrAlgorithm> ();
      it->second->SetMac (mac);
      it->second->SetFfMacScheduler (sched);
      it->second->SetFfrAlgorithm (ffrAlgorithm);
  
      it->second->SetPhy (phy);

    }

  Ptr<cv2x_LteEnbRrc> rrc = CreateObject<cv2x_LteEnbRrc> ();
  Ptr<cv2x_LteEnbComponentCarrierManager> ccmEnbManager = m_enbComponentCarrierManagerFactory.Create<cv2x_LteEnbComponentCarrierManager> ();
  rrc->ConfigureCarriers (ccMap);
  
  //cv2x_ComponentCarrierManager SAP
  rrc->SetLteCcmRrcSapProvider (ccmEnbManager->GetLteCcmRrcSapProvider ());
  ccmEnbManager->SetLteCcmRrcSapUser (rrc->GetLteCcmRrcSapUser ());
  ccmEnbManager->SetNumberOfComponentCarriers (m_noOfCcs);
  ccmEnbManager->SetRrc(rrc);

  if (m_useIdealRrc)
    {
      Ptr<cv2x_LteEnbRrcProtocolIdeal> rrcProtocol = CreateObject<cv2x_LteEnbRrcProtocolIdeal> ();
      rrcProtocol->SetLteEnbRrcSapProvider (rrc->GetLteEnbRrcSapProvider ());
      rrc->SetLteEnbRrcSapUser (rrcProtocol->GetLteEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetCellId (cellId);
    }
  else
    {
      Ptr<cv2x_LteEnbRrcProtocolReal> rrcProtocol = CreateObject<cv2x_LteEnbRrcProtocolReal> ();
      rrcProtocol->SetLteEnbRrcSapProvider (rrc->GetLteEnbRrcSapProvider ());
      rrc->SetLteEnbRrcSapUser (rrcProtocol->GetLteEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetCellId (cellId);
    }

  if (m_epcHelper != 0)
    {
      EnumValue epsBearerToRlcMapping;
      rrc->GetAttribute ("cv2x_EpsBearerToRlcMapping", epsBearerToRlcMapping);
      // it does not make sense to use RLC/SM when also using the EPC
      if (epsBearerToRlcMapping.Get () == cv2x_LteEnbRrc::RLC_SM_ALWAYS)
        {
          rrc->SetAttribute ("cv2x_EpsBearerToRlcMapping", EnumValue (cv2x_LteEnbRrc::RLC_UM_ALWAYS));
        }
    }

  rrc->SetLteHandoverManagementSapProvider (handoverAlgorithm->GetLteHandoverManagementSapProvider ());
  handoverAlgorithm->SetLteHandoverManagementSapUser (rrc->GetLteHandoverManagementSapUser ());
 
  // This RRC attribute is used to connect each new RLC instance with the MAC layer
  // (for function such as TransmitPdu, ReportBufferStatusReport).
  // Since in this new architecture, the component carrier manager acts a proxy, it
  // will have its own cv2x_LteMacSapProvider interface, RLC will see it as through original MAC
  // interface cv2x_LteMacSapProvider, but the function call will go now through cv2x_LteEnbComponentCarrierManager
  // instance that needs to implement functions of this interface, and its task will be to
  // forward these calls to the specific MAC of some of the instances of component carriers. This
  // decision will depend on the specific implementation of the component carrier manager.
  rrc->SetLteMacSapProvider (ccmEnbManager->GetLteMacSapProvider ());

  bool ccmTest;
  for (std::map<uint8_t,Ptr<cv2x_ComponentCarrierEnb> >::iterator it = ccMap.begin (); it != ccMap.end (); ++it)
    {
      it->second->GetPhy ()->SetLteEnbCphySapUser (rrc->GetLteEnbCphySapUser (it->first));
      rrc->SetLteEnbCphySapProvider (it->second->GetPhy ()->GetLteEnbCphySapProvider (), it->first);

      rrc->SetLteEnbCmacSapProvider (it->second->GetMac ()->GetLteEnbCmacSapProvider (),it->first );
      it->second->GetMac ()->SetLteEnbCmacSapUser (rrc->GetLteEnbCmacSapUser (it->first));

      it->second->GetPhy ()->SetComponentCarrierId (it->first);
      it->second->GetMac ()->SetComponentCarrierId (it->first);
      //FFR SAP
      it->second->GetFfMacScheduler ()->SetLteFfrSapProvider (it->second->GetFfrAlgorithm ()->GetLteFfrSapProvider ());
      it->second->GetFfrAlgorithm ()->SetLteFfrSapUser (it->second->GetFfMacScheduler ()->GetLteFfrSapUser ());
      rrc->SetLteFfrRrcSapProvider (it->second->GetFfrAlgorithm ()->GetLteFfrRrcSapProvider (), it->first);
      it->second->GetFfrAlgorithm ()->SetLteFfrRrcSapUser (rrc->GetLteFfrRrcSapUser (it->first));
      //FFR SAP END

      // PHY <--> MAC SAP
      it->second->GetPhy ()->SetLteEnbPhySapUser (it->second->GetMac ()->GetLteEnbPhySapUser ());
      it->second->GetMac ()->SetLteEnbPhySapProvider (it->second->GetPhy ()->GetLteEnbPhySapProvider ());
      // PHY <--> MAC SAP END

      //Scheduler SAP
      it->second->GetMac ()->SetFfMacSchedSapProvider (it->second->GetFfMacScheduler ()->GetFfMacSchedSapProvider ());
      it->second->GetMac ()->SetFfMacCschedSapProvider (it->second->GetFfMacScheduler ()->GetFfMacCschedSapProvider ());

      it->second->GetFfMacScheduler ()->SetFfMacSchedSapUser (it->second->GetMac ()->GetFfMacSchedSapUser ());
      it->second->GetFfMacScheduler ()->SetFfMacCschedSapUser (it->second->GetMac ()->GetFfMacCschedSapUser ());
      // Scheduler SAP END

      it->second->GetMac ()->SetLteCcmMacSapUser (ccmEnbManager->GetLteCcmMacSapUser ());
      ccmEnbManager->SetCcmMacSapProviders (it->first, it->second->GetMac ()->GetLteCcmMacSapProvider ());

      // insert the pointer to the cv2x_LteMacSapProvider interface of the MAC layer of the specific component carrier
      ccmTest = ccmEnbManager->SetMacSapProvider (it->first, it->second->GetMac ()->GetLteMacSapProvider());

      if (ccmTest == false)
        {
          NS_FATAL_ERROR ("Error in SetComponentCarrierMacSapProviders");
        }
    }



  dev->SetNode (n);
  dev->SetAttribute ("CellId", UintegerValue (cellId));
  dev->SetAttribute ("cv2x_LteEnbComponentCarrierManager", PointerValue (ccmEnbManager));
  dev->SetCcMap (ccMap);
  std::map<uint8_t,Ptr<cv2x_ComponentCarrierEnb> >::iterator it = ccMap.begin ();
  dev->SetAttribute ("cv2x_LteEnbRrc", PointerValue (rrc)); 
  dev->SetAttribute ("cv2x_LteHandoverAlgorithm", PointerValue (handoverAlgorithm));
  dev->SetAttribute ("cv2x_LteFfrAlgorithm", PointerValue (it->second->GetFfrAlgorithm ()));

  if (m_isAnrEnabled)
    {
      Ptr<cv2x_LteAnr> anr = CreateObject<cv2x_LteAnr> (cellId);
      rrc->SetLteAnrSapProvider (anr->GetLteAnrSapProvider ());
      anr->SetLteAnrSapUser (rrc->GetLteAnrSapUser ());
      dev->SetAttribute ("cv2x_LteAnr", PointerValue (anr));
    }

  for (it = ccMap.begin (); it != ccMap.end (); ++it)
    {
      Ptr<cv2x_LteEnbPhy> ccPhy = it->second->GetPhy ();
      ccPhy->SetDevice (dev);
      ccPhy->GetUlSpectrumPhy ()->SetDevice (dev);
      ccPhy->GetDlSpectrumPhy ()->SetDevice (dev);
      ccPhy->GetUlSpectrumPhy ()->SetLtePhyRxDataEndOkCallback (MakeCallback (&cv2x_LteEnbPhy::PhyPduReceived, ccPhy));
      ccPhy->GetUlSpectrumPhy ()->SetLtePhyRxCtrlEndOkCallback (MakeCallback (&cv2x_LteEnbPhy::ReceiveLteControlMessageList, ccPhy));
      ccPhy->GetUlSpectrumPhy ()->SetLtePhyUlHarqFeedbackCallback (MakeCallback (&cv2x_LteEnbPhy::ReceiveLteUlHarqFeedback, ccPhy));
      NS_LOG_LOGIC ("set the propagation model frequencies");
      double dlFreq = cv2x_LteSpectrumValueHelper::GetCarrierFrequency (it->second->m_dlEarfcn);
      NS_LOG_LOGIC ("DL freq: " << dlFreq);
      bool dlFreqOk = m_downlinkPathlossModel->SetAttributeFailSafe ("Frequency", DoubleValue (dlFreq));
      if (!dlFreqOk)
        {
          NS_LOG_WARN ("DL propagation model does not have a Frequency attribute");
        }

      double ulFreq = cv2x_LteSpectrumValueHelper::GetCarrierFrequency (it->second->m_ulEarfcn);

      NS_LOG_LOGIC ("UL freq: " << ulFreq);
      bool ulFreqOk = m_uplinkPathlossModel->SetAttributeFailSafe ("Frequency", DoubleValue (ulFreq));
      if (!ulFreqOk)
        {
          NS_LOG_WARN ("UL propagation model does not have a Frequency attribute");
        }
    }  //end for
  rrc->SetForwardUpCallback (MakeCallback (&cv2x_LteEnbNetDevice::Receive, dev));
  dev->Initialize ();
  n->AddDevice (dev);

  for (it = ccMap.begin (); it != ccMap.end (); ++it)
    {
      m_uplinkChannel->AddRx (it->second->GetPhy ()->GetUlSpectrumPhy ());
    }

  if (m_epcHelper != 0)
    {
      NS_LOG_INFO ("adding this eNB to the EPC");
      m_epcHelper->AddEnb (n, dev, dev->GetCellId ());
      Ptr<cv2x_EpcEnbApplication> enbApp = n->GetApplication (0)->GetObject<cv2x_EpcEnbApplication> ();
      NS_ASSERT_MSG (enbApp != 0, "cannot retrieve cv2x_EpcEnbApplication");

      // S1 SAPs
      rrc->SetS1SapProvider (enbApp->GetS1SapProvider ());
      enbApp->SetS1SapUser (rrc->GetS1SapUser ());

      // X2 SAPs
      Ptr<cv2x_EpcX2> x2 = n->GetObject<cv2x_EpcX2> ();
      x2->SetEpcX2SapUser (rrc->GetEpcX2SapUser ());
      rrc->SetEpcX2SapProvider (x2->GetEpcX2SapProvider ());
    }

  return dev;
}

Ptr<NetDevice>
cv2x_LteHelper::InstallSingleUeDevice (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this);

  Ptr<cv2x_LteUeNetDevice> dev = m_ueNetDeviceFactory.Create<cv2x_LteUeNetDevice> ();

  // Initialize the component carriers with default values in order to initialize MACs and PHYs
  // of each component carrier. These values must be updated once the UE is attached to the
  // eNB and receives RRC Connection Reconfiguration message. In case of primary carrier or
  // a single carrier, these values will be updated once the UE will receive SIB2 and MIB.
  NS_ABORT_MSG_IF (m_componentCarrierPhyParams.size() != 0, "CC map is not clean");
  DoComponentCarrierConfigure (dev->GetDlEarfcn () + 18000, dev->GetDlEarfcn (), 25, 25);
  NS_ABORT_MSG_IF (m_componentCarrierPhyParams.size() != m_noOfCcs,
                   "CC map size (" << m_componentCarrierPhyParams.size () <<
                   ") must be equal to number of carriers (" <<
                   m_noOfCcs << ")");

  std::map<uint8_t, Ptr<cv2x_ComponentCarrierUe> > ueCcMap;

  for (std::map< uint8_t, cv2x_ComponentCarrier >::iterator it = m_componentCarrierPhyParams.begin();
       it != m_componentCarrierPhyParams.end();
       ++it)
    {
      Ptr <cv2x_ComponentCarrierUe> cc = CreateObject<cv2x_ComponentCarrierUe> ();
      cc->SetUlBandwidth (it->second.GetUlBandwidth ());
      cc->SetDlBandwidth (it->second.GetDlBandwidth ());
      cc->SetDlEarfcn (it->second.GetDlEarfcn ());
      cc->SetUlEarfcn (it->second.GetUlEarfcn ());
      cc->SetAsPrimary (it->second.IsPrimary ());
      Ptr<cv2x_LteUeMac> mac = CreateObject<cv2x_LteUeMac> ();
      cc->SetMac (mac);
      // cc->GetPhy ()->Initialize (); // it is initialized within the cv2x_LteUeNetDevice::DoInitialize ()
      ueCcMap.insert (std::pair<uint8_t, Ptr<cv2x_ComponentCarrierUe> > (it->first, cc));
    }
  // CC map is not needed anymore
  m_componentCarrierPhyParams.clear ();

  Ptr<cv2x_LteSpectrumPhy> slPhy;
  for (std::map<uint8_t, Ptr<cv2x_ComponentCarrierUe> >::iterator it = ueCcMap.begin (); it != ueCcMap.end (); ++it)
    {
      Ptr<cv2x_LteSpectrumPhy> dlPhy = CreateObject<cv2x_LteSpectrumPhy> ();
      Ptr<cv2x_LteSpectrumPhy> ulPhy = CreateObject<cv2x_LteSpectrumPhy> ();
      
      if (m_useSidelink || m_useDiscovery) 
      {
        slPhy = CreateObject<cv2x_LteSpectrumPhy> ();
        slPhy->SetAttribute ("HalfDuplexPhy", PointerValue (ulPhy));
      }

      Ptr<cv2x_LteUePhy> phy = CreateObject<cv2x_LteUePhy> (dlPhy, ulPhy);
      if (m_useSidelink || m_useDiscovery) 
      {
        phy->SetSlSpectrumPhy (slPhy);
      }

      Ptr<cv2x_LteHarqPhy> harq = Create<cv2x_LteHarqPhy> ();
      dlPhy->SetHarqPhyModule (harq);
      ulPhy->SetHarqPhyModule (harq);
      if (m_useSidelink || m_useDiscovery) 
      {
        slPhy->SetHarqPhyModule (harq);
      }
      phy->SetHarqPhyModule (harq);

      Ptr<cv2x_LteChunkProcessor> pRs = Create<cv2x_LteChunkProcessor> ();
      pRs->AddCallback (MakeCallback (&cv2x_LteUePhy::ReportRsReceivedPower, phy));
      dlPhy->AddRsPowerChunkProcessor (pRs);

      Ptr<cv2x_LteChunkProcessor> pInterf = Create<cv2x_LteChunkProcessor> ();
      pInterf->AddCallback (MakeCallback (&cv2x_LteUePhy::ReportInterference, phy));
      dlPhy->AddInterferenceCtrlChunkProcessor (pInterf);   // for RSRQ evaluation of UE Measurements

      Ptr<cv2x_LteChunkProcessor> pCtrl = Create<cv2x_LteChunkProcessor> ();
      pCtrl->AddCallback (MakeCallback (&cv2x_LteSpectrumPhy::UpdateSinrPerceived, dlPhy));
      dlPhy->AddCtrlSinrChunkProcessor (pCtrl);

      Ptr<cv2x_LteChunkProcessor> pData = Create<cv2x_LteChunkProcessor> ();
      pData->AddCallback (MakeCallback (&cv2x_LteSpectrumPhy::UpdateSinrPerceived, dlPhy));
      dlPhy->AddDataSinrChunkProcessor (pData);

      if (m_useSidelink || m_useDiscovery) 
      {
        Ptr<cv2x_LteSlChunkProcessor> pSlSinr = Create<cv2x_LteSlChunkProcessor> ();
        pSlSinr->AddCallback (MakeCallback (&cv2x_LteSpectrumPhy::UpdateSlSinrPerceived, slPhy));
        slPhy->AddSlSinrChunkProcessor (pSlSinr);
        
        Ptr<cv2x_LteSlChunkProcessor> pSlSignal = Create<cv2x_LteSlChunkProcessor> ();
        pSlSignal->AddCallback (MakeCallback (&cv2x_LteSpectrumPhy::UpdateSlSigPerceived, slPhy));
        slPhy->AddSlSignalChunkProcessor (pSlSignal);

        Ptr<cv2x_LteSlChunkProcessor> pSlInterference = Create<cv2x_LteSlChunkProcessor> ();
        pSlInterference->AddCallback (MakeCallback (&cv2x_LteSpectrumPhy::UpdateSlIntPerceived, slPhy));
        slPhy->AddSlInterferenceChunkProcessor (pSlInterference);
        /*
        Ptr<cv2x_LteChunkProcessor> pData = Create<cv2x_LteChunkProcessor> ();
        pData->AddCallback (MakeCallback (&cv2x_LteSpectrumPhy::UpdateSinrPerceived, slPhy));
        slPhy->AddDataSinrChunkProcessor (pData);
        */
      }

      if (m_usePdschForCqiGeneration)
        {
          // CQI calculation based on PDCCH for signal and PDSCH for interference
          pCtrl->AddCallback (MakeCallback (&cv2x_LteUePhy::GenerateMixedCqiReport, phy));
          Ptr<cv2x_LteChunkProcessor> pDataInterf = Create<cv2x_LteChunkProcessor> ();
          pDataInterf->AddCallback (MakeCallback (&cv2x_LteUePhy::ReportDataInterference, phy));
          dlPhy->AddInterferenceDataChunkProcessor (pDataInterf);
        }
      else
        {
          // CQI calculation based on PDCCH for both signal and interference
          pCtrl->AddCallback (MakeCallback (&cv2x_LteUePhy::GenerateCtrlCqiReport, phy));
        }

      dlPhy->SetChannel (m_downlinkChannel);
      ulPhy->SetChannel (m_uplinkChannel);
      if (m_useSidelink || m_useDiscovery) 
      {
        slPhy->SetChannel (m_uplinkChannel); //want the UE to receive sidelink messages on the uplink
      }

      Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
      NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling cv2x_LteHelper::InstallUeDevice ()");
      dlPhy->SetMobility (mm);
      ulPhy->SetMobility (mm);
      if (m_useSidelink || m_useDiscovery)
      {
        slPhy->SetMobility (mm);
      }

      Ptr<AntennaModel> antenna = (m_ueAntennaModelFactory.Create ())->GetObject<AntennaModel> ();
      NS_ASSERT_MSG (antenna, "error in creating the AntennaModel object");
      dlPhy->SetAntenna (antenna);
      ulPhy->SetAntenna (antenna);
      if (m_useSidelink || m_useDiscovery)
      {
        slPhy->SetAntenna (antenna);
      }

      it->second->SetPhy(phy);
    }
  Ptr<cv2x_LteUeComponentCarrierManager> ccmUe = m_ueComponentCarrierManagerFactory.Create<cv2x_LteUeComponentCarrierManager> ();
  ccmUe->SetNumberOfComponentCarriers (m_noOfCcs);

  // create a UE mac with an attribute to indicate the UE scheduler
  Ptr<cv2x_LteUeMac> mac = CreateObjectWithAttributes<cv2x_LteUeMac> (
                      "UlScheduler",
                      StringValue (cv2x_LteHelper::GetUlSchedulerType ()));
  Ptr<cv2x_LteUeRrc> rrc = CreateObject<cv2x_LteUeRrc> ();
  rrc->m_numberOfComponentCarriers = m_noOfCcs;
  // run intializeSap to create the proper number of sap provider/users
  rrc->InitializeSap();
  rrc->SetLteMacSapProvider (ccmUe->GetLteMacSapProvider ());
  // setting cv2x_ComponentCarrierManager SAP
  rrc->SetLteCcmRrcSapProvider (ccmUe->GetLteCcmRrcSapProvider ());
  ccmUe->SetLteCcmRrcSapUser (rrc->GetLteCcmRrcSapUser ());

  if (m_useIdealRrc)
    {
      Ptr<cv2x_LteUeRrcProtocolIdeal> rrcProtocol = CreateObject<cv2x_LteUeRrcProtocolIdeal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetLteUeRrcSapProvider (rrc->GetLteUeRrcSapProvider ());
      rrc->SetLteUeRrcSapUser (rrcProtocol->GetLteUeRrcSapUser ());
    }
  else
    {
      Ptr<cv2x_LteUeRrcProtocolReal> rrcProtocol = CreateObject<cv2x_LteUeRrcProtocolReal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetLteUeRrcSapProvider (rrc->GetLteUeRrcSapProvider ());
      rrc->SetLteUeRrcSapUser (rrcProtocol->GetLteUeRrcSapUser ());
    }

  if (m_epcHelper != 0)
    {
      rrc->SetUseRlcSm (false);
    }
  Ptr<cv2x_EpcUeNas> nas = CreateObject<cv2x_EpcUeNas> ();
 
  nas->SetAsSapProvider (rrc->GetAsSapProvider ());
  rrc->SetAsSapUser (nas->GetAsSapUser ());

  for (std::map<uint8_t, Ptr<cv2x_ComponentCarrierUe> >::iterator it = ueCcMap.begin (); it != ueCcMap.end (); ++it)
    {
      rrc->SetLteUeCmacSapProvider (it->second->GetMac ()->GetLteUeCmacSapProvider (), it->first);
      it->second->GetMac ()->SetLteUeCmacSapUser (rrc->GetLteUeCmacSapUser (it->first));
      it->second->GetMac ()->SetComponentCarrierId (it->first);

      it->second->GetPhy ()->SetLteUeCphySapUser (rrc->GetLteUeCphySapUser (it->first));
      rrc->SetLteUeCphySapProvider (it->second->GetPhy ()->GetLteUeCphySapProvider (), it->first);

      it->second->GetMac ()->SetLteUeCphySapProvider (it->second->GetPhy ()->GetLteUeCphySapProvider ());

      it->second->GetPhy ()->SetComponentCarrierId (it->first);
      it->second->GetPhy ()->SetLteUePhySapUser (it->second->GetMac ()->GetLteUePhySapUser ());
      it->second->GetMac ()->SetLteUePhySapProvider (it->second->GetPhy ()->GetLteUePhySapProvider ());

      bool ccmTest = ccmUe->SetComponentCarrierMacSapProviders (it->first, it->second->GetMac ()->GetLteMacSapProvider());

      if (ccmTest == false)
        {
          NS_FATAL_ERROR ("Error in SetComponentCarrierMacSapProviders");
        }
    }

  NS_ABORT_MSG_IF (m_imsiCounter >= 0xFFFFFFFF, "max num UEs exceeded");
  uint64_t imsi = ++m_imsiCounter;

  //Initialize sidelink configuration
  Ptr<cv2x_LteUeRrcSl> ueSidelinkConfiguration = CreateObject<cv2x_LteUeRrcSl> ();
  ueSidelinkConfiguration->SetSourceL2Id ((uint32_t) (imsi & 0xFFFFFF)); //use lower 24 bits of IMSI as source
  rrc->SetAttribute ("SidelinkConfiguration", PointerValue (ueSidelinkConfiguration));

  dev->SetNode (n);
  dev->SetAttribute ("Imsi", UintegerValue (imsi));
  dev->SetCcMap (ueCcMap);
  dev->SetAttribute ("cv2x_LteUeRrc", PointerValue (rrc));
  dev->SetAttribute ("cv2x_EpcUeNas", PointerValue (nas));
  dev->SetAttribute ("cv2x_LteUeComponentCarrierManager", PointerValue (ccmUe));
  // \todo The UE identifier should be dynamically set by the EPC
  // when the default PDP context is created. This is a simplification.
  dev->SetAddress (Mac64Address::Allocate ());

  for (std::map<uint8_t, Ptr<cv2x_ComponentCarrierUe> >::iterator it = ueCcMap.begin (); it != ueCcMap.end (); ++it)
    {
      Ptr<cv2x_LteUePhy> ccPhy = it->second->GetPhy ();
      ccPhy->SetDevice (dev);
      ccPhy->GetUlSpectrumPhy ()->SetDevice (dev);
      ccPhy->GetDlSpectrumPhy ()->SetDevice (dev);
      ccPhy->GetDlSpectrumPhy ()->SetLtePhyRxDataEndOkCallback (MakeCallback (&cv2x_LteUePhy::PhyPduReceived, ccPhy));
      ccPhy->GetDlSpectrumPhy ()->SetLtePhyRxCtrlEndOkCallback (MakeCallback (&cv2x_LteUePhy::ReceiveLteControlMessageList, ccPhy));
      ccPhy->GetDlSpectrumPhy ()->SetLtePhyRxPssCallback (MakeCallback (&cv2x_LteUePhy::ReceivePss, ccPhy));
      ccPhy->GetDlSpectrumPhy ()->SetLtePhyDlHarqFeedbackCallback (MakeCallback (&cv2x_LteUePhy::ReceiveLteDlHarqFeedback, ccPhy));
    }

  if (m_useSidelink || m_useDiscovery)
    {
      slPhy->SetDevice (dev);
    }
  nas->SetDevice (dev);

  n->AddDevice (dev);

  nas->SetForwardUpCallback (MakeCallback (&cv2x_LteUeNetDevice::Receive, dev));

  if (m_useSidelink || m_useDiscovery)
    {
      for (std::map<uint8_t, Ptr<cv2x_ComponentCarrierUe> >::iterator it = ueCcMap.begin (); it != ueCcMap.end (); ++it)
        {
          Ptr<cv2x_LteUePhy> ccPhy = it->second->GetPhy ();
          slPhy->SetLtePhyRxDataEndOkCallback (MakeCallback (&cv2x_LteUePhy::PhyPduReceived, ccPhy));
          slPhy->SetLtePhyRxCtrlEndOkCallback (MakeCallback (&cv2x_LteUePhy::ReceiveLteControlMessageList, ccPhy));
          slPhy->SetLtePhyRxSlssCallback (MakeCallback (&cv2x_LteUePhy::ReceiveSlss, ccPhy));
        }
    }

  if (m_epcHelper != 0)
    {
      m_epcHelper->AddUe (dev, dev->GetImsi ());
    }

  dev->Initialize ();

  return dev;
}


void
cv2x_LteHelper::Attach (NetDeviceContainer ueDevices)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      Attach (*i);
    }
}

void
cv2x_LteHelper::Attach (Ptr<NetDevice> ueDevice)
{
  NS_LOG_FUNCTION (this);

  if (m_epcHelper == 0)
    {
      NS_FATAL_ERROR ("This function is not valid without properly configured EPC");
    }

  Ptr<cv2x_LteUeNetDevice> ueLteDevice = ueDevice->GetObject<cv2x_LteUeNetDevice> ();
  if (ueLteDevice == 0)
    {
      NS_FATAL_ERROR ("The passed NetDevice must be an cv2x_LteUeNetDevice");
    }

  // initiate cell selection
  Ptr<cv2x_EpcUeNas> ueNas = ueLteDevice->GetNas ();
  NS_ASSERT (ueNas != 0);
  uint32_t dlEarfcn = ueLteDevice->GetDlEarfcn ();
  ueNas->StartCellSelection (dlEarfcn);

  // instruct UE to immediately enter CONNECTED mode after camping
  ueNas->Connect ();

  // activate default EPS bearer
  m_epcHelper->ActivateEpsBearer (ueDevice, ueLteDevice->GetImsi (),
                                  cv2x_EpcTft::Default (),
                                  cv2x_EpsBearer (cv2x_EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
}

void
cv2x_LteHelper::Attach (NetDeviceContainer ueDevices, Ptr<NetDevice> enbDevice)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      Attach (*i, enbDevice);
    }
}

void
cv2x_LteHelper::Attach (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice)
{
  NS_LOG_FUNCTION (this);
  //enbRrc->SetCellId (enbDevice->GetObject<cv2x_LteEnbNetDevice> ()->GetCellId ());

  Ptr<cv2x_LteUeNetDevice> ueLteDevice = ueDevice->GetObject<cv2x_LteUeNetDevice> ();
  Ptr<cv2x_LteEnbNetDevice> enbLteDevice = enbDevice->GetObject<cv2x_LteEnbNetDevice> ();

  Ptr<cv2x_EpcUeNas> ueNas = ueLteDevice->GetNas ();
  ueNas->Connect (enbLteDevice->GetCellId (), enbLteDevice->GetDlEarfcn ());

  if (m_epcHelper != 0)
    {
      // activate default EPS bearer
      m_epcHelper->ActivateEpsBearer (ueDevice, ueLteDevice->GetImsi (), cv2x_EpcTft::Default (), cv2x_EpsBearer (cv2x_EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
    }

  // tricks needed for the simplified LTE-only simulations 
  if (m_epcHelper == 0)
    {
      ueDevice->GetObject<cv2x_LteUeNetDevice> ()->SetTargetEnb (enbDevice->GetObject<cv2x_LteEnbNetDevice> ());
    }
}

void
cv2x_LteHelper::AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      AttachToClosestEnb (*i, enbDevices);
    }
}

void
cv2x_LteHelper::AttachToClosestEnb (Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices)
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
  Attach (ueDevice, closestEnbDevice);
}

uint8_t
cv2x_LteHelper::ActivateDedicatedEpsBearer (NetDeviceContainer ueDevices, cv2x_EpsBearer bearer, Ptr<cv2x_EpcTft> tft)
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
cv2x_LteHelper::ActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, cv2x_EpsBearer bearer, Ptr<cv2x_EpcTft> tft)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "dedicated EPS bearers cannot be set up when the EPC is not used");

  uint64_t imsi = ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetImsi ();
  uint8_t bearerId = m_epcHelper->ActivateEpsBearer (ueDevice, imsi, tft, bearer);
  return bearerId;
}

void
cv2x_LteHelper::ActivateSidelinkBearer (NetDeviceContainer ueDevices, Ptr<cv2x_LteSlTft> tft)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      ActivateSidelinkBearer (*i, tft->Copy());
    }
}

void
cv2x_LteHelper::ActivateSidelinkBearer (Ptr<NetDevice> ueDevice, Ptr<cv2x_LteSlTft> tft)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "sidelink bearers cannot be set up when the EPC is not used");

  m_epcHelper->ActivateSidelinkBearer (ueDevice, tft);
}  

void
cv2x_LteHelper::DeactivateSidelinkBearer (NetDeviceContainer ueDevices, Ptr<cv2x_LteSlTft> tft)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      DeactivateSidelinkBearer (*i, tft);
    }
}

void
cv2x_LteHelper::DeactivateSidelinkBearer (Ptr<NetDevice> ueDevice, Ptr<cv2x_LteSlTft> tft)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "sidelink bearers cannot be set up when the EPC is not used");

  m_epcHelper->DeactivateSidelinkBearer (ueDevice, tft);
}

void
cv2x_LteHelper::StartDiscovery (NetDeviceContainer ueDevices, std::list<uint32_t> apps, bool rxtx)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      StartDiscovery (*i, apps, rxtx);
    }
}

void
cv2x_LteHelper::StartDiscovery (Ptr<NetDevice> ueDevice, std::list<uint32_t> apps, bool rxtx)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "discovery can't start when the EPC is not used");

  m_epcHelper->StartDiscovery (ueDevice, apps, rxtx);
}  

void
cv2x_LteHelper::StopDiscovery (NetDeviceContainer ueDevices, std::list<uint32_t> apps, bool rxtx)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      StopDiscovery (*i, apps, rxtx);
    }
}

void
cv2x_LteHelper::StopDiscovery (Ptr<NetDevice> ueDevice, std::list<uint32_t> apps, bool rxtx)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "no EPC is used");

  m_epcHelper->StopDiscovery (ueDevice, apps, rxtx);
}


/**
 * \ingroup lte
 *
 * DrbActivatior allows user to activate bearers for UEs
 * when EPC is not used. Activation function is hooked to
 * the Enb RRC Connection Estabilished trace source. When
 * UE change its RRC state to CONNECTED_NORMALLY, activation
 * function is called and bearer is activated.
*/
class cv2x_DrbActivator : public SimpleRefCount<cv2x_DrbActivator>
{
public:
  /**
  * cv2x_DrbActivator Constructor
  *
  * \param ueDevice the UeNetDevice for which bearer will be activated
  * \param bearer the bearer configuration
  */
  cv2x_DrbActivator (Ptr<NetDevice> ueDevice, cv2x_EpsBearer bearer);

  /**
   * Function hooked to the Enb RRC Connection Established trace source
   * Fired upon successful RRC connection establishment.
   *
   * \param a cv2x_DrbActivator object
   * \param context
   * \param imsi
   * \param cellId
   * \param rnti
   */
  static void ActivateCallback (Ptr<cv2x_DrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti);

  /**
   * Procedure firstly checks if bearer was not activated, if IMSI
   * from trace source equals configured one and if UE is really
   * in RRC connected state. If all requirements are met, it performs
   * bearer activation.
   *
   * \param imsi
   * \param cellId
   * \param rnti
   */
  void ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti);
private:
  /**
   * Bearer can be activated only once. This value stores state of
   * bearer. Initially is set to false and changed to true during
   * bearer activation.
   */
  bool m_active;
  /**
   * UeNetDevice for which bearer will be activated
   */
  Ptr<NetDevice> m_ueDevice;
  /**
   * Configuration of bearer which will be activated
   */
  cv2x_EpsBearer m_bearer;
  /**
   * imsi the unique UE identifier
   */
  uint64_t m_imsi;
};

cv2x_DrbActivator::cv2x_DrbActivator (Ptr<NetDevice> ueDevice, cv2x_EpsBearer bearer)
  : m_active (false),
    m_ueDevice (ueDevice),
    m_bearer (bearer),
    m_imsi (m_ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetImsi ())
{
}

void
cv2x_DrbActivator::ActivateCallback (Ptr<cv2x_DrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (a << context << imsi << cellId << rnti);
  a->ActivateDrb (imsi, cellId, rnti);
}

void
cv2x_DrbActivator::ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{ 
  NS_LOG_FUNCTION (this << imsi << cellId << rnti << m_active);
  if ((!m_active) && (imsi == m_imsi))
    {
      Ptr<cv2x_LteUeRrc> ueRrc = m_ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetState () == cv2x_LteUeRrc::CONNECTED_NORMALLY);
      uint16_t rnti = ueRrc->GetRnti ();
      Ptr<cv2x_LteEnbNetDevice> enbLteDevice = m_ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetTargetEnb ();
      Ptr<cv2x_LteEnbRrc> enbRrc = enbLteDevice->GetObject<cv2x_LteEnbNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetCellId () == enbLteDevice->GetCellId ());
      Ptr<cv2x_UeManager> ueManager = enbRrc->GetUeManager (rnti);
      NS_ASSERT (ueManager->GetState () == cv2x_UeManager::CONNECTED_NORMALLY
                 || ueManager->GetState () == cv2x_UeManager::CONNECTION_RECONFIGURATION);
      cv2x_EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params;
      params.rnti = rnti;
      params.bearer = m_bearer;
      params.bearerId = 0;
      params.gtpTeid = 0; // don't care
      enbRrc->GetS1SapUser ()->DataRadioBearerSetupRequest (params);
      m_active = true;
    }
}


void 
cv2x_LteHelper::ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, cv2x_EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice);
  NS_ASSERT_MSG (m_epcHelper == 0, "this method must not be used when the EPC is being used");

  // Normally it is the EPC that takes care of activating DRBs
  // when the UE gets connected. When the EPC is not used, we achieve
  // the same behavior by hooking a dedicated DRB activation function
  // to the Enb RRC Connection Established trace source


  Ptr<cv2x_LteEnbNetDevice> enbLteDevice = ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetTargetEnb ();

  std::ostringstream path;
  path << "/NodeList/" << enbLteDevice->GetNode ()->GetId () 
       << "/DeviceList/" << enbLteDevice->GetIfIndex ()
       << "/cv2x_LteEnbRrc/ConnectionEstablished";
  Ptr<cv2x_DrbActivator> arg = Create<cv2x_DrbActivator> (ueDevice, bearer);
  Config::Connect (path.str (), MakeBoundCallback (&cv2x_DrbActivator::ActivateCallback, arg));
}

void
cv2x_LteHelper::AddX2Interface (NodeContainer enbNodes)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "X2 interfaces cannot be set up when the EPC is not used");

  for (NodeContainer::Iterator i = enbNodes.Begin (); i != enbNodes.End (); ++i)
    {
      for (NodeContainer::Iterator j = i + 1; j != enbNodes.End (); ++j)
        {
          AddX2Interface (*i, *j);
        }
    }
}

void
cv2x_LteHelper::AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("setting up the X2 interface");

  m_epcHelper->AddX2Interface (enbNode1, enbNode2);
}

void
cv2x_LteHelper::HandoverRequest (Time hoTime, Ptr<NetDevice> ueDev, Ptr<NetDevice> sourceEnbDev, Ptr<NetDevice> targetEnbDev)
{
  NS_LOG_FUNCTION (this << ueDev << sourceEnbDev << targetEnbDev);
  NS_ASSERT_MSG (m_epcHelper, "Handover requires the use of the EPC - did you forget to call cv2x_LteHelper::SetEpcHelper () ?");
  uint16_t targetCellId = targetEnbDev->GetObject<cv2x_LteEnbNetDevice> ()->GetCellId ();
  Simulator::Schedule (hoTime, &cv2x_LteHelper::DoHandoverRequest, this, ueDev, sourceEnbDev, targetCellId);
}

void
cv2x_LteHelper::HandoverRequest (Time hoTime, Ptr<NetDevice> ueDev, Ptr<NetDevice> sourceEnbDev, uint16_t targetCellId)
{
  NS_LOG_FUNCTION (this << ueDev << sourceEnbDev << targetCellId);
  NS_ASSERT_MSG (m_epcHelper, "Handover requires the use of the EPC - did you forget to call cv2x_LteHelper::SetEpcHelper () ?");
  Simulator::Schedule (hoTime, &cv2x_LteHelper::DoHandoverRequest, this, ueDev, sourceEnbDev, targetCellId);
}

void
cv2x_LteHelper::DoHandoverRequest (Ptr<NetDevice> ueDev, Ptr<NetDevice> sourceEnbDev, uint16_t targetCellId)
{
  NS_LOG_FUNCTION (this << ueDev << sourceEnbDev << targetCellId);

  Ptr<cv2x_LteEnbRrc> sourceRrc = sourceEnbDev->GetObject<cv2x_LteEnbNetDevice> ()->GetRrc ();
  uint16_t rnti = ueDev->GetObject<cv2x_LteUeNetDevice> ()->GetRrc ()->GetRnti ();
  sourceRrc->SendHandoverRequest (rnti, targetCellId);
}

void
cv2x_LteHelper::DeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice,Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);
  NS_ASSERT_MSG (m_epcHelper != 0, "Dedicated EPS bearers cannot be de-activated when the EPC is not used");
  NS_ASSERT_MSG (bearerId != 1, "Default bearer cannot be de-activated until and unless and UE is released");

  DoDeActivateDedicatedEpsBearer (ueDevice, enbDevice, bearerId);
}

void
cv2x_LteHelper::DoDeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);

  //Extract IMSI and rnti
  uint64_t imsi = ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetImsi ();
  uint16_t rnti = ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetRrc ()->GetRnti ();


  Ptr<cv2x_LteEnbRrc> enbRrc = enbDevice->GetObject<cv2x_LteEnbNetDevice> ()->GetRrc ();

  enbRrc->DoSendReleaseDataRadioBearer (imsi,rnti,bearerId);
}

void
cv2x_LteHelper::DoComponentCarrierConfigure (uint32_t ulEarfcn, uint32_t dlEarfcn, uint8_t ulbw, uint8_t dlbw)
{
  NS_LOG_FUNCTION (this << ulEarfcn << dlEarfcn << ulbw << dlbw);

  NS_ABORT_MSG_IF (m_componentCarrierPhyParams.size() != 0, "CC map is not clean");
  Ptr<cv2x_CcHelper> ccHelper = CreateObject<cv2x_CcHelper> ();
  ccHelper->SetNumberOfComponentCarriers (m_noOfCcs);
  ccHelper->SetUlEarfcn (ulEarfcn);
  ccHelper->SetDlEarfcn (dlEarfcn);
  ccHelper->SetDlBandwidth (dlbw);
  ccHelper->SetUlBandwidth (ulbw);
  m_componentCarrierPhyParams = ccHelper->EquallySpacedCcs ();
  m_componentCarrierPhyParams.at (0).SetAsPrimary (true);
}

void 
cv2x_LteHelper::ActivateDataRadioBearer (NetDeviceContainer ueDevices, cv2x_EpsBearer bearer)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      ActivateDataRadioBearer (*i, bearer);
    }
}

void
cv2x_LteHelper::EnableLogComponents (void)
{
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  // Model directory
  LogComponentEnable ("cv2x_A2A4RsrqHandoverAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_A3RsrpHandoverAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_Asn1Header", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_ComponentCarrier", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_ComponentCarrierEnb", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_ComponentCarrierUe", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_CqaFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcEnbApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcMme", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcSgwPgwApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcTft", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcTftClassifier", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcUeNas", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcX2", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcX2Header", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_FdBetFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_FdMtFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_FdTbfqFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_FfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_GtpuHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteAmc", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteAnr", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteChunkProcessor", LOG_LEVEL_ALL);
  LogComponentEnable ("LteCommon", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteControlMessage", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteEnbComponentCarrierManager", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteEnbMac", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteEnbNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteEnbPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteEnbRrc", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteFfrAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteFfrDistributedAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteFfrEnhancedAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteFfrSoftAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteFrHardAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteFrNoOpAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteFrSoftAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteFrStrictAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteHandoverAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteHarqPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteInterference", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteMiErrorModel", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LtePdcp", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LtePdcpHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LtePhy", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteRlc", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteRlcAm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteRlcAmHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteRlcHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteRlcTm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteRlcUm", LOG_LEVEL_ALL);
  LogComponentEnable ("LteRrcProtocolIdeal", LOG_LEVEL_ALL);
  LogComponentEnable ("LteRrcProtocolReal", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteSpectrumPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteSpectrumSignalParameters", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteSpectrumValueHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteUeComponentCarrierManager", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteUeMac", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteUeNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteUePhy", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteUePowerControl", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteUeRrc", LOG_LEVEL_ALL);
  LogComponentEnable ("LteVendorSpecificParameters", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_NoOpComponentCarrierManager", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_NoOpHandoverAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_PfFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_PssFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_RemSpectrumPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("RrcHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_RrFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_SimpleUeComponentCarrierManager", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_TdBetFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_TdMtFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_TdTbfqFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_TraceFadingLossModel", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_TtaFfMacScheduler", LOG_LEVEL_ALL);
  // Helper directory
  LogComponentEnable ("cv2x_CcHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EmuEpcHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_EpcHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteGlobalPathlossDatabase", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteHexGridEnbTopologyHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_LteStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_MacStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_PhyRxStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_PhyStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_PhyTxStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_PointToPointEpcHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_RadioBearerStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_RadioBearerStatsConnector", LOG_LEVEL_ALL);
  LogComponentEnable ("cv2x_RadioEnvironmentMapHelper", LOG_LEVEL_ALL);
}

void
cv2x_LteHelper::EnableTraces (void)
{
  EnablePhyTraces ();
  EnableMacTraces ();
  EnableRlcTraces ();
  EnablePdcpTraces ();
}

void
cv2x_LteHelper::EnableRlcTraces (void)
{
  NS_ASSERT_MSG (m_rlcStats == 0, "please make sure that cv2x_LteHelper::EnableRlcTraces is called at most once");
  m_rlcStats = CreateObject<cv2x_RadioBearerStatsCalculator> ("RLC");
  m_radioBearerStatsConnector.EnableRlcStats (m_rlcStats);
}

int64_t
cv2x_LteHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  if ((m_fadingModule != 0) && (m_fadingStreamsAssigned == false))
    {
      Ptr<cv2x_TraceFadingLossModel> tflm = m_fadingModule->GetObject<cv2x_TraceFadingLossModel> ();
      if (tflm != 0)
        {
          currentStream += tflm->AssignStreams (currentStream);
          m_fadingStreamsAssigned = true;
        }
    }
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<cv2x_LteEnbNetDevice> lteEnb = DynamicCast<cv2x_LteEnbNetDevice> (netDevice);
      if (lteEnb)
        {
          std::map< uint8_t, Ptr <cv2x_ComponentCarrierEnb> > tmpMap = lteEnb->GetCcMap ();
          std::map< uint8_t, Ptr <cv2x_ComponentCarrierEnb> >::iterator it;
          it = tmpMap.begin ();
          Ptr<cv2x_LteSpectrumPhy> dlPhy = it->second->GetPhy ()->GetDownlinkSpectrumPhy ();
          Ptr<cv2x_LteSpectrumPhy> ulPhy = it->second->GetPhy ()->GetUplinkSpectrumPhy ();
          currentStream += dlPhy->AssignStreams (currentStream);
          currentStream += ulPhy->AssignStreams (currentStream);
        }
      Ptr<cv2x_LteUeNetDevice> lteUe = DynamicCast<cv2x_LteUeNetDevice> (netDevice);
      if (lteUe)
        {
          std::map< uint8_t, Ptr <cv2x_ComponentCarrierUe> > tmpMap = lteUe->GetCcMap ();
          std::map< uint8_t, Ptr <cv2x_ComponentCarrierUe> >::iterator it;
          it = tmpMap.begin ();
          Ptr<cv2x_LteSpectrumPhy> dlPhy = it->second->GetPhy ()->GetDownlinkSpectrumPhy ();
          Ptr<cv2x_LteSpectrumPhy> ulPhy = it->second->GetPhy ()->GetUplinkSpectrumPhy ();
          Ptr<cv2x_LteUeMac> ueMac = lteUe->GetMac ();
          currentStream += dlPhy->AssignStreams (currentStream);
          currentStream += ulPhy->AssignStreams (currentStream);
          currentStream += ueMac->AssignStreams (currentStream);
        }
    }
  return (currentStream - stream);
}


void
cv2x_LteHelper::EnablePhyTraces (void)
{
  EnableDlPhyTraces ();
  EnableUlPhyTraces ();
  EnableSlPhyTraces ();
  EnableDlTxPhyTraces ();
  EnableUlTxPhyTraces ();
  EnableSlTxPhyTraces ();
  EnableDlRxPhyTraces ();
  EnableUlRxPhyTraces ();
  EnableSlRxPhyTraces ();
  EnableSlPscchRxPhyTraces ();
}

void
cv2x_LteHelper::EnableDlTxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMap/*/cv2x_LteEnbPhy/DlPhyTransmission",
                   MakeBoundCallback (&cv2x_PhyTxStatsCalculator::DlPhyTransmissionCallback, m_phyTxStats));
}

void
cv2x_LteHelper::EnableUlTxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMapUe/*/cv2x_LteUePhy/UlPhyTransmission",
                   MakeBoundCallback (&cv2x_PhyTxStatsCalculator::UlPhyTransmissionCallback, m_phyTxStats));
}

void
cv2x_LteHelper::EnableSlTxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUePhy/SlPhyTransmission",
                   MakeBoundCallback (&cv2x_PhyTxStatsCalculator::SlPhyTransmissionCallback, m_phyTxStats));
}

void
cv2x_LteHelper::EnableDlRxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMapUe/*/cv2x_LteUePhy/DlSpectrumPhy/DlPhyReception",
                   MakeBoundCallback (&cv2x_PhyRxStatsCalculator::DlPhyReceptionCallback, m_phyRxStats));
}

void
cv2x_LteHelper::EnableUlRxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMap/*/cv2x_LteEnbPhy/UlSpectrumPhy/UlPhyReception",
                   MakeBoundCallback (&cv2x_PhyRxStatsCalculator::UlPhyReceptionCallback, m_phyRxStats));
}

void
cv2x_LteHelper::EnableSlRxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUePhy/SlSpectrumPhy/SlPhyReception",
                   MakeBoundCallback (&cv2x_PhyRxStatsCalculator::SlPhyReceptionCallback, m_phyRxStats));
}

void
cv2x_LteHelper::EnableSlPscchRxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUePhy/SlSpectrumPhy/SlPscchReception",
                   MakeBoundCallback (&cv2x_PhyRxStatsCalculator::SlPscchReceptionCallback, m_phyRxStats));
}

void
cv2x_LteHelper::EnableMacTraces (void)
{
  EnableDlMacTraces ();
  EnableUlMacTraces ();
  EnableSlUeMacTraces ();
  EnableSlSchUeMacTraces ();
}

void
cv2x_LteHelper::EnableSlUeMacTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeMac/SlUeScheduling",
                   MakeBoundCallback (&cv2x_MacStatsCalculator::SlUeSchedulingCallback, m_macStats));
}

void
cv2x_LteHelper::EnableSlSchUeMacTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeMac/SlSharedChUeScheduling",
                   MakeBoundCallback (&cv2x_MacStatsCalculator::SlSharedChUeSchedulingCallback, m_macStats));
}

void
cv2x_LteHelper::EnableDlMacTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMap/*/cv2x_LteEnbMac/DlScheduling",
                   MakeBoundCallback (&cv2x_MacStatsCalculator::DlSchedulingCallback, m_macStats));
}

void
cv2x_LteHelper::EnableUlMacTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMap/*/cv2x_LteEnbMac/UlScheduling",
                   MakeBoundCallback (&cv2x_MacStatsCalculator::UlSchedulingCallback, m_macStats));
}

void
cv2x_LteHelper::EnableDlPhyTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMapUe/*/cv2x_LteUePhy/ReportCurrentCellRsrpSinr",
                   MakeBoundCallback (&cv2x_PhyStatsCalculator::ReportCurrentCellRsrpSinrCallback, m_phyStats));
}

void
cv2x_LteHelper::EnableUlPhyTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMap/*/cv2x_LteEnbPhy/ReportUeSinr",
                   MakeBoundCallback (&cv2x_PhyStatsCalculator::ReportUeSinr, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_ComponentCarrierMap/*/cv2x_LteEnbPhy/ReportInterference",
                   MakeBoundCallback (&cv2x_PhyStatsCalculator::ReportInterference, m_phyStats));
}

void
cv2x_LteHelper::EnableSlPhyTraces (void)
{
  //TBD
}

Ptr<cv2x_RadioBearerStatsCalculator>
cv2x_LteHelper::GetRlcStats (void)
{
  return m_rlcStats;
}

void
cv2x_LteHelper::EnablePdcpTraces (void)
{
  NS_ASSERT_MSG (m_pdcpStats == 0, "please make sure that cv2x_LteHelper::EnablePdcpTraces is called at most once");
  m_pdcpStats = CreateObject<cv2x_RadioBearerStatsCalculator> ("PDCP");
  m_radioBearerStatsConnector.EnablePdcpStats (m_pdcpStats);
}

Ptr<cv2x_RadioBearerStatsCalculator>
cv2x_LteHelper::GetPdcpStats (void)
{
  return m_pdcpStats;
}

  /**
   * Deploys the Sidelink configuration to the eNodeB
   * \param enbDevices List of devices where to configure sidelink
   * \param slConfiguration Sidelink configuration
   */
void
cv2x_LteHelper::InstallSidelinkConfiguration (NetDeviceContainer enbDevices, Ptr<cv2x_LteEnbRrcSl> slConfiguration)
{
  //for each device, install a copy of the configuration
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = enbDevices.Begin (); i != enbDevices.End (); ++i)
    {
      InstallSidelinkConfiguration (*i, slConfiguration->Copy());
    }
}

  /**
   * Deploys the Sidelink configuration to the eNodeB
   * \param enbDevice List of devices where to configure sidelink
   * \param slConfiguration Sidelink configuration
   */
void
cv2x_LteHelper::InstallSidelinkConfiguration (Ptr<NetDevice> enbDevice, Ptr<cv2x_LteEnbRrcSl> slConfiguration)
{
  Ptr<cv2x_LteEnbRrc> rrc = enbDevice->GetObject<cv2x_LteEnbNetDevice> ()->GetRrc();
  NS_ASSERT_MSG (rrc != 0, "RRC layer not found");
  rrc->SetAttribute ("SidelinkConfiguration", PointerValue (slConfiguration));
}

  /**
   * Deploys the Sidelink configuration to the UEs
   * \param ueDevices List of devices where to configure sidelink
   * \param slConfiguration Sidelink configuration
   */
void
cv2x_LteHelper::InstallSidelinkConfiguration (NetDeviceContainer ueDevices, Ptr<cv2x_LteUeRrcSl> slConfiguration)
{
  //for each device, install a copy of the configuration
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      InstallSidelinkConfiguration (*i, slConfiguration);
    }
}

  /**
   * Deploys the Sidelink configuration to the UE
   * \param ueDevice The UE to configure
   * \param slConfiguration Sidelink configuration
   */
void
cv2x_LteHelper::InstallSidelinkConfiguration (Ptr<NetDevice> ueDevice, Ptr<cv2x_LteUeRrcSl> slConfiguration)
{
  Ptr<cv2x_LteUeRrc> rrc = ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetRrc();
  NS_ASSERT_MSG (rrc != 0, "RRC layer not found");
  PointerValue ptr;
  rrc->GetAttribute ("SidelinkConfiguration", ptr);
  Ptr<cv2x_LteUeRrcSl> ueConfig = ptr.Get<cv2x_LteUeRrcSl> ();
  ueConfig->SetSlPreconfiguration (slConfiguration->GetSlPreconfiguration());
  ueConfig->SetSlEnabled (slConfiguration->IsSlEnabled());
  ueConfig->SetDiscEnabled (slConfiguration->IsDiscEnabled());
  ueConfig->SetDiscTxResources (slConfiguration->GetDiscTxResources ());
  ueConfig->SetDiscInterFreq (slConfiguration->GetDiscInterFreq ());
}

/**
 * Deploys the Sidelink configuration to the UEs
 * \param ueDevices List of devices where to configure sidelink
 * \param slConfiguration Sidelink configuration
 */
void
cv2x_LteHelper::InstallSidelinkV2xConfiguration (NetDeviceContainer ueDevices, Ptr<cv2x_LteUeRrcSl> slConfiguration)
{
  //for each device, install a copy of the configuration
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      InstallSidelinkV2xConfiguration (*i, slConfiguration);
    }
}

/**
 * Deploys the Sidelink configuration to the UE
 * \param ueDevice The UE to configure
 * \param slConfiguration Sidelink configuration
 */
void
cv2x_LteHelper::InstallSidelinkV2xConfiguration (Ptr<NetDevice> ueDevice, Ptr<cv2x_LteUeRrcSl> slConfiguration)
{
  Ptr<cv2x_LteUeRrc> rrc = ueDevice->GetObject<cv2x_LteUeNetDevice> ()->GetRrc();
  NS_ASSERT_MSG (rrc != 0, "RRC layer not found");
  PointerValue ptr;
  rrc->GetAttribute ("SidelinkConfiguration", ptr);
  Ptr<cv2x_LteUeRrcSl> ueConfig = ptr.Get<cv2x_LteUeRrcSl> ();
  ueConfig->SetSlV2xPreconfiguration (slConfiguration->GetSlV2xPreconfiguration ());
  ueConfig->SetSlEnabled (slConfiguration->IsSlEnabled ());
  ueConfig->SetDiscEnabled (slConfiguration->IsDiscEnabled());
  ueConfig->SetDiscTxResources (slConfiguration->GetDiscTxResources ());
  ueConfig->SetDiscInterFreq (slConfiguration->GetDiscInterFreq ());
  ueConfig->SetV2xEnabled (slConfiguration->IsV2xEnabled ());
}


/**
 * Compute the RSRP between the given nodes for the given propagation loss model
 * This code is derived from the multi-model-spectrum-channel class
 * \param propagationLoss The loss model
 * \param psd The power spectral density of the transmitter
 * \param txPhy The transmitter
 * \param rxPhy The receiver
 * \return The RSRP 
 */
double
cv2x_LteHelper::DoCalcRsrp (Ptr<PropagationLossModel> propagationLoss, Ptr<SpectrumValue> psd, Ptr<SpectrumPhy> txPhy, Ptr<SpectrumPhy> rxPhy)
{
  Ptr<MobilityModel> txMobility = txPhy->GetMobility ();
  Ptr<MobilityModel> rxMobility = rxPhy->GetMobility ();

  double pathLossDb = 0;
  if (txPhy->GetRxAntenna() != 0)
    {
      Angles txAngles (rxMobility->GetPosition (), txMobility->GetPosition ());
      double txAntennaGain = txPhy->GetRxAntenna()->GetGainDb (txAngles);
      NS_LOG_DEBUG ("txAntennaGain = " << txAntennaGain << " dB");
      pathLossDb -= txAntennaGain;
    }
  Ptr<AntennaModel> rxAntenna = rxPhy->GetRxAntenna ();
  if (rxAntenna != 0)
    {
      Angles rxAngles (txMobility->GetPosition (), rxMobility->GetPosition ());
      double rxAntennaGain = rxAntenna->GetGainDb (rxAngles);
      NS_LOG_DEBUG ("rxAntennaGain = " << rxAntennaGain << " dB");
      pathLossDb -= rxAntennaGain;
    }
  if (propagationLoss)
    {
      double propagationGainDb = propagationLoss->CalcRxPower (0, txMobility, rxMobility);
      NS_LOG_DEBUG ("propagationGainDb = " << propagationGainDb << " dB");
      pathLossDb -= propagationGainDb;
    }                    
  NS_LOG_DEBUG ("total pathLoss = " << pathLossDb << " dB");  

  double pathGainLinear = std::pow (10.0, (-pathLossDb) / 10.0);
  Ptr<SpectrumValue> rxPsd = Copy<SpectrumValue> (psd);
  *rxPsd *= pathGainLinear;

  // RSRP evaluated as averaged received power among RBs
  double sum = 0.0;
  uint8_t rbNum = 0;
  Values::const_iterator it;
  for (it = (*rxPsd).ValuesBegin (); it != (*rxPsd).ValuesEnd (); it++)
    {
      //The non active RB will be set to -inf
      //We count only the active
      if((*it))
        {          
          // convert PSD [W/Hz] to linear power [W] for the single RE
          // we consider only one RE for the RS since the channel is 
          // flat within the same RB 
          double powerTxW = ((*it) * 180000.0) / 12.0;
          sum += powerTxW;
          rbNum++;
        }
    }
  double rsrp = (rbNum > 0) ? (sum / rbNum) : DBL_MAX;

  NS_LOG_INFO ("RSRP linear=" << rsrp << " (" << 10 * std::log10 (rsrp) + 30 << "dBm)");

  return 10 * std::log10 (rsrp) + 30;
}

/**
 * Compute the RSRP between the given nodes for the given propagation loss model
 * This code is derived from the multi-model-spectrum-channel class
 * \param propagationLoss The loss model
 * \param txPower The transmit power
 * \param txPhy The transmitter
 * \param rxPhy The receiver
 * \return The RSRP 
 */
double
cv2x_LteHelper::DoCalcRsrp (Ptr<PropagationLossModel> propagationLoss, double txPower, Ptr<SpectrumPhy> txPhy, Ptr<SpectrumPhy> rxPhy)
{
  Ptr<MobilityModel> txMobility = txPhy->GetMobility ();
  Ptr<MobilityModel> rxMobility = rxPhy->GetMobility ();

  double pathLossDb = 0;
  if (txPhy->GetRxAntenna() != 0)
    {
      Angles txAngles (rxMobility->GetPosition (), txMobility->GetPosition ());
      double txAntennaGain = txPhy->GetRxAntenna()->GetGainDb (txAngles);
      NS_LOG_DEBUG ("txAntennaGain = " << txAntennaGain << " dB");
      pathLossDb -= txAntennaGain;
    }
  Ptr<AntennaModel> rxAntenna = rxPhy->GetRxAntenna ();
  if (rxAntenna != 0)
    {
      Angles rxAngles (txMobility->GetPosition (), rxMobility->GetPosition ());
      double rxAntennaGain = rxAntenna->GetGainDb (rxAngles);
      NS_LOG_DEBUG ("rxAntennaGain = " << rxAntennaGain << " dB");
      pathLossDb -= rxAntennaGain;
    }
  if (propagationLoss)
    {
      double propagationGainDb = propagationLoss->CalcRxPower (0, txMobility, rxMobility);
      NS_LOG_DEBUG ("propagationGainDb = " << propagationGainDb << " dB");
      pathLossDb -= propagationGainDb;
    }                    
  NS_LOG_DEBUG ("total pathLoss = " << pathLossDb << " dB");  
  
  double rsrp = txPower - pathLossDb;

  NS_LOG_INFO ("RSRP=" << rsrp << " dBm");

  return rsrp;
}


/**
 * Computes the S-RSRP between 2 UEs. Information about the uplink frequency and band is necessary  to be able to call the function before the simulation starts.
 * \param txPower Transmit power for the reference signal
 * \param ulEarfcn Uplink frequency
 * \param ulBandwidth Uplink bandwidth
 * \param txDevice Transmitter UE
 * \param rxDevice Receiver UE
 * \return RSRP value
 */
double
cv2x_LteHelper::CalcSidelinkRsrp (double txPower, double ulEarfcn, double ulBandwidth, Ptr<NetDevice> txDevice, Ptr<NetDevice> rxDevice)
{
  Ptr<PropagationLossModel> lossModel = m_uplinkPathlossModel->GetObject<PropagationLossModel> ();
  NS_ASSERT_MSG (lossModel != 0, " " << m_uplinkPathlossModel << " is not a PropagationLossModel");

  /*
    Sidelink Reference Signal Received Power (S-RSRP) is defined as the linear average over the
    power contributions (in [W]) of the resource elements that carry demodulation reference signals
    associated with PSBCH, within the central 6 PRBs of the applicable subframes. 
  */
  //This method returned very low values of RSRP 
  std::vector <int> rbMask;
  for (int i = 22; i < 28 ; i++)
    {
      rbMask.push_back (i);
    }
  cv2x_LteSpectrumValueHelper psdHelper;
  Ptr<SpectrumValue> psd = psdHelper.CreateUlTxPowerSpectralDensity (ulEarfcn, ulBandwidth, txPower, rbMask);
  
  double rsrp= DoCalcRsrp (lossModel, psd, txDevice->GetObject<cv2x_LteUeNetDevice> ()->GetPhy()->GetUlSpectrumPhy (), rxDevice->GetObject<cv2x_LteUeNetDevice> ()->GetPhy()->GetUlSpectrumPhy ());
  
  NS_LOG_INFO ("S-RSRP=" << rsrp);

  return rsrp;
}

/**
 * Computes the RSRP between a transmitter UE and a receiver UE as defined in TR 36.843. Information about the uplink frequency and band is necessary  to be able to call the function before the simulation starts. 
 * \param txPower Transmit power for the reference signal
 * \param ulEarfcn Uplink frequency
 * \param txDevice Transmitter UE
 * \param rxDevice Receiver UE
 * \return RSRP value
 */
double
cv2x_LteHelper::CalcSidelinkRsrpEval (double txPower, double ulEarfcn, Ptr<NetDevice> txDevice, Ptr<NetDevice> rxDevice)
{
  Ptr<PropagationLossModel> lossModel = m_uplinkPathlossModel->GetObject<PropagationLossModel> ();
  NS_ASSERT_MSG (lossModel != 0, " " << m_uplinkPathlossModel << " is not a PropagationLossModel");

  /*
    36.843: RSRP is calculated for transmit power of 23dBm by the transmitter UE and is the received power at the receiver UE calculated after accounting for large scale path loss and shadowing. Additionally note that wrap around is used for path loss calculations except for the case of partial -coverage.
  */
  double rsrp= DoCalcRsrp (lossModel, txPower, txDevice->GetObject<cv2x_LteUeNetDevice> ()->GetPhy()->GetUlSpectrumPhy (), rxDevice->GetObject<cv2x_LteUeNetDevice> ()->GetPhy()->GetUlSpectrumPhy ());
  
  NS_LOG_INFO ("RSRP=" << rsrp);

  return rsrp;
}

} // namespace ns3
