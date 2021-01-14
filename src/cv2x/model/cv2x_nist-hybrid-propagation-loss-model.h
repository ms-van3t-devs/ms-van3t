/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * 
 * We would appreciate acknowledgement if the software is used.
 * 
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * 
 * Modified by: NIST
 * It was tested under ns-3.22
 */


#ifndef CV2X_NIST_HYBRID_PROPAGATION_LOSS_MODEL_H
#define CV2X_NIST_HYBRID_PROPAGATION_LOSS_MODEL_H

#include <ns3/propagation-loss-model.h>

namespace ns3 {


/**
 * \ingroup nist
 *
 * \brief Hybrid (outdoor to indoor) propagation model for the 700 MHz frequency (Public Safety use cases)
 * 
 * This class implements the hybrid propagation model for 700 MHz based on 3GPP sepcifications:
 * 3GPP TR 36.843 V12.0.1 (2014-03) / Section A.2.1.2 
 */
class cv2x_NistHybridPropagationLossModel : public PropagationLossModel
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
  cv2x_NistHybridPropagationLossModel ();
  virtual ~cv2x_NistHybridPropagationLossModel ();

  /** 
   * \param a the first mobility model
   * \param b the second mobility model
   * 
   * \return the loss in dBm for the propagation between
   * the two given mobility models
   */
  double GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:

  // inherited from PropagationLossModel
  virtual double DoCalcRxPower (double txPowerDbm,
                                Ptr<MobilityModel> a,
                                Ptr<MobilityModel> b) const;
  virtual int64_t DoAssignStreams (int64_t stream);
  
  double m_frequency;
  Ptr<UniformRandomVariable> m_rand;
  mutable std::map<MobilityDuo, double> m_randomMap;
};

} // namespace ns3


#endif // CV2X_NIST_HYBRID_PROPAGATION_LOSS_MODEL_H



