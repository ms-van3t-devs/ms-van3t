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


#ifndef NR_HELPER_H
#define NR_HELPER_H

#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/eps-bearer.h>
#include <ns3/object-factory.h>
#include <ns3/nr-bearer-stats-connector.h>
#include <ns3/nr-control-messages.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/nr-spectrum-phy.h>
#include "ideal-beamforming-helper.h"
#include "cc-bwp-helper.h"
#include "nr-mac-scheduling-stats.h"

namespace ns3 {

class NrUePhy;
class NrGnbPhy;
class SpectrumChannel;
class NrSpectrumValueHelper;
class NrGnbMac;
class EpcHelper;
class EpcTft;
class NrBearerStatsCalculator;
class NrMacRxTrace;
class NrPhyRxTrace;
class ComponentCarrierEnb;
class ComponentCarrier;
class NrMacScheduler;
class NrGnbNetDevice;
class NrUeNetDevice;
class NrUeMac;
class BwpManagerGnb;
class BwpManagerUe;

/**
 * \ingroup helper
 * \brief Helper for a correct setup of every NR simulation
 *
 * This class will help you in setting up a single- or multi-cell scenario
 * with NR. Most probably, you will interact with the NR module only through
 * this class, so make sure everthing that is written in the following is
 * clear enough to start creating your own scenario.
 *
 * \section helper_pre Pre-requisite: Node creation and placement
 *
 * We assume that you read the ns-3 tutorials, and you're able to create
 * your own node placement, as well as the mobility model that you prefer
 * for your scenario. For simple cases, we provide a class that can help you
 * in setting up a grid-like scenario. Please take a look at the
 * GridScenarioHelper documentation in that case.
 *
 * \section helper_creating Creating the helper
 *
 * Usually, the helper is created on the heap, and have to live till the end
 * of the simulation program:
 *
\verbatim
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);
\endverbatim
 *
 * As you can see, we have created two other object that can help this class:
 * the IdealBeamformingHelper and the NrPointToPointEpcHelper. Please refer to
 * the documentation of such classes if you need more information about them.
 *
 * \section helper_dividing Dividing the spectrum and creating the channels
 *
 * The spectrum management is delegated to the class CcBwpHelper. Please
 * refer to its documentation to have more information. After having divided
 * the spectrum in component carriers and bandwidth part, use the method
 * InitializeOperationBand() to create the channels and other things that will
 * be used by the channel modeling part.
 *
 * \section helper_configuring Configuring the cells
 *
 * After the spectrum part is ready, it is time to check how to configure the
 * cells. The configuration is done in the most typical ns-3 way, through the
 * Attributes. We assume that you already know what an attribute is; if not,
 * please read the ns-3 documentation about objects and attributes.
 *
 *
 * We have two different ways to configure the attributes of our objects
 * (and, therefore, configure the cell). The first is before the object are
 * created: as this class is responsible to create all the object that are needed
 * in the module, we provide many methods (prefixed with "Set") that can
 * store the attribute values for the objects, until they are created and then
 * these values applied. For instance, we can set the antenna dimensions for
 * all the UEs with:
 *
\verbatim
  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
\endverbatim
 *
 * These attributes will be stored, and then used inside the creation methods
 * InstallGnbDevice() and InstallUeDevice(). In this case, the antenna dimensions
 * will be taken inside the InstallUeDevice() method, and all the UEs created
 * will have these dimensions.
 *
 * Of course, it is possible to divide the nodes, and install them separately
 * with different attributes. For example:
 *
\verbatim
  NodeContainer ueFirstSet;
  NodeContainer ueSecondSet;
  ...
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  ...
  NetDeviceContainer ueFirstSetNetDevs = nrHelper->InstallUeDevice (ueFirstSet, allBwps);
  ...
  // Then, prepare the second set:
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (8));
  ...
  NetDeviceContainer ueSecondSetNetDevs = nrHelper->InstallUeDevice (ueSecondSet, allBwps);
  ...
\endverbatim
 *
 * In this way, you can configure different sets of nodes with different properties.
 *
 * The second configuration option is setting the attributes after the object are
 * created. Once you have a pointer to the object, you can use the Object::SetAttribute()
 * method on it. To get the pointer, use one of the helper methods to retrieve
 * a pointer to the PHY, MAC, or scheduler, given the index of the bandwidth part:
 *
\verbatim
  ...
  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gnbContainer, allBwps);
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (2));
\endverbatim
 *
 * In the snippet, we are selecting the first gnb (`enbNetDev.Get (0)`) and
 * then selecting the first BWP (`, 0`). We are using the method GetGnbPhy()
 * to obtain the PHY pointer, and then setting the `Numerology` attribute
 * on it. The list of attributes is present in the class description, as well
 * as some reminder on how configure it through the helper.
 *
 * \section helper_installing Installing UEs and GNBs
 *
 * The installation part is done through two methods: InstallGnbDevice()
 * and InstallUeDevice(). Pass to these methods the container of the nodes,
 * plus the spectrum configuration that comes from CcBwpHelper.
 *
 * \section helper_finishing Finishing the configuration
 *
 * After you finish the configuration, please remember to call UpdateConfig()
 * on all the NetDevices:
 *
\verbatim
  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }
\endverbatim
 *
 * The call to UpdateConfig() will finish the configuration, and update the
 * RRC layer.
 *
 * \section helper_bearers Dedicated bearers
 *
 * We have methods to open dedicated UE bearers: ActivateDedicatedEpsBearer()
 * and DeActivateDedicatedEpsBearer(). Please take a look at their documentation
 * for more information.
 *
 * \section helper_attachment Attachment of UEs to GNBs
 *
 * We provide two methods to attach a set of UE to a GNB: AttachToClosestEnb()
 * and AttachToEnb(). Through these function, you will manually attach one or
 * more UEs to a specified GNB.
 *
 * \section helper_Traces Traces
 *
 * We provide a method that enables the generation of files that include among
 * others information related to the received packets at the gNB and UE side,
 * the control messages transmitted and received from/at the gNB and UE side,
 * the SINR, as well as RLC and PDCP statistics such as the packet size.
 * Please refer to their documentation for more information.
 * Enabling the traces is done by simply adding the method enableTraces() in the
 * scenario.
 *
 */
class NrHelper : public Object
{
public:
  /**
   * \brief NrHelper constructor
   */
  NrHelper (void);
  /**
   * \brief ~NrHelper
   */
  virtual ~NrHelper (void);

