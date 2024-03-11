/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only AND NIST-Software

#ifndef NR_SL_UE_MAC_H
#define NR_SL_UE_MAC_H

#include "nr-sl-phy-mac-common.h"
#include "nr-sl-ue-mac-scheduler.h"
#include "nr-sl-ue-phy-sap.h"
#include "nr-ue-mac.h"

#include <ns3/nr-sl-comm-resource-pool.h>
#include <ns3/nr-sl-mac-sap.h>
#include <ns3/nr-sl-ue-cmac-sap.h>

#include <map>
#include <unordered_map>
#include <unordered_set>

namespace ns3
{

class NrSlUeMacHarq;
class NrSlUeMacScheduler;

/**
 * \ingroup ue-mac
 * \brief Sidelink specialization of class `NrUeMac`
 *
 * This class implements sensing and HARQ procedures for sidelink as
 * standardized in TS 38.321.  One instance of this class is paired with
 * each instance of a `NrSlBwpManagerUe`.  Similar to the parent `NrUeMac`,
 * this class has interfaces defined with the RLC and RRC layers above and
 * the PHY below.  Scheduling operations are handled in a separate class.
 * A number of attributes are defined to parameterize the sensing operation,
 * and trace sources exporting scheduling and PDU receptions are available.
 */
class NrSlUeMac : public NrUeMac
{
    /// allow MemberNrSlMacSapProvider<NrSlUeMac> class friend access
    friend class MemberNrSlMacSapProvider<NrSlUeMac>;
    /// allow MemberNrSlUeCmacSapProvider<NrSlUeMac> class friend access
    friend class MemberNrSlUeCmacSapProvider<NrSlUeMac>;
    /// allow MemberNrSlUePhySapUser<NrSlUeMac> class friend access
    friend class MemberNrSlUePhySapUser<NrSlUeMac>;

  public:
    /**
     * \brief Get the Type id
     * \return the type id
     */
    static TypeId GetTypeId();

    /**
     * \brief NrSlUeMac constructor
     */
    NrSlUeMac();

    // SCHED API primitive for NR Sidelink
    // From FAPI 2.0.0 Small Cell Forum originated LTE MAC scheduler API
    /**
     * \brief Passes the SL scheduling decision to the NrSlUeMac
     *
     * \param slotAllocList The slot allocation list from the scheduler
     */
    void SchedNrSlConfigInd(const std::set<NrSlSlotAlloc>& slotAllocList);

    // CSCHED API primitive for NR Sidelink
    /**
     * \brief Send the confirmation about the successful configuration of LC
     *        to the UE MAC.
     * \param lcg The logical group
     * \param lcId The Logical Channel id
     */
    void CschedNrSlLcConfigCnf(uint8_t lcg, uint8_t lcId);

  public:
    /**
     * TracedCallback signature for Receive with Tx RNTI
     *
     * \param [in] imsi The IMSI.
     * \param [in] rnti C-RNTI scheduled.
     * \param [in] txRnti C-RNTI of the transmitting UE.
     * \param [in] lcid The logical channel id corresponding to
     *             the sending RLC instance.
     * \param [in] bytes The packet size.
     * \param [in] delay Delay since sender timestamp, in sec.
     */
    typedef void (*ReceiveWithTxRntiTracedCallback)(uint64_t imsi,
                                                    uint16_t rnti,
                                                    uint16_t txRnti,
                                                    uint8_t lcid,
                                                    uint32_t bytes,
                                                    double delay);
    // Comparator function to sort pairs
    // according to second value
    /**
     * \brief Comparator function to sort pairs according to second value
     * \param a The first pair
     * \param b The second pair
     * \return returns true if second value of first pair is less than the second
     *         pair second value, false otherwise.
     */
    static bool CompareSecond(std::pair<uint32_t, uint8_t>& a, std::pair<uint32_t, uint8_t>& b);

    /**
     * \brief Get the NR Sidelik MAC SAP offered by MAC to RLC
     *
     * \return the NR Sidelik MAC SAP provider interface offered by
     *          MAC to RLC
     */
    NrSlMacSapProvider* GetNrSlMacSapProvider();

