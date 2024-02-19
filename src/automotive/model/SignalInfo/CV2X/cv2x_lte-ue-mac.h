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
 * Authors: Nicola Baldo  <nbaldo@cttc.es>
 *          Marco Miozzo <mmiozzo@cttc.es>
 * Modified by: NIST (D2D)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#ifndef CV2X_LTE_UE_MAC_ENTITY_H
#define CV2X_LTE_UE_MAC_ENTITY_H



#include <map>

#include <ns3/cv2x_lte-mac-sap.h>
#include <ns3/cv2x_lte-ue-cmac-sap.h>
#include <ns3/cv2x_lte-ue-cphy-sap.h>
#include <ns3/cv2x_lte-ue-phy-sap.h>
#include <ns3/cv2x_lte-amc.h>
#include <ns3/nstime.h>
#include <ns3/event-id.h>
#include <vector>
#include <ns3/packet.h>
#include <ns3/packet-burst.h>
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"


namespace ns3 {

    class UniformRandomVariable;

    class cv2x_LteUeMac :   public Object
    {
        /// allow cv2x_UeMemberLteUeCmacSapProvider class friend access
        friend class cv2x_UeMemberLteUeCmacSapProvider;
        /// allow cv2x_UeMemberLteMacSapProvider class friend access
        friend class cv2x_UeMemberLteMacSapProvider;
        /// allow cv2x_UeMemberLteUePhySapUser class friend access
        friend class cv2x_UeMemberLteUePhySapUser;

    public:
        /**
         * \brief Get the type ID.
         * \return the object TypeId
         */
        static TypeId GetTypeId (void);

        cv2x_LteUeMac ();
        virtual ~cv2x_LteUeMac ();
        virtual void DoDispose (void);

        /**
        * \brief Get the LTE MAC SAP provider
        * \return a pointer to the LTE MAC SAP provider
        */
        cv2x_LteMacSapProvider*  GetLteMacSapProvider (void);
        /**
        * \brief Set the LTE UE CMAC SAP user
        * \param s the LTE UE CMAC SAP User
        */
        void  SetLteUeCmacSapUser (cv2x_LteUeCmacSapUser* s);
        /**
        * \brief Get the LTE CMAC SAP provider
        * \return a pointer to the LTE CMAC SAP provider
        */
        cv2x_LteUeCmacSapProvider*  GetLteUeCmacSapProvider (void);

        /**
         * set the CPHY SAP this MAC should use to interact with the PHY
         *
         * \param s the CPHY SAP Provider
         */
        void SetLteUeCphySapProvider (cv2x_LteUeCphySapProvider * s);

        /**
        * \brief Set the component carried ID
        * \param index the component carrier ID
        */
        void SetComponentCarrierId (uint8_t index);

        /**
        * \brief Get the PHY SAP user
        * \return a pointer to the SAP user of the PHY
        */
        cv2x_LteUePhySapUser* GetLteUePhySapUser ();

        /**
        * \brief Set the PHY SAP Provider
        * \param s a pointer to the PHY SAP Provider
        */
        void SetLteUePhySapProvider (cv2x_LteUePhySapProvider* s);

        /**
        * \brief Forwarded from cv2x_LteUePhySapUser: trigger the start from a new frame
        *
        * \param frameNo frame number
        * \param subframeNo subframe number
        */
        void DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo);

        /**
         * Assign a fixed random variable stream number to the random variables
         * used by this model.  Return the number of streams (possibly zero) that
         * have been assigned.
         *
         * \param stream first stream index to use
         * \return the number of stream indices assigned by this model
         */
        int64_t AssignStreams (int64_t stream);

        /**
         * \return the UE scheduler type
         *
         * This allows you to get access to the Ul scheduler value in the UE side.
         */
        std::string GetUlScheduler (void) const;

        /**
         * \set the UE scheduler type
         *
         * \param sched is the UE scheduler to set in the UE side
         */
        void SetUlScheduler (std::string sched);

