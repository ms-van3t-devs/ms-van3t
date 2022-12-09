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

#include "nr-phy-mac-common.h"
#include "nr-mac-harq-vector.h"
#include "nr-mac-scheduler.h"
#include "nr-mac-scheduler-ue-info.h"
#include "nr-mac-scheduler-lcg.h"
#include "nr-mac-scheduler-cqi-management.h"
#include "nr-amc.h"
#include <memory>
#include <functional>
#include <list>

namespace ns3 {

class NrSchedGeneralTestCase;
class NrMacSchedulerHarqRr;
class NrMacSchedulerSrsDefault;

/**
 * \ingroup scheduler
 * \brief A general scheduler for nr in NS3
 *
 * This abstract class is taking care of creating a solid base for any schedulers
 * in the nr world. The class implements all the API from the FemtoForum API,
 * but in doing so, it defines a new interface that must be followed when designing
 * and writing a new scheduler type.
 *
 * The architecture has a unique representation of a UE, that is valid across
 * all the schedulers. Each one can expand the definition, adding values or
 * functions to call while doing the scheduler job.
 * The base class is defined as NrMacSchedulerUeInfo. Please refer to
 * its documentation to know the default values, and how to use or expand it.
 *
 * The documentation continues by following every step involved in the scheduling.
 * Please refer to the function documentation to see a detailed description
 * of the steps done during all the phases.
 *
 * \section scheduler_registration Registration and Configuration
 *
 * The attribute of any scheduler can be set directly calling `SetAttribute`
 * on the pointer obtained through NrHelper::GetScheduler() or, before
 * it is created, through NrHelper::SetSchedulerAttribute(). The type of
 * the scheduler can be set only before its creation, through the method
 * NrHelper::SetSchedulerTypeId().
 *
 * \section scheduler_user_managment User management (creation and removal)
 *
 * When a user arrives in the cell, it is registered with a call to
 * DoCschedUeConfigReq. When the user leaves, the class is made aware with
 * a call to DoCschedUeReleaseReq. The only important operation is the creation
 * of a UE representation and its storage in the general UE map (m_ueMap).
 *
 * A UE is represented through the class NrMacSchedulerUeInfo, which is
 * used in the internals of the general base class to store and retrieve
 * information such as Logical Channels, CQI, and other things. Please refer
 * to its documentation for a broader overview of its possibilities.
 *
 * \section scheduler_cell_conf Cell configuration
 *
 * The cell configuration, done with a call to DoCschedCellConfigReq, is ignored.
 *
 * \section scheduler_lc_creation LC creation and removal
 *
 * After the registration of a UE, the scheduler has to know how many bytes
 * there are in its queue, for both uplink and downlink. Before that,
 * the scheduler has to know how many Logical Channels are present
 * for each UE (DL and UL). Each time an LC is created, the MAC calls
 * the function DoCschedLcConfigReq(). Please refer to the documentation
 * of NrMacSchedulerUeInfo to know the details of the LC and LC Groups
 * representation in the code. The LC can be deleted with a call to
 * DoCschedLcReleaseReq (currently not implemented).
 *
 * A subclass of NrMacSchedulerNs3 can change the representation
 * of an LC and LCG by merely creating an appropriate subclass
 * of NrMacSchedulerLC or NrMacSchedulerLCG (the classes used by the
 * default implementation to store information about the LC or LCG) and then
 * reimplementing the methods CreateLCG() and CreateLC() to return a pointer to
 * a created instance.
 *
 * \section scheduler_update_lc Updating the LC bytes
 *
 * For the downlink case, the LC is updated with a message between the gNB RLC
 * layer and the MAC.  The scheduler receives a call to the method
 * DoSchedDlRlcBufferReq(), and inside this method is updating all the LC amount.
 *
 * For the uplink case, there are more passages involved. In the scheduler,
 * however, the important this is that is called the method DoSchedUlMacCtrlInfoReq().
 * Inside this method, the BSR coming from UE is used to update the LC.
 * More details can be found in the documentation of the class NrMacSchedulerLCG
 * and NrMacSchedulerLC.
 *
 * \section scheduler_cqi CQI Management
 *
 * The CQI is based on a parameter (m_cqiTimersThreshold) that indicates how
 * long a received CQI is valid. Every time that a Dl CQI is received, the
 * MAC calls the function DoSchedDlCqiInfoReq. In here, the CQI list is
 * traversed and each CQI is reported to the class NrMacSchedulerCQIManagement
 * that is responsible for calculating the CQI. The value is then stored inside
 * the UE representation, ready to be read in the future. The CQI is reset to
 * the default value once the validity timer expires. The default value permits
 * only to have an MCS value 0.
 *
 * For the UL case, the MAC is calling the method DoSchedUlCqiInfoReq(). The CQI
 * and MCS values are then derived by using a vector of SINR (see the documentation
 * of the function for the details).
 *
 * At the end of these evaluations, inside the UE representation, is available
 * the value of the DL/UL MCS, ready to be used.
 *
 * \section scheduler_scheduling_general Scheduling phase
 *
 * After gathering the information regarding CQI, active users and flows, it is
 * time to take a look into how the class manages the most important thing,
 * the scheduling. The work is about deciding how to fill the frequency/time
 * space, assigning resources to HARQ retransmission or DL/UL new data transmission.
 * The main scheduling function is NrMacSchedulerNs3::DoSchedTriggerReq.
 *
 * \section scheduler_cqi Refreshing CQI
 *
 * The refreshing of CQI consists in evaluating the validity timer of the value.
 * If the timer is equal to 0, the valued is expired, and the value is reset
 * to the default (MCS 0). The operation is managed inside the class
 * NrMacSchedulerCQIManagement, with the two functions
 * NrMacSchedulerCQIManagement::RefreshDlCQIMaps and
 * NrMacSchedulerCQIManagement::RefreshUlCQIMaps.
 *
 * \section scheduler_process_harq Process HARQ feedbacks
 *
 * To decide if it is necessary to perform HARQ retransmission, and to decide
 * how many retransmission perform, the first step is to evaluate the HARQ
 * feedback received as input. The UEs are reporting the feedbacks, and these
 * feedbacks are merged with the feedback of the previous slots that could not
 * be transmitted (function MergeDlHARQ()). Then, the code evaluates these
 * feedbacks by resetting HARQ processes with an ACK and preparing for
 * the retransmission of the HARQ processes marked with NACK (ProcessHARQFeedbacks())
 * for both UL and DL HARQs.
 *
 * At the end of the process, the code evaluates the HARQ timers, and reset the
 * processes with an expired timer (ResetExpiredHARQ()).
 *
 * To discover more about how HARQ processes are stored and managed, please take
 * a look at the HarqProcess and NrMacHarqVector documentation.
 *
 * \section scheduler_general The concept of scheduling
 *
 * The scheduling of the resources is a process that should fill the slot time
 * and the slot frequencies with retransmitted or fresh data. To simplify the
 * model, you could think that there is a 2D plan in which the "y" values are
 * the frequencies, and the "x" value is the time that passes. The plan should be
 * filled with data, or better said, with a series of blocks. Each block is the
 * minimum assignable resource, called Resource Block Group. There are constraints
 * on how these RBG can be distributed, and often these constraints follow some
 * limitation in the UE equipment, but also they are in place to limit the complexity
 * of a problem which is computationally hard. The position of each block is
 * defined by a starting point (PointInFTPlane) which is a pair (x,y) (or better
 * said (sym, rb)) that represent a point from which a block should be positioned.
 *
 * Please refer to the DoScheduling() documentation to know how the
 * scheduling is performed.
 *
 * \section scheduler_spat_mult Spatial multiplexing
 *
 * The code does not support Spatial Multiplexing. It means that it is not possible
 * to schedule UEs that are in different beams at the same time. While this has
 * no practical effects on a time-based distribution of resources, it affects
 * the frequency-based distribution. Therefore, to support both operational modes,
 * the scheduler should compute beforehand the number of active UEs, as well as
 * the number of retransmission to be done. These operations are done, respectively,
 * by the methods ComputeActiveUe() and ComputeActiveHarq(). These methods work on
 * data structures that group UE and retransmission by BeamConfId
 * (ActiveUeMap and ActiveHarqMap).
 *
 * \section scheduler_sched_ul Scheduling UL
 * It is worth explaining that the
 * schedulers working on slot x for DL, are working on slot \f$x + y\f$
 * (where y is the value of N2 delay).
 * This delay is implemented to simulate the fact that the UE receives the DCI at
 * time \f$t\f$, and then has some time (the delay) to prepare its UL data.
 * So, if the scheduler assigns some symbols for uplink data in slot \f$x + y\f$,
 * after y slots have passed (and so the scheduler is preparing the slot $x+y$
 * for DL data and HARQ retransmission) the scheduler has to remember that there
 * are fewer symbols available. Moreover, it is necessary to not overlap the
 * decisions for DL on top of the (already taken) decision for UL.
 *
 * All this details are considered in the functions ScheduleUl() and ScheduleDl().
 *
 *
 * \section scheduler_harq HARQ
 *
 * The HARQ scheduling is done, if symbols for HARQ are available, before transmitting
 * new data, and this happens for both DL and UL. The detailed documentation
 * is available in the methods ScheduleDlHarq() and ScheduleUlHarq().
 *
 * \section scheduler_sched Scheduling new data
 *
 * The scheduling of new data is performed by functions ScheduleUlData() and
 * ScheduleDlData(). The priority is for HARQ retransmission, so if the
 * retransmissions fill the slot time, there will no symbols available for
 * new data.
 * Please refer to the method documentation for more detailed information
 * about the scheduling process of new data.
 *
 * An interesting operation which is not enforced in the current scheduler
 * version is how to distribute the assigned bytes to the different LC of
 * a UE. Now it is implemented the RR assignment, but it can be modified
 * (method AssignBytesToLC()).
 *
 * The available schedulers are TDMA and OFDMA version of the Round Robin,
 * Proportional Fair, and Maximum Rate.
 *
 * \see NrMacSchedulerOfdmaPF
 * \see NrMacSchedulerOfdmaRR
 * \see NrMacSchedulerOfdmaMR
 * \see NrMacSchedulerTdmaPF
 * \see NrMacSchedulerTdmaRR
 * \see NrMacSchedulerTdmaMR
 */
class NrMacSchedulerNs3 : public NrMacScheduler
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrMacSchedulerNs3 default constructor
   */
  NrMacSchedulerNs3 ();

