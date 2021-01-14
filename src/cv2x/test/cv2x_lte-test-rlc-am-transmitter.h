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

#ifndef CV2X_LTE_TEST_RLC_AM_TRANSMITTER_H
#define CV2X_LTE_TEST_RLC_AM_TRANSMITTER_H

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
 * \brief TestSuite 4.1.1 RLC AM: Only transmitter functionality.
 */
class cv2x_LteRlcAmTransmitterTestSuite : public TestSuite
{
  public:
    cv2x_LteRlcAmTransmitterTestSuite ();
};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test case used by cv2x_LteRlcAmTransmitterOneSduTestCase to create topology 
 * and to implement functionalities and check if data received corresponds to 
 * data sent. 
 */
class cv2x_LteRlcAmTransmitterTestCase : public TestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the reference name
     */
    cv2x_LteRlcAmTransmitterTestCase (std::string name);
    cv2x_LteRlcAmTransmitterTestCase ();
    virtual ~cv2x_LteRlcAmTransmitterTestCase ();

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
     * \param shouldReceived shoul dhave received indicator
     * \param assertMsg the assert message
     */
    void DoCheckDataReceived (std::string shouldReceived, std::string assertMsg);

};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 4.1.1.1 Test that SDU transmitted at PDCP corresponds to PDU 
 * received by MAC.
 */
class cv2x_LteRlcAmTransmitterOneSduTestCase : public cv2x_LteRlcAmTransmitterTestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the reference name
     */
    cv2x_LteRlcAmTransmitterOneSduTestCase (std::string name);
    cv2x_LteRlcAmTransmitterOneSduTestCase ();
    virtual ~cv2x_LteRlcAmTransmitterOneSduTestCase ();

  private:
    virtual void DoRun (void);

};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 4.1.1.2 Test the correct functionality of the Segmentation. 
 * Test check that single SDU is properly segmented to n PDUs.
 */
class cv2x_LteRlcAmTransmitterSegmentationTestCase : public cv2x_LteRlcAmTransmitterTestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the reference name
     */
    cv2x_LteRlcAmTransmitterSegmentationTestCase (std::string name);
    cv2x_LteRlcAmTransmitterSegmentationTestCase ();
    virtual ~cv2x_LteRlcAmTransmitterSegmentationTestCase ();

  private:
    virtual void DoRun (void);

};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 4.1.1.3 Test that concatenation functionality works properly.
 * Test check if n SDUs are correctly contactenate to single PDU.
 */
class cv2x_LteRlcAmTransmitterConcatenationTestCase : public cv2x_LteRlcAmTransmitterTestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the reference name
     */
    cv2x_LteRlcAmTransmitterConcatenationTestCase (std::string name);
    cv2x_LteRlcAmTransmitterConcatenationTestCase ();
    virtual ~cv2x_LteRlcAmTransmitterConcatenationTestCase ();

  private:
    virtual void DoRun (void);

};

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 4.1.1.4 Test checks functionality of Report Buffer Status by 
 * testing primitive parameters.
 */
class cv2x_LteRlcAmTransmitterReportBufferStatusTestCase : public cv2x_LteRlcAmTransmitterTestCase
{
  public:
    /**
     * Constructor
     *
     * \param name the reference name
     */
    cv2x_LteRlcAmTransmitterReportBufferStatusTestCase (std::string name);
    cv2x_LteRlcAmTransmitterReportBufferStatusTestCase ();
    virtual ~cv2x_LteRlcAmTransmitterReportBufferStatusTestCase ();

  private:
    virtual void DoRun (void);

};

#endif // CV2X_LTE_TEST_RLC_AM_TRANSMITTER_H
