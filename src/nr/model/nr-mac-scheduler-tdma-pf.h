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
#include "nr-mac-scheduler-tdma-rr.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief Assign entire symbols in a proportional fair fashion
 *
 * Sort the UE by their current throughput. Details in the class
 * NrMacSchedulerUeInfoPF.
 */
class NrMacSchedulerTdmaPF : public NrMacSchedulerTdmaRR
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);
  /**
   * \brief NrMacSchedulerTdmaPF constructor
   */
  NrMacSchedulerTdmaPF ();

  /**
   * \brief ~NrMacSchedulerTdmaPF deconstructor
   */
  virtual ~NrMacSchedulerTdmaPF () override
  {
  }

  /**
   * \brief Set the value of attribute "FairnessIndex"
   * \param v
   */
  void SetFairnessIndex (double v);

  /**
   * \brief Get the value of attribute "FairnessIndex"
   * @return
   */
  double GetFairnessIndex () const;

  /**
   * \brief Set the attribute "LastAvgTPutWeight"
   * \param v the value to save
   */
  void SetTimeWindow (double v);
  /**
   * \brief Get the attribute "LastAvgTPutWeight"
   * \return the value of the attribute
   */
  double GetTimeWindow () const;

protected:
  // inherit
  /**
   * \brief Create an UE representation of the type NrMacSchedulerUeInfoPF
   * \param params parameters
   * \return NrMacSchedulerUeInfoRR instance
   */
  virtual std::shared_ptr<NrMacSchedulerUeInfo>
  CreateUeRepresentation (const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) const override;

  /**
   * \brief Return the comparison function to sort DL UE according to the scheduler policy
   * \return a pointer to NrMacSchedulerUeInfoPF::CompareUeWeightsDl
   */
  virtual std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareDlFn () const override;

  /**
   * \brief Return the comparison function to sort UL UE according to the scheduler policy
   * \return a pointer to NrMacSchedulerUeInfoPF::CompareUeWeightsUl
   */
  virtual std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareUlFn () const override;

  /**
   * \brief Update DL metrics by calling NrMacSchedulerUeInfoPF::UpdatePFDlMetric
   * \param ue UE to update
   * \param assigned the amount of resources assigned
   * \param totAssigned the total amount of resources assigned in the slot
   *
   * The DL metrics (current Throughput and average Throughput) will be updated
   * by calling the NrMacSchedulerUeInfoPF::UpdateDlPFMetric, which in turn will
   * call NrMacSchedulerUeInfo::UpdateDlMetric in order to get the tbSize
   * based on the resources assigned to the user. This will help the sorting
   * function to sort the UEs for resource allocation.
   */
  virtual void AssignedDlResources (const UePtrAndBufferReq &ue,
                                    const FTResources &assigned,
                                    const FTResources &totAssigned) const override;

  /**
   * \brief Update DL metrics by calling NrMacSchedulerUeInfoPF::UpdatePFDlMetric
   * \param ue UE to update (ue that didn't get any resources)
   * \param notAssigned the amount of resources not assigned
   * \param totAssigned the total amount of resources assigned in the slot
   *
   * Even if the UE did not get any resource assigned, change its current throughput
   * over the total number of symbols assigned.
   *
   * The DL metrics (current Throughput and average Throughput) will be updated
   * by calling the NrMacSchedulerUeInfoPF::UpdateDlPFMetric, which in turn will
   * call NrMacSchedulerUeInfo::UpdateDlMetric in order to get the tbSize
   * based on the resources assigned to the user. Since no resources have been
   * assigned, the tbSize will be zero. This will help the sorting function to
   * sort the UEs for resource allocation.
   */
  virtual void NotAssignedDlResources (const UePtrAndBufferReq &ue,
                                       const FTResources &notAssigned,
                                       const FTResources &totAssigned) const override;

  /**
   * \brief Update UL metrics by calling NrMacSchedulerUeInfoPF::UpdatePFUlMetric
   * \param ue UE to update
   * \param assigned the amount of resources assigned
   * \param totAssigned the total amount of resources assigned in the slot
   *
   * The UL metrics (current Throughput and average Throughput) will be updated
   * by calling the NrMacSchedulerUeInfoPF::UpdateUlPFMetric, which in turn will
   * call NrMacSchedulerUeInfo::UpdateUlMetric in order to get the tbSize
   * based on the resources assigned to the user. This will help the sorting
   * function to sort the UEs for resource allocation.
   */
  virtual void AssignedUlResources (const UePtrAndBufferReq &ue,
                                    const FTResources &assigned,
                                    const FTResources &totAssigned) const override;

  /**
   * \brief Update UL metrics by calling NrMacSchedulerUeInfoPF::UpdatePFUlMetric
   * \param ue UE to update (ue that didn't get any resources)
   * \param notAssigned the amount of resources not assigned
   * \param totAssigned the total amount of resources assigned in the slot
   *
   * Even if the UE did not get any resource assigned, change its current throughput
   * over the total number of symbols assigned.
   *
   * The UL metrics (current Throughput and average Throughput) will be updated
   * by calling the NrMacSchedulerUeInfoPF::UpdateUlPFMetric, which in turn will
   * call NrMacSchedulerUeInfo::UpdateUlMetric in order to get the tbSize
   * based on the resources assigned to the user. Since no resources have been
   * assigned, the tbSize will be zero. This will help the sorting function to
   * sort the UEs for resource allocation.
   */
  virtual void NotAssignedUlResources (const UePtrAndBufferReq &ue,
                                       const FTResources &notAssigned,
                                       const FTResources &totAssigned) const override;

  /**
   * \brief Calculate the potential throughtput for the DL based on the available resources
   * \param ue UE to which a symbol has been assigned
   * \param assignableInIteration the minimum amount of resources to be assigned
   *
   * Calculates the the potential throughput by calling
   * NrMacSchedulerUeInfoPF::CalculatePotentialTPutDl.
   */
  virtual void
  BeforeDlSched (const UePtrAndBufferReq &ue,
                 const FTResources &assignableInIteration) const override;

  /**
   * \brief Calculate the potential throughput for the UL based on the available resources
   * \param ue UE to which a symbol has been assigned
   * \param assignableInIteration the minimum amount of resources to be assigned
   *
   * Calculates the the potential throughput by calling
   * NrMacSchedulerUeInfoPF::CalculatePotentialTPutUl.
   */
  virtual void
  BeforeUlSched (const UePtrAndBufferReq &ue,
                 const FTResources &assignableInIteration) const override;

private:
  double m_timeWindow {99.0}; //!< Time window to calculate the throughput. Better to make it an attribute.
  double m_alpha {0.0}; //!< PF Fairness index
};

} // namespace ns3
