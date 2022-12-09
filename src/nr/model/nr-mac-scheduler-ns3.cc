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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);

#include "nr-mac-scheduler-ns3.h"
#include "nr-mac-scheduler-harq-rr.h"
#include "nr-mac-short-bsr-ce.h"
#include "nr-mac-scheduler-srs-default.h"

#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/log.h>
#include <ns3/eps-bearer.h>
#include <ns3/pointer.h>
#include <algorithm>
#include <ns3/integer.h>
#include <unordered_set>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerNs3");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerNs3);

NrMacSchedulerNs3::NrMacSchedulerNs3 () : NrMacScheduler ()
{
  NS_LOG_FUNCTION_NOARGS ();

  // Hardcoded, but the type can be a parameter if needed
  m_schedHarq = std::unique_ptr<NrMacSchedulerHarqRr> (new NrMacSchedulerHarqRr ());
  m_schedHarq->InstallGetBwInRBG (std::bind (&NrMacSchedulerNs3::GetBandwidthInRbg, this));
  m_schedHarq->InstallGetBwpIdFn (std::bind (&NrMacSchedulerNs3::GetBwpId, this));
  m_schedHarq->InstallGetCellIdFn (std::bind (&NrMacSchedulerNs3::GetCellId, this));

  m_cqiManagement.InstallGetBwpIdFn (std::bind (&NrMacSchedulerNs3::GetBwpId, this));
  m_cqiManagement.InstallGetCellIdFn (std::bind (&NrMacSchedulerNs3::GetCellId, this));
  m_cqiManagement.InstallGetNrAmcDlFn (std::bind ([this] () { return m_dlAmc; }));
  m_cqiManagement.InstallGetNrAmcUlFn (std::bind ([this] () { return m_ulAmc; }));
  m_cqiManagement.InstallGetStartMcsDlFn (std::bind ([this] () { return m_startMcsDl; }));
  m_cqiManagement.InstallGetStartMcsUlFn (std::bind ([this] () { return m_startMcsUl; }));

  // If more Srs allocators will be created, then we will add an attribute
  m_schedulerSrs = CreateObject<NrMacSchedulerSrsDefault> ();
}

NrMacSchedulerNs3::~NrMacSchedulerNs3 ()
{
  m_ueMap.clear ();
}

void
NrMacSchedulerNs3::InstallDlAmc (const Ptr<NrAmc> &dlAmc)
{
  m_dlAmc = dlAmc;
  m_dlAmc->SetDlMode ();
}

void
NrMacSchedulerNs3::InstallUlAmc (const Ptr<NrAmc> &ulAmc)
{
  m_ulAmc = ulAmc;
  m_ulAmc->SetUlMode ();
}

Ptr<const NrAmc>
NrMacSchedulerNs3::GetUlAmc() const
{
  NS_LOG_FUNCTION (this);
  return m_ulAmc;
}

Ptr<const NrAmc>
NrMacSchedulerNs3::GetDlAmc() const
{
  NS_LOG_FUNCTION (this);
  return m_dlAmc;
}

int64_t
NrMacSchedulerNs3::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  return m_schedulerSrs->AssignStreams (stream);;
}

TypeId
NrMacSchedulerNs3::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerNs3")
    .SetParent<NrMacScheduler> ()
    .AddAttribute ("CqiTimerThreshold",
                   "The time while a CQI is valid",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&NrMacSchedulerNs3::SetCqiTimerThreshold,
                                     &NrMacSchedulerNs3::GetCqiTimerThreshold),
                   MakeTimeChecker ())
    .AddAttribute ("FixedMcsDl",
                   "Fix MCS to value set in StartingMcsDl",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrMacSchedulerNs3::SetFixedDlMcs,
                                        &NrMacSchedulerNs3::IsDlMcsFixed),
                   MakeBooleanChecker ())
    .AddAttribute ("FixedMcsUl",
                   "Fix MCS to value set in StartingMcsUl",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrMacSchedulerNs3::SetFixedUlMcs,
                                        &NrMacSchedulerNs3::IsUlMcsFixed),
                   MakeBooleanChecker ())
    .AddAttribute ("StartingMcsDl",
                   "Starting MCS for DL",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrMacSchedulerNs3::SetStartMcsDl,
                                         &NrMacSchedulerNs3::GetStartMcsDl),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("StartingMcsUl",
                   "Starting MCS for UL",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrMacSchedulerNs3::SetStartMcsUl,
                                         &NrMacSchedulerNs3::GetStartMcsUl),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlCtrlSymbols",
                   "Number of symbols allocated for DL CTRL",
                   UintegerValue (1),
                   MakeUintegerAccessor (&NrMacSchedulerNs3::SetDlCtrlSyms,
                                         &NrMacSchedulerNs3::GetDlCtrlSyms),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("UlCtrlSymbols",
                   "Number of symbols allocated for UL CTRL",
                   UintegerValue (1),
                   MakeUintegerAccessor (&NrMacSchedulerNs3::SetUlCtrlSyms,
                                         &NrMacSchedulerNs3::GetUlCtrlSyms),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("SrsSymbols",
                   "Number of symbols allocated for UL SRS",
                   UintegerValue (1),
                   MakeUintegerAccessor (&NrMacSchedulerNs3::SetSrsCtrlSyms,
                                         &NrMacSchedulerNs3::GetSrsCtrlSyms),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("EnableSrsInUlSlots",
                   "Denotes whether the SRSs will be transmitted only in F slots"
                   "or both in F and UL slots. If False, SRS is transmitted only"
                   "in F slots, if True in both (F/UL)",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrMacSchedulerNs3::SetSrsInUlSlots,
                                        &NrMacSchedulerNs3::IsSrsInUlSlots),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableSrsInFSlots",
                   "Denotes whether the SRSs will be transmitted in F slots"
                   "If true, it can be transmitted in F slots, otherwise "
                   "it cannot.",
                    BooleanValue (true),
                    MakeBooleanAccessor (&NrMacSchedulerNs3::SetSrsInFSlots,
                                         &NrMacSchedulerNs3::IsSrsInFSlots),
                    MakeBooleanChecker ())
    .AddAttribute ("DlAmc",
                   "The DL AMC of this scheduler",
                   PointerValue (),
                   MakePointerAccessor (&NrMacSchedulerNs3::m_dlAmc),
                   MakePointerChecker <NrAmc> ())
    .AddAttribute ("UlAmc",
                   "The UL AMC of this scheduler",
                   PointerValue (),
                   MakePointerAccessor (&NrMacSchedulerNs3::m_ulAmc),
                   MakePointerChecker <NrAmc> ())
    .AddAttribute ("MaxDlMcs",
                   "Maximum MCS index for DL",
                   IntegerValue (-1),
                   MakeIntegerAccessor (&NrMacSchedulerNs3::SetMaxDlMcs,
                                         &NrMacSchedulerNs3::GetMaxDlMcs),
                   MakeIntegerChecker<int8_t> (-1,30))
    .AddAttribute ("EnableHarqReTx",
                   "If true, it would set the max HARQ ReTx to 3; otherwise it set it to 0",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrMacSchedulerNs3::EnableHarqReTx,
                                        &NrMacSchedulerNs3::IsHarqReTxEnable),
                                        MakeBooleanChecker ())
  ;

  return tid;
}

/**
 * \brief Set a fixed MCS.
 * \param mcs The MCS.
 *
 * Set a fixed MCS for all UE that will be registered *AFTER* the call to this
 * function.
 */
void
NrMacSchedulerNs3::DoSchedSetMcs (uint32_t mcs)
{
  NS_LOG_FUNCTION (this);
  m_fixedMcsDl = true;
  m_fixedMcsUl = true;
  m_startMcsDl = static_cast<uint8_t> (mcs);
  m_startMcsUl = static_cast<uint8_t> (mcs);
}

void
NrMacSchedulerNs3::DoSchedDlRachInfoReq (const NrMacSchedSapProvider::SchedDlRachInfoReqParameters &params)
{
  NS_LOG_FUNCTION (this);

  m_rachList = params.m_rachList;
}

void
NrMacSchedulerNs3::SetCqiTimerThreshold (const Time &v)
{
  NS_LOG_FUNCTION (this);
  m_cqiTimersThreshold = v;
}

Time
NrMacSchedulerNs3::GetCqiTimerThreshold () const
{
  NS_LOG_FUNCTION (this);
  return m_cqiTimersThreshold;
}

void
NrMacSchedulerNs3::SetFixedDlMcs (bool v)
{
  NS_LOG_FUNCTION (this);
  m_fixedMcsDl = v;
}

bool
NrMacSchedulerNs3::IsDlMcsFixed () const
{
  NS_LOG_FUNCTION (this);
  return m_fixedMcsDl;
}

void
NrMacSchedulerNs3::SetFixedUlMcs (bool v)
{
  NS_LOG_FUNCTION (this);
  m_fixedMcsUl = v;
}

bool
NrMacSchedulerNs3::IsUlMcsFixed () const
{
  NS_LOG_FUNCTION (this);
  return m_fixedMcsUl;
}

void
NrMacSchedulerNs3::SetStartMcsDl (uint8_t v)
{
  NS_LOG_FUNCTION (this);
  m_startMcsDl = v;
}

uint8_t
NrMacSchedulerNs3::GetStartMcsDl () const
{
  NS_LOG_FUNCTION (this);
  return m_startMcsDl;
}

void
NrMacSchedulerNs3::SetMaxDlMcs (int8_t v)
{
  NS_LOG_FUNCTION (this);
  m_maxDlMcs = v;
}

int8_t
NrMacSchedulerNs3::GetMaxDlMcs () const
{
  NS_LOG_FUNCTION (this);
  return m_maxDlMcs;
}
void
NrMacSchedulerNs3::SetStartMcsUl (uint8_t v)
{
  NS_LOG_FUNCTION (this);
  m_startMcsUl = v;
}

uint8_t
NrMacSchedulerNs3::GetStartMcsUl () const
{
  NS_LOG_FUNCTION (this);
  return m_startMcsUl;
}

void
NrMacSchedulerNs3::SetDlCtrlSyms (uint8_t v)
{
  m_dlCtrlSymbols = v;
}

uint8_t
NrMacSchedulerNs3::GetDlCtrlSyms () const
{
  return m_dlCtrlSymbols;
}

void
NrMacSchedulerNs3::SetUlCtrlSyms (uint8_t v)
{
  m_ulCtrlSymbols = v;
}

void
NrMacSchedulerNs3::SetDlNotchedRbgMask (const std::vector<uint8_t> &dlNotchedRbgsMask)
{
  NS_LOG_FUNCTION (this);
  m_dlNotchedRbgsMask = dlNotchedRbgsMask;
  std::stringstream ss;

  //print the DL mask set (prefix + is added just for printing purposes)
  for (const auto & x : m_dlNotchedRbgsMask)
    {
      ss << +x << " ";
    }
  NS_LOG_INFO ("Set DL notched mask: " << ss.str ());
}

std::vector<uint8_t>
NrMacSchedulerNs3::GetDlNotchedRbgMask (void) const
{
  return m_dlNotchedRbgsMask;
}

void
NrMacSchedulerNs3::SetUlNotchedRbgMask (const std::vector<uint8_t> &ulNotchedRbgsMask)
{
  NS_LOG_FUNCTION (this);
  m_ulNotchedRbgsMask = ulNotchedRbgsMask;
  std::stringstream ss;

  //print the UL mask set (prefix + is added just for printing purposes)
  for (const auto & x : m_ulNotchedRbgsMask)
    {
      ss << +x << " ";
    }
  NS_LOG_INFO ("Set UL notched mask: " << ss.str ());
}

std::vector<uint8_t>
NrMacSchedulerNs3::GetUlNotchedRbgMask (void) const
{
  return m_ulNotchedRbgsMask;
}

void
NrMacSchedulerNs3::SetSrsCtrlSyms (uint8_t v)
{
  m_srsCtrlSymbols = v;
}

uint8_t
NrMacSchedulerNs3::GetSrsCtrlSyms () const
{
  return m_srsCtrlSymbols;
}

void
NrMacSchedulerNs3::SetSrsInUlSlots (bool v)
{
  m_enableSrsInUlSlots = v;
}

bool
NrMacSchedulerNs3::IsSrsInUlSlots () const
{
  return m_enableSrsInUlSlots;
}


