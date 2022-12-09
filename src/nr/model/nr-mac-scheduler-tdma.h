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

#include "nr-mac-scheduler-ns3.h"
#include <memory>
#include <functional>

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief The base for all the TDMA schedulers
 *
 * An example of TDMA-based scheduling is the following:
 * <pre>
 * (f)
 * ^
 * |=|===|===|===|===|=|
 * |C| U | U | U | U |C|
 * |T| E | E | E | E |T|
 * |R|   |   |   |   |R|
 * |L| 1 | 2 | 3 | 4 |L|
 * |----------------------------> (t)
 * </pre>
 *
 * The UEs are scheduled by prioritizing the assignment of symbols: the entire
 * available spectrum is assigned, for a number of symbols that depend on the
 * LC byte requirements.
 *
 * In order to construct a slot like the previous one, the class defines
 * a general algorithm, based on top of NrMacSchedulerNs3, to know
 * how many, and what, UEs to schedule. The NrMacSchedulerNs3::ScheduleDl
 * function needs three information that should be provided by the subclasses,
 * to answer the following three questions:
 *
 * - How distribute the symbols between beams?
 * - How many RBG should be assigned to the each active UE?
 * - How to place the blocks in the 2D plan (in other words, how to create the DCIs)?
 *
 * The first two are answered by the methods AssignDLRBG() and AssignULRBG().
 * The choice of what UE should be scheduled, and their order, is demanded to the subclasses.
 * For more information, please refer to the methods documentation.
 *
 * Subclasses should also implement what actions should be done before any
 * assigment (NrMacSchedulerTdma::BeforeDlSched and
 * NrMacSchedulerTdma::BeforeUlSched) as well as what to do after
 * a symbol has been assigned (for the UE that got that symbols, and the UE
 * that did not).
 *
 * The last one is answered by CreateDlDci() or CreateUlDci(), which call CreateDci()
 * to perform the "hard" work.
 *
 * \see NrMacSchedulerTdmaRR
 * \see NrMacSchedulerTdmaPF
 * \see NrMacSchedulerTdmaMR
 */
class NrMacSchedulerTdma : public NrMacSchedulerNs3
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrMacSchedulerTdma constructor
   */
  NrMacSchedulerTdma ();
  /**
   * \brief NrMacSchedulerTdma deconstructor
   */
  virtual ~NrMacSchedulerTdma () override;

protected:
  virtual BeamSymbolMap AssignDLRBG (uint32_t symAvail, const ActiveUeMap &activeDl) const override;

  virtual BeamSymbolMap AssignULRBG (uint32_t symAvail, const ActiveUeMap &activeUl) const override;
  virtual std::shared_ptr<DciInfoElementTdma>
  CreateDlDci (PointInFTPlane *spoint, const std::shared_ptr<NrMacSchedulerUeInfo> &ueInfo,
               uint32_t maxSym) const override;
  virtual std::shared_ptr<DciInfoElementTdma>
  CreateUlDci (PointInFTPlane *spoint, const std::shared_ptr<NrMacSchedulerUeInfo> &ueInfo,
               uint32_t maxSym) const override;

  /**
   * \brief Not doing anything, moving forward the spoint is done by CreateDci
   * \param spoint Starting point
   * \param symOfBeam the number of symbols assigned to the beam
   */
  virtual void
  ChangeDlBeam ([[maybe_unused]] PointInFTPlane *spoint, [[maybe_unused]] uint32_t symOfBeam) const override
  {
  }

  /**
   * \brief Not doing anything, moving forward the spoint is done by CreateDci
   * \param spoint Starting point
   * \param symOfBeam the number of symbols assigned to the beam
   */
  virtual void
  ChangeUlBeam ([[maybe_unused]] PointInFTPlane *spoint, [[maybe_unused]] uint32_t symOfBeam) const override
  {
  }

  virtual uint8_t GetTpc () const override;

  /**
   * \brief Provide the comparison function to order the UE when scheduling DL
   * \return a function that should order two UEs based on their priority: if
   * UE a is less than UE b, it will have an higher priority.
   */
  virtual std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareDlFn () const = 0;

  /**
   * \brief Provide the comparison function to order the UE when scheduling UL
   * \return a function that should order two UEs based on their priority: if
   * UE a is less than UE b, it will have an higher priority.
   */
  virtual std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareUlFn () const = 0;

  /**
   * \brief Update the UE representation after a symbol (DL) has been assigned to it
   * \param ue UE to which a symbol has been assigned
   * \param assigned the amount of resources assigned
   * \param totalAssigned the amount of total resources assigned until now
   *
   * After an UE is selected to be eligible for a symbol assignment, its representation
   * should be updated. The subclasses, by implementing this method, update
   * the representation by updating some custom values that reflect the assignment
   * done. These values are the one that, hopefully, are checked by the
   * comparison function returned by GetUeCompareDlFn().
   */
  virtual void AssignedDlResources (const UePtrAndBufferReq &ue,
                                    const FTResources &assigned,
                                    const FTResources &totalAssigned) const = 0;

  /**
   * \brief Update the UE representation after a symbol (DL) has been assigned to it
   * \param ue UE to which a symbol has been assigned
   * \param assigned the amount of resources assigned
   * \param totalAssigned the amount of total resources assigned until now
   *
   * After an UE is selected to be eligible for a symbol assignment, its representation
   * should be updated. The subclasses, by implementing this method, update
   * the representation by updating some custom values that reflect the assignment
   * done. These values are the one that, hopefully, are checked by the
   * comparison function returned by GetUeCompareUelFn().
   */
  virtual void AssignedUlResources (const UePtrAndBufferReq &ue,
                                    const FTResources &assigned,
                                    const FTResources &totalAssigned) const = 0;

  /**
   * \brief Update the UE representation after a symbol (DL) has been assigned to other UE
   * \param ue UE to which a symbol has not been assigned
   * \param notAssigned the amount of resources not assigned
   * \param totalAssigned the amount of total resources assigned until now
   */
  virtual void NotAssignedDlResources (const UePtrAndBufferReq &ue,
                                       const FTResources &notAssigned,
                                       const FTResources &totalAssigned) const = 0;

  /**
   * \brief Update the UE representation after a symbol (UL) has been assigned to other UE
   * \param ue UE to which a symbol has not been assigned
   * \param notAssigned the amount of resources not assigned
   * \param totalAssigned the amount of total resources assigned until now
   */
  virtual void NotAssignedUlResources (const UePtrAndBufferReq &ue,
                                       const FTResources &notAssigned,
                                       const FTResources &totalAssigned) const = 0;

  /**
   * \brief Prepare UE for the DL scheduling
   * \param ue UE that is eligible for an assignation in any iteration round
   * \param assignableInIteration Resources that can be assigned in each iteration
   *
   * The default implementation is empty, but a subclass can specialize the
   * behaviour, e.g., to calculate some value before the choice of RBG to
   * assign to each UE is done.
   */
  virtual void
  BeforeDlSched (const UePtrAndBufferReq &ue,
                 const FTResources &assignableInIteration) const = 0;

  /**
   * \brief Prepare UE for the UL scheduling
   * \param ue UE that is eligible for an assignation in any iteration round
   * \param assignableInIteration Resources that can be assigned in each iteration
   *
   * The default implementation is empty, but a subclass can specialize the
   * behaviour, e.g., to calculate some value before the choice of RBG to
   * assign to each UE is done.
   */
  virtual void
  BeforeUlSched (const UePtrAndBufferReq &ue,
                 const FTResources &assignableInIteration) const = 0;