  /**
   * \brief NrMacSchedulerNs3 copy constructor (deleted)
   * \param other instance of NrMacSchedulerNs3 to be copied
   */
  NrMacSchedulerNs3 (const NrMacSchedulerNs3 &other) = delete;

  /**
    * \brief NrMacSchedulerNs3 deconstructor
    */
  ~NrMacSchedulerNs3 () override;

  /**
   * \brief Install the AMC for the DL part
   * \param dlAmc DL AMC
   *
   * Usually called by the helper
   */
  void InstallDlAmc (const Ptr<NrAmc> &dlAmc);

  /**
   * \brief Install the AMC for the DL part
   * \param ulAmc DL AMC
   *
   * Usually called by the helper
   */
  void InstallUlAmc (const Ptr<NrAmc> &ulAmc);

  /**
   * \brief Get the AMC for UL
   * \return the UL AMC
   */
  Ptr<const NrAmc> GetUlAmc () const;

  /**
   * \brief Get the AMC for DL
   * \return the DL AMC
   */
  Ptr<const NrAmc> GetDlAmc () const;

  /**
   * \brief Point in the Frequency/Time plane
   *
   * The first element represent the RB (not the RBG), while the second element
   * represent the symbol.
   * The struct represents a point in the imaginary 2D space created by having
   * frequencies on the y and time on the x.
   */
  struct PointInFTPlane
  {
    /**
     * \brief PointInFTPlane constructor
     * \param rbg RBG
     * \param sym Symbol
     */
    PointInFTPlane (uint8_t rbg, uint8_t sym) : m_rbg (rbg), m_sym (sym)
    {
    }
    uint32_t m_rbg;  //!< Represent the starting RBG
    uint8_t m_sym; //!< Represent the starting symbol
  };

