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
 * This is was inspired by parabolic-antenna-model.cc
 */

#include <ns3/log.h>
#include <ns3/double.h>
#include <cmath>

#include <ns3/antenna-model.h>
#include "cv2x_nist-parabolic-3d-antenna-model.h"


NS_LOG_COMPONENT_DEFINE ("cv2x_NistParabolic3dAntennaModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_NistParabolic3dAntennaModel);


TypeId 
cv2x_NistParabolic3dAntennaModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_NistParabolic3dAntennaModel")
    .SetParent<AntennaModel> ()
    .AddConstructor<cv2x_NistParabolic3dAntennaModel> ()
    .AddAttribute ("HorizontalBeamwidth",
                   "The 3dB beamwidth (degrees)",
                   DoubleValue (70),
                   MakeDoubleAccessor (&cv2x_NistParabolic3dAntennaModel::SetHorizontalBeamwidth,
                                       &cv2x_NistParabolic3dAntennaModel::GetHorizontalBeamwidth),
                   MakeDoubleChecker<double> (0, 180))
    .AddAttribute ("Orientation",
                   "The angle (degrees) that expresses the orientation of the antenna on the x-y plane relative to the x axis",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_NistParabolic3dAntennaModel::SetOrientation,
                                       &cv2x_NistParabolic3dAntennaModel::GetOrientation),
                   MakeDoubleChecker<double> (-360, 360))
    .AddAttribute ("MaxHorizontalAttenuation",
                   "The maximum horizontal attenuation (dB) of the antenna radiation pattern.",
                   DoubleValue (25.0),
                   MakeDoubleAccessor (&cv2x_NistParabolic3dAntennaModel::m_maxHAttenuation),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("VerticalBeamwidth",
                   "The 3dB beamwidth (degrees)",
                   DoubleValue (10),
                   MakeDoubleAccessor (&cv2x_NistParabolic3dAntennaModel::SetVerticalBeamwidth,
                                       &cv2x_NistParabolic3dAntennaModel::GetVerticalBeamwidth),
                   MakeDoubleChecker<double> (0, 180))
    .AddAttribute ("ElectricalTilt",
                   "The angle (degrees) that expresses the orientation of the antenna on the y-z plane relative to the y axis",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_NistParabolic3dAntennaModel::SetElectricalTilt,
                                       &cv2x_NistParabolic3dAntennaModel::GetElectricalTilt),
                   MakeDoubleChecker<double> (-90, 90))
    .AddAttribute ("MechanicalTilt",
                   "The angle (degrees) that expresses the orientation of the antenna on the y-z plane relative to the y axis",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_NistParabolic3dAntennaModel::SetMechanicalTilt,
                                       &cv2x_NistParabolic3dAntennaModel::GetMechanicalTilt),
                   MakeDoubleChecker<double> (-90, 90))
    .AddAttribute ("MaxVerticalAttenuation",
                   "The maximum vertical attenuation (dB) of the antenna radiation pattern.",
                   DoubleValue (20.0),
                   MakeDoubleAccessor (&cv2x_NistParabolic3dAntennaModel::m_maxVAttenuation),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

void 
cv2x_NistParabolic3dAntennaModel::SetHorizontalBeamwidth (double horizontalBeamwidthDegrees)
{ 
  NS_LOG_FUNCTION (this << horizontalBeamwidthDegrees);
  m_hBeamwidthRadians = DegreesToRadians (horizontalBeamwidthDegrees);
}

double
cv2x_NistParabolic3dAntennaModel::GetHorizontalBeamwidth () const
{
  return RadiansToDegrees (m_hBeamwidthRadians);
}

void 
cv2x_NistParabolic3dAntennaModel::SetOrientation (double orientationDegrees)
{
  NS_LOG_FUNCTION (this << orientationDegrees);
  m_orientationRadians = DegreesToRadians (orientationDegrees);
}

double
cv2x_NistParabolic3dAntennaModel::GetOrientation () const
{
  return RadiansToDegrees (m_orientationRadians);
}

void 
cv2x_NistParabolic3dAntennaModel::SetVerticalBeamwidth (double verticalBeamwidthDegrees)
{ 
  NS_LOG_FUNCTION (this << verticalBeamwidthDegrees);
  m_vBeamwidthRadians = DegreesToRadians (verticalBeamwidthDegrees);
}

double
cv2x_NistParabolic3dAntennaModel::GetVerticalBeamwidth () const
{
  return RadiansToDegrees (m_vBeamwidthRadians);
}

void 
cv2x_NistParabolic3dAntennaModel::SetElectricalTilt (double electricalTiltDegrees)
{
  NS_LOG_FUNCTION (this << electricalTiltDegrees);
  m_eTiltRadians = DegreesToRadians (electricalTiltDegrees);
}

double
cv2x_NistParabolic3dAntennaModel::GetElectricalTilt () const
{
  return RadiansToDegrees (m_eTiltRadians);
}

void 
cv2x_NistParabolic3dAntennaModel::SetMechanicalTilt (double mechanicalTiltDegrees)
{
  NS_LOG_FUNCTION (this << mechanicalTiltDegrees);
  m_mTiltRadians = DegreesToRadians (mechanicalTiltDegrees);
}

double
cv2x_NistParabolic3dAntennaModel::GetMechanicalTilt () const
{
  return RadiansToDegrees (m_mTiltRadians);
}


double 
cv2x_NistParabolic3dAntennaModel::GetGainDb (Angles a)
{
  NS_LOG_FUNCTION (this << a);

  double gainDb = -std::min (-(GetHorizontalGainDb(a)+GetVerticalGainDb(a)), m_maxHAttenuation);


  NS_LOG_LOGIC ("gain = " << gainDb);
  return gainDb;
}

double 
cv2x_NistParabolic3dAntennaModel::GetHorizontalGainDb (Angles a)
{
  NS_LOG_FUNCTION (this << a);
  
  // azimuth angle w.r.t. the reference system of the antenna
  double phi = a.GetAzimuth ()- m_orientationRadians;

  // make sure phi is in (-pi, pi]
  while (phi <= -M_PI)
    {
      phi += M_PI+M_PI;
    }
  while (phi > M_PI)
    {
      phi -= M_PI+M_PI;
    }

  NS_LOG_LOGIC ("phi = " << phi );

  double gainDb = -std::min (12 * pow (phi / m_hBeamwidthRadians, 2), m_maxHAttenuation);

  NS_LOG_LOGIC ("Horizontal gain = " << gainDb);
  return gainDb;
}

double 
cv2x_NistParabolic3dAntennaModel::GetVerticalGainDb (Angles a)
{
  NS_LOG_FUNCTION (this << a);
  // azimuth angle w.r.t. the reference system of the antenna
  double theta = a.GetInclination ()- M_PI/2 - (m_mTiltRadians + m_eTiltRadians);

  // make sure theta is in [-pi, pi]
  NS_ASSERT (theta <= M_PI && theta >= -M_PI);

  NS_LOG_LOGIC ("theta = " << theta );

  double gainDb = -std::min (12 * pow (theta / m_vBeamwidthRadians, 2), m_maxVAttenuation);

  NS_LOG_LOGIC ("Vertical gain = " << gainDb);
  return gainDb;
}

}