private:
  /**
   * \brief Retrieve the UE vector from an ActiveUeMap
   * \param activeUes UE map
   * \return A Vector of UEs and their buffer requirements (in B)
   *
   * Really used only in TDMA scheduling. Worth moving?
   */
  static std::vector<UePtrAndBufferReq>
  GetUeVectorFromActiveUeMap (const ActiveUeMap &activeUes);

private:
  typedef std::function<void (const UePtrAndBufferReq &, const FTResources &)> BeforeSchedFn; //!< Before scheduling function
  /**
   * \brief //!< Function to notify a successfull assignment
   */
  typedef std::function<void (const UePtrAndBufferReq &, const FTResources &, const FTResources &)> AfterSuccessfullAssignmentFn;
  /**
   * \brief Function to notify that the UE did not get any resource in one iteration
   */
  typedef std::function<void (const UePtrAndBufferReq &, const FTResources &, const FTResources &)> AfterUnsucessfullAssignmentFn;
  typedef std::function<uint32_t& (const UePtr &ue)> GetRBGFn; //!< Getter for the RBG of an UE
  typedef std::function<uint32_t (const UePtr &ue)> GetTBSFn; //!< Getter for the TBS of an UE
  typedef std::function<uint8_t& (const UePtr &ue)> GetSymFn;  //!< Getter for the number of symbols of an UE
  typedef std::function<bool (const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                              const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )> CompareUeFn;
  typedef std::function<CompareUeFn ()> GetCompareUeFn;

  BeamSymbolMap
  AssignRBGTDMA (uint32_t symAvail, const ActiveUeMap &activeUe,
                 const std::string &type, const BeforeSchedFn &BeforeSchedFn,
                 const GetCompareUeFn &GetCompareFn, const GetTBSFn &GetTBSFn, const GetRBGFn &GetRBGFn,
                 const GetSymFn &GetSymFn, const AfterSuccessfullAssignmentFn &SuccessfullAssignmentFn,
                 const AfterUnsucessfullAssignmentFn &UnSuccessfullAssignmentFn) const;


  std::shared_ptr<DciInfoElementTdma> CreateDci (PointInFTPlane *spoint, const std::shared_ptr<NrMacSchedulerUeInfo> &ueInfo,
                                                 std::vector<uint32_t> tbs, DciInfoElementTdma::DciFormat fmt,
                                                 std::vector<uint8_t> mcs, std::vector<uint8_t> ndi,
                                                 std::vector<uint8_t> rv, uint8_t numSym) const;
};

} // namespace ns3