  typedef PointInFTPlane FTResources; //!< Represent an amount of RBG/symbols that can be, or is, assigned

  virtual void
  DoCschedCellConfigReq (const NrMacCschedSapProvider::CschedCellConfigReqParameters& params) override;
  virtual void
  DoCschedUeConfigReq (const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) override;
  virtual void
  DoSchedDlRlcBufferReq (const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters& params) override;
  virtual void
  DoSchedUlMacCtrlInfoReq (const NrMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params) override;
  virtual void
  DoSchedDlCqiInfoReq (const NrMacSchedSapProvider::SchedDlCqiInfoReqParameters& params) override;
  virtual void
  DoSchedUlCqiInfoReq (const NrMacSchedSapProvider::SchedUlCqiInfoReqParameters& params) override;
  virtual void
  DoCschedUeReleaseReq (const NrMacCschedSapProvider::CschedUeReleaseReqParameters& params) override;
  virtual void
  DoCschedLcConfigReq (const NrMacCschedSapProvider::CschedLcConfigReqParameters& params) override;
  virtual void
  DoCschedLcReleaseReq (const NrMacCschedSapProvider::CschedLcReleaseReqParameters& params) override;
  virtual void
  DoSchedDlTriggerReq (const NrMacSchedSapProvider::SchedDlTriggerReqParameters& params) override;
  virtual void
  DoSchedUlTriggerReq (const NrMacSchedSapProvider::SchedUlTriggerReqParameters& params) override;
  virtual void
  DoSchedUlSrInfoReq (const NrMacSchedSapProvider::SchedUlSrInfoReqParameters &params) override;
  virtual void
  DoSchedSetMcs (uint32_t mcs) override;
  virtual void
  DoSchedDlRachInfoReq (const NrMacSchedSapProvider::SchedDlRachInfoReqParameters& params) override;
  uint8_t GetDlCtrlSyms () const override;
  uint8_t GetUlCtrlSyms () const override;
  /**
   * \brief Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  virtual int64_t AssignStreams (int64_t stream) override;

  // to save some typing
  using HarqVectorIterator = NrMacHarqVector::iterator;
  using HarqVectorIteratorList = std::vector<HarqVectorIterator>;

  /**
   * \brief Pair between a pointer to NrMacSchedulerUeInfo and its buffer occupancy
   */
  typedef std::pair<UePtr, uint32_t> UePtrAndBufferReq;
  /**
   * \brief Map between a BeamConfId and a vector of UE (the UE are in that beam)
   */
  typedef std::unordered_map <BeamConfId, std::vector<UePtrAndBufferReq>, BeamConfIdHash> ActiveUeMap;
  /**
   * \brief Map between a BeamConfId and the symbol assigned to that beam
   */
  typedef std::unordered_map <BeamConfId, uint32_t, BeamConfIdHash> BeamSymbolMap;
  /**
   * \brief Map between a BeamConfId and the HARQ of that beam
   */
  typedef std::unordered_map<BeamConfId, HarqVectorIteratorList, BeamConfIdHash> ActiveHarqMap;