    /**
     * \brief Set the NR Sidelik MAC SAP offered by this RLC
     *
     * \param s the NR Sidelik MAC SAP user interface offered to the
     *          MAC by RLC
     */
    void SetNrSlMacSapUser(NrSlMacSapUser* s);

    /**
     * \brief Get the NR Sidelik UE Control MAC SAP offered by MAC to RRC
     *
     * \return the NR Sidelik UE Control MAC SAP provider interface offered by
     *         MAC to RRC
     */
    NrSlUeCmacSapProvider* GetNrSlUeCmacSapProvider();

    /**
     * \brief Set the NR Sidelik UE Control MAC SAP offered by RRC to MAC
     *
     * \param s the NR Sidelik UE Control MAC SAP user interface offered to the
     *          MAC by RRC
     */
    void SetNrSlUeCmacSapUser(NrSlUeCmacSapUser* s);

    /**
     * \brief Get the NR Sidelik UE PHY SAP offered by UE MAC to UE PHY
     *
     * \return the NR Sidelik UE PHY SAP user interface offered by
     *         UE MAC to UE PHY
     */
    NrSlUePhySapUser* GetNrSlUePhySapUser();

    /**
     * \brief Set the NR Sidelik UE PHY SAP offered by UE PHY to UE MAC
     *
     * \param s the NR Sidelik UE PHY SAP provider interface offered to the
     *          UE MAC by UE PHY
     */
    void SetNrSlUePhySapProvider(NrSlUePhySapProvider* s);

    /**
     * \brief Enable sensing for NR Sidelink resource selection
     * \param enableSensing if True, sensing is used for resource selection. Otherwise, random
     * selection
     */
    void EnableSensing(bool enableSensing);

    /**
     * \brief Enable blind retransmissions for NR Sidelink
     * \param enableBlindReTx if True, blind re-transmissions, i.e.,
     *        retransmissions are done without HARQ feedback. If it is false,
     *        retransmissions are performed based on HARQ feedback from the
     *        receiving UE. The false value of this flag also means that a
     *        transmitting UE must indicate in the SCI message to a receiving
     *        UE to send the feedback.
     */
    void EnableBlindReTx(bool enableBlindReTx);

    /**
     * \brief Set the t_proc0 used for sensing window
     * \param tprocZero t_proc0 in number of slots
     */
    void SetTproc0(uint8_t tproc0);

    /**
     * \brief Get the t_proc0 used for sensing window
     * \return t_proc0 in number of slots
     */
    uint8_t GetTproc0() const;

    /**
     * \brief Set T1
     * \param t1 The start of the selection window in physical slots,
     *           accounting for physical layer processing delay.
     */
    void SetT1(uint8_t t1);

    /**
     * \brief Get T1
     * \return T1 The start of the selection window in physical slots, accounting
     *            for physical layer processing delay.
     */
    uint8_t GetT1() const;

    /**
     * \brief Set T2
     * \param t2 The end of the selection window in physical slots.
     */
    void SetT2(uint16_t t2);

    /**
     * \brief Get T2
     * \return T2 The end of the selection window in physical slots.
     */
    uint16_t GetT2() const;

    /**
     * \brief Set the pool id of the active pool
     * \param poolId The pool id
     */
    void SetSlActivePoolId(uint16_t poolId);

    /**
     * \brief Get the pool id of the active pool
     * \return the pool id
     */
    uint16_t GetSlActivePoolId() const;

    /**
     * \brief Set Reservation Period for NR Sidelink
     *
     * Only the standard compliant values, including their intermediate values
     * could be set. TS38.321 sec 5.22.1.1 instructs to select one of the
     * allowed values configured by RRC in sl-ResourceReservePeriodList and
     * set the resource reservation interval with the selected value. In the
     * simulator we made it an attribute of UE MAC so we can change it on
     * run time. Remember, this attribute value must be in the list of
     * resource reservations configured using pre-configuration. UE MAC calls
     * \link NrSlCommResourcePool::ValidateResvPeriod \endlink to validate its
     * value and also that this value is multiple of the length of the physical
     * sidelink pool (i.e., the resultant bitmap after applying SL bitmap over
     * the TDD pattern).
     *
     * \param rsvpInMs The reservation period in the milliseconds
     *
     * \see LteRrcSap::SlResourceReservePeriod
     */
    void SetReservationPeriod(const Time& rsvpInMs);

