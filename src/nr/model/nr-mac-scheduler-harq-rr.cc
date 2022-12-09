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
#include "nr-mac-scheduler-harq-rr.h"
#include <algorithm>
#include <ns3/log.h>
#include "beam-conf-id.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerHarqRr");

NrMacSchedulerHarqRr::NrMacSchedulerHarqRr ()
{
}

void
NrMacSchedulerHarqRr::InstallGetBwpIdFn (const std::function<uint16_t ()> &fn)
{
  m_getBwpId = fn;
}

void
NrMacSchedulerHarqRr::InstallGetCellIdFn (const std::function<uint16_t ()> &fn)
{
  m_getCellId = fn;
}

void
NrMacSchedulerHarqRr::InstallGetBwInRBG(const std::function<uint16_t ()> &fn)
{
  m_getBwInRbg = fn;
}


/**
 * \brief Schedule DL HARQ in RR fashion
 * \param startingPoint starting point of the first retransmission.
 * \param symAvail Available symbols
 * \param activeDlHarq Map of the active HARQ processes
 * \param ueMap Map of the UEs
 * \param dlHarqToRetransmit HARQ feedbacks that could not be transmitted (to fill)
 * \param dlHarqFeedback all the HARQ feedbacks
 * \param slotAlloc Slot allocation info
 * \return the VarTtiSlotAlloc ID to use next
 *
 * The algorithm is a bit complex, but nothing special. The HARQ should be
 * placed in 2D space as they were before. Probably there is an error in the algorithm.
 */
