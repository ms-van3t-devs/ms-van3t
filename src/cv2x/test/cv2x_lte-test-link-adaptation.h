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

#ifndef CV2X_LTE_TEST_LINK_ADAPTATION_H
#define CV2X_LTE_TEST_LINK_ADAPTATION_H

#include "ns3/test.h"


using namespace ns3;


/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 1.3 Link adaptation
 */
class cv2x_LteLinkAdaptationTestSuite : public TestSuite
{
public:
  cv2x_LteLinkAdaptationTestSuite ();
};


/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test that LTE link adaptation works according to the theoretical model.
 */
class cv2x_LteLinkAdaptationTestCase : public TestCase
{
public:
  /**
   * Constructor
   *
   * \param name the reference name
   * \param snrDb the SNR in dB
   * \param loss the loss
   * \param mcsIndex the DL se
   */
  cv2x_LteLinkAdaptationTestCase (std::string name, double snrDb, double loss, uint16_t mcsIndex);
  cv2x_LteLinkAdaptationTestCase ();
  virtual ~cv2x_LteLinkAdaptationTestCase ();

  /**
   * \brief DL scheduling function
   * \param dlInfo the DL info
   */
  void DlScheduling (cv2x_DlSchedulingCallbackInfo dlInfo);

private:
  virtual void DoRun (void);

  double m_snrDb; ///< the SNR in dB
  double m_loss; ///< the loss
  uint16_t m_mcsIndex; ///< the MCS index
};

#endif /* CV2X_LTE_TEST_LINK_ADAPTATION_H */
