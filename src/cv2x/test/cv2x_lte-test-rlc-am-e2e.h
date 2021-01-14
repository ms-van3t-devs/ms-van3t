/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 */

#ifndef CV2X_LTE_TEST_RLC_AM_E2E_H
#define CV2X_LTE_TEST_RLC_AM_E2E_H

#include "ns3/test.h"


using namespace ns3;


/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test suite for RlcAmE2e test case.
 */
class cv2x_LteRlcAmE2eTestSuite : public TestSuite
{
public:
  cv2x_LteRlcAmE2eTestSuite ();
};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * Test cases used for the test suite lte-rlc-am-e2e. See the testing section of
 * the LTE module documentation for details.  
 */
class cv2x_LteRlcAmE2eTestCase : public TestCase
{
  public:
  /**
   * Constructor
   *
   * \param name the reference name
   * \param seed the random variable seed
   * \param losses the error rate
   * \param bulkSduArrival true if bulk SDU arrival
   */
    cv2x_LteRlcAmE2eTestCase (std::string name, uint32_t seed, double losses, bool bulkSduArrival);
    cv2x_LteRlcAmE2eTestCase ();
    virtual ~cv2x_LteRlcAmE2eTestCase ();

  private:
    virtual void DoRun (void);

    /**
     * DL drop event
     * \param p the packet
     */
    void DlDropEvent (Ptr<const Packet> p);
    /**
     * UL drop event
     * \param p the packet
     */
    void UlDropEvent (Ptr<const Packet> p);

    uint32_t m_run; ///< rng run
    double   m_losses; ///< error rate
    bool m_bulkSduArrival; ///< bulk SDU arrival

    uint32_t m_dlDrops; ///< number of Dl drops
    uint32_t m_ulDrops; ///< number of UL drops

};

#endif // CV2X_LTE_TEST_RLC_AM_E2E_H