uint8_t NrMacSchedulerHarqRr::ScheduleDlHarq (NrMacSchedulerNs3::PointInFTPlane *startingPoint,
                                                  uint8_t symAvail,
                                                  const Ns3Sched::ActiveHarqMap &activeDlHarq,
                                                  const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > &ueMap,
                                                  std::vector<DlHarqInfo> *dlHarqToRetransmit,
                                                  const std::vector<DlHarqInfo> &dlHarqFeedback,
                                                  SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (startingPoint->m_rbg == 0);
  uint8_t usedSym = 0;
  uint8_t symPerBeam = symAvail / activeDlHarq.size ();

  NS_LOG_INFO ("We have " << activeDlHarq.size () <<
               " beams with data to RETX, each beam has " <<
               static_cast<uint32_t> (symPerBeam) << " symb");

  for (const auto & beam : activeDlHarq)
    {
      std::vector<uint16_t> allocatedUe;
      NS_LOG_INFO (" Try to assign HARQ resource for BeamConfId: " << beam.first <<
                   " # HARQ to Retx=" << beam.second.size ());

      for (auto it = beam.second.cbegin (); it != beam.second.cend (); ++it)
        {
          HarqProcess & harqProcess = (*it)->second;
          NS_ASSERT_MSG (harqProcess.m_status == HarqProcess::RECEIVED_FEEDBACK,
                         "Process " << static_cast<uint32_t> ((*it)->first) <<
                         " is not in RECEIVED_FEEDBACK status");

          harqProcess.m_status = HarqProcess::WAITING_FEEDBACK;
          harqProcess.m_timer = 0;

          auto & dciInfoReTx = harqProcess.m_dciElement;

          long rbgAssigned = std::count (dciInfoReTx->m_rbgBitmask.begin (),
                                         dciInfoReTx->m_rbgBitmask.end (), 1) * dciInfoReTx->m_numSym;
          uint32_t rbgAvail = (GetBandwidthInRbg () - startingPoint->m_rbg) * symPerBeam;

          NS_LOG_INFO ("Evaluating space to retransmit HARQ PID=" <<
                       static_cast<uint32_t> (dciInfoReTx->m_harqProcess) <<
                       " for UE=" << static_cast<uint32_t> (dciInfoReTx->m_rnti) <<
                       " SYM assigned previously=" << static_cast<uint32_t> (dciInfoReTx->m_numSym) <<
                       " RBG assigned previously=" << static_cast<uint32_t> (rbgAssigned) <<
                       " SYM avail for this beam=" << static_cast<uint32_t> (symPerBeam) <<
                       " RBG avail for this beam=" << rbgAvail);

          if (std::find (allocatedUe.begin (), allocatedUe.end (), dciInfoReTx->m_rnti) != allocatedUe.end ())
            {
              NS_LOG_INFO ("UE " << dciInfoReTx->m_rnti <<
                           " already has an HARQ allocated, buffer this HARQ process" <<
                           static_cast<uint32_t> (dciInfoReTx->m_harqProcess));
              BufferHARQFeedback (dlHarqFeedback, dlHarqToRetransmit, dciInfoReTx->m_rnti,
                                  dciInfoReTx->m_harqProcess);
              continue;
            }
          else if (rbgAvail < rbgAssigned)
            {
              NS_LOG_INFO ("No resource for this retx, we have to buffer it");
              BufferHARQFeedback (dlHarqFeedback, dlHarqToRetransmit, dciInfoReTx->m_rnti,
                                  dciInfoReTx->m_harqProcess);
              continue;
            }

          allocatedUe.push_back (dciInfoReTx->m_rnti);

          NS_ASSERT (dciInfoReTx->m_format == DciInfoElementTdma::DL);

          std::vector<uint32_t> tbSize;
          tbSize.resize (dciInfoReTx->m_tbSize.size ());
          std::vector<uint8_t> ndi;
          ndi.resize (dciInfoReTx->m_ndi.size ());
          std::vector<uint8_t> rv;
          rv.resize (dciInfoReTx->m_rv.size ());
          std::vector<uint8_t> mcs;
          mcs.resize (dciInfoReTx->m_mcs.size ());

          for (uint8_t stream = 0; stream < dciInfoReTx->m_tbSize.size (); stream++)
            {
              auto it = std::find (harqProcess.nackStreamIndexes.begin (), harqProcess.nackStreamIndexes.end (), stream);
              if (it != harqProcess.nackStreamIndexes.end ())
                {
                  //if stream index is in nackStreamIndexes that means
                  //we received NACK for it. Therefore, we need to use the same
                  //tbSize and the mcs, increase the RV and toggle the NDI flag.
                  tbSize.at (stream) = dciInfoReTx->m_tbSize.at (stream);
                  mcs.at (stream) = dciInfoReTx->m_mcs.at (stream);
                  rv.at (stream) = dciInfoReTx->m_rv.at (stream) + 1;
                  ndi.at (stream) = 0;
                }
              else
                {
                  //if stream index is not in nackStreamIndexes that means
                  //we received ACK for it. Therefore, no need to retransmit
                  tbSize.at (stream) = 0;
                  mcs.at (stream) = UINT8_MAX;
                  rv.at (stream) = 0;
                  ndi.at (stream) = UINT8_MAX;
                }

            }

          auto dci = std::make_shared<DciInfoElementTdma> (dciInfoReTx->m_rnti, dciInfoReTx->m_format,
                                                           startingPoint->m_sym, symPerBeam,
                                                           mcs, tbSize, ndi, rv, DciInfoElementTdma::DATA,
                                                           dciInfoReTx->m_bwpIndex, dciInfoReTx->m_tpc);

          dci->m_rbgBitmask = harqProcess.m_dciElement->m_rbgBitmask;
          dci->m_harqProcess = dciInfoReTx->m_harqProcess;

          harqProcess.m_dciElement = dci;
          dciInfoReTx = harqProcess.m_dciElement;

          if (rbgAssigned % dciInfoReTx->m_numSym == 0)
            {
              rbgAssigned = rbgAssigned / dciInfoReTx->m_numSym;
            }
          else
            {
              rbgAssigned = rbgAssigned / dciInfoReTx->m_numSym;
              ++rbgAssigned;
            }

          NS_ABORT_IF (static_cast<unsigned long> (rbgAssigned) > dciInfoReTx->m_rbgBitmask.size ());

          for (unsigned int i = 0; i < dciInfoReTx->m_rbgBitmask.size (); ++i)
            {
              if (startingPoint->m_rbg <= i && i < startingPoint->m_rbg + rbgAssigned)
                {
                  dciInfoReTx->m_rbgBitmask.at (i) = 1;
                }
              else
                {
                  dciInfoReTx->m_rbgBitmask.at (i) = 0;
                }
            }

          startingPoint->m_rbg += rbgAssigned;

          VarTtiAllocInfo slotInfo (dciInfoReTx);

          for (uint8_t stream = 0; stream < dciInfoReTx->m_tbSize.size (); stream++)
            {
              NS_LOG_DEBUG ("UE" << dciInfoReTx->m_rnti <<
                            " gets DL symbols " << static_cast<uint32_t> (dciInfoReTx->m_symStart) <<
                            "-" << static_cast<uint32_t> (dciInfoReTx->m_symStart + dciInfoReTx->m_numSym - 1) <<
                            " tbs " << dciInfoReTx->m_tbSize.at (stream) <<
                            " harqId " << static_cast<uint32_t> (dciInfoReTx->m_harqProcess) <<
                            " rv " << static_cast<uint32_t> (dciInfoReTx->m_rv.at (stream)) <<
                            " RBG start: " << static_cast<uint32_t> (startingPoint->m_rbg - rbgAssigned) <<
                            " RBG end: " << static_cast<uint32_t> (startingPoint->m_rbg) <<
                            " RETX");
            }

          for (auto rlcPdu : harqProcess.m_rlcPduInfo)
            {
              //we should have pushed the RLC PDU info
              //of only the stream we are rescheduling.
              //But I do not see any harm keeping the
              //info of all the streams since the code
              //in NrGnbMac is smart enough to retransmit
              //only if ndi == 0, and TB size > 0. Plus,
              //here we avoid extra loops.
              slotInfo.m_rlcPduInfo.push_back (rlcPdu);
            }
          slotAlloc->m_varTtiAllocInfo.push_back (slotInfo);

          ueMap.find (dciInfoReTx->m_rnti)->second->m_dlMRBRetx = dciInfoReTx->m_numSym * rbgAssigned;
        }

      if (allocatedUe.size () > 0)
        {
          startingPoint->m_sym += symPerBeam;
          startingPoint->m_rbg = 0;
          usedSym += symPerBeam;
          slotAlloc->m_numSymAlloc += symPerBeam;
          symAvail -= symPerBeam;
        }
    }
  NS_ASSERT (startingPoint->m_rbg == 0);
  return usedSym;
}

