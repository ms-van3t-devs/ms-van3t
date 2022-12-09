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
#pragma once

#include "nr-mac-sched-sap.h"
#include "nr-mac-harq-vector.h"
#include "nr-mac-scheduler-lcg.h"
#include "nr-amc.h"
#include <unordered_map>
#include <functional>
#include "beam-conf-id.h"
#include <algorithm>

namespace ns3 {

class NrMacSchedulerUeInfo;
/**
 * \brief Shared pointer to an instance of NrMacSchedulerUeInfo
 */
typedef std::shared_ptr<NrMacSchedulerUeInfo> UePtr;

/**
 * \ingroup scheduler
 * \brief The representation of an user for any Mac scheduler
 *
 * Basic representation for an UE inside any scheduler. The class is responsible
 * to store all the UE-related field that can be used by any scheduler.
 *
 * If a scheduler needs to store additional information, it is necessary to
 * create a subclass and store the information there. Then, the scheduler
 * will return a pointer to a newly created instance through
 * NrMacSchedulerNs3::CreateUeRepresentation.
 *
 * The class stores information such as RBG, MCS, and CQI. Information that
 * should be reset after each slot (such as RBG count) should be reset in the
 * method ResetDlSchedInfo() and ResetUlSchedInfo().
 *
 * When a scheduler assign new resources to the UE, it will call
 * the method UpdateDlMetric() or UpdateUlMetric(). Make sure all the relevant
 * information that should be updated for a correct sorting are updated there.
 *
 * \see NrMacSchedulerUeInfoRR
 * \see NrMacSchedulerUeInfoPF
 * \see NrMacSchedulerUeInfoMR
 */
class NrMacSchedulerUeInfo
{
public:
  /**
   * \brief Default Constructor (deleted)
   */
  NrMacSchedulerUeInfo () = delete;

  typedef std::function <uint32_t ()> GetRbPerRbgFn;

  /**
   * \brief Create a new UE representation
   * \param rnti the RNTI of the UE
   * \param beamConfId the BeamConfId of the UE (can be updated later)
   */
  NrMacSchedulerUeInfo (uint16_t rnti, BeamConfId beamConfId, const GetRbPerRbgFn &fn);

  /**
   * \brief ~NrMacSchedulerUeInfo deconstructor
   */
  virtual ~NrMacSchedulerUeInfo ();

  /**
   * \brief GetDlRBG
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static uint32_t & GetDlRBG (const UePtr &ue);
  /**
   * \brief GetUlRBG
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static uint32_t & GetUlRBG (const UePtr &ue);
  /**
   * \brief GetDlSym
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static uint8_t & GetDlSym (const UePtr &ue);
  /**
   * \brief GetUlSym
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static uint8_t & GetUlSym (const UePtr &ue);
  /**
   * \brief GetDlMcs
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static uint8_t & GetDlMcs (const UePtr &ue, uint8_t stream);
  /**
   * \brief GetUlMcs
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static uint8_t & GetUlMcs (const UePtr &ue);
  /**
   * \brief GetDlTBSPerStream
   * \param ue UE pointer from which obtain the value
   * \param stream The stream id UE pointer from which obtain the value
   * \return The TB size of the stream whose id is passed as an argument
   */
  static uint32_t & GetDlTBSPerStream (const UePtr &ue, uint8_t stream);
  /**
   * \brief GetDlTBS
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static uint32_t GetDlTBS (const UePtr &ue);
  /**
   * \brief GetUlTBS
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static uint32_t GetUlTBS (const UePtr &ue);
  /**
   * \brief GetDlLCG
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static std::unordered_map<uint8_t, LCGPtr> & GetDlLCG (const UePtr &ue);
  /**
   * \brief GetUlLCG
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static std::unordered_map<uint8_t, LCGPtr> & GetUlLCG (const UePtr &ue);
  /**
   * \brief GetDlHarqVector
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static NrMacHarqVector & GetDlHarqVector (const UePtr &ue);
  /**
   * \brief GetUlHarqVector
   * \param ue UE pointer from which obtain the value
   * \return
   */
  static NrMacHarqVector & GetUlHarqVector (const UePtr &ue);

  typedef std::function<std::unordered_map<uint8_t, LCGPtr> &(const UePtr &ue)> GetLCGFn;
  typedef std::function<NrMacHarqVector& (const UePtr &ue)> GetHarqVectorFn;

  /**
   * \brief Reset DL information
   *
   * Called after each slot. It should reset all the information that are
   * slot-dependent.
   */
  virtual void ResetDlSchedInfo ();

  /**
   * \brief Reset UL information
   *
   * Called after each slot. It should reset all the information that are
   * slot-dependent.
   */
  virtual void ResetUlSchedInfo ();

