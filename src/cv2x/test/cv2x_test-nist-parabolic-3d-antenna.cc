/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.

 * We would appreciate acknowledgement if the software is used.

 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 *
 * Modified by: NIST
 */

#include <ns3/log.h>
#include <ns3/test.h>
#include <ns3/double.h>
#include <ns3/cv2x_nist-parabolic-3d-antenna-model.h>
#include <ns3/simulator.h>
#include <cmath>
#include <string>
#include <iostream>
#include <sstream>


NS_LOG_COMPONENT_DEFINE ("cv2x_TestNistParabolic3dAntennaModel");

using namespace ns3;

enum cv2x_NistParabolic3dAntennaModelGainTestCondition  {
  EQUAL = 0,
  LESSTHAN = 1
};

class cv2x_NistParabolic3dAntennaModelTestCase : public TestCase
{
public:
  static std::string BuildNameString (Angles a, double hb, double o, double hg, double vb, double e, double m, double vg);
  cv2x_NistParabolic3dAntennaModelTestCase (Angles a, double hb, double o, double hg, double vb, double e, double m, double vg, double expectedGainDb, cv2x_NistParabolic3dAntennaModelGainTestCondition cond);


private:
  virtual void DoRun (void);

  Angles m_a;
  double m_hb;
  double m_o;
  double m_hg;
  double m_vb;
  double m_e;
  double m_m;
  double m_vg;
  double m_expectedGain;
  cv2x_NistParabolic3dAntennaModelGainTestCondition m_cond;
};

std::string cv2x_NistParabolic3dAntennaModelTestCase::BuildNameString (Angles a, double hb, double o, double hg, double vb, double e, double m, double vg)
{
  std::ostringstream oss;
  oss <<  "theta=" << a.GetInclination ()<< " , phi=" << a.GetAzimuth ()
      << ", horizontal beamdwidth=" << hb << " deg"
      << ", orientation=" << o 
      << ", horizontal maxAttenuation=" << hg << " dB"
      << ", vertical beamwidth=" << vb << " deg"
      << ", electrical tilt= " << e << " deg"
      << ", mechanical tilt= " << m << " deg"
      << ", vertical maxAttenuation=" << vb << " dB"
;
  return oss.str ();
}


cv2x_NistParabolic3dAntennaModelTestCase::cv2x_NistParabolic3dAntennaModelTestCase (Angles a, double hb, double o, double hg, double vb, double e, double m, double vg, double expectedGainDb, cv2x_NistParabolic3dAntennaModelGainTestCondition cond)
  : TestCase (BuildNameString (a, hb, o, hg, vb, e, m, vg)),
    m_a (a),
    m_hb (hb),
    m_o (o),
    m_hg (hg),
    m_vb (vb),
    m_e (e),
    m_m (m),
    m_vg (vg),
    m_expectedGain (expectedGainDb),
    m_cond (cond)
{
}

void
cv2x_NistParabolic3dAntennaModelTestCase::DoRun ()
{
  NS_LOG_FUNCTION (this << BuildNameString (m_a, m_hb, m_o, m_hg, m_vb, m_e, m_m, m_vg));

  Ptr<cv2x_NistParabolic3dAntennaModel> a = CreateObject<cv2x_NistParabolic3dAntennaModel> ();
  a->SetAttribute ("HorizontalBeamwidth", DoubleValue (m_hb));
  a->SetAttribute ("Orientation", DoubleValue (m_o));
  a->SetAttribute ("MaxHorizontalAttenuation", DoubleValue (m_hg));
  a->SetAttribute ("VerticalBeamwidth", DoubleValue (m_vb));
  a->SetAttribute ("ElectricalTilt", DoubleValue (m_e));
  a->SetAttribute ("MechanicalTilt", DoubleValue (m_m));
  a->SetAttribute ("MaxVerticalAttenuation", DoubleValue (m_vg));
  double actualGain = a->GetGainDb (m_a);
  switch (m_cond) 
    {
    case EQUAL:
      NS_TEST_EXPECT_MSG_EQ_TOL (actualGain, m_expectedGain, 0.001, "wrong value of the radiation pattern");
      break;
    case LESSTHAN:
      NS_TEST_EXPECT_MSG_LT (actualGain, m_expectedGain, "gain higher than expected");
      break;
    default:
      break;
    }
}