  /**
   * \brief Set the CqiTimerThreshold
   * \param v the value to set
   */
  void SetCqiTimerThreshold (const Time &v);
  /**
   * \brief Get the CqiTimerThreshold
   * \return the value
   */
  Time GetCqiTimerThreshold () const;

  /**
   * \brief Set if the MCS in DL is fixed (in case, it will take the starting value)
   * \param v the value
   * \see SetStartMcsDl
   */
  void SetFixedDlMcs (bool v);
  /**
   * \brief Check if the MCS in DL is fixed
   * \return true if the DL MCS is fixed, false otherwise
   */
  bool IsDlMcsFixed () const;

  /**
   * \brief Set if the MCS in UL is fixed (in case, it will take the starting value)
   * \param v the value
   * \see SetStartMcsUl
   */
  void SetFixedUlMcs (bool v);
  /**
   * \brief Check if the MCS in UL is fixed
   * \return true if the UL MCS is fixed, false otherwise
   */
  bool IsUlMcsFixed () const;

  /**
   * \brief Set the starting value for the DL MCS
   * \param v the value
   */
  void SetStartMcsDl (uint8_t v);
  /**
   * \brief Get the DL MCS starting value
   * \return the value
   */
  uint8_t GetStartMcsDl () const;

  /**
   * \brief Set the maximum index for the DL MCS
   * \param v the value
   */
  void SetMaxDlMcs (int8_t v);
  /**
   * \brief Get the maximum DL MCS index
   * \return the value
   */
  int8_t GetMaxDlMcs () const;

  /**
   * \brief Set the starting value for the UL MCS
   * \param v the value
   */
  void SetStartMcsUl (uint8_t v);
  /**
   * \brief Get the DL MCS starting value
   * \return the value
   */
  uint8_t GetStartMcsUl () const;

  /**
   * \brief Set the number of DL ctrl symbols
   * \param v number of DL ctrl symbols
   */
  void SetDlCtrlSyms (uint8_t v);

  /**
   * \brief Set the number of UL ctrl symbols
   * \param v number of UL ctrl symbols
   */
  void SetUlCtrlSyms (uint8_t v);

  /**
   * \brief Set the notched (blank) RBGs Mask for the DL
   * \param dlNotchedRbgsMask The mask of notched RBGs
   */
  void SetDlNotchedRbgMask (const std::vector<uint8_t> &dlNotchedRbgsMask);

  /**
   * \brief Get the notched (blank) RBGs Mask for the DL
   * \return The mask of notched RBGs
   */
  std::vector<uint8_t> GetDlNotchedRbgMask (void) const;

  /**
   * \brief Set the notched (blank) RBGs Mask for the UL
   * \param ulNotchedRbgsMask The mask of notched RBGs
   */
  void SetUlNotchedRbgMask (const std::vector<uint8_t> &ulNotchedRbgsMask);

  /**
   * \brief Get the notched (blank) RBGs Mask for the UL
   * \return The mask of notched RBGs
   */
  std::vector<uint8_t> GetUlNotchedRbgMask (void) const;

  /**
   * \brief Set the number of UL SRS symbols
   * \param v number of SRS symbols
   */
  void SetSrsCtrlSyms (uint8_t v);

  /**
   * \brief Get the configured value for the SRS symbols
   * \return the number of SRS symbols that will be allocated
   */
  uint8_t GetSrsCtrlSyms () const;