    /**
     * \brief Get Reservation Period for NR Sidelink
     *
     * \return The Reservation Period for NR Sidelink
     */
    Time GetReservationPeriod() const;

    /**
     * \brief Sets the number of Sidelink processes of Sidelink HARQ
     * \param numSidelinkProcess the maximum number of Sidelink processes
     */
    void SetNumSidelinkProcess(uint8_t numSidelinkProcess);

    /**
     * \brief Gets the number of Sidelink processes of Sidelink HARQ
     * \return The maximum number of Sidelink processes
     */
    uint8_t GetNumSidelinkProcess() const;

    /**
     * \brief Sets a threshold in dBm used for sensing based UE autonomous resource selection
     * \param thresRsrp The RSRP threshold in dBm
     */
    void SetSlThresPsschRsrp(int thresRsrp);

    /**
     * \brief Gets a threshold in dBm used for sensing based UE autonomous resource selection
     * \return The RSRP threshold in dBm
     */
    int GetSlThresPsschRsrp() const;

    /**
     * \brief Sets the percentage threshold to indicate the minimum number of
     *        candidate single-slot resources to be selected using sensing
     *        procedure.
     * \param percentage The percentage, e.g., 1, 2, 3,.. and so on.
     */
    void SetResourcePercentage(uint8_t percentage);

    /**
     * \brief Gets the percentage threshold to indicate the minimum number of
     *        candidate single-slot resources to be selected using sensing
     *        procedure.
     * \return The percentage, e.g., 1, 2, 3,.. and so on.
     */
    uint8_t GetResourcePercentage() const;

    /**
     * \brief Return the number of NR SL sub-channels for the active BW pool
     * \return the total number of NR SL sub-channels
     */
    uint8_t GetTotalSubCh() const;

    /**
     * \brief Return the maximum transmission number (including new transmission and
     *        retransmission) for PSSCH.
     *
     * \return The max number of PSSCH transmissions
     */
    uint8_t GetSlMaxTxTransNumPssch() const;

  protected:
    // Inherited
    void DoDispose() override;
    int64_t DoAssignStreams(int64_t stream) override;

    // forwarded from NR SL UE MAC SAP Provider
    /**
     * \brief send an NR SL RLC PDU to the MAC for transmission. This method is
     * to be called as a response to NrSlMacSapUser::NotifyNrSlTxOpportunity
     *
     * \param params NrSlRlcPduParameters
     */
    void DoTransmitNrSlRlcPdu(const NrSlMacSapProvider::NrSlRlcPduParameters& params);
    /**
     * \brief Report the RLC buffer status to the MAC
     *
     * \param params NrSlReportBufferStatusParameters
     */
    void DoReportNrSlBufferStatus(
        const NrSlMacSapProvider::NrSlReportBufferStatusParameters& params);
    /**
     * \brief Fire the trace for SL RLC Reception with Tx Rnti
     *
     * \param p the PDU
     * \param lcid The LCID
     */
    void FireTraceSlRlcRxPduWithTxRnti(const Ptr<Packet> p, uint8_t lcid);

