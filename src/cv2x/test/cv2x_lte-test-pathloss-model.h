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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 */

#ifndef CV2X_LTE_TEST_PATHLOSS_MODEL_H
#define CV2X_LTE_TEST_PATHLOSS_MODEL_H

#include "ns3/spectrum-value.h"

#include "ns3/test.h"

#include <ns3/buildings-propagation-loss-model.h>


using namespace ns3;


/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief Test 1.1 pathloss calculation
 */
class cv2x_LtePathlossModelTestSuite : public TestSuite
{
public:
  cv2x_LtePathlossModelTestSuite ();
};


/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief  Tests that the BuildingPathlossModel works according to 
 * the expected theoretical values. Theoretical reference values 
 * are obtained with the octave script src/lte/test/reference/lte_pathloss.m
 */
class cv2x_LtePathlossModelSystemTestCase : public TestCase
{
  public:
  /**
   * Constructor
   *
   * \param name the reference name
   * \param snrDb the SNR in dB
   * \param dist the distance
   * \param mcsIndex the MCS index
   */
    cv2x_LtePathlossModelSystemTestCase (std::string name, double snrDb, double dist, uint16_t mcsIndex);
    cv2x_LtePathlossModelSystemTestCase ();
    virtual ~cv2x_LtePathlossModelSystemTestCase ();
    
  /**
   * \brief DL scheduling function
   * \param dlInfo the DL info
   */
    void DlScheduling (cv2x_DlSchedulingCallbackInfo dlInfo);
                       
  private:
    virtual void DoRun (void);
    
    double m_snrDb; ///< the SNR in dB
    double m_distance; ///< the distance
    uint16_t m_mcsIndex; ///< the MCS index
};

#endif /* CV2X_LTE_TEST_PATHLOSS_MODEL_H */

