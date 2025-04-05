/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * Cni assumes no responsibility whatsever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * 
 * We would appreciate acknowledgement if the software is used.
 * 
 * Cni ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * 
 * Modified by: CNI
 * It was tested under ns-3.28
 */


#ifndef Cni_URBANMICROCELL_PROPAGATION_LOSS_MODEL_H
#define Cni_URBANMICROCELL_PROPAGATION_LOSS_MODEL_H

#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-environment.h>
#include "ns3/sionna-helper.h"

namespace ns3 {

/**
 * \ingroup cni
 *
 * \brief Urbanmicrocell propagation model for the 6 GHz frequency (Public Safety use cases)
 * 
 * This class implements the outdoor propagation model for 6 GHz based on 3GPP sepcifications:
 * 3GPP TR 36.885 V14.0.0 (2016-06) / Section A.1.4
 */
class CniUrbanmicrocellPropagationLossModel : public PropagationLossModel
{

public:

  /**
   * structure to save the two nodes mobility models
   */
  struct MobilityDuo
  {
    Ptr<MobilityModel> a; ///< mobility model of node 1
    Ptr<MobilityModel> b; ///< mobility model of node 2
    
    /**
     * equality function
     * \param mo1 mobility model for node 1
     * \param mo2 mobility model for node 2
     */
    friend bool operator==(const MobilityDuo& mo1, const MobilityDuo& mo2)
    {
      return (mo1.a == mo2.a && mo1.b == mo2.b);
    }
    /**
     * less than function
     * \param mo1 mobility model for node 1
     * \param mo2 mobility model for node 2
     */
    friend bool operator<(const MobilityDuo& mo1, const MobilityDuo& mo2)
    {
      return (mo1.a < mo2.a || ( (mo1.a == mo2.a) && (mo1.b < mo2.b)));
    }

  };

  // inherited from Object
  static TypeId GetTypeId (void);
  CniUrbanmicrocellPropagationLossModel ();
  virtual ~CniUrbanmicrocellPropagationLossModel ();

  /** 
   * set the propagation frequency
   * 
   * \param freq 
   */
  void SetFrequency (double freq);

  /** 
   * Calculate the pathloss in dBm
   * 
   * \param a the first mobility model
   * \param b the second mobility model
   * 
   * \return the loss in dBm for the propagation between
   * the two given mobility models
   */
  double GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:

  // inherited from PropagationLossModel
  virtual double DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  virtual int64_t DoAssignStreams (int64_t stream);
  
  // The propagation frequency in Hz
  double m_frequency;
  bool m_isLosEnabled;
  Ptr<UniformRandomVariable> m_rand;
  // Map to keep track of random numbers generated per pair of nodes
  //mutable std::map<Ptr<MobilityModel>, std::map<Ptr<MobilityModel>, double> > m_randomMap;
  mutable std::map<MobilityDuo, double> m_randomMap;

};

} // namespace ns3


#endif // CV2X_Cni_URBANMICROCELL_PROPAGATION_LOSS_MODEL_H