    // forwarded from UE CMAC SAP
    /**
     * \brief Adds a new Logical Channel (LC) used for Sidelink
     *
     * \param slLcInfo The sidelink LC info
     * \param msu The corresponding LteMacSapUser
     */
    void DoAddNrSlLc(const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& slLcInfo,
                     NrSlMacSapUser* msu);
    /**
     * \brief Remove an existing NR Sidelink Logical Channel for a UE in the
     * LteUeComponentCarrierManager
     *
     * \param slLcId is the Sidelink Logical Channel Id
     * \param srcL2Id is the Source L2 ID
     * \param dstL2Id is the Destination L2 ID
     */
    void DoRemoveNrSlLc(uint8_t slLcId, uint32_t srcL2Id, uint32_t dstL2Id);
    /**
     * \brief Reset Nr Sidelink LC map
     *
     */
    void DoResetNrSlLcMap();
    /**
     * \brief Add NR Sidelink communication transmission pool
     *
     * Adds transmission pool for NR Sidelink communication
     *
     * \param txPool The pointer to the NrSlCommResourcePool
     */
    void DoAddNrSlCommTxPool(Ptr<const NrSlCommResourcePool> txPool);
    /**
     * \brief Add NR Sidelink communication reception pool
     *
     * Adds reception pool for NR Sidelink communication
     *
     * \param rxPool The pointer to the NrSlCommResourcePool
     */
    void DoAddNrSlCommRxPool(Ptr<const NrSlCommResourcePool> rxPool);
    /**
     * \brief Set Sidelink probability resource keep
     *
     * \param prob Indicates the probability with which the UE keeps the
     *        current resource when the resource reselection counter reaches zero
     *        for sensing based UE autonomous resource selection (see TS 38.321)
     */
    void DoSetSlProbResoKeep(double prob);
    /**
     * \brief Set the maximum transmission number (including new transmission and
     *        retransmission) for PSSCH.
     *
     * \param maxTxPssch The max number of PSSCH transmissions
     */
    void DoSetSlMaxTxTransNumPssch(uint8_t maxTxPssch);
    /**
     * \brief Set Sidelink source layer 2 id
     *
     * \param srcL2Id The Sidelink layer 2 id of the source
     */
    void DoSetSourceL2Id(uint32_t srcL2Id);
    /**
     * \brief Add NR Sidelink destination layer 2 id for reception
     *
     * \param dstL2Id The Sidelink layer 2 id of the destination to listen to.
     */
    void DoAddNrSlRxDstL2Id(uint32_t dstL2Id);

    // Forwarded from NR SL UE PHY SAP User
    /**
     * \brief Gets the active Sidelink pool id used for transmission for a
     *        destination.
     *
     * \return The active TX pool id
     */
    uint8_t DoGetSlActiveTxPoolId();
    /**
     * \brief Get the list of Sidelink destination for transmission from UE MAC
     * \return A vector holding Sidelink communication destinations and the highest priority value
     * among its LCs
     */
    std::vector<std::pair<uint32_t, uint8_t>> DoGetSlTxDestinations();
    /**
     * \brief Get the list of Sidelink destination for reception from UE MAC
     * \return A vector holding Sidelink communication destinations for reception
     */
    std::unordered_set<uint32_t> DoGetSlRxDestinations();
    /**
     * \brief Receive NR SL PSSCH PHY PDU
     * \param pdu The NR SL PSSCH PHY PDU
     */
    void DoReceivePsschPhyPdu(Ptr<PacketBurst> pdu);
    /**
     * \brief Receive the sensing information from PHY to MAC
     * \param sensingData The sensing data
     */
    void DoReceiveSensingData(SensingData sensingData);

  private:
    void DoSlotIndication(const SfnSf& sfn) override;

    /// Sidelink Logical Channel Identifier
    struct SidelinkLcIdentifier
    {
        uint8_t lcId;     //!< Sidelink LCID
        uint32_t srcL2Id; //!< Source L2 ID
        uint32_t dstL2Id; //!< Destination L2 ID
    };

    /**
     * \brief Less than operator
     *
     * \param l first SidelinkLcIdentifier
     * \param r second SidelinkLcIdentifier
     * \returns true if first SidelinkLcIdentifier parameter values are less than the second
     * SidelinkLcIdentifier parameters"
     */
    friend bool operator<(const SidelinkLcIdentifier& l, const SidelinkLcIdentifier& r)
    {
        return l.lcId < r.lcId || (l.lcId == r.lcId && l.srcL2Id < r.srcL2Id) ||
               (l.lcId == r.lcId && l.srcL2Id == r.srcL2Id && l.dstL2Id < r.dstL2Id);
    }

    /// Sidelink Logical Channel Information
    struct SlLcInfoUeMac
    {
        NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo lcInfo; //!< LC info
        NrSlMacSapUser* macSapUser; //!< SAP pointer to the RLC instance of the LC
    };

