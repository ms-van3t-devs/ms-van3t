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

#ifndef NR_PHY_SAP_H
#define NR_PHY_SAP_H

#include <ns3/packet-burst.h>
#include <ns3/nr-phy-mac-common.h>
#include <ns3/nr-mac-sched-sap.h>
#include <ns3/nr-control-messages.h>
#include "beam-conf-id.h"

namespace ns3 {

class NrControlMessage;

/**
 * \ingroup gnb-phy
 * \ingroup gnb-mac
 * \ingroup ue-phy
 * \ingroup ue-mac
 *
 * \brief SAP interface between the MAC and the PHY
 *
 * The API between the MAC and the PHY classes, for UE and GNB, is defined in
 * this class. The direction is from the MAC to the PHY (i.e., the MAC will
 * have a pointer of this class, that points to a valid instance of the PHY).
 *
 * As a general rule, no caching is allowed for the values returned by any
 * Get* method, becaue those values can change dynamically.
 */
class NrPhySapProvider
{
public:
  /**
   * \brief ~NrPhySapProvider
   */
  virtual ~NrPhySapProvider ();

  /**
   * \brief Send a Mac PDU
   * \param p PDU
   * \param sfn SFN
   * \param symStart symbol inside the SFN
   * \param streamId The stream id through which this pkt would be transmitted
   *
   * The MAC sends to the PHY a MAC PDU, represented by the packet p. The PDU
   * MUST have a LteRadioBearerTag and a NrMacPduHeader.
   */
  virtual void SendMacPdu (const Ptr<Packet> &p, const SfnSf & sfn, uint8_t symStart, uint8_t streamId) = 0;

  /**
   * \brief Send a control message
   * \param msg the message to send
   *
   * The MAC sends to the PHY a control message. The PHY will take care of
   * considering the MAC-TO-PHY delay.
   */
  virtual void SendControlMessage (Ptr<NrControlMessage> msg) = 0;

  /**
   * \brief Send the RACH preamble
   * \param PreambleId the ID of the preamble
   * \param Rnti the RNTI
   */
  virtual void SendRachPreamble (uint8_t PreambleId, uint8_t Rnti) = 0;

  /**
   * \brief Set a SlotAllocInfo inside the PHY allocations
   * \param slotAllocInfo the allocation
   *
   * Called by the MAC to install in the PHY the allocation that has been
   * prepared.
   */
  virtual void SetSlotAllocInfo (const SlotAllocInfo &slotAllocInfo) = 0;

  /**
   * \brief Notify PHY about the successful RRC connection
   * establishment.
   */
  virtual void NotifyConnectionSuccessful () = 0;

  /**
   * \brief Get the beam conf ID from the RNTI specified. Not in any standard.
   * \param rnti RNTI of the user
   * \return Beam conf ID of the user
   *
   * The MAC asks for the BeamConfId of the specified used.
   */
  virtual BeamConfId GetBeamConfId (uint8_t rnti) const = 0;

  /**
   * \brief Retrieve the spectrum model used by the PHY layer.
   * \return the SpectrumModel
   *
   * It is used to calculate the CQI. In the future, this method may be removed
   * if the CQI calculation is done in the PHY layer, just reporting to MAC
   * its value.
   */
  virtual Ptr<const SpectrumModel> GetSpectrumModel () = 0;

  /**
   * \brief Retrieve the bandwidth part id
   * \return The Bwp id of the PHY
   */
  virtual uint16_t GetBwpId () const = 0;

  /**
   * \brief Retrieve the cell id
   * \return The cell id of the PHY
   */
  virtual uint16_t GetCellId () const = 0;

  /**
   * \brief Retrieve the number of symbols in one slot
   * \return the number of symbols in one slot (it is an attribute in the PHY,
   * so it can be changed dynamically -- don't store the value)
   */
  virtual uint32_t GetSymbolsPerSlot () const = 0;

  /**
   * \brief Retrieve the slot period
   * \return the slot period (don't store the value as it depends on the numerology)
   */
  virtual Time GetSlotPeriod () const = 0;

  /**
   * \brief Retrieve the number of resource blocks
   * \return Get the number of resource blocks configured
   */
  virtual uint32_t GetRbNum () const = 0;

};

/**
 * \brief SAP interface between the ENB PHY and the ENB MAC
 * \ingroup gnb-phy
 * \ingroup gnb-mac
 *
 * This SAP is normally used so that PHY can send to MAC indications
 * and providing to MAC some information. The relationship between MAC and PHY
 * is that PHY is service provider to MAC, and MAC is user.
 * Exceptionally, PHY can also request some information from MAC through this
 * interface, such as GetNumRbPerRbg.
 *
 * As a general rule, no caching is allowed for the values returned by any
 * Get* method, becaue those values can change dynamically.
 */
class NrGnbPhySapUser
{
public:
  /**
   * \brief ~NrGnbPhySapUser
   */
  virtual ~NrGnbPhySapUser ()
  {
  }