        /**
         * \return the UE selected resource mapping type
         *
         * This allows you to get access to the Sl UE selected resource value.
         */
        std::string GetSlUeSelResMapping (void) const;

        /**
         * \set the UE selected resource mapping type
         *
         * \param mapping is the UE selected mapping to set in the UE side
         */
        void SetSlUeSelResMapping (std::string mapping);

        /**
         * TracedCallback signature for transmission of discovery message.
         *
         * \param [in] rnti
         * \param [in] proSeAppCode
         */
        typedef void (* DiscoveryAnnouncementTracedCallback)
                (const uint16_t rnti, const uint32_t proSeAppCode);

        /**
         *
         * \return discovery reception pools
         */
        std::list< Ptr<SidelinkRxDiscResourcePool> > GetDiscRxPools ();

        /**
         *
         * \return discovery transmission pools
         */
        Ptr<SidelinkTxDiscResourcePool> GetDiscTxPool ();

        /**
         *  TracedCallback signature for SL UL scheduling events.
         *
         * \param [in] frame Frame number
         * \param [in] subframe Subframe number
         * \param [in] rnti The C-RNTI identifying the UE
         * \param [in] mcs  The MCS for transport block
         * \param [in] pscch_ri The resource index in the PSCCH
         * \param [in] pssch_rb RB start in the PSSCH
         * \param [in] pssch_lenght Number of RBs in sub-channel in the PSSCH
         * \param [in] pssch_itrp TRP index used in PSSCH
         */
/*  typedef void (* SlUeSchedulingTracedCallback)
    (const uint32_t frame, const uint32_t subframe, const uint16_t rnti,     const uint8_t mcs, const uint16_t pscch_ri, const uint16_t pssch_rb, const uint16_t pssch_length, const uint16_t pssch_itrp);
*/

    private:

        /**
         * The `DiscoveryMsgSent` trace source. Track the transmission of discovery message (announce)
         * Exporting RNTI, ProSe App Code.
         */
        TracedCallback<uint16_t, uint32_t> m_discoveryAnnouncementTrace;

        /**
         * The `SidelinkV2xMsgSent` trace source. Track the transmission of v2x message (announce)
         */
        TracedCallback<> m_sidelinkV2xAnnouncementTrace;


