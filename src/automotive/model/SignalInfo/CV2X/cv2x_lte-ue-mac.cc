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
 * Author: Nicola Baldo  <nbaldo@cttc.es>
 * Author: Marco Miozzo <mmiozzo@cttc.es>
 * Modified by: NIST (D2D)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */



#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/packet.h>
#include <ns3/packet-burst.h>
#include <ns3/random-variable-stream.h>

#include "cv2x_lte-ue-mac.h"
#include "cv2x_lte-ue-net-device.h"
#include "cv2x_lte-radio-bearer-tag.h"
#include <ns3/cv2x_ff-mac-common.h>
#include <ns3/cv2x_lte-control-messages.h>
#include <ns3/simulator.h>
#include <ns3/cv2x_lte-common.h>
#include <ns3/string.h>
#include <ns3/enum.h>

#include <ns3/boolean.h>
#include <bitset>
#include <algorithm>

#include "ns3/rssi-tag.h"
#include "ns3/rsrp-tag.h"


#include "ns3/cv2x_lte-rlc-tag.h"



namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("cv2x_LteUeMac");

    NS_OBJECT_ENSURE_REGISTERED (cv2x_LteUeMac);

///////////////////////////////////////////////////////////
// SAP forwarders
///////////////////////////////////////////////////////////

/// cv2x_UeMemberLteUeCmacSapProvider class
    class cv2x_UeMemberLteUeCmacSapProvider : public cv2x_LteUeCmacSapProvider
    {
    public:
        /**
   * Constructor
   *
   * \param mac the UE MAC
   */
        cv2x_UeMemberLteUeCmacSapProvider (cv2x_LteUeMac* mac);

        // inherited from cv2x_LteUeCmacSapProvider
        virtual void ConfigureRach (RachConfig rc);
        virtual void StartContentionBasedRandomAccessProcedure ();
        virtual void StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask);
        virtual void SetRnti (uint16_t rnti);
        virtual void AddLc (uint8_t lcId, cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu);
        virtual void AddLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id, cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu);
        virtual void RemoveLc (uint8_t lcId);
        virtual void RemoveLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id);
        virtual void Reset ();
        //D2D communication
        virtual void AddSlTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePool> pool);
        virtual void RemoveSlTxPool (uint32_t dstL2Id);
        virtual void SetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools);
        virtual void AddSlDestination (uint32_t destination);
        virtual void RemoveSlDestination (uint32_t destination);
        //V2X communication
        virtual void AddSlV2xTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePoolV2x> pool);
        virtual void RemoveSlV2xTxPool (uint32_t dstL2Id);
        virtual void SetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools);
        //Discovery
        virtual void AddSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool);
        virtual void RemoveSlTxPool ();
        virtual void SetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools);
        virtual void ModifyDiscTxApps (std::list<uint32_t> apps);
        virtual void ModifyDiscRxApps (std::list<uint32_t> apps);
        // added to handle LC priority
        virtual void AddLCPriority ( uint8_t rnti, uint8_t lcid, uint8_t priority);


    private:
        cv2x_LteUeMac* m_mac; ///< the UE MAC
    };


    cv2x_UeMemberLteUeCmacSapProvider::cv2x_UeMemberLteUeCmacSapProvider (cv2x_LteUeMac* mac)
    : m_mac (mac)
            {
            }

    void
    cv2x_UeMemberLteUeCmacSapProvider::ConfigureRach (RachConfig rc)
    {
        m_mac->DoConfigureRach (rc);
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::StartContentionBasedRandomAccessProcedure ()
    {
        m_mac->DoStartContentionBasedRandomAccessProcedure ();
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask)
    {
        m_mac->DoStartNonContentionBasedRandomAccessProcedure (rnti, preambleId, prachMask);
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::SetRnti (uint16_t rnti)
    {
        m_mac->DoSetRnti (rnti);
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::AddLc (uint8_t lcId, LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu)
    {
        m_mac->DoAddLc (lcId, lcConfig, msu);
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::AddLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id, LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu)
    {
        m_mac->DoAddLc (lcId, srcL2Id, dstL2Id, lcConfig, msu);
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::RemoveLc (uint8_t lcid)
    {
        m_mac->DoRemoveLc (lcid);
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::RemoveLc (uint8_t lcid, uint32_t srcL2Id, uint32_t dstL2Id)
    {
        m_mac->DoRemoveLc (lcid, srcL2Id, dstL2Id);
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::Reset ()
    {
        m_mac->DoReset ();
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::AddSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool)
    {
        m_mac->DoAddSlTxPool (pool);
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::RemoveSlTxPool ()
    {
        m_mac->DoRemoveSlTxPool ();
    }

    void
    cv2x_UeMemberLteUeCmacSapProvider::SetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools)
{
    m_mac->DoSetSlRxPools (pools);
}

void
cv2x_UeMemberLteUeCmacSapProvider::AddSlTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePool> pool)
{
    m_mac->DoAddSlTxPool (dstL2Id, pool);
}

void
cv2x_UeMemberLteUeCmacSapProvider::AddSlV2xTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePoolV2x> pool)
{
    m_mac->DoAddSlV2xTxPool (dstL2Id, pool);
}

void
cv2x_UeMemberLteUeCmacSapProvider::RemoveSlTxPool (uint32_t dstL2Id)
{
    m_mac->DoRemoveSlTxPool (dstL2Id);
}

void
cv2x_UeMemberLteUeCmacSapProvider::RemoveSlV2xTxPool (uint32_t dstL2Id)
{
    m_mac->DoRemoveSlV2xTxPool (dstL2Id);
}

void
cv2x_UeMemberLteUeCmacSapProvider::SetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools)
{
m_mac->DoSetSlRxPools (pools);
}

void
cv2x_UeMemberLteUeCmacSapProvider::SetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools)
{
m_mac->DoSetSlV2xRxPools (pools);
}

void
cv2x_UeMemberLteUeCmacSapProvider::AddSlDestination (uint32_t destination)
{
    m_mac->DoAddSlDestination (destination);
}

void
cv2x_UeMemberLteUeCmacSapProvider::RemoveSlDestination (uint32_t destination)
{
    m_mac->DoRemoveSlDestination (destination);
}

void
cv2x_UeMemberLteUeCmacSapProvider::ModifyDiscTxApps (std::list<uint32_t> apps)
{
    m_mac->DoModifyDiscTxApps (apps);
}

void
cv2x_UeMemberLteUeCmacSapProvider::ModifyDiscRxApps (std::list<uint32_t> apps)
{
    m_mac->DoModifyDiscRxApps (apps);
}

// added fucntion to handle priority for LC
void
cv2x_UeMemberLteUeCmacSapProvider::AddLCPriority ( uint8_t rnti, uint8_t lcid, uint8_t priority)
{
    m_mac->DoAddLCPriority (rnti, lcid, priority);
}

/// cv2x_UeMemberLteMacSapProvider class
class cv2x_UeMemberLteMacSapProvider : public cv2x_LteMacSapProvider
{
public:
    /**
   * Constructor
   *
   * \param mac the UE MAC
   */
    cv2x_UeMemberLteMacSapProvider (cv2x_LteUeMac* mac);

    // inherited from cv2x_LteMacSapProvider
    virtual void TransmitPdu (TransmitPduParameters params);
    virtual void ReportBufferStatus (ReportBufferStatusParameters params);

private:
    cv2x_LteUeMac* m_mac; ///< the UE MAC
};


cv2x_UeMemberLteMacSapProvider::cv2x_UeMemberLteMacSapProvider (cv2x_LteUeMac* mac)
        : m_mac (mac)
{
}

void
cv2x_UeMemberLteMacSapProvider::TransmitPdu (TransmitPduParameters params)
{
    m_mac->DoTransmitPdu (params);
}


void
cv2x_UeMemberLteMacSapProvider::ReportBufferStatus (ReportBufferStatusParameters params)
{
    m_mac->DoReportBufferStatus (params);
}



/**
 * cv2x_UeMemberLteUePhySapUser
 */
class cv2x_UeMemberLteUePhySapUser : public cv2x_LteUePhySapUser
{
public:
    /**
   * Constructor
   *
   * \param mac the UE MAC
   */
    cv2x_UeMemberLteUePhySapUser (cv2x_LteUeMac* mac);

    // inherited from cv2x_LtePhySapUser
    virtual void ReceivePhyPdu (Ptr<Packet> p);
    virtual void SubframeIndication (uint32_t frameNo, uint32_t subframeNo);
    virtual void ReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg);
    virtual void NotifyChangeOfTiming (uint32_t frameNo, uint32_t subframeNo);
    virtual void PassSensingData(uint32_t frameNo, uint32_t subframeNo, uint16_t pRsvp, uint8_t rbStart, uint8_t rbLen, uint8_t prio, double slRsrp, double slRssi);

private:
    cv2x_LteUeMac* m_mac; ///< the UE MAC
};

cv2x_UeMemberLteUePhySapUser::cv2x_UeMemberLteUePhySapUser (cv2x_LteUeMac* mac) : m_mac (mac)
{
}

void
cv2x_UeMemberLteUePhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
    m_mac->DoReceivePhyPdu (p);
}


void
cv2x_UeMemberLteUePhySapUser::SubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
    m_mac->DoSubframeIndication (frameNo, subframeNo);
}

void
cv2x_UeMemberLteUePhySapUser::ReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
    m_mac->DoReceiveLteControlMessage (msg);
}

void
cv2x_UeMemberLteUePhySapUser::NotifyChangeOfTiming (uint32_t frameNo, uint32_t subframeNo)
{
    m_mac->DoNotifyChangeOfTiming (frameNo, subframeNo);
}

void
cv2x_UeMemberLteUePhySapUser::PassSensingData(uint32_t frameNo, uint32_t subframeNo, uint16_t pRsvp, uint8_t rbStart, uint8_t rbLen, uint8_t prio, double slRsrp, double slRssi)
{
    m_mac->DoPassSensingData (frameNo, subframeNo, pRsvp, rbStart, rbLen, prio, slRsrp, slRssi);
}



//////////////////////////////////////////////////////////
// cv2x_LteUeMac methods
///////////////////////////////////////////////////////////


TypeId
cv2x_LteUeMac::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::cv2x_LteUeMac")
            .SetParent<Object> ()
            .SetGroupName("Lte")
            .AddConstructor<cv2x_LteUeMac> ()
            .AddAttribute ("UlScheduler",
                           "Type of Scheduler in the UE",
                           StringValue ("ns3::cv2x_RrFfMacScheduler"),
                           MakeStringAccessor (&cv2x_LteUeMac::SetUlScheduler,
                                               &cv2x_LteUeMac::GetUlScheduler),
                           MakeStringChecker ())
            .AddAttribute ("PHarq",
                           "The Probability of the HARQ retransmissions",
                           UintegerValue (100),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_pHarq),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("Ktrp",
                           "The repetition for PSSCH. Default = 0",
                           UintegerValue (0),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_slKtrp),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("SlGrantMcs",
                           "The MCS of the SL grant, must be [0..15] (default 0)",
                           UintegerValue (0),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_slGrantMcs),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("SlGrantSize",
                           "The number of RBs allocated per UE for sidelink (default 1)",
                           UintegerValue (1),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_slGrantSize),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("PucchSize",
                           "Number of RBs reserved for PUCCH (default 0)",
                           UintegerValue (0),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_pucchSize),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("UlBandwidth",
                           "UL bandwidth [MHz] (default 10)",
                           UintegerValue (10),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_ulBandwidth),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("SlSubchannelSize",
                           "Number of RBs per Subchannel (default 5)",
                           UintegerValue(5),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_sizeSubchannel),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("SlSubchannelNum",
                           "Number of Subchannels per Subframe (default 3)",
                           UintegerValue(3),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_numSubchannel),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("SlStartRbSubchannel",
                           "Index of first subchannel addicted to subchannelization (default 0)",
                           UintegerValue(0),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_startRbSubchannel),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("SlPrsvp",
                           "Resource Reservation Interval in ms (default 100)",
                           UintegerValue(100),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_pRsvp),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("SelectionWindowT1",
                           "T1 value of Selection Window (default 4)",
                           UintegerValue(4),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_t1),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("SelectionWindowT2",
                           "T2 value of Selection Window (default 100)",
                           UintegerValue(100),
                           MakeUintegerAccessor (&cv2x_LteUeMac::m_t2),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("SlProbResourceKeep",
                           "Probability for selecting the previous resource again (default 0.0)",
                           DoubleValue(0.0),
                           MakeDoubleAccessor (&cv2x_LteUeMac::m_probResourceKeep),
                           MakeDoubleChecker<double> ())
            .AddAttribute ("EnableV2xHarq",
                           "If true, HARQ retransmission is enabled (default false)",
                           BooleanValue(false),
                           MakeBooleanAccessor (&cv2x_LteUeMac::m_v2xHarqEnabled),
                           MakeBooleanChecker ())
            .AddAttribute ("EnableAdjacencyPscchPssch",
                           "If true, Adjacent Pscch+Pssch Mode is enabled (default true)",
                           BooleanValue(true),
                           MakeBooleanAccessor (&cv2x_LteUeMac::m_adjacency),
                           MakeBooleanChecker ())
            .AddAttribute ("EnablePartialSensing",
                           "If true, PartialSensing is enabled (fefault false)",
                           BooleanValue(false),
                           MakeBooleanAccessor (&cv2x_LteUeMac::m_partialSensing),
                           MakeBooleanChecker ())
            .AddTraceSource ("SlUeScheduling",
                             "Information regarding SL UE scheduling",
                             MakeTraceSourceAccessor (&cv2x_LteUeMac::m_slUeScheduling),
                             "ns3::cv2x_SlUeMacStatParameters::TracedCallback")
            .AddTraceSource ("SlUeSchedulingV2x",
                             "Information regarding SL UE V2X scheduling",
                             MakeTraceSourceAccessor (&cv2x_LteUeMac::m_slUeSchedulingV2x),
                             "ns3::cv2x_SlUeMacStatParametersV2x::TracedCallback")
            .AddTraceSource ("SlSharedChUeScheduling",
                             "Information regarding SL Shared Channel UE scheduling",
                             MakeTraceSourceAccessor (&cv2x_LteUeMac::m_slSharedChUeScheduling),
                             "ns3::cv2x_SlUeMacStatParameters::TracedCallback")
            .AddTraceSource ("SlSharedChUeSchedulingV2x",
                             "Information regarding SL Shared Channel UE scheduling",
                             MakeTraceSourceAccessor (&cv2x_LteUeMac::m_slSharedChUeSchedulingV2x),
                             "ns3::cv2x_SlUeMacStatParametersV2x::TracedCallback")
                    // Added to trace the transmission of v2x message
            .AddTraceSource ("SidelinkV2xAnnouncement",
                             "trace to track the announcement of Sidelink messages",
                             MakeTraceSourceAccessor (&cv2x_LteUeMac::m_sidelinkV2xAnnouncementTrace),
                             "ns3::cv2x_LteUeMac::SidelinkV2xAnnouncementTracedCallback")
                    // Added to trace the transmission of discovery message
            .AddTraceSource ("DiscoveryAnnouncement",
                             "trace to track the announcement of discovery messages",
                             MakeTraceSourceAccessor (&cv2x_LteUeMac::m_discoveryAnnouncementTrace),
                             "ns3::cv2x_LteUeMac::DiscoveryAnnouncementTracedCallback")
    ;
    return tid;
}


cv2x_LteUeMac::cv2x_LteUeMac ()
        :  m_cphySapProvider (0),
           m_bsrPeriodicity (MilliSeconds (1)), // ideal behavior
           m_bsrLast (MilliSeconds (0)),
           m_freshUlBsr (false),
           m_harqProcessId (0),
           m_rnti (0),
           m_rachConfigured (false),
           m_waitingForRaResponse (false),
           m_slBsrPeriodicity (MilliSeconds (1)),
           m_slBsrLast (MilliSeconds (0)),
           m_freshSlBsr (false),
           m_UlScheduler ("ns3::cv2x_RrFfMacScheduler") // UE scheduler initialization


{
    NS_LOG_FUNCTION (this);
    m_miUlHarqProcessesPacket.resize (HARQ_PERIOD);
    for (uint8_t i = 0; i < m_miUlHarqProcessesPacket.size (); i++)
    {
        Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
        m_miUlHarqProcessesPacket.at (i) = pb;
    }
    m_miUlHarqProcessesPacketTimer.resize (HARQ_PERIOD, 0);

    m_macSapProvider = new cv2x_UeMemberLteMacSapProvider (this);
    m_cmacSapProvider = new cv2x_UeMemberLteUeCmacSapProvider (this);
    m_uePhySapUser = new cv2x_UeMemberLteUePhySapUser (this);
    m_raPreambleUniformVariable = CreateObject<UniformRandomVariable> ();
    m_componentCarrierId = 0;

    m_amc = CreateObject <cv2x_LteAmc> ();
    m_ueSelectedUniformVariable = CreateObject<UniformRandomVariable> ();
    //m_slDiversity.status = SlDiversity::disabled; // enabled should be default!

    m_p1UniformVariable = CreateObject<UniformRandomVariable> ();
    m_resUniformVariable = CreateObject<UniformRandomVariable> ();
}

void
cv2x_LteUeMac::SetUlScheduler (std::string UeSched)
{
    m_UlScheduler = UeSched;
}

std::string
cv2x_LteUeMac::GetUlScheduler (void) const
{
    return m_UlScheduler;
}

cv2x_LteUeMac::~cv2x_LteUeMac ()
{
    NS_LOG_FUNCTION (this);
}

void
cv2x_LteUeMac::DoDispose ()
{
    NS_LOG_FUNCTION (this);
    m_miUlHarqProcessesPacket.clear ();
    delete m_macSapProvider;
    delete m_cmacSapProvider;
    delete m_uePhySapUser;
    Object::DoDispose ();
}

void
cv2x_LteUeMac::SetLteUeCphySapProvider (cv2x_LteUeCphySapProvider * s)
{
    NS_LOG_FUNCTION (this << s);
    m_cphySapProvider = s;
}

cv2x_LteUePhySapUser*
cv2x_LteUeMac::GetLteUePhySapUser (void)
{
    return m_uePhySapUser;
}

void
cv2x_LteUeMac::SetLteUePhySapProvider (cv2x_LteUePhySapProvider* s)
{
    m_uePhySapProvider = s;
}


cv2x_LteMacSapProvider*
cv2x_LteUeMac::GetLteMacSapProvider (void)
{
    return m_macSapProvider;
}

void
cv2x_LteUeMac::SetLteUeCmacSapUser (cv2x_LteUeCmacSapUser* s)
{
    m_cmacSapUser = s;
}

cv2x_LteUeCmacSapProvider*
cv2x_LteUeMac::GetLteUeCmacSapProvider (void)
{
    return m_cmacSapProvider;
}

void
cv2x_LteUeMac::SetComponentCarrierId (uint8_t index)
{
    m_componentCarrierId = index;
}

void
cv2x_LteUeMac::DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params)
{
    NS_LOG_FUNCTION (this);
    NS_ASSERT_MSG (m_rnti == params.rnti, "RNTI mismatch between RLC and MAC");
    if (params.srcL2Id == 0)
    {
        cv2x_LteRadioBearerTag tag (params.rnti, params.lcid, 0 /* UE works in SISO mode*/);
        params.pdu->AddPacketTag (tag);
        // store pdu in HARQ buffer
        m_miUlHarqProcessesPacket.at (m_harqProcessId)->AddPacket (params.pdu);
        m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        m_uePhySapProvider->SendMacPdu (params.pdu);
    }
    else
    {
        cv2x_LteRadioBearerTag tag (params.rnti, params.lcid, params.srcL2Id, params.dstL2Id);
        params.pdu->AddPacketTag (tag);

        // find V2X pool if V2X is configured else find Sl pool
        std::map <uint32_t, PoolInfoV2x>::iterator poolIt = m_sidelinkTxPoolsMapV2x.find (params.dstL2Id);
        if (poolIt == m_sidelinkTxPoolsMapV2x.end ())
        {
            NS_LOG_INFO ("Transmitting sidelink PDU");
            //find transmitting pool
            std::map <uint32_t, PoolInfo>::iterator poolIt = m_sidelinkTxPoolsMap.find (params.dstL2Id);
            NS_ASSERT (poolIt != m_sidelinkTxPoolsMap.end ());
            // store pdu in HARQ buffer
            poolIt->second.m_miSlHarqProcessPacket->AddPacket (params.pdu);
        }
        else
        {
            NS_LOG_INFO ("Transmitting V2X PDU");
            // store pdu in HARQ buffer
            poolIt->second.m_miSlHarqProcessPacket->AddPacket (params.pdu);
        }

        m_uePhySapProvider->SendMacPdu (params.pdu);
    }
}

