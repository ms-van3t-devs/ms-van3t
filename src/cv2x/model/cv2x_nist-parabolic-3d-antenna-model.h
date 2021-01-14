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
 * This is was inspired by parabolic-antenna-model.h
 */

#ifndef CV2X_NIST_PARABOLIC_3D_ANTENNA_MODEL_H
#define CV2X_NIST_PARABOLIC_3D_ANTENNA_MODEL_H


#include <ns3/object.h>
#include <ns3/antenna-model.h>

namespace ns3 {

/**
 * 
 * \brief  Antenna model based on a parabolic approximation of the main lobe radiation pattern.
 *
 * This class implements the parabolic model as described in 3GPP document TR 36.814
 *
 */
class cv2x_NistParabolic3dAntennaModel : public AntennaModel
{
public:

  // inherited from Object
  static TypeId GetTypeId ();

  // inherited from AntennaModel
  virtual double GetGainDb (Angles a);


  /** 
   * Sets the horizontal beamwidth
   * \param horizontalBeamwidthDegrees The horizontal beamwidth in degrees
   */
  void SetHorizontalBeamwidth (double horizontalBeamwidthDegrees);

  /**
   * Gets the horizontal beamwidth
   * \return The horizontal beamwidth
   */
  double GetHorizontalBeamwidth () const;

  /** 
   * Sets the orientation (azimuth)
   * \param orientationDegrees The orientation (azimuth) in degrees
   */
  void SetOrientation (double orientationDegrees);

  /**
   * Gets the horizontal beamwidth
   * \return The horizontal beamwidth
   */
  double GetOrientation () const;

  /** 
   * Sets the vertical beamwidth
   * \param verticalBeamwidthDegrees The vetical beamwidth in degrees
   */
  void SetVerticalBeamwidth (double verticalBeamwidthDegrees);

  /**
   * Gets the vertical beamwidth
   * \return The vertical beamwidth
   */
  double GetVerticalBeamwidth () const;

  /** 
   * Sets the electrical tilt
   * \param electricalTiltDegrees The electrical tilt in degrees
   */
  void SetElectricalTilt (double electricalTiltDegrees);

  /**
   * Gets the electrical tilt
   * \return The electrical tilt
   */
  double GetElectricalTilt () const;

  /** 
   * Sets the mechanical tilt
   * \param mechanicalTiltDegrees The mechinacal tilt in degrees
   */
  void SetMechanicalTilt (double mechanicalTiltDegrees);

  /**
   * Gets the mechanical tilt
   * \return The mechanical tilt
   */
  double GetMechanicalTilt () const;

private:

  //Computes the gain on horizontal plane
  double GetHorizontalGainDb (Angles a);
  //Computes the gain on the vertical plane
  double GetVerticalGainDb (Angles a);

  double m_hBeamwidthRadians;

  double m_orientationRadians;

  double m_maxHAttenuation;

  double m_vBeamwidthRadians;

  double m_eTiltRadians;

  double m_mTiltRadians;

  double m_maxVAttenuation;

};



} // namespace ns3


#endif // CV2X_NIST_PARABOLIC_3D_ANTENNA_MODEL_H