  /**
   * \brief Set if the UL slots are allowed for SRS transmission (if True, UL
   * and F slots may carry SRS, if False, SRS are transmitted only in F slots)
   * \param v the value
   */
  void SetSrsInUlSlots (bool v);

  /**
   * \brief Check if the UL slots are allowed for SRS transmission
   * \return true if UL slots are available for SRS (UL and F slots available
   * for SRS), false otherwise (only F slots available for SRS)
   */
  bool IsSrsInUlSlots () const;

  /**
   * \brief Set if the F slots are allowed for SRS transmission
   * \param v the value
   */
  void SetSrsInFSlots (bool v);

  /**
   * \brief Check if the F slots are allowed for SRS transmission
   * \return true if F slots are available for SRS, false otherwise
   */
  bool IsSrsInFSlots () const;
  /**
   * \brief Enable HARQ ReTx function
   *
   * Remember we introduced the EnableHarqReTx attribute only
   * for FB calibration example. We want to disable HARQ ReTx
   * because ReTx are scheduled in OFDMA fashion. In a TDMA simulation,
   * such retransmissions change the SINR trends in a scenario. Also,
   * REMEMBER that this solution of disabling the HARQ ReTx not very
   * optimized because gNB MAC will still buffer the packet and UE
   * would still transmit the HARQ feedback for the first transmission.
   *
   * \param enableFlag If true, it would set the max HARQ ReTx to 3; otherwise it set it to 0
   */
  void EnableHarqReTx (bool enableFlag);
  /**
   * \brief Is HARQ ReTx enable function
   *
   * \return Returns true if HARQ ReTx are enabled; otherwise false
   */
  bool IsHarqReTxEnable () const;

protected:
  /**
   * \brief Create an UE representation for the scheduler.
   *
   * The representation must save any important UE-specific value, and it is
   * shared across all the subclasses. A scheduler which want to save any
   * additional value per-UE, must subclass the class NrMacSchedulerUeInfo
   * and return a pointer to an instance of the new type.
   *
   * \param params Configure parameters for the UE
   * \return a pointer to the UE representation to save in the UE map (m_ueMap).
   */
  virtual std::shared_ptr<NrMacSchedulerUeInfo>
  CreateUeRepresentation (const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) const = 0;

  /**
   * \brief Returns TPC command
   */
  virtual uint8_t GetTpc () const = 0;

  /**
   * \brief Giving the input, append to slotAlloc the allocations for the DL HARQ retransmissions
   * \param startingPoint starting point of the first retransmission.
   * It should be set to the next available starting point
   * \param symAvail Available symbols
   * \param activeDlHarq Map of the active HARQ processes
   * \param ueMap Map of the UEs
   * \param dlHarqToRetransmit HARQ feedbacks that could not be transmitted (to fill)
   * \param dlHarqFeedback all the HARQ feedbacks
   * \param slotAlloc Slot allocation info
   * \return the VarTtiSlotAlloc ID to use next
   */
  virtual uint8_t ScheduleDlHarq (NrMacSchedulerNs3::PointInFTPlane *startingPoint,
                                  uint8_t symAvail,
                                  const ActiveHarqMap &activeDlHarq,
                                  const std::unordered_map<uint16_t, UePtr> &ueMap,
                                  std::vector<DlHarqInfo> *dlHarqToRetransmit,
                                  const std::vector<DlHarqInfo> &dlHarqFeedback,
                                  SlotAllocInfo *slotAlloc) const;
  /**
   * \brief Giving the input, append to slotAlloc the allocations for the DL HARQ retransmissions
   * \param startingPoint starting point of the first retransmission.
   * It should be set to the next available starting point
   * \param symAvail Available symbols
   * \param ueMap Map of the UEs
   * \param ulHarqToRetransmit HARQ feedbacks that could not be transmitted (to fill)
   * \param ulHarqFeedback all the HARQ feedbacks
   * \param slotAlloc Slot allocation info
   * \return the VarTtiSlotAlloc ID to use next
   */
  virtual uint8_t ScheduleUlHarq (NrMacSchedulerNs3::PointInFTPlane *startingPoint,
                                  uint8_t symAvail,
                                  const std::unordered_map<uint16_t, UePtr> &ueMap,
                                  std::vector<UlHarqInfo> *ulHarqToRetransmit,
                                  const std::vector<UlHarqInfo> &ulHarqFeedback,
                                  SlotAllocInfo *slotAlloc) const;