void
NrMacSchedulerNs3::SetSrsInFSlots (bool v)
{
  m_enableSrsInFSlots = v;
}

bool
NrMacSchedulerNs3::IsSrsInFSlots () const
{
  return m_enableSrsInFSlots;
}

void
NrMacSchedulerNs3::EnableHarqReTx (bool enableFlag)
{
  m_enableHarqReTx = enableFlag;
}

bool
NrMacSchedulerNs3::IsHarqReTxEnable () const
{
  return m_enableHarqReTx;
}


uint8_t
NrMacSchedulerNs3::ScheduleDlHarq (PointInFTPlane *startingPoint,
                                       uint8_t symAvail,
                                       const NrMacSchedulerNs3::ActiveHarqMap &activeDlHarq,
                                       const std::unordered_map<uint16_t, UePtr> &ueMap,
                                       std::vector<DlHarqInfo> *dlHarqToRetransmit,
                                       const std::vector<DlHarqInfo> &dlHarqFeedback,
                                       SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  return m_schedHarq->ScheduleDlHarq (startingPoint, symAvail, activeDlHarq,
                                      ueMap, dlHarqToRetransmit, dlHarqFeedback, slotAlloc);
}

uint8_t
NrMacSchedulerNs3::ScheduleUlHarq (PointInFTPlane *startingPoint,
                                       uint8_t symAvail,
                                       const std::unordered_map<uint16_t, UePtr> &ueMap,
                                       std::vector<UlHarqInfo> *ulHarqToRetransmit,
                                       const std::vector<UlHarqInfo> &ulHarqFeedback,
                                       SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  return m_schedHarq->ScheduleUlHarq (startingPoint, symAvail,
                                      ueMap, ulHarqToRetransmit, ulHarqFeedback, slotAlloc);
}

void
NrMacSchedulerNs3::SortDlHarq (NrMacSchedulerNs3::ActiveHarqMap *activeDlHarq) const
{
  NS_LOG_FUNCTION (this);
  m_schedHarq->SortDlHarq (activeDlHarq);
}

void NrMacSchedulerNs3::SortUlHarq (NrMacSchedulerNs3::ActiveHarqMap *activeUlHarq) const
{
  NS_LOG_FUNCTION (this);
  m_schedHarq->SortDlHarq (activeUlHarq);
}

uint8_t
NrMacSchedulerNs3::GetUlCtrlSyms () const
{
  return m_ulCtrlSymbols;
}

/**
 * \brief Cell configuration
 * \param params unused.
 *
 * Ignored. Always Success.
 */
void
NrMacSchedulerNs3::DoCschedCellConfigReq (const NrMacCschedSapProvider::CschedCellConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (params.m_ulBandwidth == params.m_dlBandwidth);
  m_bandwidth = params.m_dlBandwidth;

  NrMacCschedSapUser::CschedUeConfigCnfParameters cnf;
  cnf.m_result = SUCCESS;
  m_macCschedSapUser->CschedUeConfigCnf (cnf);
}

/**
 * \brief Register an UE
 * \param params params of the UE
 *
 * If the UE is not registered, then create its representation with a call to
 * CreateUeRepresentation, and then save its pointer in the m_ueMap map.
 *
 * If the UE is registered, update its corresponding beam.
 */
void
NrMacSchedulerNs3::DoCschedUeConfigReq (const NrMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this <<
                   " RNTI " << params.m_rnti <<
                   " txMode " << static_cast<uint32_t> (params.m_transmissionMode));

  auto itUe = m_ueMap.find (params.m_rnti);
  GetSecond UeInfoOf;
  if (itUe == m_ueMap.end ())
    {
      itUe = m_ueMap.insert (std::make_pair (params.m_rnti, CreateUeRepresentation (params))).first;

      UeInfoOf (*itUe)->m_dlHarq.SetMaxSize (static_cast<uint8_t> (m_macSchedSapUser->GetNumHarqProcess ()));
      UeInfoOf (*itUe)->m_ulHarq.SetMaxSize (static_cast<uint8_t> (m_macSchedSapUser->GetNumHarqProcess ()));
      UeInfoOf (*itUe)->m_dlMcs.push_back (m_startMcsDl);
      UeInfoOf (*itUe)->m_startMcsDlUe = m_startMcsDl;
      UeInfoOf (*itUe)->m_dlCqi.m_ri = 1;
      UeInfoOf (*itUe)->m_ulMcs = m_startMcsUl;

      NrMacSchedulerSrs::SrsPeriodicityAndOffset srs = m_schedulerSrs->AddUe ();

      if (! srs.m_isValid)
        {
          bool ret = m_schedulerSrs->IncreasePeriodicity (&m_ueMap); // The new UE will get the SRS offset/periodicity here
          NS_ASSERT (ret);
        }
      else
        {
          UeInfoOf (*itUe)->m_srsPeriodicity = srs.m_periodicity; // set the periodicity/offset based on the return value
          UeInfoOf (*itUe)->m_srsOffset = srs.m_offset;
        }

      NS_LOG_INFO ("Creating user, beam " << params.m_beamConfId << " and ue " << params.m_rnti <<
                   " assigned SRS periodicity " << srs.m_periodicity << " and offset " <<
                   srs.m_offset);
    }
  else
    {
      NS_LOG_LOGIC ("Updating Beam for UE " << params.m_rnti << " beam " << params.m_beamConfId);
      UeInfoOf (*itUe)->m_beamConfId = params.m_beamConfId;
    }
}

/**
 * \brief Release an UE
 * \param params params of the UE to release
 *
 * Remove the UE from the ueMap (m_ueMap) and release its SRS offset for
 * later usage.
 */
void
NrMacSchedulerNs3::DoCschedUeReleaseReq (const NrMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this << " Release RNTI " << params.m_rnti);

  auto itUe = m_ueMap.find (params.m_rnti);
  NS_ABORT_IF (itUe == m_ueMap.end ());

  m_schedulerSrs->RemoveUe (itUe->second->m_srsOffset);
  m_ueMap.erase (itUe);

  // When it will be the case of reducing the periodicity? Question for the
  // future...

  NS_LOG_INFO ("Release RNTI " << params.m_rnti);
}

uint64_t
NrMacSchedulerNs3::GetNumRbPerRbg () const
{
  return m_macSchedSapUser->GetNumRbPerRbg();
}

/**
 * \brief Create a logical channel starting from a configuration
 * \param config configuration of the logical channel
 *
 * A subclass can return its own representation of a logical channel by
 * implementing a proper subclass of NrMacSchedulerLC and returning a
 * pointer to a newly created instance.
 *
 * \return a pointer to the representation of a logical channel
 */
LCPtr
NrMacSchedulerNs3::CreateLC (const LogicalChannelConfigListElement_s &config) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<NrMacSchedulerLC> (new NrMacSchedulerLC (config));
}

/**
 * \brief Create a logical channel group starting from a configuration
 * \param config configuration of the logical channel group
 *
 * A subclass can return its own representation of a logical channel by
 * implementing a proper subclass of NrMacSchedulerLCG and returning a
 * pointer to a newly created instance.
 *
 * \return a pointer to the representation of a logical channel group
 */
LCGPtr
NrMacSchedulerNs3::CreateLCG (const LogicalChannelConfigListElement_s &config) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<NrMacSchedulerLCG> (new NrMacSchedulerLCG (config.m_logicalChannelGroup));
}

/**
 * \brief Configure a logical channel for a UE
 * \param params the params of the LC
 *
 * The UE should be previously registered in the UE map. Then, for each logical
 * channel to configure, the UE representation is updated, creating an empty
 * LC. When the direction is set to DIR_BOTH, both UL and DL are created.
 *
 * Each LC is assigned to a LC group (LCG). If the group does not exists, then
 * it is created through the method CreateLCG, and then saved in the UE representation.
 * If the LCG exists or has been created, then the LC creation is done
 * through the method CreateLC and then saved in the UE representation.
 */
void
NrMacSchedulerNs3::DoCschedLcConfigReq (const NrMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (params.m_rnti));
  auto itUe = m_ueMap.find (params.m_rnti);
  GetSecond UeInfoOf;
  NS_ABORT_IF (itUe == m_ueMap.end ());

  for (const auto & lcConfig : params.m_logicalChannelConfigList)
    {
      if (lcConfig.m_direction == LogicalChannelConfigListElement_s::DIR_DL
          || lcConfig.m_direction == LogicalChannelConfigListElement_s::DIR_BOTH)
        {
          auto itDl = UeInfoOf (*itUe)->m_dlLCG.find (lcConfig.m_logicalChannelGroup);
          auto itDlEnd = UeInfoOf (*itUe)->m_dlLCG.end ();
          if (itDl == itDlEnd)
            {
              NS_LOG_DEBUG ("Created DL LCG for UE " << UeInfoOf (*itUe)->m_rnti <<
                            " ID=" << static_cast<uint32_t> (lcConfig.m_logicalChannelGroup));
              std::unique_ptr<NrMacSchedulerLCG> lcg = CreateLCG (lcConfig);
              itDl = UeInfoOf (*itUe)->m_dlLCG.emplace (std::make_pair (lcConfig.m_logicalChannelGroup, std::move (lcg))).first;
            }

          itDl->second->Insert (CreateLC (lcConfig));
          NS_LOG_DEBUG ("Created DL LC for UE " << UeInfoOf (*itUe)->m_rnti <<
                        " ID=" << static_cast<uint32_t> (lcConfig.m_logicalChannelIdentity) <<
                        " in LCG " << static_cast<uint32_t> (lcConfig.m_logicalChannelGroup));
        }
      if (lcConfig.m_direction == LogicalChannelConfigListElement_s::DIR_UL
          || lcConfig.m_direction == LogicalChannelConfigListElement_s::DIR_BOTH)
        {
          auto itUl = UeInfoOf (*itUe)->m_ulLCG.find (lcConfig.m_logicalChannelGroup);
          auto itUlEnd = UeInfoOf (*itUe)->m_ulLCG.end ();
          if (itUl == itUlEnd)
            {
              NS_LOG_DEBUG ("Created UL LCG for UE " << UeInfoOf (*itUe)->m_rnti <<
                            " ID=" << static_cast<uint32_t> (lcConfig.m_logicalChannelGroup));
              std::unique_ptr<NrMacSchedulerLCG> lcg = CreateLCG (lcConfig);
              itUl = UeInfoOf (*itUe)->m_ulLCG.emplace (std::make_pair (lcConfig.m_logicalChannelGroup,
                                                                        std::move (lcg))).first;
            }

          // Create a LC ID only if it is the first. For detail, see documentation
          // of NrMacSchedulerLCG.
          if (itUl->second->NumOfLC () == 0)
            {
              itUl->second->Insert (CreateLC (lcConfig));
              NS_LOG_DEBUG ("Created UL LC for UE " << UeInfoOf (*itUe)->m_rnti <<
                            " ID=" << static_cast<uint32_t> (lcConfig.m_logicalChannelIdentity) <<
                            " in LCG " << static_cast<uint32_t> (lcConfig.m_logicalChannelGroup));
            }
        }
    }
}

/**
 * \brief Release a LC
 * \param params params of the LC to release.
 *
 * Not implemented.
 */
void
NrMacSchedulerNs3::DoCschedLcReleaseReq (const NrMacCschedSapProvider::CschedLcReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  for ([[maybe_unused]] const auto & lcId : params.m_logicalChannelIdentity)
    {
      auto itUe = m_ueMap.find (params.m_rnti);
      NS_ABORT_IF (itUe == m_ueMap.end ());

      // TODO !!!!
      //UeInfoOf(itUe)->ReleaseLC (lcId);
    }
}

/**
 * \brief RLC informs of DL data
 * \param params parameters of the function
 *
 * The message contains the LC and the amount of data buffered. Therefore,
 * in this method we cycle through all the UE LCG to find the LC, and once
 * it is found, it is updated with the new amount of data.
 */
void
NrMacSchedulerNs3::DoSchedDlRlcBufferReq (const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this << params.m_rnti <<
                   static_cast<uint32_t> (params.m_logicalChannelIdentity));

  GetSecond UeInfoOf;
  auto itUe = m_ueMap.find (params.m_rnti);
  NS_ABORT_IF (itUe == m_ueMap.end ());

  for (const auto &lcg : UeInfoOf (*itUe)->m_dlLCG)
    {
      if (lcg.second->Contains (params.m_logicalChannelIdentity))
        {
          NS_LOG_INFO ("Updating DL LC Info: " << params <<
                       " in LCG: " << static_cast<uint32_t> (lcg.first));
          lcg.second->UpdateInfo (params);
          return;
        }
    }
  // Fail miserabily because we didn't found any LC
  NS_FATAL_ERROR ("The LC does not exist. Can't update");
}