  /**
   * \brief Notify the MAC of the reception of a new PHY-PDU
   *
   * \param p
   */
  virtual void ReceivePhyPdu (Ptr<Packet> p) = 0;

  /**
   * \brief Receive SendLteControlMessage (PDCCH map, CQI feedbacks) using the ideal control channel
   * \param msg the Ideal Control Message to receive
   */
  virtual void ReceiveControlMessage (Ptr<NrControlMessage> msg) = 0;

  /**
   * \brief Set the current Sfn. The state machine has advanced by one slot
   * \param sfn The current sfn
   */
  virtual void SetCurrentSfn (const SfnSf &sfn) = 0;

  /**
   * \brief Trigger MAC layer to generate a DL slot for the SfnSf indicated
   * \param sfn Slot to fill with DL scheduling decisions
   * \param slotType Slot type requested (DL, S, F)
   */
  virtual void SlotDlIndication (const SfnSf &sfn, LteNrTddSlotType slotType) = 0;

  /**
   * \brief Trigger MAC layer to generate an UL slot for the SfnSf indicated
   * \param sfn Slot to fill with UL scheduling decisions
   * \param slotType Slot type requested (UL, S, F)
   */
  virtual void SlotUlIndication (const SfnSf &sfn, LteNrTddSlotType slotType) = 0;

  // We do a DL and then manually add an UL CTRL if it's an S slot.
  // virtual void SlotSIndication (const SfnSf &sfn) = 0;
  // We do UL and then DL to model an F slot.
  // virtual void SlotFIndication (const SfnSf &sfn) = 0;

  /**
   * \brief Returns to MAC level the UL-CQI evaluated
   * \param ulcqi the UL-CQI (see FF MAC API 4.3.29)
   */
  virtual void UlCqiReport (NrMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi) = 0;

  /**
   * \brief Notify the reception of a RACH preamble on the PRACH
   *
   * \param raId the ID of the preamble
   */
  virtual void ReceiveRachPreamble (uint32_t raId) = 0;

  /**
   * \brief Notify the HARQ on the UL tranmission status
   *
   * \param params Params
   */
  virtual void UlHarqFeedback (UlHarqInfo params) = 0;

  /**
   * \brief Called by the PHY to notify MAC that beam has changed. Not in any standard
   * \param beamConfId the new beam ID
   * \param rnti the RNTI of the user
   */
  virtual void BeamChangeReport (BeamConfId beamConfId, uint8_t rnti) = 0;

  /**
   * \brief PHY requests information from MAC.
   * While MAC normally act as user of PHY services, in this case
   * exceptionally MAC provides information/service to PHY.
   * \return number of resource block per resource block group
   */
  virtual uint32_t GetNumRbPerRbg () const = 0;

  /**
   * \brief Retrieve a dci for a DL CTRL allocation
   * \return a pointer to a dci that contains a DL CTRL allocation
   */
  virtual std::shared_ptr<DciInfoElementTdma> GetDlCtrlDci () const = 0;

  /**
   * \brief Retrieve a dci for a UL CTRL allocation
   * \return a pointer to a dci that contains a UL CTRL allocation
   */
  virtual std::shared_ptr<DciInfoElementTdma> GetUlCtrlDci () const = 0;
};

/**
 * \brief SAP interface between the UE PHY and the UE MAC
 *
 * \ingroup ue-phy
 * \ingroup ue-mac
 *
 * This interface specify the interaction between the UE PHY (that will use
 * this interface) and the UE MAC, that will answer.
 */
class NrUePhySapUser
{
public:
  /**
   * \brief ~NrUePhySapUser
   */
  virtual ~NrUePhySapUser ()
  {
  }

  /**
   * \brief Notify the MAC of the reception of a new PHY-PDU
   *
   * \param p
   */
  virtual void ReceivePhyPdu (Ptr<Packet> p) = 0;

  /**
   * \brief Receive SendLteControlMessage (PDCCH map, CQI feedbacks) using the ideal control channel
   * \param msg the Ideal Control Message to receive
   */
  virtual void ReceiveControlMessage (Ptr<NrControlMessage> msg) = 0;

  /**
   * \brief Trigger the indication of a new slot for the MAC
   * \param s SfnSf
   */
  virtual void SlotIndication (SfnSf s) = 0;

  /**
   * \brief Retrieve the number of HARQ processes configured
   * \return the number of the configured HARQ processes.
   */
  virtual uint8_t GetNumHarqProcess () const = 0;
};

}

#endif /* NR_PHY_SAP_H */