  /**
   * \brief Assign the DL RBG to the active UE, and return the distribution of symbols per beam
   * \param symAvail available symbols for DL
   * \param activeDl Map of Beam and active UE per beam
   * \return a map of symbols dedicated to each beam
   *
   * The function should assign RBG to each UE, modifying the value m_dlRBG
   * for each UE in the activeDl map. In doing so, it has to calculate the number
   * of symbols assigned to each beam, and return it to the caller.
   *
   * The creation of DCI will be performed by calling CreateDlDci with
   * the appropriate input parameters.
   */
  virtual BeamSymbolMap
  AssignDLRBG (uint32_t symAvail, const ActiveUeMap &activeDl) const = 0;

  /**
   * \brief Assign the UL RBG to the active UE, and return the distribution of symbols per beam
   * \param symAvail available symbols for UL
   * \param activeUl Map of Beam and active UE per beam
   * \return a map of symbols dedicated to each beam
   *
   * The function should assign RBG to each UE, modifying the value m_ulRBG
   * for each UE in the activeUl map. In doing so, it has to calculate the number
   * of symbols assigned to each UE, and return it to the caller.
   *
   * The creation of DCI will be performed by calling CreateDlDci with
   * the appropriate input parameters.
   */
  virtual BeamSymbolMap
  AssignULRBG (uint32_t symAvail, const ActiveUeMap &activeUl) const = 0;

  /**
   * \brief Create a DCI for the specified UE for DL data
   * \param spoint Starting point
   * \param ueInfo UE specified
   * \param maxSym maximum amount of symbols that can be assigned
   * \return a pointer to the DciInfoElementTdma
   *
   * The function should create a block in the 2D frequency-time plane in
   * which the specified UE will receive the DL data.
   */
  virtual std::shared_ptr<DciInfoElementTdma>
  CreateDlDci (PointInFTPlane *spoint, const std::shared_ptr<NrMacSchedulerUeInfo> &ueInfo,
               uint32_t maxSym) const = 0;

  /**
   * \brief Create a DCI for the specified UE for UL data
   * \param spoint Starting point
   * \param ueInfo UE specified
   * \param maxSym maximum amount of symbols that can be assigned
   * \return a pointer to the DciInfoElementTdma
   *
   * The function should create a block in the 2D frequency-time plane in
   * which the specified UE will receive the UL data.
   */
  virtual std::shared_ptr<DciInfoElementTdma>
  CreateUlDci (PointInFTPlane *spoint, const std::shared_ptr<NrMacSchedulerUeInfo> &ueInfo,
               uint32_t maxSym) const = 0;

  /**
   * \brief Perform a custom operation on the starting point each time all the UE of a DL beam have been scheduled
   * \param spoint starting point for the next beam to modify
   * \param symOfBeam number the symbol used for the beam
   */
  virtual void
  ChangeDlBeam (PointInFTPlane *spoint, uint32_t symOfBeam) const = 0;

  /**
   * \brief Perform a custom operation on the starting point each time all the UE of an UL beam have been scheduled
   * \param spoint starting point for the next beam to modify
   * \param symOfBeam number the symbol used for the beam
   */
  virtual void
  ChangeUlBeam (PointInFTPlane *spoint, uint32_t symOfBeam) const = 0;

  /**
   * \brief Sort the DL HARQ retransmission
   * \param activeDlHarq HARQ DL to retransmit
   *
   * The HARQ are divided by beams. In each beam, the HARQ should be ordered
   * in a way that the first element should be the first to transmit, and
   * then (if there is space) the second, the third, and so on.
   */
  virtual void SortDlHarq (ActiveHarqMap *activeDlHarq) const;

  /**
   * \brief Sort the UL HARQ retransmission
   * \param activeUlHarq HARQ UL to retransmit
   *
   * The HARQ are divided by beams. In each beam, the HARQ should be ordered
   * in a way that the first element should be the first to transmit, and
   * then (if there is space) the second, the third, and so on.
   */
  virtual void SortUlHarq (ActiveHarqMap *activeUlHarq) const;

  virtual LCGPtr
  CreateLCG (const LogicalChannelConfigListElement_s &config) const;

  virtual LCPtr
  CreateLC (const LogicalChannelConfigListElement_s &config) const;

  /**
   * \brief Private function that is used to get the number of resource
   * blocks per resource block group and also to check whether this value is
   * configured.
   * \return Returns the number of RBs per RBG
   */
  uint64_t GetNumRbPerRbg() const;

  /**
   * \brief Represent an assignation of bytes to a LCG/LC
   */
  struct Assignation
  {
    /**
     * \brief Assignation constructor (deleted)
     */
    Assignation () = delete;
    /**
     * \brief Assignation copy constructor (deleted)
     * \param o other instance
     */
    Assignation (const Assignation &o) = delete;
    /**
      * \brief Assignation move constructor (default)
      * \param o other instance
      */
    Assignation (Assignation &&o) = default;
    /**
     * \brief Assignation constructor with parameters
     * \param lcg LCG ID
     * \param lcId LC ID
     * \param bytes Assigned bytes
     */
    Assignation (uint8_t lcg, uint8_t lcId, uint32_t bytes)
      : m_lcg (lcg), m_lcId (lcId), m_bytes (bytes)
    {
    }
    /**
      * \brief Default deconstructor
      */
    ~Assignation () = default;