void
cv2x_LteUeMac::DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params)
{
    NS_LOG_FUNCTION (this << (uint32_t) params.lcid);


    if (params.srcL2Id == 0)
    {
        NS_ASSERT (params.dstL2Id == 0);
        NS_LOG_INFO ("Reporting for uplink");
        //regular uplink BSR
        if ( ( m_UlScheduler == "ns3::cv2x_PriorityFfMacScheduler") || ( m_UlScheduler == "ns3::cv2x_PfFfMacScheduler") || ( m_UlScheduler == "ns3::MtFfMacScheduler" ) || ( m_UlScheduler == "ns3::cv2x_RrSlFfMacScheduler"))
        {
            //NIST new iterator since nist_m_ulBsrReceived is modified
            std::map <uint8_t, std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters> >::iterator itNist;
            std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itTempMap;
            std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters> TempMap;

            itNist = nist_m_ulBsrReceived.find (m_rnti);

            if (itNist != nist_m_ulBsrReceived.end ())
            {
                // update entry

                TempMap = itNist->second;
                itTempMap = TempMap.find ((uint8_t)params.lcid);

                if (itTempMap == TempMap.end ())
                {
                    itNist->second.insert (std::pair<uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters> (params.lcid, params));
                    m_freshUlBsr = true;
                    return;
                }
                else
                {
                    (*itTempMap).second = params;
                    itNist->second = TempMap;
                    m_freshUlBsr = true;
                    return;
                }
            }
            else
            {
                std::map<uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters> tempMap;
                tempMap.insert (std::pair<uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters> ((uint8_t)params.lcid, params));
                nist_m_ulBsrReceived.insert (std::pair <uint8_t, std::map<uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > > (m_rnti, tempMap));
                m_freshUlBsr = true;
                return;
            }
        }
        else if ( ( m_UlScheduler == "ns3::cv2x_RrFfMacScheduler") ||  ( m_UlScheduler == "ns3::3GPPcalMacScheduler"))
        {
            std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator it;

            it = m_ulBsrReceived.find (params.lcid);
            if (it != m_ulBsrReceived.end ())
            {
                // update entry
                (*it).second = params;
            }
            else
            {
                m_ulBsrReceived.insert (std::pair<uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters> (params.lcid, params));
            }
            m_freshUlBsr = true;
        }
    }
    else
    {
        NS_LOG_INFO ("Reporting for sidelink");
        //sidelink BSR
        std::map <SidelinkLcIdentifier, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator it;

        SidelinkLcIdentifier sllcid;
        sllcid.lcId = params.lcid;
        sllcid.srcL2Id = params.srcL2Id;
        sllcid.dstL2Id = params.dstL2Id;

        it = m_slBsrReceived.find (sllcid);
        if (it != m_slBsrReceived.end ())
        {
            // update entry
            (*it).second = params;
        }
        else
        {
            m_slBsrReceived.insert (std::pair<SidelinkLcIdentifier, cv2x_LteMacSapProvider::ReportBufferStatusParameters> (sllcid, params));
        }
        m_freshSlBsr = true;
    }
}


void
cv2x_LteUeMac::SendReportBufferStatus (void)
{
    NS_LOG_FUNCTION (this);

    if (m_rnti == 0)
    {
        NS_LOG_INFO ("MAC not initialized, BSR deferred");
        return;
    }

    if (nist_m_ulBsrReceived.size () == 0)
    {
        NS_LOG_INFO ("No NIST BSR report to transmit");
        if (m_ulBsrReceived.size () == 0)
        {
            NS_LOG_INFO ("No BSR report to transmit");
            return;
        }
        cv2x_MacCeListElement_s bsr;
        bsr.m_rnti = m_rnti;
        bsr.m_macCeType = cv2x_MacCeListElement_s::BSR;

        // BSR is reported for each LCG
        std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
        std::vector<uint32_t> queue (4, 0); // one value per each of the 4 LCGs, initialized to 0
        for (it = m_ulBsrReceived.begin (); it != m_ulBsrReceived.end (); it++)
        {
            uint8_t lcid = it->first;
            std::map <uint8_t, LcInfo>::iterator lcInfoMapIt;
            lcInfoMapIt = m_lcInfoMap.find (lcid);
            NS_ASSERT (lcInfoMapIt !=  m_lcInfoMap.end ());
            NS_ASSERT_MSG ((lcid != 0) || (((*it).second.txQueueSize == 0)
                                           && ((*it).second.retxQueueSize == 0)
                                           && ((*it).second.statusPduSize == 0)),
                           "BSR should not be used for LCID 0");
            uint8_t lcg = lcInfoMapIt->second.lcConfig.logicalChannelGroup;
            queue.at (lcg) += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        }

        // FF API says that all 4 LCGs are always present
        bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (0)));
        bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (1)));
        bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (2)));
        bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (3)));

        // create the feedback to eNB
        Ptr<cv2x_BsrLteControlMessage> msg = Create<cv2x_BsrLteControlMessage> ();
        msg->SetBsr (bsr);
        m_uePhySapProvider->SendLteControlMessage (msg);
    }

    cv2x_MacCeListElement_s bsr;
    bsr.m_rnti = m_rnti;
    bsr.m_macCeType = cv2x_MacCeListElement_s::BSR;

    // BSR is reported for each LCG

    // NIST new iterator for m_ulBsrReceived
    std::map <uint8_t, std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters >>:: iterator it;
    it = nist_m_ulBsrReceived.find(m_rnti);

    if (it != nist_m_ulBsrReceived.end())
    {
        std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > mapLC = it->second;
        std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters >::iterator it1 = mapLC.begin();

        std::vector<uint32_t> queue (4, 0); // one value per each of the 4 LCGs, initialized to 0
        std::vector<uint32_t> queueLCG (4, 0); // this queue is used to fill in the ns3 structure
        for (it1= mapLC.begin (); it1 != mapLC.end (); it1++)
        {
            uint8_t lcid = it1->first;
            std::map <uint8_t, LcInfo>::iterator lcInfoMapIt;
            lcInfoMapIt = m_lcInfoMap.find (lcid);
            NS_ASSERT (lcInfoMapIt !=  m_lcInfoMap.end ());
            NS_ASSERT_MSG ((lcid != 0) || (((*it1).second.txQueueSize == 0)
                                           && ((*it1).second.retxQueueSize == 0)
                                           && ((*it1).second.statusPduSize == 0)),
                           "BSR should not be used for LCID 0");
            uint8_t lcg = lcInfoMapIt->second.lcConfig.logicalChannelGroup;
            queue.at (lcg) = ((*it1).second.txQueueSize + (*it1).second.retxQueueSize + (*it1).second.statusPduSize);
            queueLCG.at (lcg) += ((*it1).second.txQueueSize + (*it1).second.retxQueueSize + (*it1).second.statusPduSize); // this queue is used to fill in the ns3 structure
            std::vector <uint8_t>  bufferStatus (4,0);
            bufferStatus[lcg]=cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (lcg));
            bsr.m_macCeValue.m_SlBufferStatus.insert (std::pair <uint8_t, std::vector <uint8_t> > (lcid, bufferStatus));
        }

        //filling in the structure of bsr buffer implemented by ns3 beacause if UE scheduler is RR , it will check this structure
        bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queueLCG.at (0)));
        bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queueLCG.at (1)));
        bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queueLCG.at (2)));
        bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queueLCG.at (3)));
        // end added

        // create the feedback to eNB
        Ptr<cv2x_BsrLteControlMessage> msg = Create<cv2x_BsrLteControlMessage> ();
        msg->SetBsr (bsr);
        m_uePhySapProvider->SendLteControlMessage (msg);
    }
}

void
cv2x_LteUeMac::SendSidelinkReportBufferStatus (void)
{
    NS_LOG_FUNCTION (this);

    if (m_rnti == 0)
    {
        NS_LOG_INFO ("MAC not initialized, BSR deferred");
        return;
    }

    //check if we have at scheduled pools
    bool scheduled = false;
    for (std::map <uint32_t, PoolInfo >::iterator slTxPoolIt = m_sidelinkTxPoolsMap.begin (); slTxPoolIt != m_sidelinkTxPoolsMap.end () && !scheduled; slTxPoolIt++)
    {
        if (slTxPoolIt->second.m_pool->GetSchedulingType () == SidelinkCommResourcePool::SCHEDULED)
            scheduled = true;
    }

    if (m_slBsrReceived.size () == 0 || !scheduled)
    {
        NS_LOG_INFO (this << " No SL BSR report to transmit. SL BSR size=" << m_slBsrReceived.size ());
        return;
    }
    cv2x_MacCeListElement_s bsr;
    bsr.m_rnti = m_rnti;
    bsr.m_macCeType = cv2x_MacCeListElement_s::SLBSR;

    // SL BSR is reported for each Group Index

    std::map <SidelinkLcIdentifier, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator it;

    std::vector<uint32_t> queue (4, 0); //todo: change to create based on the number of destinations, initialized to 0
    for (it = m_slBsrReceived.begin (); it != m_slBsrReceived.end (); it++)
    {
        //uint8_t lcid = it->first.lcId;
        uint32_t dstL2Id = it->first.dstL2Id;

        std::map <SidelinkLcIdentifier, LcInfo>::iterator slLcInfoMapIt;
        slLcInfoMapIt = m_slLcInfoMap.find (it->first);
        NS_ASSERT (slLcInfoMapIt !=  m_slLcInfoMap.end ());
        //TODO: find the mapping between the destination and the group index (must be provided by RRC)
        //uint8_t lcg = slLcInfoMapIt->second.lcConfig.logicalChannelGroup;
        //queue.at (lcg) += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        std::map <uint32_t, PoolInfo >::iterator slTxPoolIt;
        slTxPoolIt = m_sidelinkTxPoolsMap.find (dstL2Id);
        Ptr<SidelinkTxCommResourcePool> pool = DynamicCast<SidelinkTxCommResourcePool> (slTxPoolIt->second.m_pool);
        NS_ASSERT (slTxPoolIt != m_sidelinkTxPoolsMap.end ());
        if (pool->GetSchedulingType() == SidelinkCommResourcePool::SCHEDULED)
        {
            queue.at (pool->GetIndex()) += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        }
    }

    // store
    bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (0)));
    bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (1)));
    bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (2)));
    bsr.m_macCeValue.m_bufferStatus.push_back (cv2x_BufferSizeLevelBsr::BufferSize2BsrId (queue.at (3)));

    // create the feedback to eNB
    Ptr<cv2x_BsrLteControlMessage> msg = Create<cv2x_BsrLteControlMessage> ();
    msg->SetBsr (bsr);
    m_uePhySapProvider->SendLteControlMessage (msg);

}

void
cv2x_LteUeMac::SendSidelinkReportBufferStatusV2x (void)
{
    NS_LOG_FUNCTION (this);

    if(m_rnti == 0)
    {
        NS_LOG_INFO ("MAC not initialized, BSR deferred");
        return;
    }

    //check if we have at scheduled V2X pools
    bool scheduled = false;
    for (std::map<uint32_t, PoolInfoV2x>::iterator slTxPoolIt = m_sidelinkTxPoolsMapV2x.begin(); slTxPoolIt != m_sidelinkTxPoolsMapV2x.end() && !scheduled; slTxPoolIt++)
    {
        if (slTxPoolIt->second.m_pool->GetSchedulingType () == SidelinkCommResourcePoolV2x::SCHEDULED)
            scheduled = true;
    }

    if (m_slBsrReceived.size () == 0 || !scheduled)
    {
        NS_LOG_INFO (this << " No SL BSR report to transmit. SL BSR size=" << m_slBsrReceived.size ());
        return;
    }
    cv2x_MacCeListElement_s bsr;
    bsr.m_rnti = m_rnti;
    bsr.m_macCeType = cv2x_MacCeListElement_s::SLBSR;

    // SL BSR is reported for each Group Index

    std::map <SidelinkLcIdentifier, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator it;

    std::vector<uint32_t> queue (4, 0); //todo: change to create based on the number of destinations, initialized to 0
    for (it = m_slBsrReceived.begin (); it != m_slBsrReceived.end (); it++)
    {
        //uint8_t lcid = it->first.lcId;
        uint32_t dstL2Id = it->first.dstL2Id;

        std::map <SidelinkLcIdentifier, LcInfo>::iterator slLcInfoMapIt;
        slLcInfoMapIt = m_slLcInfoMap.find (it->first);
        NS_ASSERT (slLcInfoMapIt !=  m_slLcInfoMap.end ());
        //TODO: find the mapping between the destination and the group index (must be provided by RRC)
        //uint8_t lcg = slLcInfoMapIt->second.lcConfig.logicalChannelGroup;
        //queue.at (lcg) += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        std::map <uint32_t, PoolInfoV2x >::iterator slTxPoolIt;
        slTxPoolIt = m_sidelinkTxPoolsMapV2x.find (dstL2Id);
        Ptr<SidelinkTxCommResourcePoolV2x> pool = DynamicCast<SidelinkTxCommResourcePoolV2x> (slTxPoolIt->second.m_pool);
        NS_ASSERT (slTxPoolIt != m_sidelinkTxPoolsMapV2x.end ());
        if (pool->GetSchedulingType() == SidelinkCommResourcePoolV2x::SCHEDULED)
        {
            queue.at (pool->GetIndex()) += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        }
    }
}


void
cv2x_LteUeMac::RandomlySelectAndSendRaPreamble ()
{
    NS_LOG_FUNCTION (this);
    // 3GPP 36.321 5.1.1
    NS_ASSERT_MSG (m_rachConfigured, "RACH not configured");
    // assume that there is no Random Access Preambles group B
    m_raPreambleId = m_raPreambleUniformVariable->GetInteger (0, m_rachConfig.numberOfRaPreambles - 1);
    bool contention = true;
    SendRaPreamble (contention);
}

void
cv2x_LteUeMac::SendRaPreamble (bool contention)
{
    NS_LOG_FUNCTION (this << (uint32_t) m_raPreambleId << contention);
    // Since regular UL cv2x_LteControlMessages need m_ulConfigured = true in
    // order to be sent by the UE, the rach preamble needs to be sent
    // with a dedicated primitive (not
    // m_uePhySapProvider->SendLteControlMessage (msg)) so that it can
    // bypass the m_ulConfigured flag. This is reasonable, since In fact
    // the RACH preamble is sent on 6RB bandwidth so the uplink
    // bandwidth does not need to be configured.
    NS_ASSERT (m_subframeNo > 0); // sanity check for subframe starting at 1
    m_raRnti = m_subframeNo - 1;
    m_uePhySapProvider->SendRachPreamble (m_raPreambleId, m_raRnti);
    NS_LOG_INFO (this << " sent preamble id " << (uint32_t) m_raPreambleId << ", RA-RNTI " << (uint32_t) m_raRnti);
    // 3GPP 36.321 5.1.4
    Time raWindowBegin = MilliSeconds (3);
    Time raWindowEnd = MilliSeconds (3 + m_rachConfig.raResponseWindowSize);
    Simulator::Schedule (raWindowBegin, &cv2x_LteUeMac::StartWaitingForRaResponse, this);
    m_noRaResponseReceivedEvent = Simulator::Schedule (raWindowEnd, &cv2x_LteUeMac::RaResponseTimeout, this, contention);
}

void
cv2x_LteUeMac::StartWaitingForRaResponse ()
{
    NS_LOG_FUNCTION (this);
    m_waitingForRaResponse = true;
}

void
cv2x_LteUeMac::RecvRaResponse (cv2x_BuildRarListElement_s raResponse)
{
    NS_LOG_FUNCTION (this);
    m_waitingForRaResponse = false;
    m_noRaResponseReceivedEvent.Cancel ();
    NS_LOG_INFO ("got RAR for RAPID " << (uint32_t) m_raPreambleId << ", setting T-C-RNTI = " << raResponse.m_rnti);
    m_rnti = raResponse.m_rnti;
    m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
    // in principle we should wait for contention resolution,
    // but in the current LTE model when two or more identical
    // preambles are sent no one is received, so there is no need
    // for contention resolution
    m_cmacSapUser->NotifyRandomAccessSuccessful ();
    // trigger tx opportunity for Message 3 over LC 0
    // this is needed since Message 3's UL GRANT is in the RAR, not in UL-DCIs
    const uint8_t lc0Lcid = 0;
    std::map <uint8_t, LcInfo>::iterator lc0InfoIt = m_lcInfoMap.find (lc0Lcid);
    NS_ASSERT (lc0InfoIt != m_lcInfoMap.end ());

    //added
    std::map <uint8_t, std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > >:: iterator it;
    it=nist_m_ulBsrReceived.find(m_rnti);
    if (it!=nist_m_ulBsrReceived.end())
    {
        std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > mapLC=it->second;
        std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters >::iterator lc0BsrIt;
        lc0BsrIt=mapLC.find(lc0Lcid);

        if ((lc0BsrIt != mapLC.end ())
            && (lc0BsrIt->second.txQueueSize > 0))
        {
            NS_ASSERT_MSG (raResponse.m_grant.m_tbSize > lc0BsrIt->second.txQueueSize,
                           "segmentation of Message 3 is not allowed");
            // TODO Fix
            lc0InfoIt->second.macSapUser->NotifyTxOpportunity (raResponse.m_grant.m_tbSize, 0, 0, 0, m_rnti, lc0Lcid);
            lc0BsrIt->second.txQueueSize = 0;
        }
    }
    else
    {
        std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator lc0BsrIt
                = m_ulBsrReceived.find (lc0Lcid);
        if ((lc0BsrIt != m_ulBsrReceived.end ())
            && (lc0BsrIt->second.txQueueSize > 0))
        {
            NS_ASSERT_MSG (raResponse.m_grant.m_tbSize > lc0BsrIt->second.txQueueSize,
                           "segmentation of Message 3 is not allowed");
            // this function can be called only from primary carrier
            if (m_componentCarrierId > 0)
            {
                NS_FATAL_ERROR ("Function called on wrong componentCarrier");
            }
            lc0InfoIt->second.macSapUser->NotifyTxOpportunity (raResponse.m_grant.m_tbSize, 0, 0, m_componentCarrierId, m_rnti, lc0Lcid);
            lc0BsrIt->second.txQueueSize = 0;
        }
    }
}

void
cv2x_LteUeMac::RaResponseTimeout (bool contention)
{
    NS_LOG_FUNCTION (this << contention);
    m_waitingForRaResponse = false;
    // 3GPP 36.321 5.1.4
    ++m_preambleTransmissionCounter;
    if (m_preambleTransmissionCounter == m_rachConfig.preambleTransMax + 1)
    {
        NS_LOG_INFO ("RAR timeout, preambleTransMax reached => giving up");
        m_cmacSapUser->NotifyRandomAccessFailed ();
    }
    else
    {
        NS_LOG_INFO ("RAR timeout, re-send preamble");
        if (contention)
        {
            RandomlySelectAndSendRaPreamble ();
        }
        else
        {
            SendRaPreamble (contention);
        }
    }
}

void
cv2x_LteUeMac::DoConfigureRach (cv2x_LteUeCmacSapProvider::RachConfig rc)
{
    NS_LOG_FUNCTION (this);
    m_rachConfig = rc;
    m_rachConfigured = true;
}

void
cv2x_LteUeMac::DoStartContentionBasedRandomAccessProcedure ()
{
    NS_LOG_FUNCTION (this);

    // 3GPP 36.321 5.1.1
    NS_ASSERT_MSG (m_rachConfigured, "RACH not configured");
    m_preambleTransmissionCounter = 0;
    m_backoffParameter = 0;
    RandomlySelectAndSendRaPreamble ();
}

void
cv2x_LteUeMac::DoSetRnti (uint16_t rnti)
{
    NS_LOG_FUNCTION (this);
    m_rnti = rnti;
}


void
cv2x_LteUeMac::DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask)
{
    NS_LOG_FUNCTION (this << " rnti" << rnti);
    NS_ASSERT_MSG (prachMask == 0, "requested PRACH MASK = " << (uint32_t) prachMask << ", but only PRACH MASK = 0 is supported");
    m_rnti = rnti;
    m_raPreambleId = preambleId;
    bool contention = false;
    SendRaPreamble (contention);
}

void
cv2x_LteUeMac::DoAddLc (uint8_t lcId,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu)
{
    NS_LOG_FUNCTION (this << " lcId" << (uint32_t) lcId);
    NS_ASSERT_MSG (m_lcInfoMap.find (lcId) == m_lcInfoMap.end (), "cannot add channel because LCID " << lcId << " is already present");

    LcInfo lcInfo;
    lcInfo.lcConfig = lcConfig;
    lcInfo.macSapUser = msu;
    m_lcInfoMap[lcId] = lcInfo;
}