    /// NR Sidelink grant Information
    struct NrSlGrantInfo
    {
        uint16_t cReselCounter{
            std::numeric_limits<uint8_t>::max()}; //!< The Cresel counter for the semi-persistently
                                                  //!< scheduled resources as per TS 38.214
        uint8_t slResoReselCounter{
            std::numeric_limits<uint8_t>::max()}; //!< The Sidelink resource re-selection counter
                                                  //!< for the semi-persistently scheduled resources
                                                  //!< as per TS 38.214
        std::set<NrSlSlotAlloc>
            slotAllocations; //!< List of all the slots available for transmission with the pool
        uint8_t prevSlResoReselCounter{
            std::numeric_limits<uint8_t>::max()}; //!< Previously drawn Sidelink resource
                                                  //!< re-selection counter
        uint8_t nrSlHarqId{
            std::numeric_limits<uint8_t>::max()}; //!< The NR SL HARQ process id assigned at the
                                                  //!< time of transmitting new data
        uint8_t nSelected{
            0}; //!< The number of slots selected by the scheduler for first reservation period
        uint8_t tbTxCounter{
            0}; //!< The counter to count the number of time a TB is tx/reTx in a reservation period
    };

    /**
     * \brief Add NR Sidelink destination layer 2 Id
     *
     * Adds destination layer 2 id to the list of destinations.
     * The destinations in this map are sorted w.r.t their
     * logical channel priority. That is, the destination
     * with a logical channel with a highest priority
     * comes first.
     *
     * \param dstL2Id The destination layer 2 ID
     * \param lcPriority The LC priority
     */
    void AddNrSlDstL2Id(uint32_t dstL2Id, uint8_t lcPriority);