/**
 * \brief Update the UL LC
 * \param bsr BSR received
 *
 * The UE notifies the buffer size as a sum of all the components. The BSR
 * is a vector of 4 uint8_t that represents the amount of data in each
 * LCG. A call to NrMacSchedulerLCG::UpdateInfo is then issued with
 * the amount of data as parameter.
 */
void
NrMacSchedulerNs3::BSRReceivedFromUe (const MacCeElement &bsr)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (bsr.m_macCeType == MacCeElement::BSR);
  GetSecond UeInfoOf;
  auto itUe = m_ueMap.find (bsr.m_rnti);
  NS_ABORT_IF (itUe == m_ueMap.end ());

  // The UE only notifies the buf size as sum of all components.
  // see nr-ue-mac.cc:395
  for (uint8_t lcg = 0; lcg < 4; ++lcg)
    {
      uint8_t bsrId = bsr.m_macCeValue.m_bufferStatus.at (lcg);
      uint32_t bufSize = NrMacShortBsrCe::FromLevelToBytes (bsrId);

      auto itLcg = UeInfoOf (*itUe)->m_ulLCG.find (lcg);
      if (itLcg == UeInfoOf (*itUe)->m_ulLCG.end ())
        {
          NS_ABORT_MSG_IF (bufSize > 0, "LCG " << static_cast<uint32_t> (lcg) <<
                           " not found for UE " << itUe->second->m_rnti);
          continue;
        }

      if (itLcg->second->GetTotalSize () > 0 || bufSize > 0)
        {
          NS_LOG_INFO ("Updating UL LCG " << static_cast<uint32_t> (lcg) <<
                       " for UE " << bsr.m_rnti << " size " << bufSize);
        }

      itLcg->second->UpdateInfo (bufSize);
    }
}

/**
 * \brief Evaluate different types of control messages (only BSR for the moment)
 * \param params parameters of the control message
 *
 * For each BSR received, calls BSRReceivedFromUe. Ignore all the others control
 * messages.
 */
void
NrMacSchedulerNs3::DoSchedUlMacCtrlInfoReq (const NrMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  for (const auto & element : params.m_macCeList)
    {
      if ( element.m_macCeType == MacCeElement::BSR )
        {
          BSRReceivedFromUe (element);
        }
      else
        {
          NS_LOG_INFO ("Ignoring received CTRL message because it's not a BSR");
        }
    }
}

/**
 * \brief Received a DL CQI message
 * \param params DL CQI message
 *
 * For each message in the list, calculate the expiration time in number of slots,
 * and then pass all the information to the NrMacSchedulerCQIManagement class.
 *
 * If the CQI is sub-band, the method NrMacSchedulerCQIManagement::SBCQIReported
 * will be called, otherwise NrMacSchedulerCQIManagement::WBCQIReported.
 */
void
NrMacSchedulerNs3::DoSchedDlCqiInfoReq (const NrMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  if (m_fixedMcsDl)
    {
      return;
    }

  NS_ASSERT (m_cqiTimersThreshold >= m_macSchedSapUser->GetSlotPeriod ());

  uint32_t expirationTime = static_cast<uint32_t> (m_cqiTimersThreshold.GetNanoSeconds () /
                                                   m_macSchedSapUser->GetSlotPeriod ().GetNanoSeconds ());

  for (const auto &cqi : params.m_cqiList)
    {
      NS_ASSERT (m_ueMap.find (cqi.m_rnti) != m_ueMap.end ());
      const std::shared_ptr<NrMacSchedulerUeInfo> & ue = m_ueMap.find (cqi.m_rnti)->second;

      if (cqi.m_cqiType == DlCqiInfo::WB)
        {
          m_cqiManagement.DlWBCQIReported (cqi, ue, expirationTime, m_maxDlMcs);
        }
      else
        {
          m_cqiManagement.DlSBCQIReported (cqi, ue);
        }
    }
}

/**
 * \brief Received a UL CQI message
 * \param params UL CQI message
 *
 * Calculate the expiration time in number of slots, and then pass all the
 * information to the NrMacSchedulerCQIManagement class.
 *
 * In UL, we have to know the previously allocated symbols and the total TBS
 * to be able to calculate CQI and MCS, so a special stack is maintained
 * (m_ulAllocationMap).
 *
 * Only UlCqiInfo::PUSCH is currently supported.
 */
void
NrMacSchedulerNs3::DoSchedUlCqiInfoReq (const NrMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  if (m_fixedMcsUl)
    {
      return;
    }

  GetSecond UeInfoOf;

  uint32_t expirationTime = static_cast<uint32_t> (m_cqiTimersThreshold.GetNanoSeconds () /
                                                   m_macSchedSapUser->GetSlotPeriod ().GetNanoSeconds ());

  switch (params.m_ulCqi.m_type)
    {
    case UlCqiInfo::PUSCH:
      {
        [[maybe_unused]] bool found = false;
        uint8_t symStart = params.m_symStart;
        SfnSf ulSfnSf = params.m_sfnSf;

        NS_LOG_INFO ("CQI for allocation: " << params.m_sfnSf << " started at sym: " <<
                     +symStart <<
                     " modified allocation " << ulSfnSf <<
                     " sym Start " << static_cast<uint32_t> (symStart));

        auto itAlloc = m_ulAllocationMap.find (ulSfnSf.GetEncoding ());
        NS_ASSERT_MSG (itAlloc != m_ulAllocationMap.end (),
                       "Can't find allocation for " << ulSfnSf);
        std::vector<AllocElem> & ulAllocations = itAlloc->second.m_ulAllocations;

        for (auto it = ulAllocations.cbegin (); it != ulAllocations.cend (); /* NO INC */)
          {
            const AllocElem & allocation = *(it);
            if (allocation.m_symStart == symStart)
              {
                auto itUe = m_ueMap.find (allocation.m_rnti);
                NS_ASSERT (itUe != m_ueMap.end ());
                NS_ASSERT (allocation.m_numSym > 0);
                NS_ASSERT (allocation.m_tbs > 0);

                m_cqiManagement.UlSBCQIReported (expirationTime, allocation.m_tbs,
                                                 params, UeInfoOf (*itUe),
                                                 allocation.m_rbgMask,
                                                 m_macSchedSapUser->GetNumRbPerRbg (),
                                                 m_macSchedSapUser->GetSpectrumModel ());
                found = true;
                it = ulAllocations.erase (it);
              }
            else
              {
                ++it;
              }
          }
        NS_ASSERT (found);

        if (ulAllocations.size () == 0)
          {
            // remove obsolete info on allocation; we already processed all the CQI
            NS_LOG_INFO ("Removing allocation for " << ulSfnSf);
            m_ulAllocationMap.erase (itAlloc);
          }
      }
      break;
    default:
      NS_FATAL_ERROR ("Unknown type of UL-CQI");
    }
}

/**
 * \brief Merge newly received HARQ feedbacks with existing feedbacks
 * \param existingFeedbacks a vector of old feedback (will be empty at the end)
 * \param inFeedbacks Received feedbacks
 * \param mode UL or DL, for debug printing
 * \return a vector of all the feedbacks (new + old)
 *
 * It is possible that, in one slot, some HARQ could not be transmitted (by
 * choice, or because there are not available resources). These feedbacks are
 * the 'old' ones, that should be merged with the newly arrived (the feedbacks
 * that arrived in the current slot) before processing.
 *
 */
template <typename T>
std::vector<T> NrMacSchedulerNs3::MergeHARQ (std::vector<T> *existingFeedbacks,
                                                 const std::vector<T> &inFeedbacks,
                                                 const std::string &mode) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("To retransmit : " << existingFeedbacks->size () << " " << mode <<
               " HARQ, received " << inFeedbacks.size () << " " << mode <<
               " HARQ Feedback");
  uint64_t existingSize = existingFeedbacks->size ();
  uint64_t inSize = inFeedbacks.size ();
  existingFeedbacks->insert (existingFeedbacks->end (),
                             inFeedbacks.begin (),
                             inFeedbacks.end ());
  NS_ASSERT (existingFeedbacks->size () == existingSize + inSize);

  auto ret = std::vector<T> (std::make_move_iterator (existingFeedbacks->begin ()),
                             std::make_move_iterator (existingFeedbacks->end ()));
  existingFeedbacks->clear ();

  return ret;
}

/**
 * \brief Process HARQ feedbacks
 * \param harqInfo all the known HARQ feedbacks (can be UL or DL)
 * \param GetHarqVectorFn Function to retrieve the correct Harq Vector
 * \param direction "UL" or "DL" for debug messages
 *
 * For every received feedback (even the already processed ones) the method
 * checks if the feedback is ACK or NACK. In case of ACK (represented by
 * HarqInfo::IsReceivedOk) the feedback is eliminated and the corresponding
 * HARQ process erased; if the feedback is NACK, the corresponding process
 * is marked for retransmission. The decision to retransmit or not the process
 * will be taken later.
 *
 * \see DlHarqInfo
 * \see UlHarqInfo
 * \see HarqProcess
 */
template<typename T>
void
NrMacSchedulerNs3::ProcessHARQFeedbacks (std::vector<T> *harqInfo,
                                             const NrMacSchedulerUeInfo::GetHarqVectorFn &GetHarqVectorFn,
                                             const std::string &direction) const
{
  NS_LOG_FUNCTION (this);
  uint32_t nackReceived = 0;

  // Check the HARQ feedback, erase ACKed, updated NACKed
  for (auto harqFeedbackIt = harqInfo->begin (); harqFeedbackIt != harqInfo->end (); /* nothing as increment */)
    {
      uint8_t harqId = harqFeedbackIt->m_harqProcessId;
      uint16_t rnti = harqFeedbackIt->m_rnti;
      NrMacHarqVector & ueHarqVector = GetHarqVectorFn (m_ueMap.find (rnti)->second);
      HarqProcess & ueProcess = ueHarqVector.Get (harqId);

      NS_LOG_INFO ("Evaluating feedback: " << *harqFeedbackIt);
      if (ueProcess.m_active == false)
        {
          NS_LOG_INFO ("UE " << rnti << " HARQ vector: " << ueHarqVector);
          NS_FATAL_ERROR ("Received feedback for a process which is not active");
        }
      NS_ABORT_IF (ueProcess.m_dciElement == nullptr);

      std::vector<uint8_t>::const_iterator rvIt;
      rvIt = std::max_element (ueProcess.m_dciElement->m_rv.begin(), ueProcess.m_dciElement->m_rv.end());
      //RV number should not be greater than 3. An unscheduled stream should
      //be assigned RV = 0 in MIMO.
      NS_ASSERT (*rvIt < 4);
      uint8_t maxHarqReTx = m_enableHarqReTx == true ? 3 : 0;

      if (harqFeedbackIt->IsReceivedOk () || *rvIt == maxHarqReTx)
        {
          ueHarqVector.Erase (harqId);
          harqFeedbackIt = harqInfo->erase (harqFeedbackIt);
          NS_LOG_INFO ("Erased processID " << static_cast<uint32_t> (harqId) <<
                       " of UE " << rnti << " direction " << direction);
        }
      else if (!harqFeedbackIt->IsReceivedOk ())
        {
          ueProcess.m_status = HarqProcess::RECEIVED_FEEDBACK;
          ueProcess.nackStreamIndexes = harqFeedbackIt->GetNackStreamIndexes ();
          nackReceived++;
          ++harqFeedbackIt;
          NS_LOG_INFO ("NACK received for UE " << static_cast<uint32_t> (rnti) <<
                       " process " << static_cast<uint32_t> (harqId) <<
                       " direction " << direction);
        }
    }

  NS_ASSERT (harqInfo->size () == nackReceived);
}

/**
 * \brief Reset expired HARQ
 * \param rnti RNTI of the user
 * \param harq HARQ process list
 *
 * For each process, check its timer. If it is expired, reset the
 * process.
 *
 * \see NrMacHarqVector
 * \see HarqProcess
 */
