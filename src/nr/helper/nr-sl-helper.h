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


#ifndef NR_SL_HELPER_H
#define NR_SL_HELPER_H

#include <ns3/object.h>
#include <ns3/net-device-container.h>
#include <ns3/lte-rrc-sap.h>

namespace ns3 {

class NrUeNetDevice;
class NrAmc;
class NrPointToPointEpcHelper;
class LteSlTft;
class NrSlUeMacScheduler;


class NrSlHelper : public Object
{

public:
  /**
   * \brief Constructor
   */
  NrSlHelper (void);
  /**
   * \brief Destructor
   */
  virtual ~NrSlHelper (void);
  /**
   * \brief \c GetTypeId, inherited from Object
   *
   * \returns The \c TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Prepare UE for Sidelink
   *
   * \param c The \c NetDeviceContainer
   * \param slBwpIds The container of Sidelink BWP ids
   */
  void PrepareUeForSidelink (NetDeviceContainer c, const std::set <uint8_t> &slBwpIds);

  /**
   * \brief Install NR sidelink pre-configuration in the UEs expected to use
   *        sidelink.
   *
   * \param c The \c NetDeviceContainer
   * \param preConfig The <tt> struct LteRrcSap::SidelinkPreconfigNr </tt>
   */
  void InstallNrSlPreConfiguration (NetDeviceContainer c, const LteRrcSap::SidelinkPreconfigNr preConfig);
  /**
   * \brief Set UE sidelink AMC attribute
   *
   * \todo: We might need to break this method
   * for in-coverage simulation. That is,
   * take out all the UE configurations,
   * which are independent of UE being
   * in-coverage or out of coverage.
   *
   * \param n The attribute name
   * \param v The attribute value
   */
  void SetUeSlAmcAttribute (const std::string &n, const AttributeValue &v);
  /**
   * \brief Set the \c ErrorModel for SL AMC and UE spectrum at the same time
   *
   * \param errorModelTypeId The TypeId of the error model
   *
   * Equivalent to the calls to
   *
   * <tt> SetUeSlAmcAttribute ("ErrorModelType", ....
   * SetUeSpectrumAttribute ("SlErrorModelType", ... </tt>
   *
   */
  void SetSlErrorModel (const std::string &errorModelTypeId);
  /**
   * \brief Set EPC helper
   *
   * \param epcHelper Ptr of type NrPointToPointEpcHelper
   */
  void SetEpcHelper (const Ptr<NrPointToPointEpcHelper> &epcHelper);
  /**
   * \brief Schedule the activation of a NR sidelink bearer
   *
   * \param activationTime The time to setup the sidelink bearer
   * \param ues The list of UEs where the bearer must be activated
   * \param tft The traffic flow template for the bearer (i.e. multicast address and group)
   */
  void ActivateNrSlBearer (Time activationTime, NetDeviceContainer ues, const Ptr<LteSlTft> tft);
  /**
   * \brief Activation of a sidelink bearer
   *
   * \param ues The list of UEs where the bearer must be activated
   * \param tft The traffic flow template for the bearer (i.e. multicast address and group)
   */
  void DoActivateNrSlBearer (NetDeviceContainer ues, const Ptr<LteSlTft> tft);
  /**
   * \brief Set the NR SL Scheduler TypeId. Works only before it is created.
   * \param typeId The NR SL scheduler type
   *
   * \see NrSlUeMacSchedulerSimple
   */
  void SetNrSlSchedulerTypeId (const TypeId &typeId);
  /**
   * \brief Set UE sidelink scheduler attribute
   * \param n The attribute name
   * \param v The attribute value
   */
  void SetUeSlSchedulerAttribute (const std::string &n, const AttributeValue &v);
  /**
   * \brief Create NR Sidelink UE scheduler from the object factory of the scheduler
   *
   * \return The pointer to a newly created object of the scheduler
   */
  Ptr<NrSlUeMacScheduler> CreateNrSlUeSched ();
  /**
   * \brief Get the length of the physical Sidelink pool based on
   *        SL bitmap length, TDD pattern length, and the number of UL slots
   *        in the TDD pattern.
   * \param slBitmapLen The SL bitmap length
   * \param tddPatternLen The TDD pattern length
   * \param numUlTddPattern the number of UL slots in the TDD pattern
   * \return The resultant length of the physical Sidelink pool in slots
   */
  static uint16_t GetPhySlPoolLength (uint16_t slBitmapLen, uint16_t tddPatternLen, uint16_t numUlTddPattern);
  /**
    * \brief Assign a fixed random variable stream number to the random variables used.
    *
    * The InstallUeDevice and PrepareUeForSidelink method should have previously
    * been called by the user on the given devices.
    *
    *
    * \param c NetDeviceContainer of the NR SL UE NetDevices for which
    *          we need to fix the stream
    * \param stream first stream index to use
    * \return the number of stream indices (possibly zero) that have been assigned
    */
   int64_t AssignStreams (NetDeviceContainer c, int64_t stream);

protected:
  /**
   * \brief \c DoDispose method inherited from \c Object
   */
  virtual void DoDispose (void) override;

private:
  /**
   * \brief Configure the UE parameters
   *
   * This method is used to configure the UE parameters,
   * which can not be set via RRC.
   *
   * \param dev The NrUeNetDevice
   * \param freqCommon The <tt> struct SlFreqConfigCommonNr </tt> to retrieve
   *        SL BWP related configuration
   * \param general The <tt> struct SlPreconfigGeneralNr </tt> to retrieve
   *        general parameters for a BWP, e.g., TDD pattern
   * \return true if the user indented BWP for SL is configured, false otherwise
   */
  bool ConfigUeParams (const Ptr<NrUeNetDevice> &dev,
                       const LteRrcSap::SlFreqConfigCommonNr &freqCommon,
                       const LteRrcSap::SlPreconfigGeneralNr &general);

  /**
   * \brief Prepare Single UE for Sidelink
   *
   * \param nrUeDev The Ptr to NR Ue netdevice
   * \param slBwpIds The container of Sidelink BWP ids
   */
  void PrepareSingleUeForSidelink (Ptr<NrUeNetDevice> nrUeDev, const std::set <uint8_t> &slBwpIds);

  /**
   * brief Create UE SL AMC object from UE SL AMC factory
   *
   * \returns Ptr of type \c NrAmc
   */
  Ptr<NrAmc> CreateUeSlAmc () const;

  ObjectFactory m_ueSlAmcFactory;           //!< UE SL AMC Object factory
  ObjectFactory m_ueSlSchedulerFactory;            //!< UE SL scheduler Object factory
  Ptr<NrPointToPointEpcHelper> m_epcHelper; //!< the EPC helper

};

}

#endif /* NR_SL_HELPER_H */