  /**
   * \brief Update DL metrics after resources have been assigned
   *
   * The amount of assigned resources is stored inside m_dlRBG by the scheduler.
   */
  virtual void UpdateDlMetric (const Ptr<const NrAmc> &amc);

  /**
   * \brief ResetDlMetric
   *
   * Called when the scheduler has assigned RBGs, but the sum does not arrive
   * to a TBS > 0. The assignation is, therefore, not transformed in DCI.
   * These RBG will not be assigned, they will be empty in the slot.
   */
  virtual void ResetDlMetric ();

  /**
   * \brief Update UL metrics after resources have been assigned
   *
   * The amount of assigned resources is stored inside m_ulRBG by the scheduler.
   */
  virtual void UpdateUlMetric (const Ptr<const NrAmc> &amc);

  /**
   * \brief ResetDlMetric
   *
   * Called when the scheduler has assigned RBGs, but the sum does not arrive
   * to a TBS > 0. The assignation is, therefore, not transformed in DCI.
   * These RBG will not be assigned, they will be empty in the slot.
   */
  virtual void ResetUlMetric ();

  /**
   * \brief Received CQI information
   */
  struct CqiInfo
  {
    /**
     * \brief Type of CQI
     */
    enum CqiType
    {
      WB,             //!< Wide-band
      SB              //!< Sub-band
    } m_cqiType {WB}; //!< CQI type

    std::vector<double> m_sinr;   //!< Vector of SINR for the entire band
    std::vector<int16_t> m_rbCqi; //!< CQI for each Rsc Block, set to -1 if SINR < Threshold
    uint8_t m_cqi    {0};  //!< CQI reported value
    uint32_t m_timer {0};  //!< Timer (in slot number). When the timer is 0, the value is discarded
  };

  /**
   * \brief Received CQI information
   */
  struct DlCqiInfo
  {
    /**
     * \brief Type of CQI
     */
    enum CqiType
    {
      WB,             //!< Wide-band
      SB              //!< Sub-band
    } m_cqiType {WB}; //!< CQI type

    uint8_t m_ri    {0}; //!< The rank indicator, by default UE would have only one stream
    std::vector<double> m_sinr;   //!< Vector of SINR for the entire band
    std::vector<uint8_t> m_wbCqi; //!< CQI for each stream
    uint32_t m_timer {0};  //!< Timer (in slot number). When the timer is 0, the value is discarded
  };

  uint16_t m_rnti {0};          //!< RNTI of the UE
  BeamConfId   m_beamConfId;    //!< Beam ID of the UE (kept updated as much as possible by MAC)

  std::unordered_map<uint8_t, LCGPtr> m_dlLCG;//!< DL LCG
  std::unordered_map<uint8_t, LCGPtr> m_ulLCG;//!< UL LCG

  uint32_t        m_dlMRBRetx {0};  //!< MRB assigned for retx. To update the name, what is MRB is not defined
  uint32_t        m_ulMRBRetx {0};  //!< MRB assigned for retx. To update the name, what is MRB is not defined
  uint32_t        m_dlRBG     {0};  //!< DL Resource Block Group assigned in this slot
  uint32_t        m_ulRBG     {0};  //!< UL Resource Block Group assigned in this slot
  uint8_t         m_dlSym     {0};  //!< Number of (new data) symbols assigned in this slot.
  uint8_t         m_ulSym     {0};  //!< Number of (new data) symbols assigned in this slot.

  std::vector<uint8_t> m_dlMcs;  //!< DL MCS per stream, it is initialized with a starting MCS upon UE addition to gNB and the scheduler
  uint8_t m_ulMcs     {0};  //!< UL MCS

  std::vector<uint32_t> m_dlTbSize {0};  //!< DL Transport Block Size per stream, depends on MCS and RBG, updated in UpdateDlMetric()
  uint32_t m_ulTbSize         {0};  //!< UL Transport Block Size, depends on MCS and RBG, updated in UpdateDlMetric()

  DlCqiInfo m_dlCqi;                  //!< DL CQI information
  CqiInfo m_ulCqi;                  //!< UL CQI information

  NrMacHarqVector m_dlHarq;     //!< HARQ process vector for DL
  NrMacHarqVector m_ulHarq;     //!< HARQ process vector for UL

  uint32_t m_srsPeriodicity {0}; //!< SRS periodicity
  uint32_t m_srsOffset {0};      //!< SRS offset
  uint8_t m_startMcsDlUe {0}; //!< Starting DL MCS to be used

protected:
  /**
   * \brief Retrieve the number of RB per RBG
   *
   * \return numRbPerRbg. Calls the MAC.
   */
  uint32_t GetNumRbPerRbg () const;

private:
  const GetRbPerRbgFn m_getNumRbPerRbg; //!< Function that points to a method which knows the number of RB per RBG.
};

} // namespace ns3
