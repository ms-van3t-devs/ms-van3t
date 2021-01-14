/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *         Marco Miozzo <marco.miozzo@cttc.es>
 */

#ifndef CV2X_FF_MAC_SCHEDULER_H
#define CV2X_FF_MAC_SCHEDULER_H

#include <ns3/object.h>


namespace ns3 {



class cv2x_FfMacCschedSapUser;
class cv2x_FfMacSchedSapUser;
class cv2x_FfMacCschedSapProvider;
class cv2x_FfMacSchedSapProvider;
class cv2x_LteFfrSapProvider;
class cv2x_LteFfrSapUser;

/**
 * \ingroup lte
 * \defgroup ff-api FF MAC Schedulers
 */
     
/**
 * \ingroup ff-api
 *
 * This abstract base class identifies the interface by means of which
 * the helper object can plug on the MAC a scheduler implementation based on the
 * FF MAC Sched API.
 *
 *
 */
class cv2x_FfMacScheduler : public Object
{
public:
  /**
  * The type of UL CQI to be filtered (ALL means accept all the CQI,
  * where a new CQI of any type overwrite the old one, even of another type)
  *
  */
  enum UlCqiFilter_t
  {
    SRS_UL_CQI,
    PUSCH_UL_CQI,
    ALL_UL_CQI
  };
  /**
  * constructor
  *
  */
  cv2x_FfMacScheduler ();
  /**
   * destructor
   *
   */
  virtual ~cv2x_FfMacScheduler ();

  // inherited from Object
  virtual void DoDispose (void);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * set the user part of the FfMacCschedSap that this Scheduler will
   * interact with. Normally this part of the SAP is exported by the MAC.
   *
   * \param s
   */
  virtual void SetFfMacCschedSapUser (cv2x_FfMacCschedSapUser* s) = 0;

  /**
   *
   * set the user part of the FfMacSchedSap that this Scheduler will
   * interact with. Normally this part of the SAP is exported by the MAC.
   *
   * \param s
   */
  virtual void SetFfMacSchedSapUser (cv2x_FfMacSchedSapUser* s) = 0;

  /**
   *
   * \return the Provider part of the FfMacCschedSap provided by the Scheduler
   */
  virtual cv2x_FfMacCschedSapProvider* GetFfMacCschedSapProvider () = 0;

  /**
   *
   * \return the Provider part of the FfMacSchedSap provided by the Scheduler
   */
  virtual cv2x_FfMacSchedSapProvider* GetFfMacSchedSapProvider () = 0;

  //FFR SAPs
  /**
   *
   * Set the Provider part of the LteFfrSap that this Scheduler will
   * interact with
   *
   * \param s
   */
  virtual void SetLteFfrSapProvider (cv2x_LteFfrSapProvider* s) = 0;

  /**
   *
   * \return the User part of the LteFfrSap provided by the FfrAlgorithm
   */
  virtual cv2x_LteFfrSapUser* GetLteFfrSapUser () = 0;
  
protected:
    
  UlCqiFilter_t m_ulCqiFilter; ///< UL CQI filter

};

}  // namespace ns3

#endif /* CV2X_FF_MAC_SCHEDULER_H */