/**
 * \brief Schedule the UL HARQ
 * \param startingPoint starting point of the first retransmission.
 * It should be set to the next available starting point
 * \param symAvail Available symbols
 * \param ueMap Map of the UEs
 * \param ulHarqToRetransmit HARQ feedbacks that could not be transmitted (to fill)
 * \param ulHarqFeedback all the HARQ feedbacks
 * \param slotAlloc Slot allocation info
 * \return the VarTtiSlotAlloc ID to use next
 *
 * The algorithm for scheduling the UL HARQ is straightforward. Since the UL
 * transmission are all TDMA, for each NACKed process a DCI is built, with
 * the exact same specification as the first transmission. If there aren't
 * available symbols to retransmit the data, the feedback is buffered for
 * the next slot.
 */
uint8_t
NrMacSchedulerHarqRr::ScheduleUlHarq (NrMacSchedulerNs3::PointInFTPlane *startingPoint,
                                          uint8_t symAvail,
                                          const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > &ueMap,
                                          std::vector<UlHarqInfo> *ulHarqToRetransmit,
                                          const std::vector<UlHarqInfo> &ulHarqFeedback,
                                          SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  uint8_t symUsed = 0;
  NS_ASSERT (startingPoint->m_rbg == 0);

  NS_LOG_INFO ("Scheduling UL HARQ starting from sym " << +startingPoint->m_sym <<
               " and RBG " << +startingPoint->m_rbg << ". Available symbols: " <<
               symAvail << " number of feedback: " << ulHarqFeedback.size ());

  for (uint16_t i = 0; i < ulHarqFeedback.size () && symAvail > 0; i++)
    {
      UlHarqInfo harqInfo = ulHarqFeedback.at (i);
      uint8_t harqId = harqInfo.m_harqProcessId;
      uint16_t rnti = harqInfo.m_rnti;

      NS_ABORT_IF (harqInfo.IsReceivedOk ());

      // retx correspondent block: retrieve the UL-DCI
      HarqProcess & harqProcess = ueMap.find (rnti)->second->m_ulHarq.Find (harqId)->second;
      NS_ASSERT(harqProcess.m_status == HarqProcess::RECEIVED_FEEDBACK);

      harqProcess.m_status = HarqProcess::WAITING_FEEDBACK;
      harqProcess.m_timer = 0;
      auto & dciInfoReTx = harqProcess.m_dciElement;

      NS_LOG_INFO ("Feedback is for UE " << rnti << " process " << +harqId <<
                   " sym: " << +dciInfoReTx->m_numSym);

      if (symAvail >= dciInfoReTx->m_numSym)
        {
          symAvail -= dciInfoReTx->m_numSym;
          symUsed += dciInfoReTx->m_numSym;

          NS_ASSERT (dciInfoReTx->m_format == DciInfoElementTdma::UL);

          NS_ASSERT_MSG (harqProcess.nackStreamIndexes.size () == 1, "MIMO is not supported for UL yet");

          uint8_t rvIndex = dciInfoReTx->m_rv.at (0) + 1;
          std::vector<uint8_t> rv {rvIndex};
          std::vector<uint8_t> ndi {0};

          auto dci = std::make_shared<DciInfoElementTdma> (dciInfoReTx->m_rnti, dciInfoReTx->m_format,
                                                           startingPoint->m_sym - dciInfoReTx->m_numSym,
                                                           dciInfoReTx->m_numSym,
                                                           dciInfoReTx->m_mcs, dciInfoReTx->m_tbSize,
                                                           ndi, rv, DciInfoElementTdma::DATA,
                                                           dciInfoReTx->m_bwpIndex, dciInfoReTx->m_tpc);
          dci->m_rbgBitmask = harqProcess.m_dciElement->m_rbgBitmask;
          dci->m_harqProcess = harqId;
          harqProcess.m_dciElement = dci;
          dciInfoReTx = harqProcess.m_dciElement;

          startingPoint->m_sym -= dciInfoReTx->m_numSym;

          VarTtiAllocInfo slotInfo (dciInfoReTx);
          for (uint8_t stream = 0; stream < dciInfoReTx->m_tbSize.size (); stream++)
            {
              NS_LOG_DEBUG ("UE" << dciInfoReTx->m_rnti <<
                            " gets UL symbols " << static_cast<uint32_t> (dciInfoReTx->m_symStart) <<
                            "-" << static_cast<uint32_t> (dciInfoReTx->m_symStart + dciInfoReTx->m_numSym - 1) <<
                            " tbs " << dciInfoReTx->m_tbSize.at (stream) <<
                            " harqId " << static_cast<uint32_t> (dciInfoReTx->m_harqProcess) <<
                            " rv " << static_cast<uint32_t> (dciInfoReTx->m_rv.at (stream)) <<
                            " RETX");
            }
          slotAlloc->m_varTtiAllocInfo.push_front (slotInfo);
          slotAlloc->m_numSymAlloc += dciInfoReTx->m_numSym;

          ueMap.find (rnti)->second->m_ulMRBRetx = dciInfoReTx->m_numSym * GetBandwidthInRbg ();
        }
      else
        {
          ulHarqToRetransmit->push_back (ulHarqFeedback.at (i));
        }
    }

  NS_ASSERT (startingPoint->m_rbg == 0);

  return symUsed;
}