  /**
   * \brief GetTypeId
   * \return the type id of the object
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Install one (or more) UEs
   * \param c Node container with the UEs
   * \param allBwps The spectrum configuration that comes from CcBwpHelper
   * \return a NetDeviceContainer with the net devices that have been installed.
   *
   */
  NetDeviceContainer InstallUeDevice (const NodeContainer &c,
                                      const std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> &allBwps,
                                      uint8_t numberOfPanels = 1);
  /**
   * \brief Install one (or more) GNBs
   * \param c Node container with the GNB
   * \param allBwps The spectrum configuration that comes from CcBwpHelper
   * \return a NetDeviceContainer with the net devices that have been installed.
   */
  NetDeviceContainer InstallGnbDevice (const NodeContainer &c,
                                       const std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> allBwps,
                                       uint8_t numberOfPanels = 1);

  /**
   * \brief Get the number of configured BWP for a specific GNB NetDevice
   * \param gnbDevice The GNB NetDevice, obtained from InstallGnbDevice()
   * \return the number of BWP installed, or 0 if there are errors
   */
  static uint32_t GetNumberBwp (const Ptr<const NetDevice> &gnbDevice);
  /**
   * \brief Get a pointer to the PHY of the GNB at the specified BWP
   * \param gnbDevice The GNB NetDevice, obtained from InstallGnbDevice()
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the PHY layer of the GNB, or nullptr if there are errors
   */
  static Ptr<NrGnbPhy> GetGnbPhy (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex);
  /**
   * \brief Get a pointer to the MAC of the GNB at the specified BWP
   * \param gnbDevice The GNB NetDevice, obtained from InstallGnbDevice()
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the MAC layer of the GNB, or nullptr if there are errors
   */
  static Ptr<NrGnbMac> GetGnbMac (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex);
  /**
   * \brief Get a pointer to the MAC of the UE at the specified BWP
   * \param ueDevice The UE NetDevice, obtained from InstallUeDevice()
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the MAC layer of the UE, or nullptr if there are errors
   */
  static Ptr<NrUeMac> GetUeMac (const Ptr<NetDevice> &ueDevice, uint32_t bwpIndex);
  /**
   * \brief Get a pointer to the PHY of the UE at the specified BWP
   * \param ueDevice The UE NetDevice, obtained from InstallUeDevice()
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the PHY layer of the UE, or nullptr if there are errors
   */
  static Ptr<NrUePhy> GetUePhy (const Ptr<NetDevice> &ueDevice, uint32_t bwpIndex);
  /**
   * \brief Get the BwpManager of the GNB
   * \param gnbDevice the GNB NetDevice, obtained from InstallGnbDevice()
   * \return A pointer to the BwpManager of the GNB, or nullptr if there are errors
   */
  static Ptr<BwpManagerGnb> GetBwpManagerGnb (const Ptr<NetDevice> &gnbDevice);
  /**
   * \brief Get the BwpManager of the UE
   * \param ueDevice the UE NetDevice, obtained from InstallGnbDevice()
   * \return A pointer to the BwpManager of the UE, or nullptr if there are errors
   */
  static Ptr<BwpManagerUe> GetBwpManagerUe (const Ptr<NetDevice> &ueDevice);
  /**
   * \brief Get the Scheduler from the GNB specified
   * \param gnbDevice The GNB NetDevice, obtained from InstallGnbDevice()
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the scheduler, or nullptr if there are errors
   */
  static Ptr<NrMacScheduler> GetScheduler (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex);

