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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#ifndef CV2X_LTE_TEST_RLC_UM_TRANSMITTER_H
#define CV2X_LTE_TEST_RLC_UM_TRANSMITTER_H

#include "ns3/test.h"

namespace ns3 {

class cv2x_LteTestRrc;
class cv2x_LteTestMac;
class cv2x_LteTestPdcp;

}

using namespace ns3;

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief TestSuite 4.1.1 for RLC UM: Only transmitter part.
 */
class cv2x_LteRlcUmTransmitterTestSuite : public TestSuite
{
  public:
    cv2x_LteRlcUmTransmitterTestSuite ();
};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test case used by cv2x_LteRlcUmTransmitterOneSduTestCase to create topology 
 * and to implement functionalities and check if data received corresponds to 
 * data sent. 
 */
class cv2x_LteRlcUmTransmitterTestCase : public TestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the test name
     */
    cv2x_LteRlcUmTransmitterTestCase (std::string name);
    cv2x_LteRlcUmTransmitterTestCase ();
    virtual ~cv2x_LteRlcUmTransmitterTestCase ();

    /**
     * Check data received function
     * \param time the time to check
     * \param shouldReceived shoul dhave received indicator
     * \param assertMsg the assert message
     */
    void CheckDataReceived (Time time, std::string shouldReceived, std::string assertMsg);

  protected:
    virtual void DoRun (void);

    Ptr<cv2x_LteTestPdcp> txPdcp; ///< the transmit PDCP
    Ptr<cv2x_LteRlc> txRlc; ///< the RLC
    Ptr<cv2x_LteTestMac> txMac; ///< the MAC

  private:
    /**
     * Check data received function
     * \param shouldReceived should have received indicator
     * \param assertMsg the assert message
     */
    void DoCheckDataReceived (std::string shouldReceived, std::string assertMsg);

};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 4.1.1.1 One SDU, One PDU
 */
class cv2x_LteRlcUmTransmitterOneSduTestCase : public cv2x_LteRlcUmTransmitterTestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the test name
     */
    cv2x_LteRlcUmTransmitterOneSduTestCase (std::string name);
    cv2x_LteRlcUmTransmitterOneSduTestCase ();
    virtual ~cv2x_LteRlcUmTransmitterOneSduTestCase ();

  private:
    virtual void DoRun (void);

};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 4.1.1.2 Segmentation (One SDU => n PDUs)
 */
class cv2x_LteRlcUmTransmitterSegmentationTestCase : public cv2x_LteRlcUmTransmitterTestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the reference name
     */
    cv2x_LteRlcUmTransmitterSegmentationTestCase (std::string name);
    cv2x_LteRlcUmTransmitterSegmentationTestCase ();
    virtual ~cv2x_LteRlcUmTransmitterSegmentationTestCase ();

  private:
    virtual void DoRun (void);

};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 4.1.1.3 Concatenation (n SDUs => One PDU)
 */
class cv2x_LteRlcUmTransmitterConcatenationTestCase : public cv2x_LteRlcUmTransmitterTestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the reference name
     */
    cv2x_LteRlcUmTransmitterConcatenationTestCase (std::string name);
    cv2x_LteRlcUmTransmitterConcatenationTestCase ();
    virtual ~cv2x_LteRlcUmTransmitterConcatenationTestCase ();

  private:
    virtual void DoRun (void);

};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 4.1.1.4 Report Buffer Status (test primitive parameters)
 */
class cv2x_LteRlcUmTransmitterReportBufferStatusTestCase : public cv2x_LteRlcUmTransmitterTestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the reference name
     */
    cv2x_LteRlcUmTransmitterReportBufferStatusTestCase (std::string name);
    cv2x_LteRlcUmTransmitterReportBufferStatusTestCase ();
    virtual ~cv2x_LteRlcUmTransmitterReportBufferStatusTestCase ();

  private:
    virtual void DoRun (void);

};

#endif /* CV2X_LTE_TEST_RLC_UM_TRANSMITTER_H */