void
NrMacSchedulerNs3::ResetExpiredHARQ (uint16_t rnti, NrMacHarqVector *harq)
{
  NS_LOG_FUNCTION (this << harq);

  for (auto harqIt = harq->Begin (); harqIt != harq->End (); ++harqIt)
    {
      HarqProcess & process = harqIt->second;
      uint8_t processId = harqIt->first;

      if (process.m_status == HarqProcess::INACTIVE)
        {
          continue;
        }

      if (process.m_timer <m_macSchedSapUser->GetNumHarqProcess())
        {
          ++process.m_timer;
          NS_LOG_INFO ("Updated process for UE " << rnti << " number " <<
                       static_cast<uint32_t> (processId) <<
                       ", resulting process: " << process);
        }
      else
        {
          harq->Erase (processId);
          NS_LOG_INFO ("Erased process for UE " << rnti << " number " <<
                       static_cast<uint32_t> (processId) << " for time limits");
        }
    }
}

/**
 * \brief Prepend a CTRL symbol to the allocation list
 * \param symStart starting symbol
 * \param numSymToAllocate number of symbols to allocate (each CTRL take 1 symbol)
 * \param mode Mode of the allocation (UL, DL)
 * \param allocations list of allocations to which prepend the CTRL symbol
 * \return the symbol that can be used to append other things into the allocation list
 */
uint8_t
NrMacSchedulerNs3::PrependCtrlSym (uint8_t symStart, uint8_t numSymToAllocate,
                                       DciInfoElementTdma::DciFormat mode,
                                       std::deque<VarTtiAllocInfo> *allocations) const
{
  std::vector<uint8_t> rbgBitmask (GetBandwidthInRbg (), 1);

  NS_ASSERT_MSG (rbgBitmask.size () == GetBandwidthInRbg (),
                 "bitmask size " << rbgBitmask.size () << " conf " <<
                 GetBandwidthInRbg ());
  if (mode == DciInfoElementTdma::DL)
    {
      NS_ASSERT (allocations->size () == 0); // no previous allocations
      NS_ASSERT (symStart == 0); // start from the symbol 0
    }

  for (uint8_t sym = symStart; sym < symStart + numSymToAllocate; ++sym)
    {
      allocations->emplace_front (VarTtiAllocInfo (std::make_shared<DciInfoElementTdma> (sym, 1, mode, DciInfoElementTdma::CTRL, rbgBitmask)));
      NS_LOG_INFO ("Allocating CTRL symbol, type" << mode <<
                   " in TDMA. numSym=1, symStart=" <<
                   static_cast<uint32_t> (sym) <<
                   " Remaining CTRL sym to allocate: " << sym - symStart);
    }
  return symStart + numSymToAllocate;
}

/**
 * \brief Append a CTRL symbol to the allocation list
 * \param symStart starting symbol
 * \param numSymToAllocate number of symbols to allocate (each CTRL take 1 symbol)
 * \param mode Mode of the allocation (UL, DL)
 * \param allocations list of allocations to which append the CTRL symbol
 * \return the VarTtiAllocInfo ID that can be used to append other things into the allocation list
 */
uint8_t
NrMacSchedulerNs3::AppendCtrlSym (uint8_t symStart, uint8_t numSymToAllocate,
                                      DciInfoElementTdma::DciFormat mode,
                                      std::deque<VarTtiAllocInfo> *allocations) const
{
  std::vector<uint8_t> rbgBitmask (GetBandwidthInRbg (), 1);

  NS_ASSERT (rbgBitmask.size () == GetBandwidthInRbg ());
  if (mode == DciInfoElementTdma::DL)
    {
      NS_ASSERT (allocations->size () == 0); // no previous allocations
      NS_ASSERT (symStart == 0); // start from the symbol 0
    }

  for (uint8_t sym = symStart; sym < symStart + numSymToAllocate; ++sym)
    {
      allocations->emplace_back (VarTtiAllocInfo (std::make_shared<DciInfoElementTdma> (sym, 1, mode, DciInfoElementTdma::CTRL, rbgBitmask)));
      NS_LOG_INFO ("Allocating CTRL symbol, type" << mode <<
                   " in TDMA. numSym=1, symStart=" <<
                   static_cast<uint32_t> (sym) <<
                   " Remaining CTRL sym to allocate: " << sym - symStart);
    }
  return symStart + numSymToAllocate;
}

/**
 * \brief Compute the number of active DL HARQ to perform
 *
 * \param activeDlHarq list of DL HARQ to perform (should be empty at the beginning)
 * \param dlHarqFeedback list of DL HARQ feedback received
 *
 * After calculating the active HARQ, they should be sorted. It is done by
 * subclasses in the method SortDlHarq.
 * \see SortDlHarq
 */
void
NrMacSchedulerNs3::ComputeActiveHarq (ActiveHarqMap *activeDlHarq,
                                          const std::vector<DlHarqInfo> &dlHarqFeedback) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (activeDlHarq->size () == 0);

  for (const auto &feedback : dlHarqFeedback)
    {
      uint16_t rnti = feedback.m_rnti;
      auto &schedInfo = m_ueMap.find (rnti)->second;
      auto beamIterator = activeDlHarq->find (schedInfo->m_beamConfId);

      if (beamIterator == activeDlHarq->end ())
        {
          std::vector<NrMacHarqVector::iterator> harqVector;
          NS_ASSERT (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId)->second.m_active);

          harqVector.emplace_back (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId));
          activeDlHarq->emplace (std::make_pair (schedInfo->m_beamConfId, harqVector));
        }
      else
        {
          NS_ASSERT (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId)->second.m_active);
          beamIterator->second.emplace_back (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId));
        }
      NS_LOG_INFO ("Received feedback for UE " << rnti << " ID " <<
                   static_cast<uint32_t>(feedback.m_harqProcessId) <<
                   " marked as active");
      NS_ASSERT (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId)->second.m_status == HarqProcess::RECEIVED_FEEDBACK);
    }

  SortDlHarq (activeDlHarq);
}

/**
 * \brief Compute the number of activeUL HARQ to perform
 *
 * \param activeDlHarq list of UL HARQ to perform
 * \param ulHarqFeedback list of UL HARQ feedback
 *
 * After calculating the active HARQ, they should be sorted. It is done by
 * subclasses in the method SortUlHarq.
 * \see SortUlHarq
 */
void
NrMacSchedulerNs3::ComputeActiveHarq (ActiveHarqMap *activeUlHarq,
                                          const std::vector<UlHarqInfo> &ulHarqFeedback) const
{
  NS_LOG_FUNCTION (this);

  for (const auto &feedback : ulHarqFeedback)
    {
      uint16_t rnti = feedback.m_rnti;
      auto &schedInfo = m_ueMap.find (rnti)->second;
      auto beamIterator = activeUlHarq->find (schedInfo->m_beamConfId);

      if (beamIterator == activeUlHarq->end ())
        {
          std::vector<NrMacHarqVector::iterator> harqVector;
          NS_ASSERT (schedInfo->m_ulHarq.Find (feedback.m_harqProcessId)->second.m_active);
          harqVector.emplace_back (schedInfo->m_ulHarq.Find (feedback.m_harqProcessId));
          activeUlHarq->emplace (std::make_pair (schedInfo->m_beamConfId, harqVector));
        }
      else
        {
          NS_ASSERT (schedInfo->m_ulHarq.Find (feedback.m_harqProcessId)->second.m_active);
          beamIterator->second.emplace_back (schedInfo->m_ulHarq.Find (feedback.m_harqProcessId));
        }
    }
  SortUlHarq (activeUlHarq);
}

/**
 * \brief Compute the number of active DL and UL UE
 * \param activeDlUe map of active DL UE to be filled
 * \param GetLCGFn Function to retrieve the LCG of a UE
 * \param mode UL or DL (to be printed in debug messages)
 *
 * The function loops all available UEs and checks their LC. If one (or more)
 * LC contains bytes, they are marked active and inserted in one of the
 * list passed as input parameters. Every UE is marked as active if it has
 * data to transmit; it is a duty for someone else to not assign two DCI for
 * the same RNTI.
 */
void
NrMacSchedulerNs3::ComputeActiveUe (ActiveUeMap *activeUe,
                                        const NrMacSchedulerUeInfo::GetLCGFn &GetLCGFn,
                                        const NrMacSchedulerUeInfo::GetHarqVectorFn &GetHarqVector,
                                        const std::string &mode) const
{
  NS_LOG_FUNCTION (this);
  for (const auto &ueInfo : m_ueMap)
    {
      uint32_t totBuffer = 0;
      const auto & ue = ueInfo.second;

      // compute total DL and UL bytes buffered
      for (const auto & lcgInfo : GetLCGFn (ue))
        {
          const auto & lcg = lcgInfo.second;
          if (lcg->GetTotalSize () > 0)
            {
              NS_LOG_INFO ("UE " << ue->m_rnti << " " << mode << " LCG " <<
                           static_cast<uint32_t> (lcgInfo.first) <<
                           " bytes " << lcg->GetTotalSize ());
            }
          totBuffer += lcg->GetTotalSize ();
        }

      auto harqV = GetHarqVector (ue);

      if (totBuffer > 0 && harqV.CanInsert ())
        {
          auto it = activeUe->find (ue->m_beamConfId);
          if (it == activeUe->end ())
            {
              std::vector<std::pair<std::shared_ptr<NrMacSchedulerUeInfo>, uint32_t> > tmp;
              tmp.emplace_back (ue, totBuffer);
              activeUe->insert (std::make_pair (ue->m_beamConfId, tmp));
            }
          else
            {
              it->second.emplace_back (ue, totBuffer);
            }
        }
    }
}

/**
 * \brief Method to decide how to distribute the assigned bytes to the different LCs
 * \param ueLCG LCG of an UE
 * \param tbs TBS to divide between the LCG/LC
 * \return A vector of Assignation
 *
 * The method distribute bytes evenly between LCG. This is a default;
 * more advanced methods can be inserted. Please note that the correct way
 * is to move this implementation in a class, and then implementing
 * a different method in a different class. Then, the selection between
 * the implementation should be done with an Attribute.
 *
 * Please don't try to insert if/switch statements here, NOR to make it virtual
 * and to change in the subclasses.
 */
// Assume LC are unique
std::vector<NrMacSchedulerNs3::Assignation>
NrMacSchedulerNs3::AssignBytesToLC (const std::unordered_map<uint8_t, LCGPtr> &ueLCG,
                                        uint32_t tbs) const
{
  NS_LOG_FUNCTION (this);
  GetFirst GetLCGID;
  GetSecond GetLCG;

  std::vector<Assignation> ret;

  NS_LOG_INFO ("To distribute: " << tbs << " bytes over " << ueLCG.size () << " LCG");

  uint32_t activeLc = 0;
  for (const auto & lcg : ueLCG)
    {
      std::vector<uint8_t> lcs = GetLCG (lcg)->GetLCId ();
      for (const auto & lcId : lcs)
        {
          if (GetLCG (lcg)->GetTotalSizeOfLC (lcId) > 0)
            {
              ++activeLc;
            }
        }
    }

  if (activeLc == 0)
    {
      return ret;
    }

  uint32_t amountPerLC = tbs / activeLc;
  NS_LOG_INFO ("Total LC: " << activeLc << " each one will receive " << amountPerLC << " bytes");

  for (const auto & lcg : ueLCG)
    {
      std::vector<uint8_t> lcs = GetLCG (lcg)->GetLCId ();
      for (const auto & lcId : lcs)
        {
          if (GetLCG (lcg)->GetTotalSizeOfLC (lcId) > 0)
            {
              NS_LOG_INFO ("Assigned to LCID " << static_cast<uint32_t> (lcId) <<
                           " inside LCG " << static_cast<uint32_t> (GetLCGID (lcg)) <<
                           " an amount of " << amountPerLC << " B");
              ret.emplace_back (Assignation (GetLCGID (lcg), lcId, amountPerLC));
            }
        }
    }

  return ret;
}