    uint8_t m_lcg    {0};  //!< LCG ID
    uint8_t m_lcId   {0};  //!< LC ID
    uint32_t m_bytes {0};  //!< Bytes assigned to the LC
  };

protected:
  Ptr<NrAmc> m_dlAmc; //!< AMC pointer
  Ptr<NrAmc> m_ulAmc; //!< AMC pointer

private:
  /**
   * \brief Single UL allocation for calculating CQI and the number of reserved UL symbols in slots.
   */
  struct AllocElem
  {
    /**
     * \brief AllocElem empty constructor (deleted)
     */
    AllocElem () = delete;
    /**
      * \brief AllocElement default copy constructor
      */
    AllocElem (const AllocElem &o) = default;
    /**
     * \brief AllocElem constructor
     * \param rnti RNTI
     * \param tbs Transport Block Size
     * \param numSym Number of symbols
     * \param mcs MCS
     */
    AllocElem (uint16_t rnti, uint32_t tbs, uint8_t symStart, uint8_t numSym, uint8_t mcs,
               const std::vector<uint8_t> &rbgMask)
      : m_rnti (rnti), m_tbs (tbs), m_symStart (symStart), m_numSym (numSym), m_mcs (mcs),
        m_rbgMask (rbgMask)
    {
    }

    uint16_t m_rnti {0};  //!< Allocated RNTI
    uint32_t m_tbs  {0};  //!< Allocated TBS
    uint8_t m_symStart {0}; //!< Sym start
    uint8_t m_numSym {0}; //!< Allocated symbols
    uint8_t m_mcs   {0};  //!< MCS of the transmission
    std::vector<uint8_t> m_rbgMask; //!< RBG Mask
  };

  /**
   * \brief A vector of UL allocations to calculate CQI and symbols reserved.
   */
  struct SlotElem
  {
    /**
     * \brief SlotElem default constructor (deleted)
     */
    SlotElem () = delete;
    /**
      * \brief SlotElem default copy constructor
      */
    SlotElem (const SlotElem &o) = default;
    /**
     * \brief SlotElem constructor with the number of total symbol used for UL
     * \param totUlSym symbols used for UL
     */
    SlotElem (uint8_t totUlSym)
      : m_totUlSym (totUlSym)
    {

    }

    uint8_t m_totUlSym;  //!< Total symbols used for UL
    std::vector<AllocElem> m_ulAllocations; //!< List of UL allocations
  };

  std::vector<Assignation>
  AssignBytesToLC (const std::unordered_map<uint8_t, LCGPtr> &ueLCG, uint32_t tbs) const;

  void BSRReceivedFromUe (const MacCeElement &bsr);

  template<typename T>
  std::vector<T> MergeHARQ (std::vector<T> *existingFeedbacks,
                            const std::vector<T> &inFeedbacks,
                            const std::string &mode) const;

  void ResetExpiredHARQ (uint16_t rnti, NrMacHarqVector *harq);

  template<typename T>
  void ProcessHARQFeedbacks (std::vector<T> *harqInfo,
                             const NrMacSchedulerUeInfo::GetHarqVectorFn &GetHarqVectorFn,
                             const std::string &direction) const;

  void
  ScheduleDl (const NrMacSchedSapProvider::SchedDlTriggerReqParameters& params,
              const std::vector <DlHarqInfo> &dlHarqInfo);

  void
  ScheduleUl (const NrMacSchedSapProvider::SchedUlTriggerReqParameters& params,
              const std::vector <UlHarqInfo> &ulHarqInfo);

  uint8_t AppendCtrlSym (uint8_t symStart, uint8_t numSymToAllocate,
                         DciInfoElementTdma::DciFormat mode,
                         std::deque<VarTtiAllocInfo> *allocations) const;
  uint8_t PrependCtrlSym (uint8_t symStart, uint8_t numSymToAllocate,
                          DciInfoElementTdma::DciFormat mode,
                          std::deque<VarTtiAllocInfo> *allocations) const;


  void ComputeActiveUe (ActiveUeMap *activeDlUe, const NrMacSchedulerUeInfo::GetLCGFn &GetLCGFn,
                        const NrMacSchedulerUeInfo::GetHarqVectorFn &GetHarqVector,
                        const std::string &mode) const;
  void ComputeActiveHarq (ActiveHarqMap *activeDlHarq, const std::vector <DlHarqInfo> &dlHarqFeedback) const;
  void ComputeActiveHarq (ActiveHarqMap *activeUlHarq, const std::vector <UlHarqInfo> &ulHarqFeedback) const;