        // forwarded from MAC SAP
        /**
         * Transmit PDU function
         *
         * \param params cv2x_LteMacSapProvider::TransmitPduParameters
         */
        void DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params);

        /**
         * Report buffers status function
         *
         * \param params cv2x_LteMacSapProvider::ReportBufferStatusParameters
         */
        void DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params);

        // forwarded from UE CMAC SAP
        /**
         * Configure RACH function
         *
         * \param rc cv2x_LteUeCmacSapProvider::RachConfig
         */
        void DoConfigureRach (cv2x_LteUeCmacSapProvider::RachConfig rc);

        /**
         * Start contention based random access procedure function
         */
        void DoStartContentionBasedRandomAccessProcedure ();

        /**
         * Set RNTI
         *
         * \param rnti the RNTI
         */
        void DoSetRnti (uint16_t rnti);

        /**
         * Start non contention based random access procedure function
         *
         * \param rnti the RNTI
         * \param rapId the RAPID
         * \param prachMask the PRACH mask
         */
        void DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t rapId, uint8_t prachMask);

        /**
         * Add LC function
         *
         * \param lcId the LCID
         * \param lcConfig the logical channel config
         * \param msu the MSU
         */
        void DoAddLc (uint8_t lcId, cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu);

        /**
         * Add LC function
         *
         * \param lcId the LCID
         * \param scrL2Id the source L2 Id
         * \param dstL2Id the destination L2 Id
         * \param lcConfig the logical channel config
         * \param msu the MSU
         */

        void DoAddLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id, cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu);
        /**
         * Remove LC function
         *
         * \param lcId the LCID
         */

        void DoRemoveLc (uint8_t lcId);
        /**
         * Remove LC function
         *
         * \param lcId the LCID
         * \param scrL2Id the source L2 Id
         * \param dstL2Id the destination L2 Id
         */

        void DoRemoveLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id);

        /// Reset function
        void DoReset ();

        //communication
        void DoAddSlTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePool> pool);
        void DoRemoveSlTxPool (uint32_t dstL2Id);
        void DoSetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools);
        void DoAddSlDestination (uint32_t destination);
        void DoRemoveSlDestination (uint32_t destination);
        // V2X communication
        void DoAddSlV2xTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePoolV2x> pool);
        void DoRemoveSlV2xTxPool(uint32_t dstL2Id);
        void DoSetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools);
        //discovery
        void DoAddSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool);
        void DoRemoveSlTxPool ();
        void DoSetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools);
        void DoModifyDiscTxApps (std::list<uint32_t> apps);
        void DoModifyDiscRxApps (std::list<uint32_t> apps);

        // forwarded from PHY SAP
        /**
         * Receive Phy PDU function
         *
         * \param p the packet
         */
        void DoReceivePhyPdu (Ptr<Packet> p);
        /**
         * Receive LTE control message function
         *
         * \param msg the LTE control message
         */
        void DoReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg);

        // internal methods
        /// Randomly sleect and send RA preamble function
        void RandomlySelectAndSendRaPreamble ();
        /**
         * Send RA preamble function
         *
         * \param contention if true randomly select and send the RA preamble
         */
        void SendRaPreamble (bool contention);
        /// Start waiting for RA response function
        void StartWaitingForRaResponse ();
        /**
         * Receive the RA response function
         *
         * \param raResponse RA response received
         */
        void RecvRaResponse (cv2x_BuildRarListElement_s raResponse);
        /**
         * RA response timeout function
         *
         * \param contention if true randomly select and send the RA preamble
         */
        void RaResponseTimeout (bool contention);
        /// Send report buffer status
        void SendReportBufferStatus (void);
        /// Send sidelink report buffer status
        void SendSidelinkReportBufferStatus (void);
        /// Send sidelink report buffer status
        void SendSidelinkReportBufferStatusV2x (void);
        /// Refresh HARQ processes packet buffer function
        void RefreshHarqProcessesPacketBuffer (void);

        /// function to assign priority to each flow
        void DoAddLCPriority(uint8_t rnti, uint8_t lcid ,uint8_t  priority);

        // handle the different schedulers for the UE
        void DoReceivePFLteControlMessage (Ptr<cv2x_LteControlMessage> msg);
        void DoReceiveMTLteControlMessage (Ptr<cv2x_LteControlMessage> msg);
        void DoReceivePrLteControlMessage (Ptr<cv2x_LteControlMessage> msg);
        void DoReceiveRrLteControlMessage (Ptr<cv2x_LteControlMessage> msg);

        /// component carrier Id --> used to address sap
        uint8_t m_componentCarrierId;

        void SetSignalInfo(double rssi, double rsrp) {m_rssi = rssi; m_rsrp = rsrp;};
        std::tuple<double, double> GetSignalInfo() {return std::make_tuple(m_rssi, m_rsrp);};

    private:

        /// LcInfo structure
        struct LcInfo
        {
            cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig; ///< logical channel config
            cv2x_LteMacSapUser* macSapUser; ///< MAC SAP user
        };

        std::map <uint8_t, LcInfo> m_lcInfoMap; ///< logical channel info map
        cv2x_LteUeCphySapProvider* m_cphySapProvider; ///< UE CPHY SAP provider
        cv2x_LteMacSapProvider* m_macSapProvider; ///< MAC SAP provider

        cv2x_LteUeCmacSapUser* m_cmacSapUser; ///< CMAC SAP user
        cv2x_LteUeCmacSapProvider* m_cmacSapProvider; ///< CMAC SAP provider

        cv2x_LteUePhySapProvider* m_uePhySapProvider; ///< UE Phy SAP provider
        cv2x_LteUePhySapUser* m_uePhySapUser; ///< UE Phy SAP user

        std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters> m_ulBsrReceived; ///< BSR received from RLC (the last one)

        // NIST: new structure for m_ulBsrReceived to handle UL per flow scheduling
        std::map <uint8_t, std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > > nist_m_ulBsrReceived;

        // added map to handle priority
        std::map <uint8_t, std::map <uint8_t, uint8_t> > PriorityMap;

        Time m_bsrPeriodicity; ///< BSR periodicity
        Time m_bsrLast; ///< BSR last

        bool m_freshUlBsr; ///< true when a BSR has been received in the last TTI

        uint8_t m_harqProcessId; ///< HARQ process ID
        std::vector < Ptr<PacketBurst> > m_miUlHarqProcessesPacket; ///< Packets under transmission of the UL HARQ processes
        std::vector < uint8_t > m_miUlHarqProcessesPacketTimer; ///< timer for packet life in the buffer

        uint16_t m_rnti; ///< RNTI

        bool m_rachConfigured; ///< is RACH configured?
        cv2x_LteUeCmacSapProvider::RachConfig m_rachConfig; ///< RACH configuration
        uint8_t m_raPreambleId; ///< RA preamble ID
        uint8_t m_preambleTransmissionCounter; ///< preamble tranamission counter
        uint16_t m_backoffParameter; ///< backoff parameter
        EventId m_noRaResponseReceivedEvent; ///< no RA response received event ID
        Ptr<UniformRandomVariable> m_raPreambleUniformVariable; ///< RA preamble random variable

        uint32_t m_frameNo; ///< frame number
        uint32_t m_subframeNo; ///< subframe number
        uint8_t m_raRnti; ///< RA RNTI
        bool m_waitingForRaResponse; ///< waiting for RA response
        uint8_t m_ulBandwidth; ///< LTE bandwidth [MHz]
        uint8_t m_pHarq; ///< probability of HARQ retransmissions

        // V2x
        bool m_v2xHarqEnabled; ///< harq enabled?
        bool m_adjacency; ///< adjacent PSCCH+PSSCH scheme enabled
        bool m_partialSensing; ///< partial sensing enabled
        double m_probResourceKeep; ///< probability for selecting the previous resource again
        uint8_t m_t1; ///< defining the size of the selection window
        uint8_t m_t2; ///< defining the size of the selection window
        uint8_t m_sizeSubchannel; ///< number of resource blocks per subframe
        uint8_t m_numSubchannel; ///< number of subchannels per subframe
        uint8_t m_startRbSubchannel; ///< resource block index where the subchannels begin
        uint16_t m_pRsvp; ///< Resource Reservation Interval in ms
        uint16_t rndmStart = (rand()%((3000+1)-2000))+2000; ///< counter for random start of resource allocation process
        bool firstTx = true;

        std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> txOpps; // list with all tx opportunities calculated by SPS
        SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo txInfo; // selected resource

        double m_rssi;
        double m_rsrp;


        //sidelink variables
        struct SidelinkLcIdentifier
        {
            uint8_t lcId;
            uint32_t srcL2Id;
            uint32_t dstL2Id;
        };

        friend bool operator < (const SidelinkLcIdentifier &l, const SidelinkLcIdentifier &r) { return l.lcId < r.lcId || (l.lcId == r.lcId && l.srcL2Id < r.srcL2Id) || (l.lcId == r.lcId && l.srcL2Id == r.srcL2Id && l.dstL2Id < r.dstL2Id); }

        std::map <SidelinkLcIdentifier, LcInfo> m_slLcInfoMap;
        Time m_slBsrPeriodicity;
        Time m_slBsrLast;
        bool m_freshSlBsr; // true when a BSR has been received in the last TTI
        std::map <SidelinkLcIdentifier, cv2x_LteMacSapProvider::ReportBufferStatusParameters> m_slBsrReceived; // BSR received from RLC (the last one)

        struct SidelinkGrant
        {
            //fields common with SL_DCI
            uint16_t m_resPscch;
            uint8_t m_tpc;
            uint8_t m_hopping;
            uint8_t m_rbStart; //models rb assignment
            uint8_t m_rbLen;   //models rb assignment
            uint8_t m_trp;

            //other fields
            uint8_t m_mcs;
            uint32_t m_tbSize;
        };

        struct PoolInfo
        {
            Ptr<SidelinkCommResourcePool> m_pool; //the pool
            SidelinkCommResourcePool::SubframeInfo m_currentScPeriod; //start of current period
            SidelinkGrant m_currentGrant; //grant for the next SC period
            SidelinkCommResourcePool::SubframeInfo m_nextScPeriod; //start of next period

            uint32_t m_npscch; // number of PSCCH available in the pool

            bool m_grant_received;
            SidelinkGrant m_nextGrant; //grant received for the next SC period

            std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> m_pscchTx;//list of PSCCH transmissions within the pool
            std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> m_psschTx; //list of PSSCH transmissions within the pool

            Ptr<PacketBurst> m_miSlHarqProcessPacket; // Packets under trasmission of the SL HARQ process
        };


        std::map <uint32_t, PoolInfo > m_sidelinkTxPoolsMap;
        std::list <Ptr<SidelinkRxCommResourcePool> > m_sidelinkRxPools;
        std::list <uint32_t> m_sidelinkDestinations;

        Ptr<cv2x_LteAmc> m_amc; //needed now since UE is doing scheduling
        Ptr<UniformRandomVariable> m_ueSelectedUniformVariable;
        //fields for fixed UE_SELECTED pools
        uint8_t m_slKtrp;
        uint8_t m_slGrantMcs;
        uint8_t m_slGrantSize;
        uint8_t m_pucchSize;

        std::string m_UlScheduler;      // the UE scheduler attribute

        struct SidelinkGrantV2x {
            uint8_t m_prio;
            uint16_t m_pRsvp;
            uint16_t m_riv;
            uint8_t m_sfGap;
            uint8_t m_mcs;
            uint8_t m_reTxIdx;

            uint8_t m_resPscch;
            uint32_t m_tbSize;
        };

        struct PoolInfoV2x{
            Ptr<SidelinkCommResourcePoolV2x> m_pool; //the pool
            SidelinkCommResourcePoolV2x::SubframeInfo m_currentFrameInfo; //currentFrameInfo
            uint32_t m_npssch;

            bool m_grant_received;
            SidelinkGrantV2x m_currentGrant;
            SidelinkGrantV2x m_nextGrant;
            std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> m_pscchTx;
            std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> m_psschTx;

            Ptr<PacketBurst> m_miSlHarqProcessPacket; // Packets under transmission of the SL HARQ process

        };

        std::map <uint32_t, PoolInfoV2x> m_sidelinkTxPoolsMapV2x;
        std::list <Ptr<SidelinkRxCommResourcePoolV2x> > m_sidelinkRxPoolsV2x;
        uint8_t m_reselCtr = 0; // Reselection Counter for resource allocation
        uint8_t m_subchLen = 1;

        struct SidelinkGrantInfoV2x {
            SidelinkGrantV2x m_grant;
            std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> m_pscchTx;
            std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> m_psschTx;
            bool m_grant_received;
        };

        struct SensingData{
            SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo m_rxInfo;
            uint8_t m_prioRx;
            uint16_t m_pRsvpRx;
            double m_slRsrp;
            double m_slRssi;
        };

        std::list<SensingData> m_sensingData;

        struct CandidateResource{
            SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo m_txInfo;
            double m_avg_rssi;
        };

        struct SidelinkTransmissionInfoExtended {
            SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo m_txInfo;
            uint8_t m_sfGap;
            uint8_t m_reTxIdx;
        };

        //discovery
        struct DiscGrant
        {
            uint16_t m_rnti;
            uint8_t m_resPsdch;

            //uint32_t m_tbSize; //232 bits (fixed)
        };

        struct DiscPoolInfo
        {
            Ptr<SidelinkTxDiscResourcePool> m_pool; //the pool
            SidelinkDiscResourcePool::SubframeInfo m_currentDiscPeriod; //start of current period
            DiscGrant m_currentGrant; //grant for the next discovery period
            SidelinkDiscResourcePool::SubframeInfo m_nextDiscPeriod; //start of next period

            uint32_t m_npsdch; // number of PSDCH available in the pool

            bool m_grant_received;
            DiscGrant m_nextGrant; //grant received for the next discovery period

            std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo> m_psdchTx;//list of PSDCH transmissions within the pool

            //HARQ is not considered for now
            //Ptr<PacketBurst> m_miSlHarqProcessPacket; // Packets under trasmission of the SL HARQ process
        };

        DiscPoolInfo m_discTxPools;
        std::list <Ptr<SidelinkRxDiscResourcePool> > m_discRxPools;

        std::list<uint32_t> m_discTxApps;
        std::list<uint32_t> m_discRxApps;

        Ptr<UniformRandomVariable> m_p1UniformVariable;
        Ptr<UniformRandomVariable> m_resUniformVariable;


        /**
         * Trace information regarding SL UE scheduling.
         * Frame number, Subframe number, RNTI, MCS, PSCCH resource index, PSSCH RB start, PSSCH lenght in RBs, PSSCH TRP index
         */
        TracedCallback<cv2x_SlUeMacStatParameters> m_slUeScheduling;

        TracedCallback<cv2x_SlUeMacStatParameters> m_slSharedChUeScheduling;


        /**
         * Trace information regarding SL UE scheduling.
         * Frame number, Subframe number, RNTI, MCS, PSCCH resource index, PSSCH RB start, PSSCH length in RBs
         */
        TracedCallback<cv2x_SlUeMacStatParametersV2x> m_slUeSchedulingV2x;

        TracedCallback<cv2x_SlUeMacStatParametersV2x> m_slSharedChUeSchedulingV2x;

        /**
         * True if there is data to transmit in the PSSCH
         */
        bool m_slHasDataToTx;
        //The PHY notifies the change of timing as consequence of a change of SyncRef, the MAC adjust its timing
        void DoNotifyChangeOfTiming (uint32_t frameNo, uint32_t subframeNo);

        // The PHY pass the sensing data for SPS to MAC
        void DoPassSensingData (uint32_t frameNo, uint32_t subframeNo, uint16_t pRsvp, uint8_t rbStart, uint8_t rbLen, uint8_t prio, double slRsrp, double slRssi);

        /**
         * Update the sensing window (1000 ms)
         */
        void UpdateSensingWindow (SidelinkCommResourcePoolV2x::SubframeInfo subframe);
        /**
        * \brief See 36.213 section 14.1.1.7 V15.0.0
        */
        std::list<SidelinkTransmissionInfoExtended> GetReTxResources (SidelinkCommResourcePoolV2x::SubframeInfo subframe, std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> txOpps);
        /**
        * \brief See 36.213 section 14.1.1.6 V15.0.0
        */
        std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> GetTxResources (SidelinkCommResourcePoolV2x::SubframeInfo subframe, PoolInfoV2x pool);
        /**
         * \brief See 36.321 section 5.14.1.1 V15.0.0
         */
        uint8_t GetRndmReselectionCounter(uint16_t pRsvp);
        /**
         * \brief See 36.331 section 6.3.8 V15.0.1
         */
        uint8_t GetSlThresPsschRsrpVal(uint8_t a, uint8_t b);
        /**
         * \brief See 36.213 section 14.1.1.4C V15.0.0
         */
        uint8_t CalcRiv(uint8_t lSubch, uint8_t startSubchIdx);

    };

} // namespace ns3

#endif // LTE_UE_MAC_ENTITY