    /**
     * \brief NR sidelink slot indication
     * \param sfn
     */
    void DoNrSlSlotIndication(const SfnSf& sfn);
    /**
     * \brief Get NR Sidelink transmit opportunities
     * \param sfn The current system frame, subframe, and slot number. This SfnSf
     *        is aligned with the SfnSf of the physical layer.
     * \return The list of the transmit opportunities (slots) asper the TDD pattern
     *         and the NR SL bitmap
     */
    std::list<NrSlSlotInfo> GetNrSlTxOpportunities(const SfnSf& sfn);
    /**
     * \brief Get the list of the future transmission slots based on sensed data.
     * \param sensedData The data extracted from the sensed SCI 1-A.
     * \return The list of the future transmission slots based on sensed data.
     */
    std::list<SlotSensingData> GetFutSlotsBasedOnSens(SensingData sensedData);
    /**
     * \brief Method to convert the list of NrSlCommResourcePool::SlotInfo to
     *        NrSlSlotInfo
     *
     * NrSlCommResourcePool class exists in the LTE module, therefore, we can not
     * have an object of NR SfnSf class there due to dependency issue. The use of
     * SfnSf class makes our life easier since it already implements the necessary
     * arithmetics of adding slots, constructing new SfnSf given the slot offset,
     * and e.t.c. In this method, we use the slot offset value, which is the
     * offset in number of slots from the current slot to construct the object of
     * SfnSf class.
     *
     * \param sfn The current system frame, subframe, and slot number. This SfnSf
     *        is aligned with the SfnSf of the physical layer.
     * \param slotInfo the list of LTE module compatible slot info
     * \return The list of NR compatible slot info
     */
    std::list<NrSlSlotInfo> GetNrSupportedList(const SfnSf& sfn,
                                               std::list<NrSlCommResourcePool::SlotInfo> slotInfo);
    /**
     * \brief Get the random selection counter
     * \return The randomly selected reselection counter
     *
     * See 38.321 section 5.22.1.1 V16
     *
     * For 50 ms we use the range as per 36.321 section 5.14.1.1
     */
    uint8_t GetRndmReselectionCounter() const;
    /**
     * \brief Get the lower bound for the Sidelink resource re-selection
     *        counter when the resource reservation period is less than
     *        100 ms. It is as per the Change Request (CR) R2-2005970
     *        to TS 38.321.
     * \param pRsrv The resource reservation period
     * \return The lower bound of the range from which Sidelink resource re-selection
     *         counter will be drawn.
     */
    uint8_t GetLoBoundReselCounter(uint16_t pRsrv) const;
    /**
     * \brief Get the upper bound for the Sidelink resource re-selection
     *        counter when the resource reservation period is less than
     *        100 ms. It is as per the Change Request (CR) R2-2005970
     *        to TS 38.321.
     * \param pRsrv The resource reservation period
     * \return The upper bound of the range from which Sidelink resource re-selection
     *         counter will be drawn.
     */
    uint8_t GetUpBoundReselCounter(uint16_t pRsrv) const;
    /**
     * \brief Create grant info
     *
     * \param slotAllocList The slot allocation list passed by a specific
     *        scheduler to NrSlUeMac
     * \return The grant info for a destination based on the scheduler allocation
     *
     * \see NrSlGrantInfo
     */
    NrSlGrantInfo CreateGrantInfo(const std::set<NrSlSlotAlloc>& params);
    /**
     * \brief Filter the Transmit opportunities.
     *
     * Due to the semi-persistent scheduling, after calling the GetNrSlTxOpportunities
     * method, and before asking the scheduler for resources, we need to remove
     * those available slots, which are already part of the existing grant.
     *
     * \param txOppr The list of available slots
     * \return The list of slots which are not used by any existing semi-persistent grant.
     */
    std::list<NrSlSlotInfo> FilterTxOpportunities(std::list<NrSlSlotInfo> txOppr);
    /**
     * \brief Update the sensing window
     * \param sfn The current system frame, subframe, and slot number. This SfnSf
     *        is aligned with the SfnSf of the physical layer.
     * It will remove the sensing data, which lies outside the sensing window length.
     */
    void UpdateSensingWindow(const SfnSf& sfn);
    /**
     * \brief Compute the gaps in slots for the possible retransmissions
     *        indicated by an SCI 1-A.
     * \param sfn The SfnSf of the current slot which will carry the SCI 1-A.
     * \param it The iterator to a slot allocation list, which is pointing to the
     *        first possible retransmission.   *
     * \param slotNumInd The parameter indicating how many gaps we need to compute.
     * \return A vector containing the value gaps in slots for the possible retransmissions
     *        indicated by an SCI 1-A.
     */
    std::vector<uint8_t> ComputeGaps(const SfnSf& sfn,
                                     std::set<NrSlSlotAlloc>::const_iterator it,
                                     uint8_t slotNumInd);
    /**
     * \brief Get the start subchannel index of the possible retransmissions
     *        which would be indicated by an SCI 1-A.
     * \param it The iterator to a slot allocation list, which is pointing to the
     *        first possible retransmission.
     * \param slotNumInd The parameter indicating how many
     *        slots (first TX plus one or two ReTx) an SCI 1-A is indicating.
     * \return A vector containing the starting subchannel index of the possible
     *         retransmissions which would be indicated by an SCI 1-A.
     */
    std::vector<uint8_t> GetStartSbChOfReTx(std::set<NrSlSlotAlloc>::const_iterator it,
                                            uint8_t slotNumInd);