/**
 * \brief Scheduling new DL data
 * \param spoint Starting point of the blocks to add to the allocation list
 * \param symAvail Number of available symbols
 * \param activeDl List of active UE with data to transmit in DL
 * \param slotAlloc The allocation info to which append the allocations
 * \return The number of symbols used in the allocation
 *
 * The method is doing the scheduling of new data in the DL direction, delegating
 * three phases to subclasses:
 *
 * - How distribute the symbols between beams?
 * - How many RBG should be assigned to the each active UE?
 * - How to place the blocks in the 2D plan (in other words, how to create the DCIs)?
 *
 * The first two phases are managed by the function AssignDLRBG. Once the map
 * between the beamConfId and the symbols assigned to it has been returned, the
 * active user list has been updated by assigning to each user an amount of
 * RBG. Then, it is necessary to iterate through the beams, and for each beam,
 * iterating through the users of that beam, creating a DCI (function CreateDlDci).
 * Creating the DCI is the third phase in the previous list, because the DCI
 * specifies where the imaginary block containing data is placed.
 *
 * After the DCI has been created, it is necessary to prepare the HarqProcess,
 * store it in the UE pointer, and distribute the TBS among the active LC of the
 * UE, through the method AssignBytesToLC, and creating the corresponding list
 * of RlcPduInfo.
 *
 * Before looping and changing the beam, the starting point should be advanced.
 * How that is done is a matter for the subclasses (method ChangeDlBeam).
 */
uint8_t
NrMacSchedulerNs3::DoScheduleDlData (PointInFTPlane *spoint, uint32_t symAvail,
                                         const ActiveUeMap &activeDl,
                                         SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this << symAvail);
  NS_ASSERT (spoint->m_rbg == 0);
  BeamSymbolMap symPerBeam = AssignDLRBG (symAvail, activeDl);
  GetFirst GetBeam;
  uint8_t usedSym = 0;

  for (const auto &beam : activeDl)
    {
      uint32_t availableRBG = (GetBandwidthInRbg () - spoint->m_rbg) * symPerBeam.at (GetBeam (beam));
      bool assigned = false;
      std::unordered_set <uint8_t> symbStartDci;
      //allocSym is used to count the number of allocated symbols to the UEs of the beam
      //we are iterating over
      uint32_t allocSym = 0;

      NS_LOG_DEBUG (activeDl.size () << " active DL beam, this beam has " <<
                    symPerBeam.at (GetBeam (beam)) << " SYM, starts from RB " << static_cast<uint32_t> (spoint->m_rbg) <<
                    " and symbol " << static_cast<uint32_t> (spoint->m_sym) << " for a total of " <<
                    availableRBG << " RBG. In one symbol we have " << GetBandwidthInRbg () <<
                    " RBG.");

      if (symPerBeam.at (GetBeam (beam)) == 0)
        {
          NS_LOG_INFO ("No available symbols for this beam, continue");
          continue;
        }

      for (const auto &ue : beam.second)
        {
          if (ue.first->m_dlRBG == 0)
            {
              NS_LOG_INFO ("UE " << ue.first->m_rnti << " does not have RBG assigned");
              continue;
            }

          std::shared_ptr<DciInfoElementTdma> dci = CreateDlDci (spoint, ue.first,
                                                                 symPerBeam.at (GetBeam (beam)));
          if (dci == nullptr)
            {
              //By continuing to the next UE means that we are
              //wasting a resource assign to this UE. For a TDMA
              //scheduler this resource would be one or more
              //symbols, and for OFDMA scheduler it would be a
              //chunk of time + freq, i.e., one or more
              //symbols in time and one ore more RBG in freq.
              //TODO To avoid this, a more accurate solution
              //is needed to assign resources. That is, a solution
              //that would not assign resources to a UE if the assigned resources
              //result a TB size of less than 7 bytes (3 mac header, 2 rlc header, 2 data).
              //Because if this happens CreateDlDci will not create DCI.
              NS_LOG_DEBUG ("No DCI has been created, ignoring");
              ue.first->ResetDlMetric ();
              continue;
            }

          assigned = true;

          if (symbStartDci.insert (dci->m_symStart).second)
            {
              allocSym += dci->m_numSym;
            }

          NS_LOG_INFO ("UE " << ue.first->m_rnti << " has " << ue.first->m_dlRBG <<
                       " RBG assigned");
          NS_ASSERT_MSG (dci->m_symStart + dci->m_numSym <= m_macSchedSapUser->GetSymbolsPerSlot (),
                         "symStart: " << static_cast<uint32_t> (dci->m_symStart) << " symEnd: " <<
                         static_cast<uint32_t> (dci->m_numSym) << " symbols: " <<
                         static_cast<uint32_t> (m_macSchedSapUser->GetSymbolsPerSlot ()));

          HarqProcess harqProcess (true, HarqProcess::WAITING_FEEDBACK, 0, dci);
          uint8_t id;

          if (!ue.first->m_dlHarq.CanInsert ())
            {
              NS_LOG_INFO ("Harq Vector condition for UE " << ue.first->m_rnti <<
                           std::endl << ue.first->m_dlHarq);
              NS_FATAL_ERROR ("UE " << ue.first->m_rnti << " does not have DL HARQ space");
            }

          ue.first->m_dlHarq.Insert (&id, harqProcess);
          ue.first->m_dlHarq.Get (id).m_dciElement->m_harqProcess = id;


          std::vector <std::vector<Assignation> > bytesPerLcPerStream;

          for (const auto &it:dci->m_tbSize)
            {
              //distribute tbsize of each stream among the LCs of the UE
              //distributedBytes size is equal to the number of LCs
              auto distributedBytes = AssignBytesToLC (ue.first->m_dlLCG, it);
              if (bytesPerLcPerStream.size () == 0)
                {
                  bytesPerLcPerStream.resize (distributedBytes.size ());
                }
              for (uint16_t numLc = 0; numLc < distributedBytes.size (); numLc++)
                {
                  bytesPerLcPerStream.at (numLc).emplace_back (Assignation (distributedBytes.at (numLc).m_lcg,
                                                                            distributedBytes.at (numLc).m_lcId,
                                                                            distributedBytes.at (numLc).m_bytes));
                }
            }


    //      auto distributedBytes = AssignBytesToLC (ue.first->m_dlLCG, dci->m_tbSize);

          VarTtiAllocInfo slotInfo (dci);

          NS_LOG_INFO ("Assigned process ID " << static_cast<uint32_t> (dci->m_harqProcess) <<
                       " to UE " << ue.first->m_rnti);
          for (uint32_t stream = 0; stream < dci->m_tbSize.size (); stream++)
            {
              NS_LOG_DEBUG (" UE" << dci->m_rnti << " stream " << stream <<
                            " gets DL symbols " << static_cast<uint32_t> (dci->m_symStart) <<
                            "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym) <<
                            " tbs " << dci->m_tbSize.at (stream) <<
                            " mcs " << static_cast<uint32_t> (dci->m_mcs.at (stream)) <<
                            " harqId " << static_cast<uint32_t> (id) <<
                            " rv " << static_cast<uint32_t> (dci->m_rv.at (stream))
                            );
            }



          for (const auto & bytesPerLc : bytesPerLcPerStream)
            {
              //bytesPerLc is a vector
              std::vector<RlcPduInfo> rlcPdusInfoPerStream;
              for (const auto & bytesPerStream : bytesPerLc)
                {
                  if (bytesPerStream.m_bytes != 0)
                    {
                      NS_ASSERT (bytesPerStream.m_bytes >= 3);
                      uint8_t lcId = bytesPerStream.m_lcId;
                      uint8_t lcgId = bytesPerStream.m_lcg;
                      uint32_t bytes = bytesPerStream.m_bytes - 3; // Consider the subPdu overhead
                      RlcPduInfo newRlcPdu (lcId, bytes);
                      rlcPdusInfoPerStream.push_back (newRlcPdu);
                      ue.first->m_dlLCG.at (lcgId)->AssignedData (lcId, bytes, "DL");

                      NS_LOG_DEBUG ("DL LCG " << static_cast<uint32_t> (lcgId) <<
                                    " LCID " << static_cast<uint32_t> (lcId) <<
                                    " got bytes " << newRlcPdu.m_size);
                    }
                  else
                    {
                      uint8_t lcId = bytesPerStream.m_lcId;
                      RlcPduInfo newRlcPdu (lcId, 0);
                      rlcPdusInfoPerStream.push_back (newRlcPdu);
                    }
                }
              //insert rlcPduInforPerStream of a LC
              slotInfo.m_rlcPduInfo.push_back (rlcPdusInfoPerStream);
              HarqProcess & process = ue.first->m_dlHarq.Get (dci->m_harqProcess);
              process.m_rlcPduInfo.push_back (rlcPdusInfoPerStream);
            }


/*
          for (const auto & byteDistribution : distributedBytes)
            {
              NS_ASSERT (byteDistribution.m_bytes >= 3);
              uint8_t lcId = byteDistribution.m_lcId;
              uint8_t lcgId = byteDistribution.m_lcg;
              uint32_t bytes = byteDistribution.m_bytes - 3; // Consider the subPdu overhead

              RlcPduInfo newRlcPdu (lcId, bytes);
              HarqProcess & process = ue.first->m_dlHarq.Get (dci->m_harqProcess);

              slotInfo.m_rlcPduInfo.push_back (newRlcPdu);
              process.m_rlcPduInfo.push_back (newRlcPdu);

              ue.first->m_dlLCG.at (lcgId)->AssignedData (lcId, bytes);

              NS_LOG_DEBUG ("DL LCG " << static_cast<uint32_t> (lcgId) <<
                            " LCID " << static_cast<uint32_t> (lcId) <<
                            " got bytes " << newRlcPdu.m_size);
            }*/

          NS_ABORT_IF (slotInfo.m_rlcPduInfo.size () == 0);

          slotAlloc->m_varTtiAllocInfo.emplace_back (slotInfo);
        }
      if (assigned)
        {
          ChangeDlBeam (spoint, symPerBeam.at (GetBeam (beam)));
          usedSym += allocSym;
          slotAlloc->m_numSymAlloc += allocSym;
        }
    }

  for (auto & beam : activeDl)
    {
      for (auto & ue : beam.second)
        {
          ue.first->ResetDlSchedInfo ();
        }
    }

  NS_ASSERT (spoint->m_rbg == 0);

  return usedSym;
}

/**
 * \brief Scheduling new UL data
 * \param spoint Starting point of the blocks to add to the allocation list
 * \param symAvail Number of available symbols
 * \param activeDl List of active UE with data to transmit in UL
 * \param slotAlloc The allocation info to which append the allocations
 * \return The number of symbols used in the allocation
 *
 * The method is doing the scheduling of new data in the UL direction. Before
 * doing that, it is necessary to schedule the UEs that requested a SR.
 * Then, to decide how to schedule the data, it delegates three phases to subclasses:
 *
 * - How distribute the symbols between beams?
 * - How many RBG should be assigned to the each active UE?
 * - How to place the blocks in the 2D plan (in other words, how to create the DCIs)?
 *
 * The first two phases are managed by the function AssignULRBG. Once the map
 * between the beamConfId and the symbols assigned to it has been returned, the
 * active user list has been updated by assigning to each user an amount of
 * RBG. Then, it is necessary to iterate through the beams, and for each beam,
 * iterating through the users of that beam, creating a DCI (function CreateUlDci).
 * Creating the DCI is the third phase in the previous list, because the DCI
 * specifies where the imaginary block containing data is placed. The DCI should
 * be created by placing the blocks from the last available symbol (e.g., 13)
 * going backwards. This restriction comes from the need of keeping the order
 * of DCI: DL CTRL, DL Data, UL Data, UL CTRL.
 *
 * After the DCI has been created, it is necessary to prepare the HarqProcess,
 * store it in the UE pointer, and distribute the TBS among the active LC of the
 * UE, through the method AssignBytesToLC, and creating the corresponding list
 * of RlcPduInfo.
 *
 * Before looping and changing the beam, the starting point should be advanced.
 * How that is done is a matter for the subclasses (method ChangeUlBeam).
 */