void
cv2x_LteUeMac::DoAddLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id, cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu)
{
    NS_LOG_FUNCTION (this << (uint32_t) lcId << srcL2Id << dstL2Id);
    SidelinkLcIdentifier sllcid;
    sllcid.lcId = lcId;
    sllcid.srcL2Id = srcL2Id;
    sllcid.dstL2Id = dstL2Id;

    NS_ASSERT_MSG (m_slLcInfoMap.find (sllcid) == m_slLcInfoMap.end (), "cannot add channel because LCID " << lcId << ", srcL2Id " << srcL2Id << ", dstL2Id " << dstL2Id << " is already present");

    LcInfo lcInfo;
    lcInfo.lcConfig = lcConfig;
    lcInfo.macSapUser = msu;
    m_slLcInfoMap[sllcid] = lcInfo;
}

// added function to handle LC priority
void
cv2x_LteUeMac::DoAddLCPriority ( uint8_t rnti, uint8_t lcid, uint8_t priority)
{
    std::map <uint8_t, std::map <uint8_t, uint8_t> >::iterator it;
    it = PriorityMap.find(rnti);
    if (it == PriorityMap.end())
    {
        // insert new rnti in the map
        std::map<uint8_t, uint8_t> tempMap;
        tempMap.insert (std::pair<uint8_t, uint8_t> (lcid, priority));
        PriorityMap.insert (std::pair <uint8_t, std::map<uint8_t, uint8_t > > (rnti, tempMap));

    }
    else
    {
        // check if LC exists already or not
        std::map <uint8_t, uint8_t> mapLC=it->second;
        std::map <uint8_t, uint8_t>::iterator itLC;
        itLC=mapLC.find(lcid);
        if (itLC==mapLC.end())
        {
            // LC doesn't exist in the map
            it->second.insert (std::pair<uint8_t, uint8_t> (lcid, priority));
        }
    }
    return;
}

void
cv2x_LteUeMac::DoRemoveLc (uint8_t lcId)
{
    NS_LOG_FUNCTION (this << " lcId" << lcId);
    NS_ASSERT_MSG (m_lcInfoMap.find (lcId) != m_lcInfoMap.end (), "could not find LCID " << lcId);
    m_lcInfoMap.erase (lcId);

    // added code to remove the LC from the LC priority map
    std::map <uint8_t, std::map <uint8_t, uint8_t> >::iterator it;
    it=PriorityMap.find(m_rnti);
    it->second.erase(lcId);
}

void
cv2x_LteUeMac::DoRemoveLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id)
{
    NS_LOG_FUNCTION (this << " lcId" << lcId << ", srcL2Id=" << srcL2Id << ", dstL2Id" << dstL2Id);
    //    NS_ASSERT_MSG (m_lcInfoMap.find (lcId) != m_lcInfoMap.end (), "could not find LCID " << lcId);
    //    m_lcInfoMap.erase (lcId);
}

void
cv2x_LteUeMac::DoReset ()
{
    NS_LOG_FUNCTION (this);
    std::map <uint8_t, LcInfo>::iterator it = m_lcInfoMap.begin ();
    while (it != m_lcInfoMap.end ())
    {
        // don't delete CCCH)
        if (it->first == 0)
        {
            ++it;
        }
        else
        {
            // note: use of postfix operator preserves validity of iterator
            m_lcInfoMap.erase (it++);
        }
    }

    m_noRaResponseReceivedEvent.Cancel ();
    m_rachConfigured = false;
    m_freshUlBsr = false;
    m_ulBsrReceived.clear ();
}

void
cv2x_LteUeMac::DoAddSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool)
{
    //NS_ASSERT_MSG (m_discTxPools.m_pool != NULL, "Cannot add discovery transmission pool for " << m_rnti << ". Pool already exist for destination");
    DiscPoolInfo info;
    info.m_pool = pool;
    info.m_npsdch = info.m_pool->GetNPsdch();
    info.m_currentDiscPeriod.frameNo = 0; //init to 0 to make it invalid
    info.m_currentDiscPeriod.subframeNo = 0; //init to 0 to make it invalid
    info.m_nextDiscPeriod = info.m_pool->GetNextDiscPeriod (m_frameNo, m_subframeNo);
    //adjust because scheduler starts with frame/subframe = 1
    info.m_nextDiscPeriod.frameNo++;
    info.m_nextDiscPeriod.subframeNo++;
    info.m_grant_received = false;
    m_discTxPools = info;
}

void
cv2x_LteUeMac::DoRemoveSlTxPool ()
{
    m_discTxPools.m_pool = NULL;
}

void
cv2x_LteUeMac::DoSetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools)
{
m_discRxPools = pools;
}

void
cv2x_LteUeMac::DoModifyDiscTxApps (std::list<uint32_t> apps)
{
    m_discTxApps = apps;
    m_cphySapProvider->AddDiscTxApps (apps);
}

void
cv2x_LteUeMac::DoModifyDiscRxApps (std::list<uint32_t> apps)
{
    m_discRxApps = apps;
    m_cphySapProvider->AddDiscRxApps (apps);
}

void
cv2x_LteUeMac::DoAddSlTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePool> pool)
{
    std::map <uint32_t, PoolInfo >::iterator it;
    it = m_sidelinkTxPoolsMap.find (dstL2Id);
    NS_ASSERT_MSG (it == m_sidelinkTxPoolsMap.end (), "Cannot add sidelink transmission pool for " << dstL2Id << ". Pool already exist for destination");
    PoolInfo info;
    info.m_pool = pool;
    info.m_npscch = info.m_pool->GetNPscch();
    info.m_currentScPeriod.frameNo = 0; //init to 0 to make it invalid
    info.m_currentScPeriod.subframeNo = 0; //init to 0 to make it invalid
    info.m_nextScPeriod = info.m_pool->GetNextScPeriod (m_frameNo, m_subframeNo);
    //adjust because scheduler starts with frame/subframe = 1
    info.m_nextScPeriod.frameNo++;
    info.m_nextScPeriod.subframeNo++;
    info.m_grant_received = false;

    m_sidelinkTxPoolsMap.insert (std::pair<uint32_t, PoolInfo > (dstL2Id, info));
}

void
cv2x_LteUeMac::DoAddSlV2xTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePoolV2x> pool)
{
    std::map <uint32_t, PoolInfoV2x>::iterator it;
    it = m_sidelinkTxPoolsMapV2x.find(dstL2Id);
    NS_ASSERT_MSG (it == m_sidelinkTxPoolsMapV2x.end (), "Cannot add sidelink transmission pool for " << dstL2Id << ". Pool already exist for destination");
    PoolInfoV2x info;
    info.m_pool = pool;
    info.m_currentFrameInfo.frameNo = 0; //init to 0 to make it invalid
    info.m_currentFrameInfo.subframeNo = 0; //init to 0 to make it invalid
    info.m_grant_received = false;

    m_sidelinkTxPoolsMapV2x.insert (std::pair<uint32_t, PoolInfoV2x> (dstL2Id, info));
}

void
cv2x_LteUeMac::DoRemoveSlTxPool (uint32_t dstL2Id)
{
    std::map <uint32_t, PoolInfo >::iterator it;
    it = m_sidelinkTxPoolsMap.find (dstL2Id);
    NS_ASSERT_MSG (it != m_sidelinkTxPoolsMap.end (), "Cannot remove sidelink transmission pool for " << dstL2Id << ". Unknown destination");
    m_sidelinkTxPoolsMap.erase (dstL2Id);
}

void
cv2x_LteUeMac::DoRemoveSlV2xTxPool (uint32_t dstL2Id)
{
    std::map <uint32_t, PoolInfoV2x>::iterator it;
    it = m_sidelinkTxPoolsMapV2x.find (dstL2Id);
    NS_ASSERT_MSG (it != m_sidelinkTxPoolsMapV2x.end (), "Cannot remove sidelink transmission pool for " << dstL2Id << ". Unknown destination");
    m_sidelinkTxPoolsMapV2x.erase (dstL2Id);
}

void
cv2x_LteUeMac::DoSetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools)
{
m_sidelinkRxPools = pools;
}

void
cv2x_LteUeMac::DoSetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools)
{
m_sidelinkRxPoolsV2x = pools;
}

void
cv2x_LteUeMac::DoReceivePhyPdu (Ptr<Packet> p)
{
    std::tuple<double, double> signalInfo = GetSignalInfo();

    RssiTag rssi;
    rssi.Set (std::get<0>(signalInfo));
    p->AddPacketTag (rssi);

    RsrpTag rsrp;
    rsrp.Set(std::get<1>(signalInfo));
    p->AddPacketTag (rsrp);

    cv2x_LteRadioBearerTag tag;
    p->RemovePacketTag (tag);

    if (tag.GetSourceL2Id () == 0)
    {
        if (tag.GetRnti () == m_rnti)
        {
            NS_LOG_INFO ("Received downlink packet");
            //regular downlink packet
            // packet is for the current user
            std::map <uint8_t, LcInfo>::const_iterator it = m_lcInfoMap.find (tag.GetLcid ());
            if (it != m_lcInfoMap.end ())
            {
                it->second.macSapUser->ReceivePdu (p, m_rnti, tag.GetLcid ());
            }
            else
            {
                NS_LOG_WARN ("received packet with unknown lcid " << (uint32_t) tag.GetLcid ());
            }
        }
    }
    else
    {
        //sidelink packet. Perform L2 filtering
        NS_LOG_INFO ("Received sidelink packet");
        std::list <uint32_t>::iterator dstIt;
        bool found = false;
        for (dstIt = m_sidelinkDestinations.begin (); dstIt != m_sidelinkDestinations.end () ; dstIt++)
        {
            NS_LOG_LOGIC (this << " dstIt: " << *dstIt << "; destinationL2Id: " << tag.GetDestinationL2Id ());
            if ((*dstIt) == tag.GetDestinationL2Id ())
            {
                //the destination is a group we want to listen to
                SidelinkLcIdentifier identifier;
                identifier.lcId = tag.GetLcid ();
                identifier.srcL2Id = tag.GetSourceL2Id ();
                identifier.dstL2Id = tag.GetDestinationL2Id ();

                std::map <SidelinkLcIdentifier, LcInfo>::iterator it = m_slLcInfoMap.find (identifier);
                if (it == m_slLcInfoMap.end ())
                {
                    //notify RRC to setup bearer
                    m_cmacSapUser->NotifySidelinkReception (tag.GetLcid(), tag.GetSourceL2Id (), tag.GetDestinationL2Id ());

                    //should be setup now
                    it = m_slLcInfoMap.find (identifier);
                    if (it == m_slLcInfoMap.end ())
                    {
                        NS_LOG_WARN ("Failure to setup sidelink radio bearer");
                    }
                }
                it->second.macSapUser->ReceivePdu (p, m_rnti, tag.GetLcid ());

                found = true;
                break;
            }
        }
        if (!found)
        {
            NS_LOG_INFO ("received packet with unknown destination " << tag.GetDestinationL2Id ());
        }
    }
}

void
cv2x_LteUeMac::DoReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
    if ( m_UlScheduler == "ns3::cv2x_PfFfMacScheduler")
    {
        DoReceivePFLteControlMessage (msg);
    }
    else if ( m_UlScheduler == "ns3::MtFfMacScheduler" )
    {
        DoReceiveMTLteControlMessage (msg);
    }
    else if ( m_UlScheduler == "ns3::cv2x_PriorityFfMacScheduler" )
    {
        DoReceivePrLteControlMessage (msg);
    }
    else if (m_UlScheduler == "ns3::cv2x_RrSlFfMacScheduler")
    {
        DoReceiveRrLteControlMessage (msg);
    }
    else
    {
        DoReceiveRrLteControlMessage (msg);
        //std::cout<<" UE RR SCHEDULER "<<std::endl;
    }
}