    std::map<SidelinkLcIdentifier, SlLcInfoUeMac>
        m_nrSlLcInfoMap; //!< Sidelink logical channel info map
    NrSlMacSapProvider*
        m_nrSlMacSapProvider; //!< SAP interface to receive calls from the UE RLC instance
    NrSlMacSapUser* m_nrSlMacSapUser{
        nullptr}; //!< SAP interface to call the methods of UE RLC instance
    NrSlUeCmacSapProvider* m_nrSlUeCmacSapProvider; //!< Control SAP interface to receive calls from
                                                    //!< the UE RRC instance
    NrSlUeCmacSapUser* m_nrSlUeCmacSapUser{
        nullptr}; //!< Control SAP interface to call the methods of UE RRC instance
    NrSlUePhySapProvider* m_nrSlUePhySapProvider{
        nullptr}; //!< SAP interface to call the methods of UE PHY instance
    NrSlUePhySapUser*
        m_nrSlUePhySapUser; //!< SAP interface to receive calls from the UE PHY instance
    Ptr<const NrSlCommResourcePool> m_slTxPool; //!< Sidelink communication transmission pools
    Ptr<const NrSlCommResourcePool> m_slRxPool; //!< Sidelink communication reception pools
    std::vector<std::pair<uint32_t, uint8_t>>
        m_sidelinkTxDestinations; //!< vector holding Sidelink communication destinations for
                                  //!< transmission and the highest priority value among its LCs
    std::unordered_set<uint32_t>
        m_sidelinkRxDestinations;  //!< vector holding Sidelink communication destinations for
                                   //!< reception
    bool m_enableSensing{false};   //!< Flag to enable NR Sidelink resource selection based on
                                   //!< sensing; otherwise, use random selection
    bool m_enableBlindReTx{false}; //!< Flag to enable blind retransmissions for NR Sidelink
    uint8_t m_tproc0{0};           //!< t_proc0 in slots
    uint8_t m_t1{0}; //!< The offset in number of slots between the slot in which the resource
                     //!< selection is triggered and the start of the selection window
    uint16_t m_t2{
        0}; //!< The offset in number of slots between T1 and the end of the selection window
    std::map<SidelinkLcIdentifier, NrSlMacSapProvider::NrSlReportBufferStatusParameters>
        m_nrSlBsrReceived;                                   ///< NR Sidelink BSR received from RLC
    uint16_t m_poolId{std::numeric_limits<uint16_t>::max()}; //!< Sidelink active pool id
    Time m_pRsvpTx{
        MilliSeconds(std::numeric_limits<uint8_t>::max())}; //!< Resource Reservation Interval for
                                                            //!< NR Sidelink in ms
    Ptr<UniformRandomVariable>
        m_ueSelectedUniformVariable; //!< uniform random variable used for NR Sidelink
    typedef std::unordered_map<uint32_t, struct NrSlGrantInfo>
        GrantInfo_t; //!< The typedef for the map of grant info per destination layer 2 id
    typedef std::unordered_map<uint32_t, struct NrSlGrantInfo>::iterator
        GrantInfoIt_t;                //!< The typedef for the iterator of the grant info map
    GrantInfo_t m_grantInfo;          //!< The map of grant info per destination layer 2 id
    double m_slProbResourceKeep{0.0}; //!< Sidelink probability of keeping a resource after resource
                                      //!< re-selection counter reaches zero
    uint8_t m_slMaxTxTransNumPssch{0};            /**< Indicates the maximum transmission number
                                                 (including new transmission and
                                                 retransmission) for PSSCH.
                                                 */
    uint8_t m_numSidelinkProcess{0};              //!< Maximum number of Sidelink processes
    Ptr<NrSlUeMacHarq> m_nrSlHarq;                //!< Pointer to the NR SL UE MAC HARQ object
    Ptr<NrSlUeMacScheduler> m_nrSlUeMacScheduler; //!< Pointer to the NR SL UE MAC scheduler
    uint32_t m_srcL2Id{std::numeric_limits<uint32_t>::max()}; //!< The NR Sidelink Source L2 id;
    bool m_nrSlMacPduTxed{false};         //!< Flag to indicate the TX of SL MAC PDU to PHY
    std::list<SensingData> m_sensingData; //!< List to store sensing data
    int m_thresRsrp{
        -128}; //!< A threshold in dBm used for sensing based UE autonomous resource selection
    uint8_t m_resPercentage{0}; /**< The percentage threshold to indicate the
                                     minimum number of candidate single-slot
                                     resources to be selected using sensing procedure.
                                     */
    uint8_t m_reselCounter{0};  //!< The resource selection counter
    uint16_t m_cResel{0};       //!< The C_resel counter

    /**
     * Trace information regarding NR Sidelink PSCCH UE scheduling.
     * SlPscchUeMacStatParameters (see nr-sl-phy-mac-common.h)
     */
    TracedCallback<SlPscchUeMacStatParameters>
        m_slPscchScheduling; //!< NR SL PSCCH scheduling trace source
    /**
     * Trace information regarding NR Sidelink PSSCH UE scheduling.
     * SlPsschUeMacStatParameters (see nr-sl-phy-mac-common.h)
     */
    TracedCallback<SlPsschUeMacStatParameters>
        m_slPsschScheduling; //!< NR SL PSCCH scheduling trace source

    /**
     * Trace information regarding RLC PDU reception from MAC
     */
    TracedCallback<uint64_t, uint16_t, uint16_t, uint8_t, uint32_t, double> m_rxRlcPduWithTxRnti;
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_H */