uint8_t
NrMacSchedulerNs3::DoScheduleUlData (PointInFTPlane *spoint, uint32_t symAvail,
                                         const ActiveUeMap &activeUl, SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (symAvail > 0 && activeUl.size () > 0);
  NS_ASSERT (spoint->m_rbg == 0);

  BeamSymbolMap symPerBeam = AssignULRBG (symAvail, activeUl);
  uint8_t usedSym = 0;
  GetFirst GetBeam;

  for (const auto &beam : activeUl)
    {
      uint32_t availableRBG = (GetBandwidthInRbg () - spoint->m_rbg) * symPerBeam.at (GetBeam (beam));
      bool assigned = false;
      std::unordered_set <uint8_t> symbStartDci;
      //allocSym is used to count the number of allocated symbols to the UEs of the beam
      //we are iterating over
      uint32_t allocSym = 0;

      NS_LOG_DEBUG (activeUl.size () << " active UL beam, this beam has " <<
                    symPerBeam.at (GetBeam (beam)) << " SYM, starts from RBG " <<
                    static_cast<uint32_t> (spoint->m_rbg) <<
                    " and symbol " << static_cast<uint32_t> (spoint->m_sym) <<
                    " (going backward) for a total of " << availableRBG <<
                    " RBG. In one symbol we have " <<
                    GetBandwidthInRbg () << " RBG.");

      if (symPerBeam.at (GetBeam (beam)) == 0)
        {
          NS_LOG_INFO ("No available symbols for this beam, continue");
          continue;
        }

      for (const auto &ue : beam.second)
        {
          if (ue.first->m_ulRBG == 0)
            {
              NS_LOG_INFO ("UE " << ue.first->m_rnti << " does not have RBG assigned");
              continue;
            }

          std::shared_ptr<DciInfoElementTdma> dci = CreateUlDci (spoint, ue.first, symPerBeam.at (GetBeam (beam)));

          if (dci == nullptr)
            {
              //By continuing to the next UE means that we are
              //wasting a resource assign to this UE. For a TDMA
              //scheduler this resource would be one or more
              //symbols, and for OFDMA scheduler it would be a
              //chunk of time + freq, i.e., one or more
              //symbols in time and one ore more RBG in freq.
              //TODO To avoid this, a more accurate solution
              //is needed to assign resources. That is, a solution
              //that would not assign resources to a UE if the assigned resources
              //result a TB size of less than 7 bytes (3 mac header, 2 rlc header, 2 data).
              //Because if this happens CreateUlDci will not create DCI.
              NS_LOG_DEBUG ("No DCI has been created, ignoring");
              ue.first->ResetUlMetric ();
              continue;
            }

          assigned = true;

          if (symbStartDci.insert (dci->m_symStart).second)
            {
              allocSym += dci->m_numSym;
            }

          if (!ue.first->m_ulHarq.CanInsert ())
            {
              NS_LOG_INFO ("Harq Vector condition for UE " << ue.first->m_rnti <<
                           std::endl << ue.first->m_ulHarq);
              NS_FATAL_ERROR ("UE " << ue.first->m_rnti << " does not have UL HARQ space");
            }

          HarqProcess harqProcess (true, HarqProcess::WAITING_FEEDBACK, 0, dci);
          uint8_t id;
          ue.first->m_ulHarq.Insert (&id, harqProcess);

          ue.first->m_ulHarq.Get (id).m_dciElement->m_harqProcess = id;

          VarTtiAllocInfo slotInfo (dci);

          NS_LOG_INFO ("Assigned process ID " << static_cast<uint32_t> (dci->m_harqProcess) <<
                       " to UE " << ue.first->m_rnti);
          for (uint32_t stream = 0; stream < dci->m_tbSize.size (); stream++)
            {
              NS_LOG_DEBUG (" UE" << dci->m_rnti <<
                            " gets UL symbols " << static_cast<uint32_t> (dci->m_symStart) <<
                            "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym) <<
                            " tbs " << dci->m_tbSize.at (stream) <<
                            " mcs " << static_cast<uint32_t> (dci->m_mcs.at (stream)) <<
                            " harqId " << static_cast<uint32_t> (id) <<
                            " rv " << static_cast<uint32_t> (dci->m_rv.at (stream))
                            );
            }


          auto distributedBytes = AssignBytesToLC (ue.first->m_ulLCG, dci->m_tbSize.at (0));
          bool assignedToLC = false;
          for (const auto & byteDistribution : distributedBytes)
            {
              assignedToLC = true;
              ue.first->m_ulLCG.at (byteDistribution.m_lcg)->AssignedData (byteDistribution.m_lcId, byteDistribution.m_bytes, "UL");
              NS_LOG_DEBUG ("UL LCG " << static_cast<uint32_t> (byteDistribution.m_lcg) <<
                            " assigned bytes " << byteDistribution.m_bytes << " to LCID " <<
                            static_cast<uint32_t> (byteDistribution.m_lcId));
            }
          NS_ASSERT (assignedToLC);
          slotAlloc->m_varTtiAllocInfo.emplace_front (slotInfo);
        }
      if (assigned)
        {
          ChangeUlBeam (spoint, symPerBeam.at (GetBeam (beam)));
          usedSym += allocSym;
          slotAlloc->m_numSymAlloc += allocSym;
        }
    }

  for (auto & beam : activeUl)
    {
      for (auto & ue : beam.second)
        {
          ue.first->ResetUlSchedInfo ();
        }
    }

  NS_ASSERT (spoint->m_rbg == 0);

  return usedSym;
}

/**
 * \brief Schedule received SR
 * \param spoint Starting point for allocation
 * \param rntiList list of RNTI which asked for a SR
 *
 * Each time an UE asks for SR, the scheduler will assign a fixed amount of
 * data (12 bytes) to the UE's UL LCG. Then, the routine for scheduling the data
 * will take care to create an assignation for the UE, to be able to send
 * some data and, eventually, a BSR.
 *
 */
void
NrMacSchedulerNs3::DoScheduleUlSr (PointInFTPlane *spoint, const std::list<uint16_t> &rntiList) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (spoint->m_rbg == 0);

  for (const auto & v : rntiList)
    {
      for (auto & ulLcg : NrMacSchedulerUeInfo::GetUlLCG (m_ueMap.at (v)))
        {
          NS_LOG_DEBUG ("Assigning 12 bytes to UE " << v << " because of a SR");
          ulLcg.second->UpdateInfo (12);
        }
    }
}

/**
 * \brief Do the process of scheduling for the DL
 * \param params scheduling parameters
 * \param dlHarqFeedback vector of DL HARQ negative feedback
 *
 * The scheduling process is composed by the UL and the DL parts, and it
 * decides how the resources should be divided between UEs. An important
 * thing to remember is that the slot being considered for DL decision can be
 * different for the slot for UL decision. This offset is due to the parameter
 * N2Delay (previously: UlSchedDelay).
 *
 * Another parameter to consider is the L1L2CtrlLatency that defines the delay (in slots number) between
 * the slot that is currently "in the air" and the slot which is being prepared
 * for DL.
 * The default value for both L1L2CtrlLatency and N2Delay (previously: UlSchedDelay)
 * is 2, so it means that while the slot number (frame, subframe, slot) is in the air,
 * the scheduler in this function will take decisions for DL in slot number
 * (frame, subframe, slot) + 2 and for UL in slot number (frame, subframe, slot) + 4.
 *
 * The consequences are an additional complexity derived from the fact that the
 * DL scheduling for a slot should remember the previous UL scheduling done in the
 * past for that slot.
 *
 * The process of scheduling DL data is defined as follows:
 *
 * - Retrieve the allocation done in the past (UL) for this slot
 * - Prepend DL CTRL symbol to the allocation list;
 * - Compute the list of active HARQ/new data UE
 * - Perform a scheduling for DL HARQ/data for this (DoScheduleDl());
 * - Indicate to the MAC the decision for that slot through SchedConfigInd()
 *
 * To know how the scheduling for DL is performed, take a look to the
 * function documentation for DoScheduleDl().
 *
 * At the end, the return of DoScheduleDl is passed to MAC through the function
 * SchedConfigInd().
 *
 * \see PointInFTPlane
 */
void
NrMacSchedulerNs3::ScheduleDl (const NrMacSchedSapProvider::SchedDlTriggerReqParameters& params,
                                   const std::vector <DlHarqInfo> &dlHarqFeedback)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Scheduling invoked for slot " << params.m_snfSf << " of type " << params.m_slotType);

  NrMacSchedSapUser::SchedConfigIndParameters dlSlot (params.m_snfSf);
  dlSlot.m_slotAllocInfo.m_sfnSf = params.m_snfSf;
  dlSlot.m_slotAllocInfo.m_type = SlotAllocInfo::DL;
  auto ulAllocationIt = m_ulAllocationMap.find (params.m_snfSf.GetEncoding ()); // UL allocations for this slot
  if (ulAllocationIt == m_ulAllocationMap.end ())
    {
      ulAllocationIt = m_ulAllocationMap.insert(std::make_pair (params.m_snfSf.GetEncoding (), SlotElem (0))).first;
    }
  auto & ulAllocations = ulAllocationIt->second;

  // add slot for DL control, at symbol 0
  PrependCtrlSym (0, m_dlCtrlSymbols, DciInfoElementTdma::DL,
                  &dlSlot.m_slotAllocInfo.m_varTtiAllocInfo);
  dlSlot.m_slotAllocInfo.m_numSymAlloc += m_dlCtrlSymbols;

  // In case of S slot, add UL CTRL and update the symbol used count
  if (params.m_slotType == LteNrTddSlotType::S)
    {
      NS_LOG_INFO ("S slot, adding UL CTRL");
      AppendCtrlSym (static_cast<uint8_t> (m_macSchedSapUser->GetSymbolsPerSlot () - 1),
                     m_ulCtrlSymbols, DciInfoElementTdma::UL,
                     &dlSlot.m_slotAllocInfo.m_varTtiAllocInfo);
      ulAllocations.m_totUlSym += m_ulCtrlSymbols;
      dlSlot.m_slotAllocInfo.m_numSymAlloc += m_ulCtrlSymbols;
    }

  // RACH
  for (const auto & rachReq : m_rachList)
    {
      BuildRarListElement_s newRar;
      newRar.m_rnti = rachReq.m_rnti;
      // newRar.m_ulGrant is not used
      dlSlot.m_buildRarList.push_back (newRar);
    }
  m_rachList.clear ();

  // compute active ue in the current subframe, group them by BeamConfId
  ActiveHarqMap activeDlHarq;
  ComputeActiveHarq (&activeDlHarq, dlHarqFeedback);

  ActiveUeMap activeDlUe;
  ComputeActiveUe (&activeDlUe, &NrMacSchedulerUeInfo::GetDlLCG,
                   &NrMacSchedulerUeInfo::GetDlHarqVector, "DL");

  DoScheduleDl (dlHarqFeedback, activeDlHarq, &activeDlUe, params.m_snfSf,
                ulAllocations, &dlSlot.m_slotAllocInfo);

  // if the number of allocated symbols is greater than GetUlCtrlSymbols (), then don't delete
  // the allocation, as it will be removed when the CQI will be processed.
  // Otherwise, delete the allocation history for the slot.
  if (ulAllocations.m_totUlSym <= GetUlCtrlSyms ())
    {
      NS_LOG_INFO ("Removing UL allocation for slot " << params.m_snfSf <<
                   " size " << m_ulAllocationMap.size ());
      m_ulAllocationMap.erase (ulAllocationIt);
    }

  NS_LOG_INFO ("Total DCI for DL : " << dlSlot.m_slotAllocInfo.m_varTtiAllocInfo.size () <<
               " including DL CTRL");
  m_macSchedSapUser->SchedConfigInd (dlSlot);
}

/**
 * \brief Do the process of scheduling for the UL
 * \param params scheduling parameters
 * \param ulHarqFeedback vector of UL HARQ negative feedback
 *
 * The scheduling process is composed by the UL and the DL parts, and it
 * decides how the resources should be divided between UEs. An important
 * thing to remember is that the slot being considered for DL decision can be
 * different for the slot for UL decision. This offset is due to the parameter
 * N2Delay (previously: UlSchedDelay).
 *
 * Another parameter to consider is the L1L2CtrlLatency  that defines the delay (in slots number) between
 * the slot that is currently "in the air" and the slot which is being prepared
 * for DL.
 * The default value for both L1L2CtrlLatency and N2Delay (previously: UlSchedDelay)
 * is 2, so it means that while the slot number (frame, subframe, slot) is in the air,
 * the scheduler in this function will take decisions for DL in slot number
 * (frame, subframe, slot) + 2 and for UL in slot number (frame, subframe, slot) + 4.
 *
 * The consequences are an additional complexity derived from the fact that the
 * DL scheduling for a slot should remember the previous UL scheduling done in the
 * past for that slot.
 *
 * The process of scheduling UL data is defined as follows:
 *
 * - Append a UL CTRL symbol to the allocation list;
 * - Perform a scheduling for UL HARQ/data for this (DoScheduleUl());
 * - Indicate to the MAC the decision for that slot through SchedConfigInd()
 *
 * To know how the scheduling for UL is performed, take a look to the
 * function documentation for DoScheduleUl().
 *
 * At the end, the return of DoScheduleUl is passed to MAC through the function
 * SchedConfigInd().
 *
 * \see PointInFTPlane
 */
