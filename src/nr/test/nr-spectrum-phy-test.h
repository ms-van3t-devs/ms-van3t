/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

// An essential include is test.h
#include "ns3/test.h"
#include "ns3/spectrum-propagation-loss-model.h"

/**
 * \ingroup test
 * \file test-nr-spectrum-phy.h
 *
 * \brief This test sets two times noise figure and checks if this setting is applied
 * correctly to connected classes of SpectrumPhy, i.e. SpectrumModel, SpectrumValue,
 * SpectrumChannel etc.
 */

namespace ns3 {

class MobilityModel;

/**
 * \ingroup test
 * \brief No loss spectrum propagation loss model created for testing purposes.
 * As its name says, there are no losses.
 */
class NoLossSpectrumPropagationLossModel : public SpectrumPropagationLossModel
{
public:

  NoLossSpectrumPropagationLossModel ();
  virtual ~NoLossSpectrumPropagationLossModel ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

private:

  virtual Ptr<SpectrumValue> DoCalcRxPowerSpectralDensity (Ptr<const SpectrumValue> txPsd,
                                                           Ptr<const MobilityModel> a,
                                                           Ptr<const MobilityModel> b) const override;
};

class SetNoisePsdTestCase : public TestCase
{
public:
  /** Constructor. */
  SetNoisePsdTestCase (double txPower, double bandwidth, double noiseFigureFirst, double noiseFigureSecond,
                       double expectedSnrFirst, double expectedSnrSecond, uint8_t numerology);
  /** Destructor. */
  virtual ~SetNoisePsdTestCase ();

  /**
   * \brief Save SNR value in the list of values
   */
  void SaveSnr (double snr);

private:

  /**
   * \brief Run test case
   */
  virtual void DoRun (void);

  /**
   * \brief Check if the test case has pass or failed
   */
  void DoEvaluateTest ();

  std::vector<double> m_snr; //!< list of SNR values that are used if the test has passed or failed
  double m_txPower; //!< transmission power
  double m_bandwidth; //!< system bandwidth in Hz
  double m_noiseFigureFirst; //!< noise figure that is used for the first configuration of the spectrum phy
  double m_noiseFigureSecond; //!< noise figure that is used for the second configuration of the spectrum phy
  double m_expectedSnrFirst; //!< expected SNR value when configured the first noise figure value
  double m_expectedSnrSecond; //!< expected SNR value when configured the second noise figure value
  uint8_t m_numerology; //!< numerology to be used to create spectrum phy
};

/**
 * \ingroup test
 * The test suite that runs different test cases to test NrSpectrumPhy.
 */
class NrSpectrumPhyTestSuite : public TestSuite
{
public:
  /** Constructor. */
  NrSpectrumPhyTestSuite ();
};


}  // namespace ns3