  uint8_t DoScheduleDlData (PointInFTPlane *spoint, uint32_t symAvail,
                            const ActiveUeMap &activeDl, SlotAllocInfo *slotAlloc) const;
  uint8_t DoScheduleUlData (PointInFTPlane *spoint, uint32_t symAvail,
                            const ActiveUeMap &activeUl, SlotAllocInfo *slotAlloc) const;
  void DoScheduleUlSr (PointInFTPlane *spoint, const std::list<uint16_t> &rntiList) const;
  uint8_t DoScheduleDl (const std::vector <DlHarqInfo> &dlHarqFeedback, const ActiveHarqMap &activeDlHarq,
                        ActiveUeMap *activeDlUe, const SfnSf &dlSfnSf,
                        const SlotElem &ulAllocations, SlotAllocInfo *allocInfo);
  uint8_t DoScheduleUl (const std::vector <UlHarqInfo> &ulHarqFeedback, const SfnSf &ulSfn,
                        SlotAllocInfo *allocInfo, LteNrTddSlotType type);
  uint8_t DoScheduleSrs (PointInFTPlane *spoint, SlotAllocInfo *allocInfo);

  static const unsigned m_macHdrSize = 0;  //!< Mac Header size
  static const uint32_t m_subHdrSize = 4;  //!< Sub Header size (?)
  static const unsigned m_rlcHdrSize = 3;  //!< RLC Header size

protected:
  /**
   * \brief Get the bwp id of this MAC
   * \return the bwp id
   */
  uint16_t GetBwpId () const;

  /**
   * \brief Get the cell id of this MAC
   * \return the cell id
   */
  uint16_t GetCellId () const;

  /**
   * \return the bandwidth in RBG
   */
  uint16_t GetBandwidthInRbg () const;

private:
  std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > m_ueMap; //!< The map of between RNTI and their data

  /**
   * Map of previous allocated UE per RBG
   * (used to retrieve info from UL-CQI)
   */
  std::map <uint64_t, SlotElem> m_ulAllocationMap;

  bool    m_fixedMcsDl {false}; //!< Fixed MCS for *all* UE in DL
  bool    m_fixedMcsUl {false}; //!< Fixed MCS for *all* UE in UL
  uint8_t m_startMcsDl   {0};   //!< Starting (or fixed) value for DL MCS
  uint8_t m_startMcsUl   {0};   //!< Starting (or fixed) value for UL MCS
  int8_t m_maxDlMcs   {0};    //!< Maximum index for DL MCS
  Time    m_cqiTimersThreshold; //!< The time while a CQI is valid

  NrMacSchedulerCQIManagement m_cqiManagement; //!< CQI Management

  std::vector <DlHarqInfo> m_dlHarqToRetransmit; //!< List of DL HARQ that could not have been retransmitted
  std::vector <UlHarqInfo> m_ulHarqToRetransmit; //!< List of UL HARQ that could not have been retransmitted

  std::list<uint16_t> m_srList;  //!< List of RNTI of UEs that asked for a SR

  std::vector <struct RachListElement_s> m_rachList; //!< rach list

  uint16_t m_bandwidth {0}; //!< Bandwidth in number of RBG
  uint8_t m_dlCtrlSymbols {0}; //!< DL ctrl symbols (attribute)
  uint8_t m_ulCtrlSymbols {0}; //!< UL ctrl symbols (attribute)
  uint8_t m_srsCtrlSymbols {0}; //!< SRS symbols (attribute)
  bool m_enableSrsInUlSlots  {true}; //!< SRS allowed in UL slots (attribute)
  bool m_enableSrsInFSlots  {true}; //!< SRS allowed in F slots (attribute)

  std::vector<uint8_t> m_dlNotchedRbgsMask; //!< The mask of notched (blank) RBGs for the DL
  std::vector<uint8_t> m_ulNotchedRbgsMask; //!< The mask of notched (blank) RBGs for the UL

  std::unique_ptr <NrMacSchedulerHarqRr> m_schedHarq; //!< Pointer to the real HARQ scheduler

  Ptr<NrMacSchedulerSrsDefault> m_schedulerSrs; //!< Pointer to the SRS algorithm

  uint32_t m_srsSlotCounter {0}; //!< Counter for UL slots

  friend NrSchedGeneralTestCase;

  bool m_enableHarqReTx  {true}; //!< Flag to enable or disable HARQ ReTx (attribute)
};

} //namespace ns3