void
NrMacSchedulerNs3::ScheduleUl (const NrMacSchedSapProvider::SchedUlTriggerReqParameters& params,
                                   const std::vector <UlHarqInfo> &ulHarqFeedback)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Scheduling invoked for slot " << params.m_snfSf);

  NrMacSchedSapUser::SchedConfigIndParameters ulSlot (params.m_snfSf);
  ulSlot.m_slotAllocInfo.m_sfnSf = params.m_snfSf;
  ulSlot.m_slotAllocInfo.m_type = SlotAllocInfo::UL;

  // add slot for UL control, at last symbol, for slot type F and UL.
  AppendCtrlSym (static_cast<uint8_t> (m_macSchedSapUser->GetSymbolsPerSlot () - 1),
                 m_ulCtrlSymbols, DciInfoElementTdma::UL,
                 &ulSlot.m_slotAllocInfo.m_varTtiAllocInfo);
  ulSlot.m_slotAllocInfo.m_numSymAlloc += m_ulCtrlSymbols;

  // Doing UL for slot ulSlot
  DoScheduleUl (ulHarqFeedback, params.m_snfSf, &ulSlot.m_slotAllocInfo, params.m_slotType);

  NS_LOG_INFO ("Total DCI for UL : " << ulSlot.m_slotAllocInfo.m_varTtiAllocInfo.size () <<
               " including UL CTRL");
  m_macSchedSapUser->SchedConfigInd (ulSlot);
}

/**
 * \brief Schedule UL HARQ and data
 * \param activeUlHarq List of active HARQ processes in UL
 * \param ulSfn Slot number
 * \param allocInfo Allocation info pointer (where to save the allocations)
 * \param type LTE/NR TDD slot type
 * \return the number of symbols used for the UL allocation
 *
 * The UL phase is, in this implementation, based completely on TDMA, so
 * in this method (even if the code is modular enough to be subclassed to
 * create a OFDMA-based scheduler) expects a TDMA-based allocations. You have
 * been warned!!
 *
 * The UL allocation is done for a slot number that is greater or equal to the
 * slot number for DL. Therefore, the maximum symbols available for this phase
 * are all the available data symbols in the slot. This opens up fairness problems
 * with DL, but how to balance UL and DL is still an open problem.
 *
 * If there are HARQ to retransmit, they will be retransmitted in the function
 * ScheduleUlHarq() until the code runs out of available symbols or
 * available HARQ to retransmit. The UE that can be selected for such assignation
 * are decided in the function ComputeActiveHarq.
 *
 * Then, for all the UE that requested a SR, it will be allocated one entire
 * symbol. After that, if any symbol remains, the function will schedule data.
 * The data allocation is made in DoScheduleUlData(), only for UEs that
 * have been selected by the function ComputeActiveUe and the ones that does not
 * have already a grant for HARQ (or SR).
 *
 * If any assignation is made, then the member variable m_ulAllocationMap (SlotElem)
 * is updated by storing the total UL symbols used in this slot, and
 * the details of each assignation, which include TBS, symStart, numSym, and
 * MCS. The assignations are stored as a list of AllocElem.
 *
 * Please note that the allocation must be done backwards from the last available
 * symbol to respect the final slot composition: DL CTRL, DL Data, UL Data, UL CTRL.
 */
uint8_t
NrMacSchedulerNs3::DoScheduleUl (const std::vector <UlHarqInfo> &ulHarqFeedback,
                                     const SfnSf &ulSfn, SlotAllocInfo *allocInfo,
                                     LteNrTddSlotType type)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (allocInfo->m_varTtiAllocInfo.size () == 1); // Just the UL CTRL

  uint8_t dataSymPerSlot = m_macSchedSapUser->GetSymbolsPerSlot () - m_ulCtrlSymbols;
  if (type == LteNrTddSlotType::F)
    { // if it's a type F, we have to consider DL CTRL symbols, otherwise, don't
      dataSymPerSlot -= m_dlCtrlSymbols;
    }

  ActiveHarqMap activeUlHarq;
  ComputeActiveHarq (&activeUlHarq, ulHarqFeedback);

  // Start the assignation from the last available data symbol, and like a shrimp
  // go backward.
  uint8_t lastSym = m_macSchedSapUser->GetSymbolsPerSlot () - m_ulCtrlSymbols;
  PointInFTPlane ulAssignationStartPoint (0, lastSym);
  uint8_t ulSymAvail = dataSymPerSlot;

  // Create the UL allocation map entry
  m_ulAllocationMap.emplace (ulSfn.GetEncoding (), SlotElem (0));

  if ((m_enableSrsInFSlots == true && type == LteNrTddSlotType::F) || (m_enableSrsInUlSlots == true && type == LteNrTddSlotType::UL))
    { // SRS are included in F slots, and in UL slots if m_enableSrsInUlSlots=true
      m_srsSlotCounter++; // It's an uint, don't worry about wrap around
      NS_ASSERT (m_srsCtrlSymbols <= ulSymAvail);
      uint8_t srsSym = DoScheduleSrs (&ulAssignationStartPoint, allocInfo);
      ulSymAvail -= srsSym;
    }

  NS_LOG_DEBUG ("Scheduling UL " << ulSfn <<
                " UL HARQ to retransmit: " << ulHarqFeedback.size () <<
                " Active Beams UL HARQ: " << activeUlHarq.size () <<
                " starting from (" << +ulAssignationStartPoint.m_rbg << ", " <<
                +ulAssignationStartPoint.m_sym << ")");

  if (activeUlHarq.size () > 0)
    {
      uint8_t usedHarq = ScheduleUlHarq (&ulAssignationStartPoint, ulSymAvail,
                                         m_ueMap, &m_ulHarqToRetransmit, ulHarqFeedback,
                                         allocInfo);
      NS_ASSERT_MSG (ulSymAvail >= usedHarq, "Available: " << +ulSymAvail <<
                     " used by HARQ: " << +usedHarq);
      NS_LOG_INFO ("For the slot " << ulSfn << " reserved " <<
                   static_cast<uint32_t> (usedHarq) << " symbols for UL HARQ retx");
      ulSymAvail -= usedHarq;
    }

  NS_ASSERT (ulAssignationStartPoint.m_rbg == 0);

  if (ulSymAvail > 0 && m_srList.size () > 0)
    {
      DoScheduleUlSr (&ulAssignationStartPoint, m_srList);
      m_srList.clear ();
    }

  ActiveUeMap activeUlUe;
  ComputeActiveUe (&activeUlUe, &NrMacSchedulerUeInfo::GetUlLCG,
                   &NrMacSchedulerUeInfo::GetUlHarqVector, "UL");

  GetSecond GetUeInfoList;
  for (const auto & alloc : allocInfo->m_varTtiAllocInfo)
    {
      for (auto it = activeUlUe.begin(); it != activeUlUe.end (); ++it)
        {
          auto & ueInfos = GetUeInfoList(*it);
          for (auto ueIt = ueInfos.begin(); ueIt != ueInfos.end(); /* no incr */)
            {
              GetFirst GetUeInfoPtr;
              if (GetUeInfoPtr (*ueIt)->m_rnti == alloc.m_dci->m_rnti)
                {
                  NS_LOG_INFO ("Removed RNTI " << alloc.m_dci->m_rnti << " from active ue list "
                               "because it has already an HARQ scheduled");
                  ueInfos.erase (ueIt);
                  break;
                }
              else
                {
                  ++ueIt;
                }
            }
        }
    }

  if (ulSymAvail > 0 && activeUlUe.size () > 0)
    {
      uint8_t usedUl = DoScheduleUlData (&ulAssignationStartPoint, ulSymAvail,
                                         activeUlUe, allocInfo);
      NS_LOG_INFO ("For the slot " << ulSfn << " reserved " <<
                   static_cast<uint32_t> (usedUl) << " symbols for UL data tx");
      ulSymAvail -= usedUl;
    }

  std::vector<uint32_t> symToAl;
  symToAl.resize (15, 0);

  auto & totUlSym = m_ulAllocationMap.at (ulSfn.GetEncoding ()).m_totUlSym;
  auto & allocations = m_ulAllocationMap.at (ulSfn.GetEncoding ()).m_ulAllocations;
  for (const auto &alloc : allocInfo->m_varTtiAllocInfo)
    {
      if (alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
          // Here we are assuming (with the assignment) that all the
          // allocations starting at a particular symbol will have the same
          // length.
          symToAl[alloc.m_dci->m_symStart] = alloc.m_dci->m_numSym;
          NS_LOG_INFO ("UL Allocation. RNTI " <<
                       alloc.m_dci->m_rnti << ", symStart " <<
                       static_cast<uint32_t>(alloc.m_dci->m_symStart) <<
                       " numSym " << +alloc.m_dci->m_numSym);

          if (alloc.m_dci->m_type == DciInfoElementTdma::DATA)
            {
              NS_LOG_INFO ("Placed the above allocation in the CQI map");
              allocations.emplace_back (AllocElem (alloc.m_dci->m_rnti,
                                                   alloc.m_dci->m_tbSize.at (0),
                                                   alloc.m_dci->m_symStart,
                                                   alloc.m_dci->m_numSym,
                                                   alloc.m_dci->m_mcs.at (0),
                                                   alloc.m_dci->m_rbgBitmask));
            }
        }
    }

  for (const auto & v : symToAl)
    {
      totUlSym += v;
    }

  NS_ASSERT_MSG ((dataSymPerSlot + m_ulCtrlSymbols) - ulSymAvail == totUlSym,
                 "UL symbols available: " << static_cast<uint32_t> (dataSymPerSlot + m_ulCtrlSymbols) <<
                 " UL symbols available at end of sched: " << static_cast<uint32_t> (ulSymAvail) <<
                 " total of symbols registered in the allocation: " << static_cast<uint32_t> (totUlSym) <<
                 " slot type " << type);

  NS_LOG_INFO ("For the slot " << ulSfn << " registered a total of " <<
               static_cast<uint32_t> (totUlSym) <<
               " symbols and " << allocations.size () << " data allocations, with a total of " <<
               allocInfo->m_varTtiAllocInfo.size ());
  NS_ASSERT (m_ulAllocationMap.at (ulSfn.GetEncoding ()).m_totUlSym == totUlSym);

  return dataSymPerSlot - ulSymAvail;
}

uint8_t
NrMacSchedulerNs3::DoScheduleSrs (PointInFTPlane *spoint, SlotAllocInfo *allocInfo)
{
  NS_LOG_FUNCTION (this);

  uint8_t used = 0;

  // Without UE, don't schedule any SRS
  if (m_ueMap.size () == 0)
    {
      return used;
    }

  // Find the UE for which this is true:
  // absolute_slot_number % periodicity = offset_UEx
  // Assuming that all UEs share the same periodicity.

  uint32_t offset_UEx = m_srsSlotCounter % m_ueMap.begin()->second->m_srsPeriodicity;
  uint16_t rnti = 0;

  for (const auto & ue : m_ueMap)
    {
      if (ue.second->m_srsOffset == offset_UEx)
        {
          rnti = ue.second->m_rnti;
        }
    }

  if (rnti == 0)
    {
      return used; // No SRS in this slot!
    }

  // Schedule 4 allocation, of 1 symbol each, in TDMA mode, for the RNTI found.

  for (uint32_t i = 0; i < m_srsCtrlSymbols; ++i)
    {
      std::vector<uint8_t> rbgAssigned (GetBandwidthInRbg (), 1);

      NS_LOG_INFO ("UE " << rnti << " assigned symbol " << +spoint->m_sym << " for SRS tx");

      std::vector<uint8_t> rbgBitmask (GetBandwidthInRbg (), 1);

      spoint->m_sym--;

      //Due to MIMO implementation MCS, TB size, ndi, rv, are vectors
      std::vector<uint8_t> mcs = {0};
      std::vector<uint32_t> tbs = {0};
      std::vector<uint8_t> ndi = {1};
      std::vector<uint8_t> rv = {0};

      auto dci = std::make_shared<DciInfoElementTdma> (rnti, DciInfoElementTdma::UL,
                                                       spoint->m_sym, 1, mcs, tbs,
                                                       ndi, rv,
                                                       DciInfoElementTdma::SRS,
                                                       GetBwpId(), GetTpc ());
      dci->m_rbgBitmask = rbgBitmask;

      allocInfo->m_numSymAlloc += 1;
      allocInfo->m_varTtiAllocInfo.emplace_front (VarTtiAllocInfo (dci));

      used++;
    }

  return used;
}