class cv2x_NistParabolic3dAntennaModelTestSuite : public TestSuite
{
public:
  cv2x_NistParabolic3dAntennaModelTestSuite ();
};

cv2x_NistParabolic3dAntennaModelTestSuite::cv2x_NistParabolic3dAntennaModelTestSuite ()
  : TestSuite ("parabolic-3d-antenna-model", UNIT)
{ 

  // with a 60 deg beamwidth, gain is -20dB at +-77.460 degrees from boresight
  // testing horizontal plane
  //                                                                         phi,                     theta, h beamwidth, orientation, h maxAttn, v beamwidth, etilt, mtilt, v maxAttn, expectedGain,   condition
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,            0,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (30),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (-30),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (-90),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,          -25,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (90),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,          -25,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (100),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,          -25,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (150),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,          -25,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (180),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,          -25,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians (-100),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,          -25,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians (-150),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,          -25,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians (-180),    DegreesToRadians(90)),        60,           0,       25,           10,     0,     0,        20,          -25,     EQUAL), TestCase::QUICK);
  
  // with a 60 deg beamwidth, gain is -10dB at +-54.772 degrees from boresight
  // test positive orientation
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (60),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,            0,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (90),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (30),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (-30),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,          -15,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (150),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,          -15,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (160),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,          -15,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (210),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,          -15,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (240),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,          -15,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (-40),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,          -15,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (-90),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,          -15,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians (-120),    DegreesToRadians(90)),        60,           60,       15,           10,     0,     0,        20,          -15,     EQUAL), TestCase::QUICK);

  // test negative orientation and different beamwidths
  // with a 80 deg beamwidth, gain is -20dB at +- 73.030 degrees from boresight
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians (-150),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,            0,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians (-110),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians (-190),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (-70),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,          -10,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (92),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,          -10,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians  (-30),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,          -10,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,          -10,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (60),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,          -10,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (90),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,          -10,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (90),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,          -10,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians   (30),    DegreesToRadians(90)),        80,         -150,       10,           10,     0,     0,        20,          -10,     EQUAL), TestCase::QUICK);

  // testing vertical plane
  //                                                                         phi,                     theta, h beamwidth, orientation, h maxAttn, v beamwidth, etilt, mtilt, v maxAttn, expectedGain,   condition
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians  (90)),        60,           0,       25,           10,     0,     0,        20,            0,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians  (85)),        60,           0,       25,           10,     0,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians  (95)),        60,           0,       25,           10,     0,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians   (0)),        60,           0,       25,           10,     0,     0,        20,          -20,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians (180)),        60,           0,       25,           10,     0,     0,        20,          -20,     EQUAL), TestCase::QUICK);

  // testing tilt
  
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians  (95)),        60,           0,       25,           10,     5,     0,        20,            0,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians  (95)),        60,           0,       25,           10,     0,     5,        20,            0,     EQUAL), TestCase::QUICK);    
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians  (90)),        60,           0,       25,           10,     5,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians  (90)),        60,           0,       25,           10,     0,     5,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians (100)),        60,           0,       25,           10,     5,     0,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians (100)),        60,           0,       25,           10,     0,     5,        20,           -3,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians   (0)),        60,           0,       25,           10,     5,     0,        20,          -20,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians   (0)),        60,           0,       25,           10,     0,     5,        20,          -20,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians (180)),        60,           0,       25,           10,     5,     0,        20,          -20,     EQUAL), TestCase::QUICK);
  AddTestCase (new cv2x_NistParabolic3dAntennaModelTestCase (Angles (DegreesToRadians    (0),  DegreesToRadians (180)),        60,           0,       25,           10,     0,     5,        20,          -20,     EQUAL), TestCase::QUICK);


};

static cv2x_NistParabolic3dAntennaModelTestSuite staticNistParabolic3dAntennaModelTestSuiteInstance;