void
cv2x_LteUeMac::DoReceiveRrLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
    //std::cout<<" ENTER DoReceiveRrLteControlMessage "<<std::endl;
    NS_LOG_FUNCTION (this);
    if (msg->GetMessageType () == cv2x_LteControlMessage::UL_DCI)
    {
        Ptr<cv2x_UlDciLteControlMessage> msg2 = DynamicCast<cv2x_UlDciLteControlMessage> (msg);
        cv2x_UlDciListElement_s dci = msg2->GetDci ();
        if (dci.m_ndi == 1)
        {
            // New transmission -> emtpy pkt buffer queue (for deleting eventual pkts not acked )
            Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
            m_miUlHarqProcessesPacket.at (m_harqProcessId) = pb;
            // Retrieve data from RLC
            std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
            uint16_t activeLcs = 0;
            uint32_t statusPduMinSize = 0;
            //added code
            if (m_ulBsrReceived.size()!=0)
            {
                for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
                {
                    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                    {
                        activeLcs++;
                        if (((*itBsr).second.statusPduSize != 0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                        {
                            statusPduMinSize = (*itBsr).second.statusPduSize;
                        }
                        if (((*itBsr).second.statusPduSize != 0)&&(statusPduMinSize == 0))
                        {
                            statusPduMinSize = (*itBsr).second.statusPduSize;
                        }
                    }
                }
                if (activeLcs == 0)
                {
                    NS_LOG_ERROR (this << " No active flows for this UL-DCI");
                    return;
                }
                std::map <uint8_t, LcInfo>::iterator it;
                uint32_t bytesPerActiveLc = dci.m_tbSize / activeLcs;
                bool statusPduPriority = false;
                if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
                {
                    // send only the status PDU which has highest priority
                    statusPduPriority = true;
                    NS_LOG_DEBUG (this << " Reduced resource -> send only Status, bytes " << statusPduMinSize);
                    if (dci.m_tbSize < statusPduMinSize)
                    {
                        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                    }
                }
                NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);
                for (it = m_lcInfoMap.begin (); it != m_lcInfoMap.end (); it++)
                {
                    itBsr = m_ulBsrReceived.find ((*it).first);
                    NS_LOG_DEBUG (this << " Processing LC " << (uint32_t)(*it).first << " bytesPerActiveLc " << bytesPerActiveLc);
                    if ( (itBsr != m_ulBsrReceived.end ())
                         && ( ((*itBsr).second.statusPduSize > 0)
                              || ((*itBsr).second.retxQueueSize > 0)
                              || ((*itBsr).second.txQueueSize > 0)) )
                    {
                        if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                        {
                            (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  " << (*itBsr).second.statusPduSize << " status bytes to LC " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            (*itBsr).second.statusPduSize = 0;
                            break;
                        }
                        else
                        {
                            uint32_t bytesForThisLc = bytesPerActiveLc;
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                            {
                                (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                bytesForThisLc -= (*itBsr).second.statusPduSize;
                                NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {
                                if ((*itBsr).second.statusPduSize > bytesForThisLc)
                                {
                                    NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                }
                            }

                            if ((bytesForThisLc > 7)    // 7 is the min TxOpportunity useful for Rlc
                                && (((*itBsr).second.retxQueueSize > 0)
                                    || ((*itBsr).second.txQueueSize > 0)))
                            {
                                if ((*itBsr).second.retxQueueSize > 0)
                                {
                                    NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                    {
                                        (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                    }
                                    else
                                    {
                                        (*itBsr).second.retxQueueSize = 0;
                                    }
                                }
                                else if ((*itBsr).second.txQueueSize > 0)
                                {
                                    uint16_t lcid = (*it).first;
                                    uint32_t rlcOverhead;
                                    if (lcid == 1)
                                    {
                                        // for SRB1 (using RLC AM) it's better to
                                        // overestimate RLC overhead rather than
                                        // underestimate it and risk unneeded
                                        // segmentation which increases delay
                                        rlcOverhead = 4;
                                    }
                                    else
                                    {
                                        // minimum RLC overhead due to header
                                        rlcOverhead = 2;
                                    }
                                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                    {
                                        (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                                    }
                                    else
                                    {
                                        (*itBsr).second.txQueueSize = 0;
                                    }
                                }
                            }
                            else
                            {
                                if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                {
                                    // resend BSR info for updating eNB peer MAC
                                    m_freshUlBsr = true;
                                }
                            }
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                        }

                    }
                }
            } // end if m_ulBsrReceived.size()!=0
            else
            {
                // NIST IMPLEMENTATION OF nist_m_ulBsrReceived
                uint16_t activeLcs = 0;
                uint32_t statusPduMinSize = 0;
                std::map <uint8_t, std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > >:: iterator it;
                it=nist_m_ulBsrReceived.find(m_rnti);
                if (it!=nist_m_ulBsrReceived.end())
                {
                    // Retrieve data from RLC
                    std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > mapLC=it->second;
                    std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;

                    for (itBsr = mapLC.begin (); itBsr != mapLC.end (); itBsr++)
                    {
                        if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                        {
                            activeLcs++;
                            if (((*itBsr).second.statusPduSize!=0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                            {
                                statusPduMinSize = (*itBsr).second.statusPduSize;
                            }
                            if (((*itBsr).second.statusPduSize!=0)&&(statusPduMinSize == 0))
                            {
                                statusPduMinSize = (*itBsr).second.statusPduSize;
                            }
                        }// end if
                    }// end for

                    if (activeLcs == 0)
                    {
                        NS_LOG_ERROR (this << " No active flows for this UL-DCI");
                        return;
                    }
                    for (itBsr = mapLC.begin (); itBsr != mapLC.end (); itBsr++)
                    {
                        // compute tb size for this lc
                        uint32_t bytesPerActiveLc= dci.m_tbSize / activeLcs ;
                        std::map <uint8_t, LcInfo>::iterator itLcInfo;
                        bool statusPduPriority = false;
                        if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
                        {
                            // send only the status PDU which has highest priority
                            statusPduPriority = true;
                            NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes " << statusPduMinSize);
                            if (dci.m_tbSize < statusPduMinSize)
                            {
                                NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                            }
                        }
                        NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);

                        if ( (itBsr!=mapLC.end ()) &&
                             ( ((*itBsr).second.statusPduSize > 0) ||
                               ((*itBsr).second.retxQueueSize > 0) ||
                               ((*itBsr).second.txQueueSize > 0)) )
                        {
                            itLcInfo=m_lcInfoMap.find((*itBsr).first);

                            if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                            {

                                (*itLcInfo).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);

                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {

                                uint32_t bytesForThisLc = bytesPerActiveLc;
                                NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*itLcInfo).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                                if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                                {
                                    (*itLcInfo).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    bytesForThisLc -= (*itBsr).second.statusPduSize;
                                    NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                    (*itBsr).second.statusPduSize = 0;

                                }
                                else
                                {
                                    if ((*itBsr).second.statusPduSize>bytesForThisLc)
                                    {
                                        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                    }
                                }

                                if ((bytesForThisLc > 7) && // 7 is the min TxOpportunity useful for Rlc
                                    (((*itBsr).second.retxQueueSize > 0) ||
                                     ((*itBsr).second.txQueueSize > 0)))
                                {
                                    if ((*itBsr).second.retxQueueSize > 0)
                                    {
                                        NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);

                                        (*itLcInfo).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                        if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                        {
                                            (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                        }
                                        else
                                        {
                                            (*itBsr).second.retxQueueSize = 0;
                                        }
                                    }
                                    else if ((*itBsr).second.txQueueSize > 0)
                                    {
                                        uint16_t lcid = (*itLcInfo).first;
                                        uint32_t rlcOverhead;
                                        if (lcid == 1)
                                        {
                                            // for SRB1 (using RLC AM) it's better to
                                            // overestimate RLC overhead rather than
                                            // underestimate it and risk unneeded
                                            // segmentation which increases delay
                                            rlcOverhead = 4;
                                        }
                                        else
                                        {
                                            // minimum RLC overhead due to header
                                            rlcOverhead = 2;
                                        }
                                        NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                        (*itLcInfo).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                        if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                        {
                                            (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;

                                        }
                                        else
                                        {
                                            (*itBsr).second.txQueueSize = 0;

                                        }
                                    }
                                }
                                else
                                {
                                    if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                    {
                                        // resend BSR info for updating eNB peer MAC
                                        m_freshUlBsr = true;
                                    }
                                }
                                NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*itLcInfo).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            }

                        }
                    }
                }// end for p
            } // end it!=nist_m_ulBsrReceived.end()

        }
        else
        {
            // HARQ retransmission -> retrieve data from HARQ buffer
            NS_LOG_DEBUG (this << " UE MAC RETX HARQ " << (uint16_t)m_harqProcessId);
            Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_harqProcessId);
            for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
            {
                Ptr<Packet> pkt = (*j)->Copy ();
                m_uePhySapProvider->SendMacPdu (pkt);
            }
            m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        }

    }
    else if (msg->GetMessageType () == cv2x_LteControlMessage::RAR)
    {
        if (m_waitingForRaResponse)
        {
            Ptr<cv2x_RarLteControlMessage> rarMsg = DynamicCast<cv2x_RarLteControlMessage> (msg);
            uint16_t raRnti = rarMsg->GetRaRnti ();
            NS_LOG_LOGIC (this << "got RAR with RA-RNTI " << (uint32_t) raRnti << ", expecting " << (uint32_t) m_raRnti);
            if (raRnti == m_raRnti) // RAR corresponds to TX subframe of preamble
            {
                for (std::list<cv2x_RarLteControlMessage::Rar>::const_iterator it = rarMsg->RarListBegin ();
                     it != rarMsg->RarListEnd ();
                     ++it)
                {
                    if (it->rapId == m_raPreambleId) // RAR is for me
                    {
                        RecvRaResponse (it->rarPayload);
                        /// \todo RRC generates the RecvRaResponse messaged
                        /// for avoiding holes in transmission at PHY layer
                        /// (which produce erroneous UL CQI evaluation)
                    }
                }
            }
        }
    }
    else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DCI)
    {
        Ptr<cv2x_SlDciLteControlMessage> msg2 = DynamicCast<cv2x_SlDciLteControlMessage> (msg);
        cv2x_SlDciListElement_s dci = msg2->GetDci ();

        //store the grant for the next SC period
        //TODO: distinguish SL grants for different pools. Right now just assume there is only one pool
        Ptr<SidelinkTxCommResourcePool> pool = DynamicCast<SidelinkTxCommResourcePool> (m_sidelinkTxPoolsMap.begin()->second.m_pool);
        NS_ASSERT (pool->GetSchedulingType() == SidelinkCommResourcePool::SCHEDULED);

        SidelinkGrant grant;
        grant.m_resPscch = dci.m_resPscch;
        grant.m_tpc = dci.m_tpc;
        grant.m_hopping = dci.m_hopping;
        grant.m_rbStart = dci.m_rbStart;
        grant.m_rbLen = dci.m_rbLen;
        grant.m_trp = dci.m_trp;
        grant.m_mcs = pool->GetMcs();
        grant.m_tbSize = 0; //computed later
        m_sidelinkTxPoolsMap.begin()->second.m_nextGrant = grant;
        m_sidelinkTxPoolsMap.begin()->second.m_grant_received = true;

        NS_LOG_INFO (this << "Received SL_DCI message rnti=" << m_rnti << " res=" << (uint16_t) dci.m_resPscch);
    }

    else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG)
    {
        NS_LOG_INFO (this << " Received discovery message");
        //notify RRC (pass msg to RRC where we can filter)
        m_cmacSapUser->NotifyDiscoveryReception (msg);
    }

    else
    {
        NS_LOG_WARN (this << " cv2x_LteControlMessage not recognized");
    }
}

void
cv2x_LteUeMac::DoReceivePFLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{

    NS_LOG_FUNCTION (this);
    if (msg->GetMessageType () == cv2x_LteControlMessage::UL_DCI)
    {
        Ptr<cv2x_UlDciLteControlMessage> msg2 = DynamicCast<cv2x_UlDciLteControlMessage> (msg);
        cv2x_UlDciListElement_s dci = msg2->GetDci ();
        if (dci.m_ndi==1)
        {
            // New transmission -> emtpy pkt buffer queue (for deleting eventual pkts not acked )
            Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
            m_miUlHarqProcessesPacket.at (m_harqProcessId) = pb;
            // Retrieve data from RLC
            std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
            uint16_t activeLcs = 0;
            uint32_t statusPduMinSize = 0;
            //added code
            if (m_ulBsrReceived.size()!=0)
            {
                for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
                {
                    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                    {
                        activeLcs++;
                        if (((*itBsr).second.statusPduSize!=0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                        {
                            statusPduMinSize = (*itBsr).second.statusPduSize;
                        }
                        if (((*itBsr).second.statusPduSize!=0)&&(statusPduMinSize == 0))
                        {
                            statusPduMinSize = (*itBsr).second.statusPduSize;
                        }
                    }
                }
                if (activeLcs == 0)
                {
                    NS_LOG_ERROR (this << " No active flows for this UL-DCI");
                    return;
                }
                std::map <uint8_t, LcInfo>::iterator it;
                uint32_t bytesPerActiveLc = dci.m_tbSize / activeLcs;
                bool statusPduPriority = false;
                if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
                {
                    // send only the status PDU which has highest priority
                    statusPduPriority = true;
                    NS_LOG_DEBUG (this << " Reduced resource -> send only Status, bytes " << statusPduMinSize);
                    if (dci.m_tbSize < statusPduMinSize)
                    {
                        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                    }
                }
                NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);
                for (it = m_lcInfoMap.begin (); it!=m_lcInfoMap.end (); it++)
                {
                    itBsr = m_ulBsrReceived.find ((*it).first);
                    NS_LOG_DEBUG (this << " Processing LC " << (uint32_t)(*it).first << " bytesPerActiveLc " << bytesPerActiveLc);
                    if ( (itBsr!=m_ulBsrReceived.end ()) &&
                         ( ((*itBsr).second.statusPduSize > 0) ||
                           ((*itBsr).second.retxQueueSize > 0) ||
                           ((*itBsr).second.txQueueSize > 0)) )
                    {
                        if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                        {
                            (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  " << (*itBsr).second.statusPduSize << " status bytes to LC " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            (*itBsr).second.statusPduSize = 0;
                            break;
                        }
                        else
                        {
                            uint32_t bytesForThisLc = bytesPerActiveLc;
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                            {
                                (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                bytesForThisLc -= (*itBsr).second.statusPduSize;
                                NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {
                                if ((*itBsr).second.statusPduSize>bytesForThisLc)
                                {
                                    NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                }
                            }

                            if ((bytesForThisLc > 7) && // 7 is the min TxOpportunity useful for Rlc
                                (((*itBsr).second.retxQueueSize > 0) ||
                                 ((*itBsr).second.txQueueSize > 0)))
                            {
                                if ((*itBsr).second.retxQueueSize > 0)
                                {
                                    NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                    {
                                        (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                    }
                                    else
                                    {
                                        (*itBsr).second.retxQueueSize = 0;
                                    }
                                }
                                else if ((*itBsr).second.txQueueSize > 0)
                                {
                                    uint16_t lcid = (*it).first;
                                    uint32_t rlcOverhead;
                                    if (lcid == 1)
                                    {
                                        // for SRB1 (using RLC AM) it's better to
                                        // overestimate RLC overhead rather than
                                        // underestimate it and risk unneeded
                                        // segmentation which increases delay
                                        rlcOverhead = 4;
                                    }
                                    else
                                    {
                                        // minimum RLC overhead due to header
                                        rlcOverhead = 2;
                                    }
                                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                    {
                                        (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                                    }
                                    else
                                    {
                                        (*itBsr).second.txQueueSize = 0;
                                    }
                                }
                            }
                            else
                            {
                                if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                {
                                    // resend BSR info for updating eNB peer MAC
                                    m_freshUlBsr = true;
                                }
                            }
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                        }

                    }
                }
            } // end if m_ulBsrReceived.size()!=0
            else
            {
                // NIST IMPLEMENTATION OF nist_m_ulBsrReceived
                uint16_t activeLcs = 0;
                uint32_t statusPduMinSize = 0;
                std::map <uint8_t, std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > >:: iterator it;
                it=nist_m_ulBsrReceived.find(m_rnti);
                if (it!=nist_m_ulBsrReceived.end())
                {
                    // Retrieve data from RLC
                    std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > mapLC=it->second;
                    std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;

                    for (itBsr = mapLC.begin (); itBsr != mapLC.end (); itBsr++)
                    {
                        if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                        {
                            activeLcs++;
                            if (((*itBsr).second.statusPduSize!=0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                            {
                                statusPduMinSize = (*itBsr).second.statusPduSize;
                            }
                            if (((*itBsr).second.statusPduSize!=0)&&(statusPduMinSize == 0))
                            {
                                statusPduMinSize = (*itBsr).second.statusPduSize;
                            }
                        }// end if
                    }// end for

                    if (activeLcs == 0)
                    {
                        NS_LOG_ERROR (this << " No active flows for this UL-DCI");
                        return;
                    }

                    uint32_t totalQueue =0;
                    for (itBsr = mapLC.begin (); itBsr != mapLC.end (); itBsr++)
                    {
                        totalQueue=totalQueue + (*itBsr).second.txQueueSize;
                    }
                    for (itBsr = mapLC.begin (); itBsr != mapLC.end (); itBsr++)
                    {
                        // compute tb size for this lc
                        uint32_t bytesPerActiveLc= dci.m_tbSize / activeLcs ;
                        if (totalQueue > dci.m_tbSize)
                        {
                            double pfCoef= (double) (*itBsr).second.txQueueSize / (double)totalQueue;
                            bytesPerActiveLc = floor (pfCoef * (double)dci.m_tbSize);
                        }
                        std::map <uint8_t, LcInfo>::iterator itLcInfo;
                        bool statusPduPriority = false;
                        if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
                        {
                            // send only the status PDU which has highest priority
                            statusPduPriority = true;
                            NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes " << statusPduMinSize);
                            if (dci.m_tbSize < statusPduMinSize)
                            {
                                NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                            }
                        }
                        NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);

                        if ( (itBsr!=mapLC.end ()) &&
                             ( ((*itBsr).second.statusPduSize > 0) ||
                               ((*itBsr).second.retxQueueSize > 0) ||
                               ((*itBsr).second.txQueueSize > 0)) )
                        {
                            itLcInfo=m_lcInfoMap.find((*itBsr).first);

                            if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                            {

                                (*itLcInfo).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);

                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {

                                uint32_t bytesForThisLc = bytesPerActiveLc;
                                NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*itLcInfo).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                                if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                                {
                                    (*itLcInfo).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    bytesForThisLc -= (*itBsr).second.statusPduSize;
                                    NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                    (*itBsr).second.statusPduSize = 0;

                                }
                                else
                                {
                                    if ((*itBsr).second.statusPduSize>bytesForThisLc)
                                    {
                                        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                    }
                                }

                                if ((bytesForThisLc > 7) && // 7 is the min TxOpportunity useful for Rlc
                                    (((*itBsr).second.retxQueueSize > 0) ||
                                     ((*itBsr).second.txQueueSize > 0)))
                                {
                                    if ((*itBsr).second.retxQueueSize > 0)
                                    {
                                        NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);

                                        (*itLcInfo).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                        if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                        {
                                            (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                        }
                                        else
                                        {
                                            (*itBsr).second.retxQueueSize = 0;
                                        }
                                    }
                                    else if ((*itBsr).second.txQueueSize > 0)
                                    {
                                        uint16_t lcid = (*itLcInfo).first;
                                        uint32_t rlcOverhead;
                                        if (lcid == 1)
                                        {
                                            // for SRB1 (using RLC AM) it's better to
                                            // overestimate RLC overhead rather than
                                            // underestimate it and risk unneeded
                                            // segmentation which increases delay
                                            rlcOverhead = 4;
                                        }
                                        else
                                        {
                                            // minimum RLC overhead due to header
                                            rlcOverhead = 2;
                                        }
                                        NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                        (*itLcInfo).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                        if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                        {
                                            (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;

                                        }
                                        else
                                        {
                                            (*itBsr).second.txQueueSize = 0;

                                        }
                                    }
                                }
                                else
                                {
                                    if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                    {
                                        // resend BSR info for updating eNB peer MAC
                                        m_freshUlBsr = true;
                                    }
                                }
                                NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*itLcInfo).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            }

                        }
                    }
                }// end for p
            } // end it!=nist_m_ulBsrReceived.end()
        } //end if ndi
        else  //else ndi
        {
            // HARQ retransmission -> retrieve data from HARQ buffer
            NS_LOG_DEBUG (this << " UE MAC RETX HARQ " << (uint16_t)m_harqProcessId);
            Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_harqProcessId);
            for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
            {
                Ptr<Packet> pkt = (*j)->Copy ();
                m_uePhySapProvider->SendMacPdu (pkt);
            }
            m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        }

    }
    else if (msg->GetMessageType () == cv2x_LteControlMessage::RAR)
    {
        if (m_waitingForRaResponse)
        {
            Ptr<cv2x_RarLteControlMessage> rarMsg = DynamicCast<cv2x_RarLteControlMessage> (msg);
            uint16_t raRnti = rarMsg->GetRaRnti ();
            NS_LOG_LOGIC (this << "got RAR with RA-RNTI " << (uint32_t) raRnti << ", expecting " << (uint32_t) m_raRnti);
            if (raRnti == m_raRnti) // RAR corresponds to TX subframe of preamble
            {
                for (std::list<cv2x_RarLteControlMessage::Rar>::const_iterator it = rarMsg->RarListBegin ();
                     it != rarMsg->RarListEnd ();
                     ++it)
                {
                    if (it->rapId == m_raPreambleId) // RAR is for me
                    {
                        RecvRaResponse (it->rarPayload);
                        /// \todo RRC generates the RecvRaResponse messaged
                        /// for avoiding holes in transmission at PHY layer
                        /// (which produce erroneous UL CQI evaluation)
                    }
                }
            }
        }
    }
    else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DCI)
    {
        Ptr<cv2x_SlDciLteControlMessage> msg2 = DynamicCast<cv2x_SlDciLteControlMessage> (msg);
        cv2x_SlDciListElement_s dci = msg2->GetDci ();

        //store the grant for the next SC period
        //TODO: distinguish SL grants for different pools. Right now just assume there is only one pool
        Ptr<SidelinkTxCommResourcePool> pool = DynamicCast<SidelinkTxCommResourcePool> (m_sidelinkTxPoolsMap.begin()->second.m_pool);
        NS_ASSERT (pool->GetSchedulingType() == SidelinkCommResourcePool::SCHEDULED);

        SidelinkGrant grant;
        grant.m_resPscch = dci.m_resPscch;
        grant.m_tpc = dci.m_tpc;
        grant.m_hopping = dci.m_hopping;
        grant.m_rbStart = dci.m_rbStart;
        grant.m_rbLen = dci.m_rbLen;
        grant.m_trp = dci.m_trp;
        grant.m_mcs = pool->GetMcs();
        grant.m_tbSize = 0; //computed later
        m_sidelinkTxPoolsMap.begin()->second.m_nextGrant = grant;
        m_sidelinkTxPoolsMap.begin()->second.m_grant_received = true;

        NS_LOG_INFO (this << "Received SL_DCI message rnti=" << m_rnti << " res=" << (uint16_t) dci.m_resPscch);
    }

    else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG)
    {
        NS_LOG_INFO (this << " Received discovery message");
        //notify RRC (pass msg to RRC where we can filter)
        m_cmacSapUser->NotifyDiscoveryReception (msg);
    }

    else
    {
        NS_LOG_WARN (this << " cv2x_LteControlMessage not recognized");
    }
}

void
cv2x_LteUeMac::DoReceiveMTLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
    NS_LOG_FUNCTION (this);
    if (msg->GetMessageType () == cv2x_LteControlMessage::UL_DCI)
    {
        Ptr<cv2x_UlDciLteControlMessage> msg2 = DynamicCast<cv2x_UlDciLteControlMessage> (msg);
        cv2x_UlDciListElement_s dci = msg2->GetDci ();
        if (dci.m_ndi==1)
        {
            // New transmission -> emtpy pkt buffer queue (for deleting eventual pkts not acked )
            Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
            m_miUlHarqProcessesPacket.at (m_harqProcessId) = pb;
            // Retrieve data from RLC
            std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
            uint16_t activeLcs = 0;
            uint32_t statusPduMinSize = 0;
            //added code
            if (m_ulBsrReceived.size()!=0)
            {
                for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
                {
                    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                    {
                        activeLcs++;
                        if (((*itBsr).second.statusPduSize!=0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                        {
                            statusPduMinSize = (*itBsr).second.statusPduSize;
                        }
                        if (((*itBsr).second.statusPduSize!=0)&&(statusPduMinSize == 0))
                        {
                            statusPduMinSize = (*itBsr).second.statusPduSize;
                        }
                    }
                }
                if (activeLcs == 0)
                {
                    NS_LOG_ERROR (this << " No active flows for this UL-DCI");
                    return;
                }
                std::map <uint8_t, LcInfo>::iterator it;
                uint32_t bytesPerActiveLc = dci.m_tbSize / activeLcs;
                bool statusPduPriority = false;
                if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
                {
                    // send only the status PDU which has highest priority
                    statusPduPriority = true;
                    NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes " << statusPduMinSize);
                    if (dci.m_tbSize < statusPduMinSize)
                    {
                        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                    }
                }
                NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);
                for (it = m_lcInfoMap.begin (); it!=m_lcInfoMap.end (); it++)
                {
                    itBsr = m_ulBsrReceived.find ((*it).first);
                    NS_LOG_DEBUG (this << " Processing LC " << (uint32_t)(*it).first << " bytesPerActiveLc " << bytesPerActiveLc);
                    if ( (itBsr!=m_ulBsrReceived.end ()) &&
                         ( ((*itBsr).second.statusPduSize > 0) ||
                           ((*itBsr).second.retxQueueSize > 0) ||
                           ((*itBsr).second.txQueueSize > 0)) )
                    {
                        if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                        {
                            (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  " << (*itBsr).second.statusPduSize << " status bytes to LC " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            (*itBsr).second.statusPduSize = 0;
                            break;
                        }
                        else
                        {
                            uint32_t bytesForThisLc = bytesPerActiveLc;
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                            {
                                (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                bytesForThisLc -= (*itBsr).second.statusPduSize;
                                NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {
                                if ((*itBsr).second.statusPduSize>bytesForThisLc)
                                {
                                    NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                }
                            }

                            if ((bytesForThisLc > 7) && // 7 is the min TxOpportunity useful for Rlc
                                (((*itBsr).second.retxQueueSize > 0) ||
                                 ((*itBsr).second.txQueueSize > 0)))
                            {
                                if ((*itBsr).second.retxQueueSize > 0)
                                {
                                    NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                    {
                                        (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                    }
                                    else
                                    {
                                        (*itBsr).second.retxQueueSize = 0;
                                    }
                                }
                                else if ((*itBsr).second.txQueueSize > 0)
                                {
                                    uint16_t lcid = (*it).first;
                                    uint32_t rlcOverhead;
                                    if (lcid == 1)
                                    {
                                        // for SRB1 (using RLC AM) it's better to
                                        // overestimate RLC overhead rather than
                                        // underestimate it and risk unneeded
                                        // segmentation which increases delay
                                        rlcOverhead = 4;
                                    }
                                    else
                                    {
                                        // minimum RLC overhead due to header
                                        rlcOverhead = 2;
                                    }
                                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                    {
                                        (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                                    }
                                    else
                                    {
                                        (*itBsr).second.txQueueSize = 0;
                                    }
                                }
                            }
                            else
                            {
                                if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                {
                                    // resend BSR info for updating eNB peer MAC
                                    m_freshUlBsr = true;
                                }
                            }
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                        }

                    }
                }
            } // end if m_ulBsrReceived.size()!=0
            else
            {
                // NIST IMPLEMENTATION OF nist_m_ulBsrReceived
                uint16_t activeLcs = 0;
                uint32_t statusPduMinSize = 0;
                std::map <uint8_t, std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > >:: iterator it;
                it=nist_m_ulBsrReceived.find(m_rnti);
                if (it!=nist_m_ulBsrReceived.end())
                {
                    // Retrieve data from RLC
                    std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > mapLC=it->second;
                    std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;

                    for (itBsr = mapLC.begin (); itBsr != mapLC.end (); itBsr++)
                    {
                        if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                        {
                            activeLcs++;
                            if (((*itBsr).second.statusPduSize!=0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                            {
                                statusPduMinSize = (*itBsr).second.statusPduSize;
                            }
                            if (((*itBsr).second.statusPduSize!=0)&&(statusPduMinSize == 0))
                            {
                                statusPduMinSize = (*itBsr).second.statusPduSize;
                            }
                        }// end if
                    }// end for

                    if (activeLcs == 0)
                    {
                        NS_LOG_ERROR (this << " No active flows for this UL-DCI");
                        return;
                    }

                    for (itBsr = mapLC.begin (); itBsr != mapLC.end (); itBsr++)
                    {
                        // compute tb size for this lc
                        uint32_t bytesPerActiveLc= dci.m_tbSize / activeLcs ;
                        std::map <uint8_t, LcInfo>::iterator itLcInfo;
                        bool statusPduPriority = false;
                        if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
                        {
                            // send only the status PDU which has highest priority
                            statusPduPriority = true;
                            NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes " << statusPduMinSize);
                            if (dci.m_tbSize < statusPduMinSize)
                            {
                                NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                            }
                        }
                        NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);

                        if ( (itBsr!=mapLC.end ()) &&
                             ( ((*itBsr).second.statusPduSize > 0) ||
                               ((*itBsr).second.retxQueueSize > 0) ||
                               ((*itBsr).second.txQueueSize > 0)) )
                        {
                            itLcInfo=m_lcInfoMap.find((*itBsr).first);

                            if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                            {

                                (*itLcInfo).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);

                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {

                                uint32_t bytesForThisLc = bytesPerActiveLc;
                                NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*itLcInfo).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                                if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                                {
                                    (*itLcInfo).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    bytesForThisLc -= (*itBsr).second.statusPduSize;
                                    NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                    (*itBsr).second.statusPduSize = 0;

                                }
                                else
                                {
                                    if ((*itBsr).second.statusPduSize>bytesForThisLc)
                                    {
                                        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                    }
                                }

                                if ((bytesForThisLc > 7) && // 7 is the min TxOpportunity useful for Rlc
                                    (((*itBsr).second.retxQueueSize > 0) ||
                                     ((*itBsr).second.txQueueSize > 0)))
                                {
                                    if ((*itBsr).second.retxQueueSize > 0)
                                    {
                                        NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);

                                        (*itLcInfo).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                        if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                        {
                                            (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                        }
                                        else
                                        {
                                            (*itBsr).second.retxQueueSize = 0;
                                        }
                                    }
                                    else if ((*itBsr).second.txQueueSize > 0)
                                    {
                                        uint16_t lcid = (*itLcInfo).first;
                                        uint32_t rlcOverhead;
                                        if (lcid == 1)
                                        {
                                            // for SRB1 (using RLC AM) it's better to
                                            // overestimate RLC overhead rather than
                                            // underestimate it and risk unneeded
                                            // segmentation which increases delay
                                            rlcOverhead = 4;
                                        }
                                        else
                                        {
                                            // minimum RLC overhead due to header
                                            rlcOverhead = 2;
                                        }
                                        NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                        (*itLcInfo).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                        if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                        {
                                            (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;

                                        }
                                        else
                                        {
                                            (*itBsr).second.txQueueSize = 0;

                                        }
                                    }
                                }
                                else
                                {
                                    if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                    {
                                        // resend BSR info for updating eNB peer MAC
                                        m_freshUlBsr = true;
                                    }
                                }
                                NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*itLcInfo).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            }

                        }
                    }
                }// end for p
            } // end it!=nist_m_ulBsrReceived.end()
        } //end if ndi
        else  //else ndi
        {
            // HARQ retransmission -> retrieve data from HARQ buffer
            NS_LOG_DEBUG (this << " UE MAC RETX HARQ " << (uint16_t)m_harqProcessId);
            Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_harqProcessId);
            for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
            {
                Ptr<Packet> pkt = (*j)->Copy ();
                m_uePhySapProvider->SendMacPdu (pkt);
            }
            m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        }

    }
    else if (msg->GetMessageType () == cv2x_LteControlMessage::RAR)
    {
        if (m_waitingForRaResponse)
        {
            Ptr<cv2x_RarLteControlMessage> rarMsg = DynamicCast<cv2x_RarLteControlMessage> (msg);
            uint16_t raRnti = rarMsg->GetRaRnti ();
            NS_LOG_LOGIC (this << "got RAR with RA-RNTI " << (uint32_t) raRnti << ", expecting " << (uint32_t) m_raRnti);
            if (raRnti == m_raRnti) // RAR corresponds to TX subframe of preamble
            {
                for (std::list<cv2x_RarLteControlMessage::Rar>::const_iterator it = rarMsg->RarListBegin ();
                     it != rarMsg->RarListEnd ();
                     ++it)
                {
                    if (it->rapId == m_raPreambleId) // RAR is for me
                    {
                        RecvRaResponse (it->rarPayload);
                        /// \todo RRC generates the RecvRaResponse messaged
                        /// for avoiding holes in transmission at PHY layer
                        /// (which produce erroneous UL CQI evaluation)
                    }
                }
            }
        }
    }
    else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DCI)
    {
        Ptr<cv2x_SlDciLteControlMessage> msg2 = DynamicCast<cv2x_SlDciLteControlMessage> (msg);
        cv2x_SlDciListElement_s dci = msg2->GetDci ();

        //store the grant for the next SC period
        //TODO: distinguish SL grants for different pools. Right now just assume there is only one pool
        Ptr<SidelinkTxCommResourcePool> pool = DynamicCast<SidelinkTxCommResourcePool> (m_sidelinkTxPoolsMap.begin()->second.m_pool);
        NS_ASSERT (pool->GetSchedulingType() == SidelinkCommResourcePool::SCHEDULED);

        SidelinkGrant grant;
        grant.m_resPscch = dci.m_resPscch;
        grant.m_tpc = dci.m_tpc;
        grant.m_hopping = dci.m_hopping;
        grant.m_rbStart = dci.m_rbStart;
        grant.m_rbLen = dci.m_rbLen;
        grant.m_trp = dci.m_trp;
        grant.m_mcs = pool->GetMcs();
        grant.m_tbSize = 0; //computed later
        m_sidelinkTxPoolsMap.begin()->second.m_nextGrant = grant;
        m_sidelinkTxPoolsMap.begin()->second.m_grant_received = true;

        NS_LOG_INFO (this << "Received SL_DCI message rnti=" << m_rnti << " res=" << (uint16_t) dci.m_resPscch);
    }

    else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG)
    {
        NS_LOG_INFO (this << " Received discovery message");
        //notify RRC (pass msg to RRC where we can filter)
        m_cmacSapUser->NotifyDiscoveryReception (msg);
    }

    else
    {
        NS_LOG_WARN (this << " cv2x_LteControlMessage not recognized");
    }

}

void
cv2x_LteUeMac::DoReceivePrLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
    NS_LOG_FUNCTION (this);
    if (msg->GetMessageType () == cv2x_LteControlMessage::UL_DCI)
    {
        Ptr<cv2x_UlDciLteControlMessage> msg2 = DynamicCast<cv2x_UlDciLteControlMessage> (msg);
        cv2x_UlDciListElement_s dci = msg2->GetDci ();
        if (dci.m_ndi==1)
        {
            // New transmission -> emtpy pkt buffer queue (for deleting eventual pkts not acked )
            Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
            m_miUlHarqProcessesPacket.at (m_harqProcessId) = pb;
            // Retrieve data from RLC
            std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
            uint16_t activeLcs = 0;
            uint32_t statusPduMinSize = 0;

            if (m_ulBsrReceived.size()!=0)
            {
                for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
                {
                    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                    {
                        activeLcs++;
                        if (((*itBsr).second.statusPduSize!=0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                        {
                            statusPduMinSize = (*itBsr).second.statusPduSize;
                        }
                        if (((*itBsr).second.statusPduSize!=0)&&(statusPduMinSize == 0))
                        {
                            statusPduMinSize = (*itBsr).second.statusPduSize;
                        }
                    }
                }
                if (activeLcs == 0)
                {
                    NS_LOG_ERROR (this << " No active flows for this UL-DCI");
                    return;
                }
                std::map <uint8_t, LcInfo>::iterator it;
                uint32_t bytesPerActiveLc = dci.m_tbSize / activeLcs;
                bool statusPduPriority = false;
                if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
                {
                    // send only the status PDU which has highest priority
                    statusPduPriority = true;
                    NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes " << statusPduMinSize);
                    if (dci.m_tbSize < statusPduMinSize)
                    {
                        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                    }
                }
                NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);
                for (it = m_lcInfoMap.begin (); it!=m_lcInfoMap.end (); it++)
                {
                    itBsr = m_ulBsrReceived.find ((*it).first);
                    NS_LOG_DEBUG (this << " Processing LC " << (uint32_t)(*it).first << " bytesPerActiveLc " << bytesPerActiveLc);
                    if ( (itBsr!=m_ulBsrReceived.end ()) &&
                         ( ((*itBsr).second.statusPduSize > 0) ||
                           ((*itBsr).second.retxQueueSize > 0) ||
                           ((*itBsr).second.txQueueSize > 0)) )
                    {
                        if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                        {
                            (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  " << (*itBsr).second.statusPduSize << " status bytes to LC " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            (*itBsr).second.statusPduSize = 0;
                            break;
                        }
                        else
                        {
                            uint32_t bytesForThisLc = bytesPerActiveLc;
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                            {
                                (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                bytesForThisLc -= (*itBsr).second.statusPduSize;
                                NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {
                                if ((*itBsr).second.statusPduSize>bytesForThisLc)
                                {
                                    NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                }
                            }

                            if ((bytesForThisLc > 7) && // 7 is the min TxOpportunity useful for Rlc
                                (((*itBsr).second.retxQueueSize > 0) ||
                                 ((*itBsr).second.txQueueSize > 0)))
                            {
                                if ((*itBsr).second.retxQueueSize > 0)
                                {
                                    NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                    {
                                        (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                    }
                                    else
                                    {
                                        (*itBsr).second.retxQueueSize = 0;
                                    }
                                }
                                else if ((*itBsr).second.txQueueSize > 0)
                                {
                                    uint16_t lcid = (*it).first;
                                    uint32_t rlcOverhead;
                                    if (lcid == 1)
                                    {
                                        // for SRB1 (using RLC AM) it's better to
                                        // overestimate RLC overhead rather than
                                        // underestimate it and risk unneeded
                                        // segmentation which increases delay
                                        rlcOverhead = 4;
                                    }
                                    else
                                    {
                                        // minimum RLC overhead due to header
                                        rlcOverhead = 2;
                                    }
                                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                    {
                                        (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                                    }
                                    else
                                    {
                                        (*itBsr).second.txQueueSize = 0;
                                    }
                                }
                            }
                            else
                            {
                                if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                {
                                    // resend BSR info for updating eNB peer MAC
                                    m_freshUlBsr = true;
                                }
                            }
                            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*it).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                        }

                    }
                }
            } // end if m_ulBsrReceived.size()!=0
            else
            {
                // NIST IMPLEMENTATION OF nist_m_ulBsrReceived
                uint16_t activeLcs = 0;
                uint32_t statusPduMinSize = 0;
                std::map <uint8_t, std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > >:: iterator it;
                it=nist_m_ulBsrReceived.find(m_rnti);
                if (it!=nist_m_ulBsrReceived.end())
                {
                    // Retrieve data from RLC
                    std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters > mapLC=it->second;
                    std::map <uint8_t, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;

                    for (itBsr = mapLC.begin (); itBsr != mapLC.end (); itBsr++)
                    {
                        if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                        {
                            activeLcs++;
                            if (((*itBsr).second.statusPduSize!=0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                            {
                                statusPduMinSize = (*itBsr).second.statusPduSize;
                            }
                            if (((*itBsr).second.statusPduSize!=0)&&(statusPduMinSize == 0))
                            {
                                statusPduMinSize = (*itBsr).second.statusPduSize;
                            }
                        }// end if
                    }// end for

                    if (activeLcs == 0)
                    {
                        NS_LOG_ERROR (this << " No active flows for this UL-DCI");
                        return;
                    }

                    std::vector<uint8_t> TreatedLCs;

                    std::map <uint8_t, std::map <uint8_t, uint8_t> >::iterator itP;
                    std::map <uint8_t, uint8_t> mapLCP;
                    std::map <uint8_t, uint8_t>::iterator itLCP;
                    itP= PriorityMap.find(m_rnti);
                    mapLCP=itP->second;
                    uint16_t TbTemp=dci.m_tbSize;
                    for (uint16_t p=0; p < activeLcs; p++)
                    {
                        uint8_t MinPriority=10;
                        uint8_t lcidMin=0;

                        for (itLCP= mapLCP.begin (); itLCP != mapLCP.end (); itLCP++)
                        {
                            //CHECK IF THIS LCID IS ALREADY TREATED
                            std::vector <uint8_t>::iterator Ft;
                            bool findF=false;
                            for(Ft=TreatedLCs.begin();Ft!=TreatedLCs.end();Ft++)
                            {
                                if((*Ft)==(*itLCP).first)
                                {
                                    findF=true;

                                    break;
                                }
                            }
                            if (findF==true)
                            {
                                MinPriority=10;
                                continue;
                            }
                            if (findF==false)
                            {
                                //this LC doesn't exist in Treated LC
                                if ((*itLCP).second < MinPriority )
                                {
                                    lcidMin=(*itLCP).first;

                                    MinPriority=(*itLCP).second;

                                }
                            }
                        }//end for

                        TreatedLCs.push_back (lcidMin);
                        // compute tb size for this lc
                        itBsr=mapLC.find (lcidMin);
                        uint32_t bytesPerActiveLc;
                        if ( TbTemp >= (*itBsr).second.txQueueSize )
                        {
                            bytesPerActiveLc=(*itBsr).second.txQueueSize;
                            TbTemp = TbTemp - (*itBsr).second.txQueueSize;
                        }
                        else
                        {
                            bytesPerActiveLc = TbTemp;
                            TbTemp=0;
                        }
                        std::map <uint8_t, LcInfo>::iterator itLcInfo;
                        bool statusPduPriority = false;
                        if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
                        {
                            // send only the status PDU which has highest priority
                            statusPduPriority = true;
                            NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes " << statusPduMinSize);
                            if (dci.m_tbSize < statusPduMinSize)
                            {
                                NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                            }
                        }
                        NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);

                        itBsr=mapLC.begin();
                        itBsr = mapLC.find (lcidMin);

                        if ( (itBsr!=mapLC.end ()) &&
                             ( ((*itBsr).second.statusPduSize > 0) ||
                               ((*itBsr).second.retxQueueSize > 0) ||
                               ((*itBsr).second.txQueueSize > 0)) )
                        {
                            itLcInfo=m_lcInfoMap.find((*itBsr).first);

                            if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                            {

                                (*itLcInfo).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);

                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {

                                uint32_t bytesForThisLc = bytesPerActiveLc;
                                NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*itLcInfo).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                                if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                                {
                                    (*itLcInfo).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    bytesForThisLc -= (*itBsr).second.statusPduSize;
                                    NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                    (*itBsr).second.statusPduSize = 0;

                                }
                                else
                                {
                                    if ((*itBsr).second.statusPduSize>bytesForThisLc)
                                    {
                                        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                    }
                                }

                                if ((bytesForThisLc > 7) && // 7 is the min TxOpportunity useful for Rlc
                                    (((*itBsr).second.retxQueueSize > 0) ||
                                     ((*itBsr).second.txQueueSize > 0)))
                                {
                                    if ((*itBsr).second.retxQueueSize > 0)
                                    {
                                        NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);

                                        (*itLcInfo).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                        if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                        {
                                            (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                        }
                                        else
                                        {
                                            (*itBsr).second.retxQueueSize = 0;
                                        }
                                    }
                                    else if ((*itBsr).second.txQueueSize > 0)
                                    {
                                        uint16_t lcid = (*itLcInfo).first;
                                        uint32_t rlcOverhead;
                                        if (lcid == 1)
                                        {
                                            // for SRB1 (using RLC AM) it's better to
                                            // overestimate RLC overhead rather than
                                            // underestimate it and risk unneeded
                                            // segmentation which increases delay
                                            rlcOverhead = 4;
                                        }
                                        else
                                        {
                                            // minimum RLC overhead due to header
                                            rlcOverhead = 2;
                                        }
                                        NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                        (*itLcInfo).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                        if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                        {
                                            (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;

                                        }
                                        else
                                        {
                                            (*itBsr).second.txQueueSize = 0;

                                        }
                                    }
                                }
                                else
                                {
                                    if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                    {
                                        // resend BSR info for updating eNB peer MAC
                                        m_freshUlBsr = true;
                                    }
                                }
                                NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*itLcInfo).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            }

                        }
                    }
                }// end for p
            } // end it!=nist_m_ulBsrReceived.end()
        } //end if ndi
        else  //else ndi
        {
            // HARQ retransmission -> retrieve data from HARQ buffer
            NS_LOG_DEBUG (this << " UE MAC RETX HARQ " << (uint16_t)m_harqProcessId);
            Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_harqProcessId);
            for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
            {
                Ptr<Packet> pkt = (*j)->Copy ();
                m_uePhySapProvider->SendMacPdu (pkt);
            }
            m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        }

    }
    else if (msg->GetMessageType () == cv2x_LteControlMessage::RAR)
    {
        if (m_waitingForRaResponse)
        {
            Ptr<cv2x_RarLteControlMessage> rarMsg = DynamicCast<cv2x_RarLteControlMessage> (msg);
            uint16_t raRnti = rarMsg->GetRaRnti ();
            NS_LOG_LOGIC (this << "got RAR with RA-RNTI " << (uint32_t) raRnti << ", expecting " << (uint32_t) m_raRnti);
            if (raRnti == m_raRnti) // RAR corresponds to TX subframe of preamble
            {
                for (std::list<cv2x_RarLteControlMessage::Rar>::const_iterator it = rarMsg->RarListBegin ();
                     it != rarMsg->RarListEnd ();
                     ++it)
                {
                    if (it->rapId == m_raPreambleId) // RAR is for me
                    {
                        RecvRaResponse (it->rarPayload);
                        /// \todo RRC generates the RecvRaResponse messaged
                        /// for avoiding holes in transmission at PHY layer
                        /// (which produce erroneous UL CQI evaluation)
                    }
                }
            }
        }
    }
    else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DCI)
    {
        Ptr<cv2x_SlDciLteControlMessage> msg2 = DynamicCast<cv2x_SlDciLteControlMessage> (msg);
        cv2x_SlDciListElement_s dci = msg2->GetDci ();

        //store the grant for the next SC period
        //TODO: distinguish SL grants for different pools. Right now just assume there is only one pool
        Ptr<SidelinkTxCommResourcePool> pool = DynamicCast<SidelinkTxCommResourcePool> (m_sidelinkTxPoolsMap.begin()->second.m_pool);
        NS_ASSERT (pool->GetSchedulingType() == SidelinkCommResourcePool::SCHEDULED);

        SidelinkGrant grant;
        grant.m_resPscch = dci.m_resPscch;
        grant.m_tpc = dci.m_tpc;
        grant.m_hopping = dci.m_hopping;
        grant.m_rbStart = dci.m_rbStart;
        grant.m_rbLen = dci.m_rbLen;
        grant.m_trp = dci.m_trp;
        grant.m_mcs = pool->GetMcs();
        grant.m_tbSize = 0; //computed later
        m_sidelinkTxPoolsMap.begin()->second.m_nextGrant = grant;
        m_sidelinkTxPoolsMap.begin()->second.m_grant_received = true;

        NS_LOG_INFO (this << "Received SL_DCI message rnti=" << m_rnti << " res=" << (uint16_t) dci.m_resPscch);
    }

    else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG)
    {
        NS_LOG_INFO (this << " Received discovery message");
        //notify RRC (pass msg to RRC where we can filter)
        m_cmacSapUser->NotifyDiscoveryReception (msg);
    }

    else
    {
        NS_LOG_WARN (this << " cv2x_LteControlMessage not recognized");
    }
}

void
cv2x_LteUeMac::RefreshHarqProcessesPacketBuffer (void)
{
    NS_LOG_FUNCTION (this);

    for (uint16_t i = 0; i < m_miUlHarqProcessesPacketTimer.size (); i++)
    {
        if (m_miUlHarqProcessesPacketTimer.at (i) == 0)
        {
            if (m_miUlHarqProcessesPacket.at (i)->GetSize () > 0)
            {
                // timer expired: drop packets in buffer for this process
                NS_LOG_INFO (this << " HARQ Proc Id " << i << " packets buffer expired");
                Ptr<PacketBurst> emptyPb = CreateObject <PacketBurst> ();
                m_miUlHarqProcessesPacket.at (i) = emptyPb;
            }
        }
        else
        {
            m_miUlHarqProcessesPacketTimer.at (i)--;
        }
    }
}

uint8_t
cv2x_LteUeMac::CalcRiv(uint8_t lSubch, uint8_t startSubchIdx)
{
    if ((lSubch-1) <= std::floor(m_numSubchannel/2)){
        return m_numSubchannel*(lSubch-1) + startSubchIdx;
    }
    else{
        return m_numSubchannel*(m_numSubchannel-lSubch+1) + (m_numSubchannel-1-startSubchIdx);
    }
}

uint8_t
cv2x_LteUeMac::GetRndmReselectionCounter(uint16_t pRsvp)
{
    uint8_t min, max;

    switch(pRsvp) {
        case 20:
            min = 25;
            max = 75;
            break;
        case 50:
            min = 10;
            max = 30;
            break;
        case 100:
        case 200:
        case 300:
        case 400:
        case 500:
        case 600:
        case 700:
        case 800:
        case 900:
        case 1000:
            min = 5;
            max = 15;
            break;
        default:
            NS_FATAL_ERROR ("VALUE NOT SUPPORTED!");
            break;
    }
    return (rand()%((max+1)-min))+min;
}

uint8_t
cv2x_LteUeMac::GetSlThresPsschRsrpVal(uint8_t a, uint8_t b)
{
    const uint8_t i = a*8+b+1;

    NS_ASSERT(i >= 0 && i <= 66);

    uint8_t rsrpVal; // INTEGER (0..66)

    if(i==0){
        rsrpVal = 0;            // !!! val must set to -infinity dBm!!!
    }
    else if(i==66){
        rsrpVal = 0;            // !!! val must set to +infinity dBm!!!
    }
    else{
        rsrpVal = -128+(i-1)*2;
    }
    return rsrpVal;
}

void
cv2x_LteUeMac::UpdateSensingWindow(SidelinkCommResourcePoolV2x::SubframeInfo subframe)
{
    std::list<SensingData>::iterator it = m_sensingData.begin();
    while (it != m_sensingData.end())
    {
        uint32_t tmpFrameNo = it->m_rxInfo.subframe.frameNo + 100;
        if (tmpFrameNo > 1024)
        {
            // if frameNo+100 is here less than 1024 a new superframe is already started
            // but the beginning of the sensing window is still in the last superframe
            if((subframe.frameNo + 100) < 1024) {
                tmpFrameNo -= 1024;
            }
        }

        // check if the actual data is still in the sensing window
        // if true the data is outside the sensing window and have to be removed
        // if false iterate next sensed element
        if (tmpFrameNo < subframe.frameNo || ((tmpFrameNo == subframe.frameNo) && (it->m_rxInfo.subframe.subframeNo < subframe.subframeNo))){
            it = m_sensingData.erase(it);
        }
        else {
            it++;
        }
    }
}

std::list<cv2x_LteUeMac::SidelinkTransmissionInfoExtended>
cv2x_LteUeMac::GetReTxResources(SidelinkCommResourcePoolV2x::SubframeInfo initialTx, std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> txOpps)
{
    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator it;
    std::list<SidelinkTransmissionInfoExtended>::iterator it2;


    std::list<SidelinkTransmissionInfoExtended> reTxOpps;
    std::list<SidelinkTransmissionInfoExtended> allPossibleFrames;
    SidelinkTransmissionInfoExtended tmpPlus;
    SidelinkTransmissionInfoExtended tmpMinus;
    bool erase = true;

    for(it = txOpps.begin(); it != txOpps.end(); it++)
    {
        SidelinkTransmissionInfoExtended tmp;
        tmp.m_txInfo = (*it);
        reTxOpps.push_back(tmp);
    }

    tmpMinus.m_txInfo.subframe = initialTx;
    tmpMinus.m_reTxIdx = 1;
    tmpPlus.m_txInfo.subframe = initialTx;
    tmpPlus.m_reTxIdx = 0;
    for(uint8_t i = 1; i <= 15; i++)
    {
        tmpPlus.m_sfGap = i;
        // Calculate all SubframeInfos for k = -1 .. -15
        if (tmpMinus.m_txInfo.subframe.subframeNo == 1)
        {
            tmpMinus.m_txInfo.subframe.subframeNo = 10;
            if (tmpMinus.m_txInfo.subframe.frameNo == 1){
                tmpMinus.m_txInfo.subframe.frameNo = 1024;
            }
            else {
                tmpMinus.m_txInfo.subframe.frameNo -= 1;
            }
        }
        else
        {
            tmpMinus.m_txInfo.subframe.subframeNo -= 1;
        }
        allPossibleFrames.push_back(tmpMinus);

        tmpMinus.m_sfGap = i;
        // Calculate all SubframeInfos for k = 1 .. 15
        tmpPlus.m_txInfo.subframe.subframeNo += 1;
        if (tmpPlus.m_txInfo.subframe.subframeNo > 10)
        {
            ++tmpPlus.m_txInfo.subframe.frameNo;
            if(tmpPlus.m_txInfo.subframe.frameNo > 1024){
                tmpPlus.m_txInfo.subframe.frameNo = 1;
            }
            tmpPlus.m_txInfo.subframe.subframeNo -= 10;
        }
        allPossibleFrames.push_back(tmpPlus);
    }

    // check if there is a tx opportunity which frameNo/subframeNo matches with the previous
    // calculated SubframeInfos
    it2 = reTxOpps.begin();
    while(it2 != reTxOpps.end())
    {
        for(std::list<SidelinkTransmissionInfoExtended>::iterator infoIt = allPossibleFrames.begin(); infoIt != allPossibleFrames.end(); infoIt++)
        {
            // if true there is a tx opportunity for retransmission
            // if false there is no tx opportunity for retransmission and the element have to be erased
            if (infoIt->m_txInfo.subframe.frameNo == it2->m_txInfo.subframe.frameNo && infoIt->m_txInfo.subframe.subframeNo == it2->m_txInfo.subframe.subframeNo){
                erase = false;
                it2->m_sfGap = infoIt->m_sfGap;
                it2->m_reTxIdx = infoIt->m_reTxIdx;
                break;
            }
        }

        if(erase){
            it2 = reTxOpps.erase(it2);
        }
        else{
            it2++;
        }
    }
    return reTxOpps;
}

std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>
cv2x_LteUeMac::GetTxResources(SidelinkCommResourcePoolV2x::SubframeInfo subframe, PoolInfoV2x pool)
{
    NS_LOG_INFO (this << "Start Resource Allocation - Semi Persistent Scheduling");
    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> csrA, csrB, copyCsrA;
    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator csrIt;
    std::list<CandidateResource>::iterator sortedCsrIt;
    std::list<SensingData>::iterator sensingIt;

    uint16_t numCsr; // number of all Candidate Resources
    int threshRsrp;
    bool erase;

    if(m_partialSensing)
    {
        // Partial Sensing not implemented yet
    } // endif m_partialSensing
    else
    {
        // init
        csrA = pool.m_pool->GetCandidateResources(subframe, m_t1, m_t2, m_subchLen); // SA = {ALL CSRs}
        numCsr = csrA.size();
        copyCsrA = csrA;
        //std::cout << "---------------" << std::endl;

        //std::cout << subframe.frameNo << "/" << subframe.subframeNo <<"\t NumCsr=" << (int) csrA.size() << std::endl;
        threshRsrp = -110;

        /*for (csrIt = csrA.begin(); csrIt != csrA.end(); ++csrIt)
		{
			std::cout << " " << csrIt->subframe.frameNo << "/" << csrIt->subframe.subframeNo << "\t rbStart=" << (int) csrIt->rbStart << "\t rbLen=" << (int) csrIt->rbLen << std::endl;
		}*/

        /*std::cout << "Sensed Data" << std::endl;
		for (sensingIt = m_sensingData.begin(); sensingIt != m_sensingData.end(); ++sensingIt)
		{
			std::cout << " " << sensingIt->m_rxInfo.subframe.frameNo << "/" << sensingIt->m_rxInfo.subframe.subframeNo << "\t rbStart=" << (int) sensingIt->m_rxInfo.rbStart << "\t rbLen=" << (int) sensingIt->m_rxInfo.rbLen << std::endl;
		}*/

        do
        {
            csrA = copyCsrA;

            // iterate over all Candidate Resources
            csrIt = csrA.begin();
            while (csrIt != csrA.end())
            {
                erase = false;

                // calculate all proposed transmissions of current candidate resource
                SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo csrTransmission;
                csrTransmission.subframe.subframeNo = csrIt->subframe.subframeNo;
                csrTransmission.rbStart = csrIt->rbStart;
                csrTransmission.rbLen = csrIt->rbLen;

                std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> csrTx;
                for (uint8_t ctr = 0; ctr < m_reselCtr; ctr++)
                {
                    csrTransmission.subframe.frameNo = csrIt->subframe.frameNo + ctr*m_pRsvp/10;
                    if (csrTransmission.subframe.frameNo > 2048) {
                        csrTransmission.subframe.frameNo -= 2048;
                    }
                    else if (csrTransmission.subframe.frameNo > 1024) {
                        csrTransmission.subframe.frameNo -= 1024;
                    }
                    csrTx.push_back (csrTransmission);
                }

                // check all sensed data
                for (sensingIt = m_sensingData.begin(); sensingIt != m_sensingData.end(); sensingIt++)
                {
                    // calculate all possible transmissions of sensed data
                    SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo sensTransmission;
                    sensTransmission.subframe.subframeNo = sensingIt->m_rxInfo.subframe.subframeNo;
                    sensTransmission.rbStart = sensingIt->m_rxInfo.rbStart;
                    sensTransmission.rbLen = sensingIt->m_rxInfo.rbLen;

                    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> sensTx;
                    for(uint8_t ctr = 1; ctr <= 15; ctr++)
                    {
                        sensTransmission.subframe.frameNo = sensingIt->m_rxInfo.subframe.frameNo + ctr*sensingIt->m_pRsvpRx/10;
                        if (sensTransmission.subframe.frameNo > 2048) {
                            sensTransmission.subframe.frameNo -= 2048;
                        }
                        else if (sensTransmission.subframe.frameNo > 1024) {
                            sensTransmission.subframe.frameNo -= 1024;
                        }
                        sensTx.push_back (sensTransmission);
                    }

                    // for all proposed transmissions of current candidate resource
                    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator csrTxIt;
                    for (csrTxIt = csrTx.begin(); csrTxIt != csrTx.end(); csrTxIt++)
                    {
                        NS_ASSERT (csrTxIt->subframe.frameNo > 0 && csrTxIt->subframe.frameNo <= 1024 && csrTxIt->subframe.subframeNo > 0 && csrTxIt->subframe.subframeNo <= 10);

                        std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator sensTxIt;
                        for (sensTxIt = sensTx.begin(); sensTxIt != sensTx.end(); sensTxIt++)
                        {
                            // check if candidate resource transmission and possible transmission
                            // of sensed data occur in the same subframe
                            if(csrTxIt->subframe.frameNo == sensTxIt->subframe.frameNo && csrTxIt->subframe.subframeNo == sensTxIt->subframe.subframeNo)
                            {
                                // check if the utilizied RBs overlaps with candidate resource RBs
                                for (int i = csrTxIt->rbStart; i < csrTxIt->rbStart+csrTxIt->rbLen; i++)
                                {
                                    for(int j = sensTxIt->rbStart; j < sensTxIt->rbStart+sensTxIt->rbLen; j++)
                                    {
                                        if (i == j && sensingIt->m_slRsrp > threshRsrp) {
                                            erase = true;
                                            break;
                                        }
                                    }
                                    if (erase) break;
                                }
                            }
                            if (erase) break;
                        }
                    } // end for all proposed transmission of current candidate resource
                    if (erase) break;
                } // end for all sensed data
                if (erase) {
                    //std::cout << "erase \t" << csrIt->subframe.frameNo << "/" << csrIt->subframe.subframeNo << "\t rbStart=" << (int) csrIt->rbStart << "\t rbLen=" << (int) csrIt->rbLen << std::endl;
                    csrIt = csrA.erase(csrIt);
                }
                else {
                    csrIt++;
                }
            } // end while
            threshRsrp += 3;
        } // end do
        while(csrA.size() < 0.2*numCsr); // Step 7: Repeat until the size of the resulting CSR-list is greater than the 20% of the size of all CSR

        /*std::cout << "remaining csrs " << (int) csrA.size() << std::endl;
		for (csrIt = csrA.begin(); csrIt != csrA.end(); csrIt++)
		{
			std::cout << " " << csrIt->subframe.frameNo << "/" << csrIt->subframe.subframeNo << "\t rbStart=" << (int) csrIt->rbStart << "\t rbLen=" << (int) csrIt->rbLen << std::endl;
		}*/

        // Step 8: Calculate metric E defined as the linear average of S-RSSI
        std::list <CandidateResource> m_csr;
        for(csrIt = csrA.begin(); csrIt != csrA.end(); csrIt++) // for all remaining CSRs
        {
            double avg_rssi = 0;
            uint8_t nbTx = 0;

            // Calculate the first transmission of current CSR frameNo/subframeNo in the sensing Window
            SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo sensingWindowTransmission;
            sensingWindowTransmission.subframe.subframeNo = csrIt->subframe.subframeNo;
            sensingWindowTransmission.rbStart = csrIt->rbStart;
            sensingWindowTransmission.rbLen = csrIt->rbLen;

            if (csrIt->subframe.frameNo <= 100) {
                uint8_t diff = 100 - csrIt->subframe.frameNo;
                sensingWindowTransmission.subframe.frameNo = 1024 - diff;
            }
            else {
                sensingWindowTransmission.subframe.frameNo = csrIt->subframe.frameNo - 100;
            }

            // For the last 10 transmissions on CSR frameNo/subframeNo calculate
            // the average S-RSSI
            for (uint8_t i = 0; i < 10; i++)
            {
                sensingWindowTransmission.subframe.frameNo +=  10;
                if(sensingWindowTransmission.subframe.frameNo > 1024) {
                    sensingWindowTransmission.subframe.frameNo -= 1024;
                }
                // check if we received data on the frameNo/subframeNo and same subchannel
                for (sensingIt = m_sensingData.begin(); sensingIt != m_sensingData.end(); sensingIt++)
                {
                    //if (sensingWindowTransmission.subframe.frameNo == sensingIt->m_rxInfo.subframe.frameNo && sensingWindowTransmission.subframe.subframeNo == sensingIt->m_rxInfo.subframe.subframeNo)
                    if (sensingWindowTransmission.subframe.frameNo == sensingIt->m_rxInfo.subframe.frameNo && sensingWindowTransmission.subframe.subframeNo == sensingIt->m_rxInfo.subframe.subframeNo && sensingWindowTransmission.rbStart == sensingIt->m_rxInfo.rbStart)
                    {
                        nbTx++;
                        avg_rssi += sensingIt->m_slRssi;
                        break; // if we find frameNo/subframeNo we can skip to next transmission
                    }
                }
            }

            if(nbTx != 0) {
                avg_rssi = avg_rssi / nbTx;
            }
            else {
                avg_rssi = -200.0; // assumend that nothing is received
            }

            CandidateResource csr;
            csr.m_txInfo = *csrIt;
            csr.m_avg_rssi = avg_rssi;
            m_csr.push_back(csr);
        }

        // mix values in m_csr otherwise only the first resources in
        // selection window will be choosen
        std::list<CandidateResource> copy = m_csr;
        m_csr.clear();

        while (copy.size() != 0)
        {
            std::list<CandidateResource>::iterator it = copy.begin();
            std::advance(it, m_ueSelectedUniformVariable->GetInteger (0, copy.size()-1));
            m_csr.push_back((*it));
            copy.erase(it);
        }

        // Step 9: Select CSRs with smallest metric until the size of SB is greater than or equal to 20% of the size of all CSRs
        // sort by average RSSI
        if (m_csr.size() != 0)
        {
            m_csr.sort([](const CandidateResource & a, const CandidateResource & b){return a.m_avg_rssi < b.m_avg_rssi;});
        }

        for(sortedCsrIt = m_csr.begin(); sortedCsrIt != m_csr.end(); sortedCsrIt++)
        {
            if(csrB.size() >= 0.2*numCsr) {
                break;
            }
            else {
                csrB.push_back((sortedCsrIt->m_txInfo));
            }
        }
    }

    /*std::cout << "remaining csrs " << (int) csrB.size() << std::endl;
	for (csrIt = csrB.begin(); csrIt != csrB.end(); csrIt++)
	{
		std::cout << " " << csrIt->subframe.frameNo << "/" << csrIt->subframe.subframeNo << "\t rbStart=" << (int) csrIt->rbStart << "\t rbLen=" << (int) csrIt->rbLen << std::endl;
	}*/
    return csrB;
}


void
cv2x_LteUeMac::DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
    NS_LOG_FUNCTION (this << " Frame no. " << frameNo << " subframe no. " << subframeNo);
    m_frameNo = frameNo;
    m_subframeNo = subframeNo;

    //RefreshHarqProcessesPacketBuffer ();
    if ((Simulator::Now () >= m_bsrLast + m_bsrPeriodicity) && (m_freshUlBsr == true))
    {
        SendReportBufferStatus ();
        m_bsrLast = Simulator::Now ();
        m_freshUlBsr = false;
        m_harqProcessId = (m_harqProcessId + 1) % HARQ_PERIOD;
    }

    //sidelink processes

    //there is a delay between the MAC scheduling and the transmission so we assume that we are ahead
    subframeNo += 4;
    if (subframeNo > 10)
    {
        ++frameNo;
        if (frameNo > 1024)
        {
            frameNo = 1;
        }
        subframeNo -= 10;
    }
    NS_LOG_INFO (this << " Adjusted Frame no. " << frameNo << " subframe no. " << subframeNo);


    //discovery
    //Check if this is a new disc period
    if (frameNo == m_discTxPools.m_nextDiscPeriod.frameNo && subframeNo == m_discTxPools.m_nextDiscPeriod.subframeNo)
    {
        //define periods and frames
        m_discTxPools.m_currentDiscPeriod = m_discTxPools.m_nextDiscPeriod;
        m_discTxPools.m_nextDiscPeriod = m_discTxPools.m_pool->GetNextDiscPeriod (frameNo, subframeNo);
        m_discTxPools.m_nextDiscPeriod.frameNo++;
        m_discTxPools.m_nextDiscPeriod.subframeNo++;
        NS_LOG_INFO (this << " starting new discovery period " << ". Next period at " << m_discTxPools.m_nextDiscPeriod.frameNo << "/" << m_discTxPools.m_nextDiscPeriod.subframeNo);

        if (m_discTxPools.m_pool->GetSchedulingType() == SidelinkDiscResourcePool::UE_SELECTED)
        {
            //use txProbability
            DiscGrant grant;
            double p1 = m_p1UniformVariable->GetValue (0, 1);

            double txProbability = m_discTxPools.m_pool->GetTxProbability (); //calculate txProbability
            if (p1 <= txProbability/100)
            {
                grant.m_resPsdch = m_resUniformVariable->GetInteger (0, m_discTxPools.m_npsdch-1);
                grant.m_rnti = m_rnti;
                m_discTxPools.m_nextGrant = grant;
                m_discTxPools.m_grant_received = true;
                NS_LOG_INFO (this << " UE selected grant: resource=" << (uint16_t) grant.m_resPsdch << "/" << m_discTxPools.m_npsdch);
            }
        }
        else //scheduled
        {
            //TODO
            //use defined grant : SL-TF-IndexPair
        }

        //if we received a grant
        if (m_discTxPools.m_grant_received)
        {
            m_discTxPools.m_currentGrant = m_discTxPools.m_nextGrant;
            NS_LOG_INFO (this << " Discovery grant received resource " << (uint32_t) m_discTxPools.m_currentGrant.m_resPsdch);

            SidelinkDiscResourcePool::SubframeInfo tmp;
            tmp.frameNo = m_discTxPools.m_currentDiscPeriod.frameNo-1;
            tmp.subframeNo = m_discTxPools.m_currentDiscPeriod.subframeNo-1;

            m_discTxPools.m_psdchTx = m_discTxPools.m_pool->GetPsdchTransmissions (m_discTxPools.m_currentGrant.m_resPsdch);
            for (std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo>::iterator txIt = m_discTxPools.m_psdchTx.begin (); txIt != m_discTxPools.m_psdchTx.end (); txIt++)
            {
                txIt->subframe = txIt->subframe + tmp;
                //adjust for index starting at 1
                txIt->subframe.frameNo++;
                txIt->subframe.subframeNo++;
                NS_LOG_INFO (this << " PSDCH: Subframe " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << ": rbStart=" << (uint32_t) txIt->rbStart << ", rbLen=" << (uint32_t) txIt->nbRb);
                //std::cout <<  " PSDCH: Subframe " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << ": rbStart=" << (uint32_t) txIt->rbStart << ", rbLen=" << (uint32_t) txIt->nbRb << std::endl;
            }

            //Inform PHY: find a way to inform the PHY layer of the resources
            m_cphySapProvider->SetDiscGrantInfo (m_discTxPools.m_currentGrant.m_resPsdch);
            //clear the grant
            m_discTxPools.m_grant_received = false;
        }
    }
    std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo>::iterator allocIt;
    //check if we need to transmit PSDCH
    allocIt = m_discTxPools.m_psdchTx.begin();
    if (allocIt != m_discTxPools.m_psdchTx.end() && (*allocIt).subframe.frameNo == frameNo && (*allocIt).subframe.subframeNo == subframeNo)
    {
        NS_LOG_INFO (this << "PSDCH transmission");
        for (std::list<uint32_t>::iterator txApp = m_discTxApps.begin (); txApp != m_discTxApps.end (); ++txApp)
        {

            //Create Discovery message for each discovery application announcing
            cv2x_SlDiscMsg discMsg;
            discMsg.m_rnti = m_rnti;
            discMsg.m_resPsdch = m_discTxPools.m_currentGrant.m_resPsdch;

            discMsg.m_proSeAppCode =  (std::bitset <184>)*txApp;

            Ptr<cv2x_SlDiscMessage> msg = Create<cv2x_SlDiscMessage> ();
            msg->SetSlDiscMessage (discMsg);
            NS_LOG_INFO ("discovery message sent by " << m_rnti << ", proSeAppCode = " << *txApp);
            m_discoveryAnnouncementTrace (m_rnti, *txApp);
            m_uePhySapProvider->SendLteControlMessage (msg);

        }
        m_discTxPools.m_psdchTx.erase (allocIt);
    }

    //communication
    if ((Simulator::Now () >= m_slBsrLast + m_slBsrPeriodicity) && (m_freshSlBsr == true))
    {
        SendSidelinkReportBufferStatusV2x ();
        m_slBsrLast = Simulator::Now ();
        m_freshSlBsr = false;
        //m_harqProcessId = (m_harqProcessId + 1) % HARQ_PERIOD; //is this true?
    }

    std::map <uint32_t, PoolInfo>::iterator poolIt;
    for (poolIt = m_sidelinkTxPoolsMap.begin() ; poolIt != m_sidelinkTxPoolsMap.end() ; poolIt++)
    {
        //Check if this is a new SC period
        if (frameNo == poolIt->second.m_nextScPeriod.frameNo && subframeNo == poolIt->second.m_nextScPeriod.subframeNo)
        {
            poolIt->second.m_currentScPeriod = poolIt->second.m_nextScPeriod;
            poolIt->second.m_nextScPeriod = poolIt->second.m_pool->GetNextScPeriod (frameNo, subframeNo);
            //adjust because scheduler starts with frame/subframe = 1
            poolIt->second.m_nextScPeriod.frameNo++;
            poolIt->second.m_nextScPeriod.subframeNo++;
            NS_LOG_INFO (this << " Starting new SC period for pool of group " << poolIt->first << ". Next period at " << poolIt->second.m_nextScPeriod.frameNo << "/" << poolIt->second.m_nextScPeriod.subframeNo);

            Ptr<PacketBurst> emptyPb = CreateObject <PacketBurst> ();
            poolIt->second.m_miSlHarqProcessPacket = emptyPb;

            if (poolIt->second.m_pool->GetSchedulingType() == SidelinkCommResourcePool::UE_SELECTED)
            {

                //If m_slHasDataToTx is False here (at the beginning of the period), it means
                //that no transmissions in the PSSCH occurred in the previous SC period.
                //Notify the RRC for stopping SLSS transmissions if appropriate
                if (!m_slHasDataToTx){
                    m_cmacSapUser->NotifyMacHasNotSlDataToSend();
                }
                //Make m_slHasDataToTx = false here (beginning of the period) to detect if transmissions
                //in the PSSCH are performed in this period
                m_slHasDataToTx=false;

                //get the BSR for this pool
                //if we have data in the queue
                //find the BSR for that pool (will also give the SidleinkLcIdentifier)
                std::map <SidelinkLcIdentifier, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
                for (itBsr = m_slBsrReceived.begin () ;  itBsr != m_slBsrReceived.end () ; itBsr++)
                {
                    if (itBsr->first.dstL2Id == poolIt->first)
                    {
                        //this is the BSR for the pool
                        break;
                    }
                }

                if (itBsr == m_slBsrReceived.end () || (*itBsr).second.txQueueSize == 0)
                {
                    NS_LOG_INFO (this << " no BSR received. Assume no data to transfer");

                }
                else
                {
                    //we need to pick a random resource from the pool
                    //NS_ASSERT_MSG (0, "UE_SELECTED pools not implemented");
                    NS_LOG_DEBUG (this << "SL BSR size=" << m_slBsrReceived.size ());
                    SidelinkGrant grant;
                    //in order to pick a resource that is valid, we compute the number of subchannels
                    //on the PSSCH
                    NS_ASSERT_MSG (m_pucchSize % 2 == 0, "Number of RBs for PUCCH must be multiple of 2");
                    //TODO: add function so the RRC tells the MAC what the UL bandwidth is.
                    //currently only the phy has it
                    //uint16_t nbSubchannels = std::floor ((50 - m_pucchSize) / m_slGrantSize);
                    uint16_t nbSubchannels = m_ulBandwidth * 2 / m_slGrantSize; // TODO done?
                    uint16_t nbTxOpt = poolIt->second.m_npscch;//before was (-1)

                    grant.m_resPscch = m_ueSelectedUniformVariable->GetInteger (0, nbTxOpt-1); //Randomly selected Resource in PSCCH.
                    grant.m_tpc = 0;
                    grant.m_hopping = 0;
                    uint16_t subCh = 0;
                    subCh = m_ueSelectedUniformVariable->GetInteger (0, nbSubchannels-1);
                    switch (m_slKtrp)
                    {
                        case 1:
                            grant.m_trp = m_ueSelectedUniformVariable->GetInteger (0, 7);
                            break;
                        case 2:
                            grant.m_trp = m_ueSelectedUniformVariable->GetInteger (8, 35);
                            break;
                        case 4:
                            grant.m_trp = m_ueSelectedUniformVariable->GetInteger (36, 105);
                            break;
                        case 8:
                            grant.m_trp = 106;
                            break;
                        default:
                            NS_FATAL_ERROR ("Invalid KTRP value " << (uint16_t) m_slKtrp << ". Supported values: [1, 2, 4, 8]");
                    }

                    grant.m_rbStart = m_pucchSize / 2 + m_slGrantSize * subCh;
                    grant.m_rbLen = m_slGrantSize;


                    //grant.m_trp = (uint16_t) std::floor (grant.m_resPscch / nbSubchannels)/*m_slItrp*/;
                    grant.m_mcs = m_slGrantMcs;
                    grant.m_tbSize = 0; //computed later
                    poolIt->second.m_nextGrant = grant;
                    poolIt->second.m_grant_received = true;
                    NS_LOG_INFO (this << " UE selected grant: resource=" << (uint16_t) grant.m_resPscch << "/" << poolIt->second.m_npscch << ", rbStart=" << (uint16_t) grant.m_rbStart << ", rbLen=" << (uint16_t) grant.m_rbLen << ", mcs=" << (uint16_t) grant.m_mcs << ", ch=" << subCh << ",itrp=" << (uint16_t) grant.m_trp);

                    /* // Trace SL UE mac scheduling
		                cv2x_SlUeMacStatParameters stats_params;
		                stats_params.m_timestamp = Simulator::Now ().GetMilliSeconds ();
		                stats_params.m_frameNo = frameNo;
		                stats_params.m_subframeNo = subframeNo;
		                stats_params.m_rnti = m_rnti;
		                stats_params.m_mcs = grant.m_mcs;
		                stats_params.m_pscchRi = grant.m_resPscch;
		                stats_params.m_pscchTx1 = 1; //NEED to obtain SF of first Tx in PSCCH!!!!!!!!!!!!!!
		                stats_params.m_pscchTx2 = 2; //NEED to obtain SF of second Tx in PSCCH!!!!!!!!!!!!!!
		                stats_params.m_psschTxStartRB = grant.m_rbStart;
		                stats_params.m_psschTxLengthRB = grant.m_rbLen;
		                stats_params.m_psschItrp = grant.m_trp;

		                m_slUeScheduling (stats_params);
					 */
                }
            }

            //if we received a grant, compute the transmission opportunities for PSCCH and PSSCH
            if (poolIt->second.m_grant_received) {
                //make the grant our current grant
                poolIt->second.m_currentGrant = poolIt->second.m_nextGrant;

                NS_LOG_INFO (this << " Sidelink grant received resource " << (uint32_t) poolIt->second.m_currentGrant.m_resPscch);

                SidelinkCommResourcePool::SubframeInfo tmp;
                tmp.frameNo = poolIt->second.m_currentScPeriod.frameNo-1;
                tmp.subframeNo = poolIt->second.m_currentScPeriod.subframeNo-1;

                // Collect statistics for SL UE mac scheduling trace
                cv2x_SlUeMacStatParameters stats_params;
                stats_params.m_frameNo = tmp.frameNo+1;
                stats_params.m_subframeNo = tmp.subframeNo+1;
                stats_params.m_pscchRi = poolIt->second.m_currentGrant.m_resPscch;
                stats_params.m_cellId = 0;
                stats_params.m_imsi = 0 ;
                stats_params.m_pscchFrame1 = 0;
                stats_params.m_pscchSubframe1 = 0;
                stats_params.m_pscchFrame2 = 0;
                stats_params.m_pscchSubframe2 = 0;
                stats_params.m_psschFrame = 0;
                stats_params.m_psschSubframeStart = 0;


                poolIt->second.m_pscchTx = poolIt->second.m_pool->GetPscchTransmissions (poolIt->second.m_currentGrant.m_resPscch);
                uint16_t tx_counter = 1;
                for (std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt = poolIt->second.m_pscchTx.begin (); txIt != poolIt->second.m_pscchTx.end (); txIt++)
                {
                    txIt->subframe = txIt->subframe + tmp;
                    //adjust for index starting at 1
                    txIt->subframe.frameNo++;
                    txIt->subframe.subframeNo++;
                    NS_LOG_INFO (this << " PSCCH: Subframe " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << ": rbStart=" << (uint32_t) txIt->rbStart << ", rbLen=" << (uint32_t) txIt->nbRb);
                    switch (tx_counter){
                        case 1:
                            stats_params.m_pscchFrame1 = txIt->subframe.frameNo;
                            stats_params.m_pscchSubframe1 = txIt->subframe.subframeNo;
                            break;
                        case 2:
                            stats_params.m_pscchFrame2 = txIt->subframe.frameNo;
                            stats_params.m_pscchSubframe2 = txIt->subframe.subframeNo;
                            break;
                        default:
                            NS_FATAL_ERROR(this << "PSCCH ONLY SUPPORTS 2 TRANSMISSIONS PER UE GRANT!");
                    }
                    tx_counter++;

                }

                poolIt->second.m_psschTx = poolIt->second.m_pool->GetPsschTransmissions (tmp, poolIt->second.m_currentGrant.m_trp, poolIt->second.m_currentGrant.m_rbStart, poolIt->second.m_currentGrant.m_rbLen);
                //adjust PSSCH frame to next period
                for (std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt = poolIt->second.m_psschTx.begin (); txIt != poolIt->second.m_psschTx.end (); txIt++)
                {
                    //txIt->subframe = txIt->subframe + tmp;
                    //adjust for index starting at 1
                    txIt->subframe.frameNo++;
                    txIt->subframe.subframeNo++;
                    NS_LOG_INFO (this << " PSSCH: Subframe " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << ": rbStart=" << (uint32_t) txIt->rbStart << ", rbLen=" << (uint32_t) txIt->nbRb);
                }

                //compute the tb size
                poolIt->second.m_currentGrant.m_tbSize = m_amc->GetUlTbSizeFromMcs (poolIt->second.m_currentGrant.m_mcs, poolIt->second.m_currentGrant.m_rbLen) / 8;
                NS_LOG_INFO ("Sidelink Tb size = " << poolIt->second.m_currentGrant.m_tbSize << " bytes (mcs=" << (uint32_t) poolIt->second.m_currentGrant.m_mcs << ")");

                stats_params.m_rnti = m_rnti;
                stats_params.m_mcs = poolIt->second.m_currentGrant.m_mcs;
                stats_params.m_tbSize = poolIt->second.m_currentGrant.m_tbSize;
                stats_params.m_psschTxStartRB = poolIt->second.m_currentGrant.m_rbStart;
                stats_params.m_psschTxLengthRB = poolIt->second.m_currentGrant.m_rbLen;
                stats_params.m_psschItrp = poolIt->second.m_currentGrant.m_trp;
                stats_params.m_timestamp = Simulator::Now ().GetMilliSeconds ();

                // Call trace
                m_slUeScheduling (stats_params);

                //clear the grant
                poolIt->second.m_grant_received = false;
            }
        }

        std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator allocIt;
        //check if we need to transmit PSCCH
        allocIt = poolIt->second.m_pscchTx.begin();
        if (allocIt != poolIt->second.m_pscchTx.end() && (*allocIt).subframe.frameNo == frameNo && (*allocIt).subframe.subframeNo == subframeNo)
        {
            //transmission of PSCCH, no need for HARQ
            if (poolIt->second.m_pscchTx.size () == 2) {
                NS_LOG_INFO (this << " First PSCCH transmission");
            } else {
                NS_LOG_INFO (this << " Second PSCCH transmission");
            }
            //create SCI message
            cv2x_SciListElement_s sci;
            sci.m_rnti = m_rnti;
            sci.m_resPscch = poolIt->second.m_currentGrant.m_resPscch;
            sci.m_rbStart = poolIt->second.m_currentGrant.m_rbStart;
            sci.m_rbLen = poolIt->second.m_currentGrant.m_rbLen;
            sci.m_trp = poolIt->second.m_currentGrant.m_trp;
            sci.m_mcs = poolIt->second.m_currentGrant.m_mcs;
            sci.m_tbSize = poolIt->second.m_currentGrant.m_tbSize;
            sci.m_groupDstId = (poolIt->first & 0xFF);

            Ptr<cv2x_SciLteControlMessage> msg = Create<cv2x_SciLteControlMessage> ();
            msg->SetSci (sci);
            m_uePhySapProvider->SendLteControlMessage (msg);

            poolIt->second.m_pscchTx.erase (allocIt);
        }

        //check if we need to transmit PSSCH
        allocIt = poolIt->second.m_psschTx.begin();
        if (allocIt != poolIt->second.m_psschTx.end() && (*allocIt).subframe.frameNo == frameNo && (*allocIt).subframe.subframeNo == subframeNo)
        {
            // Collect statistics for SL share channel UE mac scheduling trace
            cv2x_SlUeMacStatParameters stats_sch_params;
            stats_sch_params.m_frameNo = poolIt->second.m_currentScPeriod.frameNo;
            stats_sch_params.m_subframeNo = poolIt->second.m_currentScPeriod.subframeNo;
            stats_sch_params.m_psschFrame = frameNo;
            stats_sch_params.m_psschSubframe = subframeNo;
            stats_sch_params.m_cellId = 0;
            stats_sch_params.m_imsi = 0 ;
            stats_sch_params.m_pscchRi = 0 ;
            stats_sch_params.m_pscchFrame1 = 0;
            stats_sch_params.m_pscchSubframe1 = 0;
            stats_sch_params.m_pscchFrame2 = 0;
            stats_sch_params.m_pscchSubframe2 = 0;
            stats_sch_params.m_psschItrp = 0;
            stats_sch_params.m_psschFrameStart = 0;
            stats_sch_params.m_psschSubframeStart = 0;

            //Get first subframe of PSSCH
            SidelinkCommResourcePool::SubframeInfo currScPeriod;
            currScPeriod.frameNo = poolIt->second.m_currentScPeriod.frameNo-1;
            currScPeriod.subframeNo = poolIt->second.m_currentScPeriod.subframeNo-1;

            std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> psschTx = poolIt->second.m_pool->GetPsschTransmissions (currScPeriod, 0, poolIt->second.m_currentGrant.m_rbStart, poolIt->second.m_currentGrant.m_rbLen);
            for (std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt = psschTx.begin (); txIt != psschTx.end (); txIt++)
            {
                //adjust for index starting at 1
                txIt->subframe.frameNo++;
                txIt->subframe.subframeNo++;
                stats_sch_params.m_psschFrameStart = txIt->subframe.frameNo;
                stats_sch_params.m_psschSubframeStart = txIt->subframe.subframeNo;
                break; //Just need the first one!
            }

            stats_sch_params.m_rnti = m_rnti;
            stats_sch_params.m_mcs = poolIt->second.m_currentGrant.m_mcs;
            stats_sch_params.m_tbSize = poolIt->second.m_currentGrant.m_tbSize;
            stats_sch_params.m_psschTxStartRB = poolIt->second.m_currentGrant.m_rbStart;
            stats_sch_params.m_psschTxLengthRB = poolIt->second.m_currentGrant.m_rbLen;
            stats_sch_params.m_timestamp = Simulator::Now ().GetMilliSeconds ();

            // Call trace
            m_slSharedChUeScheduling (stats_sch_params);


            if (poolIt->second.m_psschTx.size () % 4 == 0)
            {
                NS_LOG_INFO (this << " New PSSCH transmission");
                Ptr<PacketBurst> emptyPb = CreateObject <PacketBurst> ();
                poolIt->second.m_miSlHarqProcessPacket = emptyPb;

                //get the BSR for this pool
                //if we have data in the queue
                //find the BSR for that pool (will also give the SidleinkLcIdentifier)
                std::map <SidelinkLcIdentifier, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
                for (itBsr = m_slBsrReceived.begin () ;  itBsr != m_slBsrReceived.end () ; itBsr++)
                {
                    if (itBsr->first.dstL2Id == poolIt->first)
                    {
                        //this is the BSR for the pool
                        std::map <SidelinkLcIdentifier, LcInfo>::iterator it = m_slLcInfoMap.find (itBsr->first);
                        //for sidelink we should never have retxQueueSize since it is unacknowledged mode
                        //we still keep the process similar to uplink to be more generic (and maybe handle
                        //future modifications)
                        if ( ((*itBsr).second.statusPduSize > 0)
                             || ((*itBsr).second.retxQueueSize > 0)
                             || ((*itBsr).second.txQueueSize > 0))
                        {

                            //We have data to send in the PSSCH, notify the RRC to start/continue sending SLSS if appropriate
                            m_slHasDataToTx = true;
                            m_cmacSapUser->NotifyMacHasSlDataToSend();

                            NS_ASSERT ((*itBsr).second.statusPduSize == 0 && (*itBsr).second.retxQueueSize == 0);
                            //similar code as uplink transmission
                            uint32_t bytesForThisLc = poolIt->second.m_currentGrant.m_tbSize;
                            NS_LOG_LOGIC (this << " RNTI " << m_rnti << " Sidelink Tx " << bytesForThisLc << " bytes to LC " << (uint32_t)(*itBsr).first.lcId << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                            if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                            {
                                (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                                bytesForThisLc -= (*itBsr).second.statusPduSize; //decrement size available for data
                                NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                                (*itBsr).second.statusPduSize = 0;
                            }
                            else
                            {
                                if ((*itBsr).second.statusPduSize > bytesForThisLc)
                                {
                                    NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                                }
                            }

                            if ((bytesForThisLc > 7)    // 7 is the min TxOpportunity useful for Rlc
                                && (((*itBsr).second.retxQueueSize > 0)
                                    || ((*itBsr).second.txQueueSize > 0)))
                            {
                                if ((*itBsr).second.retxQueueSize > 0)
                                {
                                    NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                    {
                                        (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                    }
                                    else
                                    {
                                        (*itBsr).second.retxQueueSize = 0;
                                    }
                                }
                                else if ((*itBsr).second.txQueueSize > 0)
                                {
                                    // minimum RLC overhead due to header
                                    uint32_t rlcOverhead = 2;

                                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                    (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                    if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                    {
                                        (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                                    }
                                    else
                                    {
                                        (*itBsr).second.txQueueSize = 0;
                                    }
                                }
                            }
                            else
                            {
                                if ( ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                                {
                                    if (poolIt->second.m_pool->GetSchedulingType() == SidelinkCommResourcePool::SCHEDULED)
                                    {
                                        // resend BSR info for updating eNB peer MAC
                                        m_freshSlBsr = true;
                                    }
                                }
                            }
                            NS_LOG_LOGIC (this << " RNTI " << m_rnti << " Sidelink Tx " << bytesForThisLc << "\t new queues " << (uint32_t)(*it).first.lcId << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                        }
                        break;
                    }
                }
            }
            else
            {
                NS_LOG_INFO (this << " PSSCH retransmission " << (4 - poolIt->second.m_psschTx.size () % 4));
                Ptr<PacketBurst> pb = poolIt->second.m_miSlHarqProcessPacket;
                for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
                {
                    Ptr<Packet> pkt = (*j)->Copy ();

                    int randm = m_ueSelectedUniformVariable->GetInteger (1, 100);
                    if(randm <= m_pHarq)
                    {
                        m_uePhySapProvider->SendMacPdu (pkt);
                    }
                }
            }

            poolIt->second.m_psschTx.erase (allocIt);
        }
    }
    // V2X communication
    std::map<uint32_t, PoolInfoV2x>::iterator poolIt2;
    SidelinkCommResourcePoolV2x::SubframeInfo tmp;
    tmp.frameNo = frameNo;
    tmp.subframeNo = subframeNo;
    UpdateSensingWindow(tmp);

    if (rndmStart != 0) {
        rndmStart--; // decrease counter until the value is equal to zero
    }

    for(poolIt2 = m_sidelinkTxPoolsMapV2x.begin(); poolIt2 != m_sidelinkTxPoolsMapV2x.end(); poolIt2++)
    {
        //std::cout << frameNo << "/" << subframeNo << ", m_reselctr: " << (int) m_reselCtr << std::endl;
        if (m_reselCtr == 0 && rndmStart == 0)
        {
            if(poolIt2->second.m_pool->GetSchedulingType() == SidelinkCommResourcePoolV2x::UE_SELECTED)
            {
                if (!m_slHasDataToTx)
                {
                    m_cmacSapUser->NotifyMacHasNotSlDataToSend();
                }

                m_slHasDataToTx=false;

                m_reselCtr = GetRndmReselectionCounter(m_pRsvp);
                NS_LOG_DEBUG (this << "New Selected Reselection Counter = " << (int) m_reselCtr);


                SidelinkGrantV2x grant;
                grant.m_prio = 0;
                grant.m_pRsvp = m_pRsvp;

                // if true reuse the previous resource
                // if false calculcate new resource
                double randVal = (double) rand() / (double) RAND_MAX;
                if(randVal < m_probResourceKeep && firstTx == false)
                {
                    NS_ASSERT_MSG (m_probResourceKeep >= 0 && m_probResourceKeep <= 0.8, "Parameter probResourceKeep must be between 0 and 0.8");
                    txInfo.subframe.subframeNo = subframeNo + (m_pRsvp-1)%10;
                    txInfo.subframe.frameNo = frameNo + (m_pRsvp-1)/10;
                    if (txInfo.subframe.subframeNo > 10)
                    {
                        txInfo.subframe.frameNo++;
                        txInfo.subframe.subframeNo -= 10;
                    }
                    if (txInfo.subframe.frameNo > 1024)
                    {
                        txInfo.subframe.frameNo -= 1024;
                    }
                }
                else
                {
                    firstTx = false;
                    SidelinkCommResourcePoolV2x::SubframeInfo subframe;
                    subframe.frameNo = frameNo;
                    subframe.subframeNo = subframeNo;

                    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator txOppsIt; // iterator for tx opportunities
                    PoolInfoV2x pool = poolIt2->second;
                    txOpps = GetTxResources(subframe, pool);
                    txOppsIt = txOpps.begin();

                    // Walk through list until the random element is reached
                    std::advance(txOppsIt, m_ueSelectedUniformVariable->GetInteger (0, txOpps.size()-1));
                    txInfo = *txOppsIt;
                    //std::cout << "selected resource " << txInfo.subframe.frameNo << "/" << txInfo.subframe.subframeNo << "\t rbStart=" << (int) txInfo.rbStart << "\t rbLen=" << (int) txInfo.rbLen << std::endl;
                }

                if(m_v2xHarqEnabled)
                {
                    std::list<SidelinkTransmissionInfoExtended> reTxOpps;
                    SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo reTxInfo; // TransmissionInfo of retransmission resource

                    // monitor only subframes within range +-15ms of first subframes
                    reTxOpps = GetReTxResources(txInfo.subframe, txOpps);

                    if(reTxOpps.size() != 0)
                    {
                        std::list<SidelinkTransmissionInfoExtended>::iterator reTxOppsIt;
                        reTxOppsIt = reTxOpps.begin();
                        std::advance(reTxOppsIt, m_ueSelectedUniformVariable->GetInteger (0, reTxOpps.size()-1));
                        reTxInfo = reTxOppsIt->m_txInfo;
                        grant.m_sfGap = reTxOppsIt->m_sfGap;
                        grant.m_reTxIdx = reTxOppsIt->m_reTxIdx;
                        grant.m_riv = CalcRiv(m_subchLen, std::floor(reTxInfo.rbStart/m_sizeSubchannel));

                    }
                    else
                    {
                        grant.m_sfGap = 0;
                        grant.m_reTxIdx = 0;
                        grant.m_riv = CalcRiv(m_subchLen,0);
                    }

                }
                else
                {
                    grant.m_sfGap = 0;
                    grant.m_reTxIdx = 0;
                    grant.m_riv = CalcRiv(m_subchLen,0);
                }

                if (m_adjacency)
                {
                    grant.m_resPscch = (txInfo.rbStart-2)/m_sizeSubchannel;
                }
                else
                {
                    grant.m_resPscch = txInfo.rbStart / m_sizeSubchannel;
                }
                grant.m_mcs = m_slGrantMcs;
                grant.m_tbSize = 0; // computed later
                poolIt2->second.m_nextGrant = grant;
                poolIt2->second.m_grant_received = true;
                //std::cout << "UE selected grant: SubchannelLength=" << (uint16_t) m_subchLen << ", PscchResource=" << (int) grant.m_resPscch << ", MCS=" << (int) grant.m_mcs << ", SfGap=" << (int) grant.m_sfGap << ", Retransmissionindex=" << (int) grant.m_reTxIdx << std::endl;
                NS_LOG_INFO (this << " UE selected grant: SubchannelLength=" << (uint16_t) m_subchLen << ", PscchResource=" << (int) grant.m_resPscch << ", MCS=" << (int) grant.m_mcs << ", SfGap=" << (int) grant.m_sfGap << ", Retransmissionindex=" << (int) grant.m_reTxIdx);
            }
            //if we received a grant, compute the transmission for PSCCH and PSSCH
            if(poolIt2->second.m_grant_received)
            {
                // make the grant our current grant
                poolIt2->second.m_currentGrant = poolIt2->second.m_nextGrant;

                NS_LOG_INFO (this << " Sidelink grant received resource " << (uint32_t) poolIt2->second.m_currentGrant.m_resPscch);

                // Collect statistics for SL UE mac scheduling trace
                cv2x_SlUeMacStatParametersV2x stats_params;
                stats_params.m_frameNo = frameNo;
                stats_params.m_subframeNo = subframeNo;
                stats_params.m_resPscch = poolIt2->second.m_currentGrant.m_resPscch;
                stats_params.m_cellId = 0;
                stats_params.m_imsi = 0;
                stats_params.m_txFrame = 0;
                stats_params.m_txSubframe = 0;

                std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator txIt;
                poolIt2->second.m_pscchTx = poolIt2->second.m_pool->GetPscchTransmissions(txInfo.subframe,poolIt2->second.m_currentGrant.m_riv,poolIt2->second.m_currentGrant.m_pRsvp,poolIt2->second.m_currentGrant.m_sfGap,poolIt2->second.m_currentGrant.m_reTxIdx,poolIt2->second.m_currentGrant.m_resPscch,m_reselCtr);
                for(txIt = poolIt2->second.m_pscchTx.begin(); txIt != poolIt2->second.m_pscchTx.end(); txIt++)
                {
                    //std::cout << "MAC PSCCH TX at " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) txIt->rbStart << "\t rbLen=" << (uint32_t) txIt->rbLen << std::endl;
                    NS_LOG_INFO(this << " PSCCH TX: " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) txIt->rbStart << "\t rbLen=" << (uint32_t) txIt->rbLen);
                    stats_params.m_txFrame = txIt->subframe.frameNo;
                    stats_params.m_txSubframe = txIt->subframe.subframeNo;
                    NS_ASSERT (txIt->subframe.subframeNo > 0 && txIt->subframe.subframeNo <= 10 && txIt->subframe.frameNo > 0 && txIt->subframe.frameNo <= 1024);
                }

                poolIt2->second.m_psschTx = poolIt2->second.m_pool->GetPsschTransmissions(txInfo.subframe,poolIt2->second.m_currentGrant.m_riv,poolIt2->second.m_currentGrant.m_pRsvp,poolIt2->second.m_currentGrant.m_sfGap,poolIt2->second.m_currentGrant.m_reTxIdx,poolIt2->second.m_currentGrant.m_resPscch,m_reselCtr);
                for(txIt = poolIt2->second.m_psschTx.begin(); txIt != poolIt2->second.m_psschTx.end(); txIt++)
                {
                    //std::cout << "Rnti=" <<  m_rnti << "\t MAC PSSCH TX at " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) txIt->rbStart << "\t rbLen=" << (uint32_t) txIt->rbLen << std::endl;
                    NS_LOG_INFO(this << " PSSCH TX: " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) txIt->rbStart << "\t rbLen=" << (uint32_t) txIt->rbLen);
                    NS_ASSERT (txIt->subframe.subframeNo > 0 && txIt->subframe.subframeNo <= 10 && txIt->subframe.frameNo > 0 && txIt->subframe.frameNo <= 1024);
                }

                //compute the tb size
                if (m_adjacency) {
                    stats_params.m_psschTxLengthRB = m_subchLen*m_sizeSubchannel-2;
                    stats_params.m_psschTxStartRB = poolIt2->second.m_currentGrant.m_resPscch*m_sizeSubchannel+m_startRbSubchannel+2;
                    poolIt2->second.m_currentGrant.m_tbSize = m_amc->GetUlTbSizeFromMcs(poolIt2->second.m_currentGrant.m_mcs, m_subchLen*m_sizeSubchannel-2) / 8;
                }
                else {
                    stats_params.m_psschTxLengthRB = m_subchLen*m_sizeSubchannel;
                    stats_params.m_psschTxStartRB = poolIt2->second.m_currentGrant.m_resPscch*m_sizeSubchannel+m_startRbSubchannel;
                    poolIt2->second.m_currentGrant.m_tbSize = m_amc->GetUlTbSizeFromMcs(poolIt2->second.m_currentGrant.m_mcs, m_subchLen*m_sizeSubchannel) / 8;
                }
                NS_LOG_INFO ("Sidelink Tb size = " << poolIt2->second.m_currentGrant.m_tbSize << " bytes (mcs=" << (uint32_t) poolIt2->second.m_currentGrant.m_mcs << ")");

                stats_params.m_rnti = m_rnti;
                stats_params.m_mcs = poolIt2->second.m_currentGrant.m_mcs;
                stats_params.m_tbSize = poolIt2->second.m_currentGrant.m_tbSize;

                stats_params.m_timestamp = Simulator::Now().GetMilliSeconds();

                // Call trace
                m_slUeSchedulingV2x(stats_params);

                // clear the grant
                poolIt2->second.m_grant_received = false;
            }
        }

        NS_ASSERT (poolIt2->second.m_pscchTx.size() == poolIt2->second.m_psschTx.size());

        std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator allocItPscch;
        allocItPscch = poolIt2->second.m_pscchTx.begin();
        std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator allocItPssch;
        allocItPssch = poolIt2->second.m_psschTx.begin();

        // check if we need to transmit PSCCH + PSSCH
        if(allocItPscch != poolIt2->second.m_pscchTx.end() && (*allocItPscch).subframe.frameNo == frameNo && (*allocItPscch).subframe.subframeNo == subframeNo && allocItPssch != poolIt2->second.m_psschTx.end() && (*allocItPssch).subframe.frameNo == frameNo && (*allocItPssch).subframe.subframeNo == subframeNo)
        {
            //decrease reselection counter
            m_reselCtr--;

            NS_LOG_INFO (this << " New PSCCH transmission");
            // create SCI-1 message
            cv2x_SciListElementV2x sci1;
            sci1.m_rnti = m_rnti;
            sci1.m_prio = poolIt2->second.m_currentGrant.m_prio;
            sci1.m_pRsvp = poolIt2->second.m_currentGrant.m_pRsvp;
            sci1.m_riv = poolIt2->second.m_currentGrant.m_riv;
            sci1.m_sfGap = poolIt2->second.m_currentGrant.m_sfGap;
            sci1.m_mcs = poolIt2->second.m_currentGrant.m_mcs;
            sci1.m_reTxIdx = poolIt2->second.m_currentGrant.m_reTxIdx;
            sci1.m_tbSize = poolIt2->second.m_currentGrant.m_tbSize;
            sci1.m_resPscch = poolIt2->second.m_currentGrant.m_resPscch;

            Ptr<cv2x_SciLteControlMessageV2x> msg = Create<cv2x_SciLteControlMessageV2x> ();
            msg->SetSci (sci1);
            m_sidelinkV2xAnnouncementTrace();
            m_uePhySapProvider->SendLteControlMessage (msg);
            poolIt2->second.m_pscchTx.erase (allocItPscch);

            // Collect statistics for SL share channel UE mac scheduling trace
            cv2x_SlUeMacStatParametersV2x stats_sch_params;
            stats_sch_params.m_frameNo = frameNo;
            stats_sch_params.m_subframeNo = subframeNo;
            stats_sch_params.m_cellId = 0;
            stats_sch_params.m_imsi = 0;
            stats_sch_params.m_rnti = m_rnti;
            stats_sch_params.m_mcs = poolIt2->second.m_currentGrant.m_mcs;
            stats_sch_params.m_tbSize = poolIt2->second.m_currentGrant.m_tbSize;
            stats_sch_params.m_timestamp = Simulator::Now ().GetMilliSeconds ();

            // Call trace
            m_slSharedChUeSchedulingV2x (stats_sch_params);

            NS_LOG_INFO (this << " New PSSCH transmission");
            Ptr<PacketBurst> emptyPb = CreateObject <PacketBurst> ();
            poolIt2->second.m_miSlHarqProcessPacket = emptyPb;

            std::map <SidelinkLcIdentifier, cv2x_LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
            for (itBsr = m_slBsrReceived.begin(); itBsr != m_slBsrReceived.end(); itBsr++)
            {
                if (itBsr->first.dstL2Id == poolIt2->first)
                {
                    //this is the BSR for the pool
                    std::map <SidelinkLcIdentifier, LcInfo>::iterator it = m_slLcInfoMap.find (itBsr->first);
                    //for sidelink we should never have retxQueueSize since it is unacknowledged mode
                    //we still keep the process similar to uplink to be more generic (and maybe handle
                    //future modifications)
                    if (((*itBsr).second.statusPduSize > 0)
                        || ((*itBsr).second.retxQueueSize > 0)
                        || ((*itBsr).second.txQueueSize > 0))
                    {
                        //we have data to send in the PSSCH, notify the RRC to start/continue sending SLSS if appropriate
                        m_slHasDataToTx = true;
                        m_cmacSapUser->NotifyMacHasSlDataToSend();

                        NS_ASSERT ((*itBsr).second.statusPduSize == 0 && (*itBsr).second.retxQueueSize == 0);

                        uint32_t bytesForThisLc = poolIt2->second.m_currentGrant.m_tbSize;
                        NS_LOG_LOGIC(this << "RNTI " << m_rnti << " Sidelink Tx " << bytesForThisLc << " bytes to LC " << (uint32_t)(*itBsr).first.lcId << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" << (*itBsr).second.txQueueSize);

                        if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                        {
                            (*it).second.macSapUser->NotifyTxOpportunity ((*itBsr).second.statusPduSize, 0, 0, m_componentCarrierId, m_rnti, 0);
                            bytesForThisLc -= (*itBsr).second.statusPduSize;
                            NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                            (*itBsr).second.statusPduSize = 0;
                        }
                        else
                        {
                            if ((*itBsr).second.statusPduSize > bytesForThisLc)
                            {
                                NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                            }
                        }

                        if ((bytesForThisLc > 7) // 7 is the min TxOpportunity useful for Rlc
                            && (((*itBsr).second.retxQueueSize > 0)
                                || ((*itBsr).second.txQueueSize > 0)))
                        {
                            if ((*itBsr).second.retxQueueSize > 0)
                            {
                                NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                                (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                {
                                    (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                }
                                else
                                {
                                    (*itBsr).second.retxQueueSize = 0;
                                }
                            }
                            else if ((*itBsr).second.txQueueSize > 0)
                            {
                                // minimum RLC overhead due to header
                                uint32_t rlcOverhead = 2;

                                NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                                (*it).second.macSapUser->NotifyTxOpportunity (bytesForThisLc, 0, 0, m_componentCarrierId, m_rnti, 0);
                                if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                {
                                    (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                                }
                                else
                                {
                                    (*itBsr).second.txQueueSize = 0;
                                }
                            }
                        }
                        else
                        {
                            if (((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
                            {
                                if (poolIt2->second.m_pool->GetSchedulingType() == SidelinkCommResourcePoolV2x::SCHEDULED)
                                {
                                    // resend BSR info for updating eNb peer
                                    m_freshSlBsr = true;
                                }
                            }
                        }
                        NS_LOG_LOGIC (this << " RNTI " << m_rnti << " Sidelink Tx " << bytesForThisLc << "\t new queues " << (uint32_t)(*it).first.lcId << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" << (*itBsr).second.txQueueSize);
                    }
                    break;
                }
            }
            poolIt2->second.m_psschTx.erase (allocItPssch);
        }
    }
}

int64_t
cv2x_LteUeMac::AssignStreams (int64_t stream)
{
    NS_LOG_FUNCTION (this << stream);
    m_raPreambleUniformVariable->SetStream (stream);
    return 1;
}

void
cv2x_LteUeMac::DoAddSlDestination (uint32_t destination)
{
    std::list <uint32_t>::iterator it;
    for (it = m_sidelinkDestinations.begin (); it != m_sidelinkDestinations.end ();it++) {
        if ((*it) == destination) {
            break;
        }
    }
    if (it == m_sidelinkDestinations.end ()) {
        //did not find it, so insert
        m_sidelinkDestinations.push_back (destination);
    }
}


void
cv2x_LteUeMac::DoRemoveSlDestination (uint32_t destination)
{
    std::list <uint32_t>::iterator it = m_sidelinkDestinations.begin ();
    while (it != m_sidelinkDestinations.end ()) {
        if ((*it) == destination) {
            m_sidelinkDestinations.erase (it);
            break;//leave the loop
        }
        it++;
    }
}

void
cv2x_LteUeMac::DoPassSensingData(uint32_t frameNo, uint32_t subframeNo, uint16_t pRsvp, uint8_t rbStart, uint8_t rbLen, uint8_t prio, double slRsrp, double slRssi)
{
    SensingData sensingData;
    sensingData.m_rxInfo.rbStart = rbStart;
    sensingData.m_rxInfo.rbLen = rbLen;
    sensingData.m_pRsvpRx = pRsvp;
    sensingData.m_prioRx = prio;
    sensingData.m_slRsrp = slRsrp;
    sensingData.m_slRssi = slRssi;

    if (slRssi != 0.0) {
        std::cout << "Here";
    }

    if (frameNo == 1 && subframeNo == 1)
    {
        sensingData.m_rxInfo.subframe.frameNo = 1024;
        sensingData.m_rxInfo.subframe.subframeNo = 10;
    }
    else if (subframeNo == 1)
    {
        sensingData.m_rxInfo.subframe.frameNo = frameNo-1;
        sensingData.m_rxInfo.subframe.subframeNo = 10;
    }
    else
    {
        sensingData.m_rxInfo.subframe.frameNo = frameNo;
        sensingData.m_rxInfo.subframe.subframeNo = subframeNo-1;
    }
    SetSignalInfo(slRssi, slRsrp);
    m_sensingData.push_back(sensingData);
}

void
cv2x_LteUeMac::DoNotifyChangeOfTiming(uint32_t frameNo, uint32_t subframeNo)
{
    NS_LOG_FUNCTION (this);

    //there is a delay between the MAC scheduling and the transmission so we assume that we are ahead
    subframeNo += 4;
    if (subframeNo > 10)
    {
        ++frameNo;
        if (frameNo > 1024)
            frameNo = 1;
        subframeNo -= 10;
    }

    std::map <uint32_t, PoolInfo>::iterator poolIt;
    for (poolIt = m_sidelinkTxPoolsMap.begin() ; poolIt != m_sidelinkTxPoolsMap.end() ; poolIt++)
    {
        poolIt->second.m_currentScPeriod = poolIt->second.m_pool->GetCurrentScPeriod (frameNo, subframeNo);
        poolIt->second.m_nextScPeriod = poolIt->second.m_pool->GetNextScPeriod (poolIt->second.m_currentScPeriod.frameNo, poolIt->second.m_currentScPeriod.subframeNo);
        //adjust because scheduler starts with frame/subframe = 1
        poolIt->second.m_nextScPeriod.frameNo++;
        poolIt->second.m_nextScPeriod.subframeNo++;
        NS_LOG_INFO (this << " Adapting the period for pool of group " << poolIt->first << ". Next period at " << poolIt->second.m_nextScPeriod.frameNo << "/" << poolIt->second.m_nextScPeriod.subframeNo);
    }
}

std::list< Ptr<SidelinkRxDiscResourcePool> >
cv2x_LteUeMac::GetDiscRxPools ()
{
    NS_LOG_FUNCTION (this);
    return m_discRxPools;
}

Ptr<SidelinkTxDiscResourcePool>
cv2x_LteUeMac::GetDiscTxPool ()
{
    NS_LOG_FUNCTION (this);
    return m_discTxPools.m_pool;
}

} // namespace ns3