  /**
   * \brief Attach the UE specified to the closest GNB
   * \param ueDevices UE devices to attach
   * \param enbDevices GNB devices from which the algorithm has to select the closest
   */
  void AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices);
  /**
   * \brief Attach a UE to a particular GNB
   * \param ueDevice the UE device
   * \param gnbDevice the GNB device to which attach the UE
   */
  void AttachToEnb (const Ptr<NetDevice> &ueDevice, const Ptr<NetDevice> &gnbDevice);

  /**
   * \brief Enables the following traces:
   * Transmitted/Received Control Messages
   * DL/UL Phy Traces
   * RLC traces
   * PDCP traces
   *
   */
  void EnableTraces ();

  /**
   * \brief Activate a Data Radio Bearer on a given UE devices
   *
   * \param ueDevices the set of UE devices
   * \param bearer the characteristics of the bearer to be activated
   */
  void ActivateDataRadioBearer (NetDeviceContainer ueDevices, EpsBearer bearer);
  /**
   * \brief Activate a Data Radio Bearer on a UE device.
   *
   * This method will schedule the actual activation
   * the bearer so that it happens after the UE got connected.
   *
   * \param ueDevice the UE device
   * \param bearer the characteristics of the bearer to be activated
   */
  void ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer);
  /**
   * Set the EpcHelper to be used to setup the EPC network in
   * conjunction with the setup of the LTE radio access network.
   *
   * \note if no EpcHelper is ever set, then LteHelper will default
   * to creating a simulation with no EPC, using LteRlcSm as
   * the RLC model, and without supporting any IP networking. In other
   * words, it will be a radio-level simulation involving only LTE PHY
   * and MAC and the Scheduler, with a saturation traffic model for
   * the RLC.
   *
   * \param epcHelper a pointer to the EpcHelper to be used
   */
  void SetEpcHelper (Ptr<EpcHelper> epcHelper);

  /**
   * \brief Set an ideal beamforming helper
   * \param beamformingHelper a pointer to the beamforming helper
   *
   */
  void SetBeamformingHelper (Ptr<BeamformingHelperBase> beamformingHelper);

  /**
   * \brief SetHarqEnabled
   * \param harqEnabled
   *
   * We never really tested this function, so please be careful when using it.
   */
  void SetHarqEnabled (bool harqEnabled);
  /**
   * \brief GetHarqEnabled
   * \return the value of HarqEnabled variable
   */
  bool GetHarqEnabled ();
  /**
   * \brief SetSnrTest
   * \param snrTest
   *
   * We never really tested this function, so please be careful when using it.
   */
  void SetSnrTest (bool snrTest);
  /**
   * \brief GetSnrTest
   * \return the value of SnrTest variable
   */
  bool GetSnrTest ();

  /**
   * \brief Flags for OperationBand initialization.
   */
  enum OperationBandFlags : uint8_t
  {
    INIT_PROPAGATION = 0x01, //!< Initialize the propagation loss model
    INIT_FADING = 0x02,      //!< Initialize the fading model
    INIT_CHANNEL = 0x04      //!< Initialize the channel model
  };

  /**
   * \brief Initialize the bandwidth parts by creating and configuring the channel
   * models, if they are not already initialized.
   *
   * If the models are already set (i.e., the pointers are not null) the helper
   * will not touch anything.
   *
   * \param band the band representation
   * \param flags the flags for the initialization. Default to initialize everything
   */
  void InitializeOperationBand (OperationBandInfo *band,
                                uint8_t flags = INIT_PROPAGATION | INIT_FADING | INIT_CHANNEL);

  /**
   * Activate a dedicated EPS bearer on a given set of UE devices.
   *
   * \param ueDevices the set of UE devices
   * \param bearer the characteristics of the bearer to be activated
   * \param tft the Traffic Flow Template that identifies the traffic to go on this bearer
   * \returns bearer ID
   */
  uint8_t ActivateDedicatedEpsBearer (NetDeviceContainer ueDevices, EpsBearer bearer, Ptr<EpcTft> tft);

  /**
   * Activate a dedicated EPS bearer on a given UE device.
   *
   * \param ueDevice the UE device
   * \param bearer the characteristics of the bearer to be activated
   * \param tft the Traffic Flow Template that identifies the traffic to go on this bearer.
   * \returns bearer ID
   */
  uint8_t ActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer, Ptr<EpcTft> tft);

  /**
   *  \brief Manually trigger dedicated bearer de-activation at specific simulation time
   *  \param ueDevice the UE on which dedicated bearer to be de-activated must be of the type LteUeNetDevice
   *  \param enbDevice eNB, must be of the type LteEnbNetDevice
   *  \param bearerId Bearer Identity which is to be de-activated
   *
   *  \warning Requires the use of EPC mode. See SetEpcHelper() method.
   */

  void DeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId);

  /**
   * \brief Set an attribute for the UE MAC, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   * \see NrUeMac
   */
  void SetUeMacAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the GNB MAC, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   * \see NrGnbMac
   */
  void SetGnbMacAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the GNB spectrum, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   * \see NrSpectrumPhy
   */
  void SetGnbSpectrumAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the UE spectrum, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   * \see NrSpectrumPhy
   */
  void SetUeSpectrumAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the UE channel access manager, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   *
   * \see NrChAccessManager
   */
  void SetUeChannelAccessManagerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the GNB channel access manager, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   *
   * \see NrChAccessManager
   */
  void SetGnbChannelAccessManagerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the scheduler, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   * \see NrMacSchedulerNs3
   */
  void SetSchedulerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the UE PHY, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   *
   * \see NrUePhy
   */
  void SetUePhyAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the GNB PHY, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   *
   * \see NrGnbPhy
   */
  void SetGnbPhyAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the UE antenna, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   *
   * \see UniformPlanarArray (in ns-3 documentation)
   */
  void SetUeAntennaAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the GNB antenna, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   *
   * \see UniformPlanarArray (in ns-3 documentation)
   */
  void SetGnbAntennaAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set the TypeId of the UE Channel Access Manager. Works only before it is created.
   *
   * \param typeId the type of the object
   *
   * \see NrChAccessManager
   * \see NrAlwaysOnAccessManager
   */
  void SetUeChannelAccessManagerTypeId (const TypeId &typeId);

  /**
   * \brief Set the TypeId of the GNB Channel Access Manager. Works only before it is created.
   *
   * \param typeId the type of the object
   *
   * \see NrChAccessManager
   * \see NrAlwaysOnAccessManager
   */
  void SetGnbChannelAccessManagerTypeId (const TypeId &typeId);

  /**
   * \brief Set the Scheduler TypeId. Works only before it is created.
   * \param typeId The scheduler type
   *
   * \see NrMacSchedulerOfdmaPF
   * \see NrMacSchedulerOfdmaRR
   * \see NrMacSchedulerOfdmaMR
   * \see NrMacSchedulerTdmaPF
   * \see NrMacSchedulerTdmaRR
   * \see NrMacSchedulerTdmaMR
   */
  void SetSchedulerTypeId (const TypeId &typeId);

  /**
   * \brief Set the TypeId of the GNB BWP Manager. Works only before it is created.
   * \param typeId Type of the object
   *
   * \see BwpManagerAlgorithm
   */
  void SetGnbBwpManagerAlgorithmTypeId (const TypeId &typeId);

  /**
   * \brief Set an attribute for the GNB BWP Manager, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbBwpManagerAlgorithmAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set the TypeId of the UE BWP Manager. Works only before it is created.
   * \param typeId Type of the object
   *
   * \see BwpManagerAlgorithm
   */
  void SetUeBwpManagerAlgorithmTypeId (const TypeId &typeId);

  /*
   * \brief Sets the TypeId of the PhasedArraySpectrumPropagationLossModel to be used
   * \param typeId Type of the object
   */
  void SetPhasedArraySpectrumPropagationLossModelTypeId (const TypeId &typeId);

  /**
   * \brief Set an attribute for the GNB BWP Manager, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUeBwpManagerAlgorithmAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set an attribute for the PhasedArraySpectrumPropagationLossModel before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetPhasedArraySpectrumPropagationLossModelAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the Channel Condition model, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetChannelConditionModelAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the pathloss model, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetPathlossAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the GNB DL AMC, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   *
   * \see NrAmc
   */
  void SetGnbDlAmcAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the GNB UL AMC, before it is created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   *
   * \see NrAmc
   */
  void SetGnbUlAmcAttribute (const std::string &n, const AttributeValue &v);

  /*
   * \brief Sets beam managers attribute.
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbBeamManagerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set the TypeId of the beam manager
   * \param typeId the type of the object
   *
   */
  void SetGnbBeamManagerTypeId (const TypeId &typeId);

  /**
   * \brief Set the ErrorModel for UL AMC and UE spectrum at the same time
   * \param errorModelTypeId The TypeId of the error model
   *
   * Equivalent to the calls to
   *
   * * SetGnbUlAmcAttribute ("ErrorModelType", ....
   * * SetGnbSpectrumAttribute ("ErrorModelType", ...
   *
   * \see NrErrorModel
   * \see NrEesmIrT2
   * \see NrEesmIrT1
   * \see NrEesmCcT1
   * \see NrEesmCcT2
   * \see NrLteMiErrorModel
   *
   */
  void SetUlErrorModel (const std::string & errorModelTypeId);

  /**
   * \brief Set the ErrorModel for DL AMC and GNB spectrum at the same time
   * \param errorModelTypeId The TypeId of the error model
   *
   * Equivalent to the calls to
   *
   * * SetGnbDlAmcAttribute ("ErrorModelType", ....
   * * SetUeSpectrumAttribute ("ErrorModelType", ...
   *
   * \see NrErrorModel
   * \see NrEesmIrT2
   * \see NrEesmIrT1
   * \see NrEesmCcT1
   * \see NrEesmCcT2
   * \see NrLteMiErrorModel
   */
  void SetDlErrorModel (const std::string & errorModelTypeId);

  /**
   * \brief Enable DL DATA PHY traces
   */
  void EnableDlDataPhyTraces ();

  /**
   * \brief Enable DL CTRL PHY traces
   */
  void EnableDlCtrlPhyTraces ();

  /**
   * \brief Enable UL PHY traces
   */
  void EnableUlPhyTraces ();

  /**
   * \brief Get the phy traces object
   *
   * \return The NrPhyRxTrace object to write PHY traces
   */
  Ptr<NrPhyRxTrace> GetPhyRxTrace (void);

  /**
   * \brief Enable gNB packet count trace
   */
  void EnableGnbPacketCountTrace ();

  /**
   * \brief Enable UE packet count trace
   *
   */
  void EnableUePacketCountTrace ();

  /**
   * \brief Enable transport block trace
   *
   * At the time of writing this documentation
   * this method only connect the ReportDownlinkTbSize
   * of NrUePhy.
   */
  void EnableTransportBlockTrace ();

  /**
   * \brief Enable gNB PHY CTRL TX and RX traces
   */
  void EnableGnbPhyCtrlMsgsTraces (void);

  /**
   * \brief Enable UE PHY CTRL TX and RX traces
   */
  void EnableUePhyCtrlMsgsTraces (void);

  /**
   * \brief Enable gNB MAC CTRL TX and RX traces
   */
  void EnableGnbMacCtrlMsgsTraces (void);

  /**
   * \brief Enable UE MAC CTRL TX and RX traces
   */
  void EnableUeMacCtrlMsgsTraces (void);

  /**
   * \brief Get the RLC stats calculator object
   *
   * \return The NrBearerStatsCalculator stats calculator object to write RLC traces
   */
  Ptr<NrBearerStatsCalculator> GetRlcStatsCalculator (void);

  /**
   * \brief Enable RLC simple traces (DL RLC TX, DL RLC RX, UL DL TX, UL DL RX)
   */
  void EnableRlcSimpleTraces (void);

  /**
   * \brief Enable PDCP traces (DL PDCP TX, DL PDCP RX, UL PDCP TX, UL PDCP RX)
   */
  void EnablePdcpSimpleTraces (void);

  /**
   * \brief Enable RLC calculator and end-to-end RCL traces to file
   */
  void EnableRlcE2eTraces (void);

  /**
   * \brief Enable PDCP calculator and end-to-end PDCP traces to file
   */
  void EnablePdcpE2eTraces (void);

  /**
   * \brief Get the PDCP stats calculator object
   *
   * \return The NrBearerStatsCalculator stats calculator object to write PDCP traces
   */
  Ptr<NrBearerStatsCalculator> GetPdcpStatsCalculator (void);

  /**
   * Enable trace sinks for DL MAC layer scheduling.
   */
  void EnableDlMacSchedTraces (void);

  /**
   * Enable trace sinks for UL MAC layer scheduling.
   */
  void EnableUlMacSchedTraces (void);

  /**
   * \brief Enable trace sinks for DL and UL pathloss
   */
  void EnablePathlossTraces ();

  /**
    * Assign a fixed random variable stream number to the random variables used.
    *
    * The InstallGnbDevice() or InstallUeDevice method should have previously
    * been called by the user on the given devices.
    *
    *
    * \param c NetDeviceContainer of the set of net devices for which the
    *          LteNetDevice should be modified to use a fixed stream
    * \param stream first stream index to use
    * \return the number of stream indices (possibly zero) that have been assigned
   */
   int64_t AssignStreams (NetDeviceContainer c, int64_t stream);