/**
 * \brief Sort Dl Harq retx based on their symbol requirement
 * \param activeDlHarq map of the active retx
 */
void
NrMacSchedulerHarqRr::SortDlHarq (NrMacSchedulerNs3::ActiveHarqMap *activeDlHarq) const
{
  NS_LOG_FUNCTION (this);
  // Order based on required sym
  static struct
  {
    bool operator() (const NrMacSchedulerNs3::HarqVectorIterator &a,
                     const NrMacSchedulerNs3::HarqVectorIterator &b) const
    {
      return a->second.m_dciElement->m_numSym > b->second.m_dciElement->m_numSym;
    }
  } CompareNumSym;

  for (auto & it : *activeDlHarq)
    {
      std::sort (it.second.begin (), it.second.end (), CompareNumSym);
    }
}

/**
 * \brief (In theory) sort UL HARQ retx
 * \param activeUlHarq map of the active retx
 *
 * Since in the uplink we are still TDMA, there is no need of sorting
 * the HARQ. The HARQ will be picked one by one until there are no
 * available symbol to transmit, and what is not transmitted will be queued
 * for the next slot.
 */
void
NrMacSchedulerHarqRr::SortUlHarq ([[maybe_unused]] NrMacSchedulerNs3::ActiveHarqMap *activeUlHarq) const
{
  NS_LOG_FUNCTION (this);
}

/**
 * \brief Find the specified HARQ process and buffer it into a vector
 * \param dlHarqFeedback HARQ not retransmitted list
 * \param dlHarqToRetransmit HARQ buffer list (to retransmit)
 * \param rnti RNTI to find
 * \param harqProcess process ID to find
 */
void
NrMacSchedulerHarqRr::BufferHARQFeedback (const std::vector <DlHarqInfo> &dlHarqFeedback,
                                              std::vector<DlHarqInfo> *dlHarqToRetransmit,
                                              uint16_t rnti, uint8_t harqProcess) const
{
  NS_LOG_INFO (this);

  bool found = false;
  for (const auto & feedback : dlHarqFeedback)
    {
      if (feedback.m_rnti == rnti
          && feedback.m_harqProcessId == harqProcess)
        {
          dlHarqToRetransmit->push_back (feedback);
          found = true;
          break;
        }
    }
  NS_ASSERT (found);
}

uint16_t NrMacSchedulerHarqRr::GetBwpId () const
{
  return m_getBwpId ();
}

uint16_t NrMacSchedulerHarqRr::GetCellId () const
{
  return m_getCellId ();
}

uint16_t
NrMacSchedulerHarqRr::GetBandwidthInRbg() const
{
  return m_getBwInRbg ();
}

} // namespace ns3