uint16_t
NrMacSchedulerNs3::GetBwpId () const
{
  if (m_macSchedSapUser)
    {
      return m_macSchedSapUser->GetBwpId ();
    }
  else
    {
      return UINT16_MAX;
    }

}

uint16_t
NrMacSchedulerNs3::GetCellId () const
{
  if (m_macSchedSapUser)
    {
      return m_macSchedSapUser->GetCellId ();
    }
  else
    {
      return UINT16_MAX;
    }

}

uint16_t
NrMacSchedulerNs3::GetBandwidthInRbg() const
{
  return m_bandwidth;
}

/**
 * \brief Schedule DL HARQ and data
 * \param dlSfnSf Slot number
 * \param ulAllocations Uplink allocation for this slot
 * \param allocInfo Allocation info pointer (where to save the allocations)
 * \return the number of symbols used for the DL allocation
 *
 * The DL phase can be OFDMA-based or TDMA-based. The method calculates the
 * number of available symbols as the total number of symbols in one slot
 * minus the number of symbols already allocated for UL. Then, it defines
 * the data starting point by keeping in consideration the number of symbols
 * previously allocated. In this way, DL and UL allocation will not overlap.
 *
 * HARQ retx processing is done in the function  ScheduleDlHarq(), while
 * DL new data processing in the function ScheduleDlData(). The method is
 * ensuring that if an UE gets a DCI for HARQ, it will not be scheduled for new
 * data as well. We have a limit of 1 DCI per UE.
 *
 */
uint8_t
NrMacSchedulerNs3::DoScheduleDl (const std::vector <DlHarqInfo> &dlHarqFeedback,
                                     const ActiveHarqMap &activeDlHarq,
                                     ActiveUeMap *activeDlUe,
                                     const SfnSf &dlSfnSf,
                                     const SlotElem &ulAllocations,
                                     SlotAllocInfo *allocInfo)
{
  NS_LOG_INFO (this);
  NS_ASSERT (activeDlUe != nullptr);

  uint8_t dataSymPerSlot = m_macSchedSapUser->GetSymbolsPerSlot () - m_dlCtrlSymbols;

  uint8_t dlSymAvail = dataSymPerSlot - ulAllocations.m_totUlSym;
  PointInFTPlane dlAssignationStartPoint (0, m_dlCtrlSymbols);

  NS_LOG_DEBUG ("Scheduling DL for slot " << dlSfnSf <<
                " DL HARQ to retransmit: " << dlHarqFeedback.size () <<
                " Active Beams DL HARQ: " << activeDlHarq.size () <<
                " sym available: " << static_cast<uint32_t> (dlSymAvail) <<
                " starting from sym " << static_cast<uint32_t> (m_dlCtrlSymbols));

  if (activeDlHarq.size () > 0)
    {
      uint8_t usedHarq = ScheduleDlHarq (&dlAssignationStartPoint, dlSymAvail,
                                         activeDlHarq, m_ueMap, &m_dlHarqToRetransmit,
                                         dlHarqFeedback, allocInfo);
      NS_ASSERT (dlSymAvail >= usedHarq);
      dlSymAvail -= usedHarq;
    }

  GetSecond GetUeInfoList;

  for (const auto & alloc : allocInfo->m_varTtiAllocInfo)
    {
      for (auto it = activeDlUe->begin(); it != activeDlUe->end (); ++it)
        {
          auto & ueInfos = GetUeInfoList(*it);
          for (auto ueIt = ueInfos.begin(); ueIt != ueInfos.end(); /* no incr */)
            {
              GetFirst GetUeInfoPtr;
              if (GetUeInfoPtr (*ueIt)->m_rnti == alloc.m_dci->m_rnti)
                {
                  NS_LOG_INFO ("Removed RNTI " << alloc.m_dci->m_rnti << " from active ue list "
                               "because it has already an HARQ scheduled");
                  ueInfos.erase (ueIt);
                  break;
                }
              else
                {
                  ++ueIt;
                }
            }
        }
    }

  NS_ASSERT (dlAssignationStartPoint.m_rbg == 0);

  if (dlSymAvail > 0 && activeDlUe->size () > 0)
    {
      uint8_t usedDl = DoScheduleDlData (&dlAssignationStartPoint, dlSymAvail,
                                         *activeDlUe, allocInfo);
      NS_ASSERT (dlSymAvail >= usedDl);
      dlSymAvail -= usedDl;
    }

  return (dataSymPerSlot - ulAllocations.m_totUlSym) - dlSymAvail;
}

/**
 * \brief Decide how to fill the frequency/time of a DL slot
 * \param params parameters for the scheduler
 *
 * The function starts by refreshing the CQI received, and eventually resetting
 * the expired values. Then, the HARQ feedback are processed (ProcessHARQFeedbacks),
 * and finally the expired HARQs are canceled (ResetExpiredHARQ).
 *
 * \see ScheduleDl
 */
void
NrMacSchedulerNs3::DoSchedDlTriggerReq (const NrMacSchedSapProvider::SchedDlTriggerReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  // process received CQIs
  m_cqiManagement.RefreshDlCqiMaps (m_ueMap);

  // reset expired HARQ
  for (const auto & itUe : m_ueMap)
    {
      ResetExpiredHARQ (itUe.second->m_rnti, &itUe.second->m_dlHarq);
    }

  // Merge not-retransmitted and received feedback
  std::vector <DlHarqInfo> dlHarqFeedback;

  if (params.m_dlHarqInfoList.size () > 0 || m_dlHarqToRetransmit.size () > 0)
    {
      // m_dlHarqToRetransmit will be cleared inside MergeHARQ
      uint64_t existingSize = m_dlHarqToRetransmit.size ();
      uint64_t inSize = params.m_dlHarqInfoList.size ();

      dlHarqFeedback = MergeHARQ (&m_dlHarqToRetransmit, params.m_dlHarqInfoList, "DL");

      NS_ASSERT (m_dlHarqToRetransmit.size () == 0);
      NS_ASSERT_MSG (existingSize + inSize == dlHarqFeedback.size (),
                     " existing: " << existingSize << " received: " << inSize <<
                     " calculated: " << dlHarqFeedback.size ());

      std::unordered_map<uint16_t, std::set<uint32_t>> feedbacksDup;

      // Let's find out:
      // 1) Feedback that arrived late (i.e., their process has been marked inactive
      //    due to timings
      // 2) Duplicated feedbacks (same UE, same process ID). I don't know why
      //    these are generated.. but anyway..
      for (auto it = dlHarqFeedback.begin (); it != dlHarqFeedback.end (); /* no inc */)
        {
          auto & ueInfo = m_ueMap.find (it->m_rnti)->second;
          auto & process = ueInfo->m_dlHarq.Find (it->m_harqProcessId)->second;
          NS_LOG_INFO ("Analyzing feedback for UE " << it->m_rnti << " process " <<
                       static_cast<uint32_t> (it->m_harqProcessId));
          if (!process.m_active)
            {
              NS_LOG_INFO ("Feedback for UE " << it->m_rnti << " process " <<
                           static_cast<uint32_t> (it->m_harqProcessId) <<
                           " ignored because process is INACTIVE");
              it = dlHarqFeedback.erase (it);    /* INC */
            }
          else
            {
              auto itDuplicated = feedbacksDup.find(it->m_rnti);
              if (itDuplicated == feedbacksDup.end())
                {
                  feedbacksDup.insert(std::make_pair (it->m_rnti, std::set<uint32_t> ()));
                  feedbacksDup.at(it->m_rnti).insert(it->m_harqProcessId);
                  ++it; /* INC */
                }
              else
                {
                  if (itDuplicated->second.find(it->m_harqProcessId) == itDuplicated->second.end())
                    {
                      itDuplicated->second.insert(it->m_harqProcessId);
                      ++it; /* INC */
                    }
                  else
                    {
                      NS_LOG_INFO ("Feedback for UE " << it->m_rnti << " process " <<
                                   static_cast<uint32_t> (it->m_harqProcessId) <<
                                   " ignored because is a duplicate of another feedback");
                      it = dlHarqFeedback.erase (it); /* INC */
                    }
                }
            }
        }

      ProcessHARQFeedbacks (&dlHarqFeedback, NrMacSchedulerUeInfo::GetDlHarqVector,
                            "DL");
    }

  ScheduleDl (params, dlHarqFeedback);
}

/**
 * \brief Decide how to fill the frequency/time of a UL slot
 * \param params parameters for the scheduler
 *
 * The function starts by refreshing the CQI received, and eventually resetting
 * the expired values. Then, the HARQ feedback are processed (ProcessHARQFeedbacks),
 * and finally the expired HARQs are canceled (ResetExpiredHARQ).
 *
 * \see ScheduleUl
 */
void
NrMacSchedulerNs3::DoSchedUlTriggerReq (const NrMacSchedSapProvider::SchedUlTriggerReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  // process received CQIs
  m_cqiManagement.RefreshUlCqiMaps (m_ueMap);

  // reset expired HARQ
  for (const auto & itUe : m_ueMap)
    {
      ResetExpiredHARQ (itUe.second->m_rnti, &itUe.second->m_ulHarq);
    }

  // Merge not-retransmitted and received feedback
  std::vector <UlHarqInfo> ulHarqFeedback;
  if (params.m_ulHarqInfoList.size () > 0 || m_ulHarqToRetransmit.size () > 0)
    {
      // m_ulHarqToRetransmit will be cleared inside MergeHARQ
      uint64_t existingSize = m_ulHarqToRetransmit.size ();
      uint64_t inSize = params.m_ulHarqInfoList.size ();

      ulHarqFeedback = MergeHARQ (&m_ulHarqToRetransmit, params.m_ulHarqInfoList, "UL");

      NS_ASSERT (m_ulHarqToRetransmit.size () == 0);
      NS_ASSERT_MSG (existingSize + inSize == ulHarqFeedback.size (),
                     " existing: " << existingSize << " received: " << inSize <<
                     " calculated: " << ulHarqFeedback.size ());

      // if there are feedbacks for expired process, remove them
      for (auto it = ulHarqFeedback.begin (); it != ulHarqFeedback.end (); /* no inc */)
        {
          auto & ueInfo = m_ueMap.find (it->m_rnti)->second;
          auto & process = ueInfo->m_ulHarq.Find (it->m_harqProcessId)->second;
          if (!process.m_active)
            {
              NS_LOG_INFO ("Feedback for UE " << it->m_rnti << " process " <<
                           static_cast<uint32_t> (it->m_harqProcessId) <<
                           " ignored because process is INACTIVE");
              it = ulHarqFeedback.erase (it);
            }
          else
            {
              ++it;
            }
        }

      ProcessHARQFeedbacks (&ulHarqFeedback, NrMacSchedulerUeInfo::GetUlHarqVector,
                            "UL");
    }

  ScheduleUl (params, ulHarqFeedback);
}

/**
 * \brief Save the SR list into m_srList
 * \param params SR list
 *
 * m_srList will be evaluated in DoScheduleUlSr()
 */
void
NrMacSchedulerNs3::DoSchedUlSrInfoReq (const NrMacSchedSapProvider::SchedUlSrInfoReqParameters &params)
{
  NS_LOG_FUNCTION (this);

  // Merge RNTI in our current list
  for (const auto & ue : params.m_srList)
    {
      NS_LOG_INFO ("UE " << ue << " asked for a SR ");

      auto it = std::find (m_srList.begin(), m_srList.end(), ue);
      if (it == m_srList.end())
        {
          m_srList.push_back (ue);
        }
    }
  NS_ASSERT (m_srList.size () >= params.m_srList.size ());
}

} // namespace ns3