private:

   /**
    * Assign a fixed random variable stream number to the channel and propagation
    * objects. This function will save the objects to which it has assigned stream
    * to not overwrite assignment, because these objects are shared by gNB and UE
    * devices.
    *
    * The InstallGnbDevice() or InstallUeDevice method should have previously
    * been called by the user on the given devices.
    *
    *
    * \param c NetDeviceContainer of the set of net devices for which the
    *          LteNetDevice should be modified to use a fixed stream
    * \param stream first stream index to use
    * \return the number of stream indices (possibly zero) that have been assigned
    */
   int64_t DoAssignStreamsToChannelObjects (Ptr<NrSpectrumPhy> phy, int64_t currentStream);

  /**
   *  \brief The actual function to trigger a manual bearer de-activation
   *  \param ueDevice the UE on which bearer to be de-activated must be of the type LteUeNetDevice
   *  \param enbDevice eNB, must be of the type LteEnbNetDevice
   *  \param bearerId Bearer Identity which is to be de-activated
   *
   *  This method is normally scheduled by DeActivateDedicatedEpsBearer() to run at a specific
   *  time when a manual bearer de-activation is desired by the simulation user.
   */
  void DoDeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId);

  Ptr<NrGnbPhy> CreateGnbPhy (const Ptr<Node> &n, const std::unique_ptr<BandwidthPartInfo> &bwp,
                                  const Ptr<NrGnbNetDevice> &dev,
                                  const NrSpectrumPhy::NrPhyRxCtrlEndOkCallback &phyEndCtrlCallback,
                                  uint8_t numberOfPanels);
  Ptr<NrMacScheduler> CreateGnbSched ();
  Ptr<NrGnbMac> CreateGnbMac ();

  Ptr<NrUeMac> CreateUeMac () const;
  Ptr<NrUePhy> CreateUePhy (const Ptr<Node> &n, const std::unique_ptr<BandwidthPartInfo> &bwp,
                                const Ptr<NrUeNetDevice> &dev,
                                const NrSpectrumPhy::NrPhyRxCtrlEndOkCallback &phyRxCtrlCallback,
                                uint8_t numberOfPanels);

  Ptr<NetDevice> InstallSingleUeDevice (const Ptr<Node> &n,
                                        const std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> allBwps,
                                        uint8_t numberOfPanels);
  Ptr<NetDevice> InstallSingleGnbDevice (const Ptr<Node> &n,
                                         const std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> allBwps,
                                         uint8_t numberOfPanels);
  void AttachToClosestEnb (Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices);

  std::map<uint8_t, ComponentCarrier> GetBandwidthPartMap ();

  ObjectFactory m_gnbNetDeviceFactory;  //!< NetDevice factory for gnb
  ObjectFactory m_ueNetDeviceFactory;   //!< NetDevice factory for ue
  ObjectFactory m_channelFactory;       //!< Channel factory
  ObjectFactory m_ueMacFactory;         //!< UE MAC factory
  ObjectFactory m_gnbMacFactory;        //!< GNB MAC factory
  ObjectFactory m_ueSpectrumFactory;    //!< UE Spectrum factory
  ObjectFactory m_gnbSpectrumFactory;   //!< GNB spectrum factory
  ObjectFactory m_uePhyFactory;         //!< UE PHY factory
  ObjectFactory m_gnbPhyFactory;        //!< GNB PHY factory
  ObjectFactory m_ueChannelAccessManagerFactory; //!< UE Channel access manager factory
  ObjectFactory m_gnbChannelAccessManagerFactory; //!< GNB Channel access manager factory
  ObjectFactory m_schedFactory;         //!< Scheduler factory
  ObjectFactory m_ueAntennaFactory;     //!< UE antenna factory
  ObjectFactory m_gnbAntennaFactory;    //!< UE antenna factory
  ObjectFactory m_gnbBwpManagerAlgoFactory;//!< BWP manager algorithm factory
  ObjectFactory m_ueBwpManagerAlgoFactory;//!< BWP manager algorithm factory
  ObjectFactory m_channelConditionModelFactory; //!< Channel condition factory
  ObjectFactory m_spectrumPropagationFactory; //!< Spectrum Factory
  ObjectFactory m_pathlossModelFactory;  //!< Pathloss factory
  ObjectFactory m_gnbDlAmcFactory;       //!< DL AMC factory
  ObjectFactory m_gnbUlAmcFactory;       //!< UL AMC factory
  ObjectFactory m_gnbBeamManagerFactory; //!< gNb Beam manager factory
  ObjectFactory m_ueBeamManagerFactory;  //!< UE beam manager factory

  uint64_t m_imsiCounter {0};    //!< Imsi counter
  uint16_t m_cellIdCounter {1};  //!< CellId Counter

  Ptr<EpcHelper> m_epcHelper {nullptr};                           //!< Ptr to the EPC helper (optional)
  Ptr<BeamformingHelperBase> m_beamformingHelper {nullptr}; //!< Ptr to the beamforming helper

  bool m_harqEnabled {false};
  bool m_snrTest {false};

  Ptr<NrPhyRxTrace> m_phyStats; //!< Pointer to the PhyRx stats
  Ptr<NrMacRxTrace> m_macStats; //!< Pointer to the MacRx stats

  NrBearerStatsConnector m_radioBearerStatsConnectorSimpleTraces; //!< RLC and PDCP statistics connector for simple file statistics
  NrBearerStatsConnector m_radioBearerStatsConnectorCalculator; //!< RLC and PDCP statistics connector for complex calculator statistics

  std::map<uint8_t, ComponentCarrier> m_componentCarrierPhyParams; //!< component carrier map
  std::vector< Ptr <Object> > m_channelObjectsWithAssignedStreams; //!< channel and propagation objects to which NrHelper has assigned streams in order to avoid double assignments
  Ptr<NrMacSchedulingStats> m_macSchedStats; //!<< Pointer to NrMacStatsCalculator

  //NR Sidelink code and additions
public:
  /**
   * \brief Set the bandwidth part manager TypeId. Works only before it is created.
   * \param typeId Bandwidth part manager type id
   *
   * \see ns3::BwpManagerUe
   * \see ns3::NrSlBwpManagerUe
   */
  void SetBwpManagerTypeId (const TypeId &typeId);

private:
  ObjectFactory m_bwpManagerFactory;       //!< Bandwidth part manager factory
};

}

#endif /* NR_HELPER_H */

