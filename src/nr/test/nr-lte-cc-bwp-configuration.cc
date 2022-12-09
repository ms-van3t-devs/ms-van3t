/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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


#include <ns3/test.h>
#include <ns3/object-factory.h>
#include <ns3/cc-bwp-helper.h>
#include "ns3/nr-module.h"
#include <memory>


/**
 * \file nr-lte-cc-bwp-configuration.cc
 * \ingroup test
 *
 * \brief The test aims at proving that the creation of operation bands, component carriers
 * (CC) and bandwidth parts (BWP) is correct within the limitations of the NR
 * implementation. The main limitation of BWPs is that they do not overlap,
 * because in such case, the interference calculation would be erroneous. This test
 * also proves that the creation of BWP information with the CcBwpHelper is correct.
 */


namespace ns3 {



class CcBwpTestCase : public TestCase
{
public:

  enum OperationMode
  {
    UNCONF = 0,
    TDD = 1,
    FDD = 2
  };

  /**
   * \brief Create LtePatternTestCase
   * \param name Name of the test
   */
  CcBwpTestCase (const std::string &name)
    : TestCase (name)
  {}

private:
  virtual void DoRun (void) override;

  /**
   * \brief Test the automatic creation of a single operation band
   *
   * This test gets a frequency band configuration and determines whether it is a
   * valid a configuration or not. The validation of the configuration is
   * performed at band, carrier and bandwidth part level. All BWPs are either
   * TDD or FDD.
   *
   * \param bandConfig The band properties (operation frequency, bandwidth, numCcs and numBwps)
   * \param mode The operation mode: [TDD,FDD]
   */
  void TestBandConfiguration (double centralFrequency,
                              uint32_t bandwidth,
                              uint8_t numBandCcs,
                              OperationMode mode);

  /**
   * \brief Test the created operation bands
   *
   * This test gets all the operation band configuration descriptors containing
   * all the configured BWPs and validates them. Validation is done one band at
   * a time
   *
   * \param operationBands Operation band vector (operation frequency, bandwidth, CCs and BWPs)
   */
  void TestMultiBandConfiguration (const std::vector<std::reference_wrapper<OperationBandInfo> > &operationBands);

  /**
   * \brief Test that the automatic creation of OperationBandInfo is consistent
   * with the configuration information in each element of the operationBandConfigs
   *
   * \param operationBandConfigs Operation band configurations (operation frequency, bandwidth and CCs)
   */
  void TestCcBwpNumbers (std::vector<std::reference_wrapper<CcBwpCreator::SimpleOperationBandConf> > &operationBandConfigs);

  void TestUeBwps (double centralFrequency,
                   uint32_t bandwidth,
                   uint8_t numerology,
                   uint8_t numCcs,
                   OperationMode mode,
                   std::vector<uint8_t> activeBwp);

  /**
   * \brief Validates a number of created operation bands
   *
   * This test aims at determining if the created operation bands are correct.
   * The configured bands are correct if they do not overlap and their individual
   * CC information is consistent (all of them inside the band bounds)
   *
   * \return Whether the test passed or failed
   */
  bool ValidateCaBwpConfiguration (const std::vector<std::reference_wrapper<OperationBandInfo> > &operationBands);

  /**
   * \brief Validates a configured operation band
   *
   * This test aims at determining if the band's CC information is consistent
   * (CCs do not overlap and all CCs are inside the band's bounds)
   *
   * \return Whether the test passed or failed
   */
  bool ValidateOperationBand (const OperationBandInfo &band);

  /**
   * \brief Validates a list of configured component carriers
   *
   * This test aims at determining if BWPs contained in the CC are correctly
   * configured (BWPs do not overlap and go out of CC bounds)
   *
   * \return Whether the test passed or failed
   */
  bool CheckBwpsInCc (const ComponentCarrierInfoPtr &cc);

};

/**
 * \brief The NrLteCcBwpTestSuite class
 */
class NrLteCcBwpTestSuite : public TestSuite
{
public:
  NrLteCcBwpTestSuite () : TestSuite ("nr-lte-cc-bwp-configuration", UNIT)
  {
    AddTestCase (new CcBwpTestCase ("CC and BWP test"), QUICK);
  }
};

static NrLteCcBwpTestSuite nrLteCcBwpTestSuite; //!< CC BWP test suite


void
CcBwpTestCase::DoRun ()
{

  CcBwpCreator creator;

  /**
   * Create different SimpleOperationBandConf with some common parameters
   */
  double centralFrequency1 = 27e9;
  double bandwidth1 = 1e9;
  uint8_t numCcPerBand1 = 4;
  OperationMode mode1 = TDD;

  double centralFrequency2 = 28e9;
  double bandwidth2 = 1e9;
  uint8_t numCcPerBand2 = 4;
  OperationMode mode2 = FDD;

  /**
   * The first set of tests consist of checking the consistency of the automatic
   * generation of CCs, depending whether the OperationMode is TDD or FDD
   */

  TestBandConfiguration (centralFrequency1,
                         bandwidth1,
                         numCcPerBand1 * static_cast<uint8_t> (mode1),
                         mode1); //!< First test

  TestBandConfiguration (centralFrequency2,
                         bandwidth2,
                         numCcPerBand2 * static_cast<uint8_t> (mode2),
                         mode2); //!< Second test

  CcBwpCreator::SimpleOperationBandConf bandConf1 (centralFrequency1,
                                                   bandwidth1,
                                                   numCcPerBand1 * static_cast<uint8_t> (mode1),
                                                   BandwidthPartInfo::UMi_StreetCanyon);

  CcBwpCreator::SimpleOperationBandConf bandConf2 (centralFrequency2,
                                                   bandwidth2,
                                                   numCcPerBand2 * static_cast<uint8_t> (mode2),
                                                   BandwidthPartInfo::UMi_StreetCanyon);

  OperationBandInfo band1 = creator.CreateOperationBandContiguousCc (bandConf1);
  OperationBandInfo band2 = creator.CreateOperationBandContiguousCc (bandConf2);

  TestMultiBandConfiguration ({band1,band2});  //!< Third test


  std::vector<std::reference_wrapper<CcBwpCreator::SimpleOperationBandConf> > operationBandConfigs = {bandConf1,bandConf2};
  TestCcBwpNumbers (operationBandConfigs); //!< Fourth test

}


void
CcBwpTestCase::TestBandConfiguration (double centralFrequency,
                                      uint32_t bandwidth,
                                      uint8_t numBandCcs,
                                      OperationMode mode)
{

  CcBwpCreator creator;

  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency,
                                                  bandwidth,
                                                  numBandCcs * static_cast<uint8_t> (mode),
                                                  BandwidthPartInfo::UMi_StreetCanyon);

  OperationBandInfo band = creator.CreateOperationBandContiguousCc (bandConf);

  bool ret = ValidateOperationBand (band);
  NS_TEST_ASSERT_MSG_EQ (ret, true, "Frequency configuration is not valid");

}


void
CcBwpTestCase::TestMultiBandConfiguration (const std::vector<std::reference_wrapper<OperationBandInfo> > &operationBands)
{
  bool ret = ValidateCaBwpConfiguration (operationBands);
  NS_TEST_ASSERT_MSG_EQ (ret, true, "Frequency configuration is not valid");
}


void
CcBwpTestCase::TestCcBwpNumbers (std::vector<std::reference_wrapper<CcBwpCreator::SimpleOperationBandConf> > &operationBandConfigs)
{

  NS_TEST_ASSERT_MSG_GT (operationBandConfigs.size (), 0, "No operation band info");

  CcBwpCreator creator;

  for (uint16_t i = 0; i < operationBandConfigs.size (); ++i)
    {
      CcBwpCreator::SimpleOperationBandConf bandConfig = operationBandConfigs.at (i);
      OperationBandInfo band =  creator.CreateOperationBandContiguousCc (bandConfig);

      NS_TEST_ASSERT_MSG_EQ (band.m_cc.size (), bandConfig.m_numCc, "Unexpected number of CCs");

      uint16_t numBwp = 0;
      for (const auto & cc : band.m_cc)
        {
          numBwp += cc.get ()->m_bwp.size ();
        }

      NS_TEST_ASSERT_MSG_EQ (numBwp, bandConfig.m_numCc, "Unexpected number of BWPs");
    }
}


bool
CcBwpTestCase::ValidateCaBwpConfiguration (const std::vector<std::reference_wrapper<OperationBandInfo> > &operationBands)
{
  if (operationBands.empty () == true)
    {
      std::cout << "No band information has been provided" << std::endl;
      return false;
    }

  uint16_t numBands = operationBands.size ();

  /*
   *  First validation: Operation bands shall not overlap
   */
  if (numBands > 1)
    {
      for (uint16_t i = 0; i < numBands - 1; ++i)
        {
          if (operationBands.at (i).get ().m_higherFrequency > operationBands.at (i + 1).get ().m_lowerFrequency)
            {
              std::cout << "Bands overlap" << std::endl;
              return false;
            }
        }
    }

  /*
   *  Second validation: Check that the CC configuration is valid (assume bands
   *  are sorted by increasing operation frequency)
   */
  for (const auto & band : operationBands)
    {
      ValidateOperationBand (band.get ());
    }

  return true;
}


bool
CcBwpTestCase::ValidateOperationBand (const OperationBandInfo &band)
{

  NS_ABORT_MSG_IF (band.m_cc.empty (),"No CC information provided");
  uint8_t numCcs = band.m_cc.size ();

  // Loop checks if CCs are overlap and contiguous or not
  uint8_t c = 0;
  while (c < numCcs - 1)
    {
      double freqSeparation = band.m_cc.at (c + 1)->m_lowerFrequency - band.m_cc.at (c)->m_higherFrequency < 0;
      if (freqSeparation < 0)
        {
          std::cout << "CCs overlap" << std::endl;
//            NS_TEST_ASSERT_MSG_EQ (freqSeparation, 1.0, "CCs overlap");
          return false;
        }
      ++c;
    }

  // Check if each CC has BWP configuration and validate them
  for (const auto & cc : band.m_cc)
    {
      if (CheckBwpsInCc (cc) == false)
        {
          return false;
        }
    }

  return true;
}

bool
CcBwpTestCase::CheckBwpsInCc (const ComponentCarrierInfoPtr &cc)
{
  //  //First check: number of BWP shall be larger than 0
  if (cc->m_bwp.empty () == true)
    {
      std::cout << "No BWPs inside the CC" << std::endl;
      return false;
    }

  // Second check: BWP shall not exceed CC limits and the sum of BWPs cannot be larger than the CC bandwidth
  double totalBandwidth = 0;
  for (const auto & bwp : cc->m_bwp)
    {
      if (bwp->m_higherFrequency > cc->m_higherFrequency
          || bwp->m_lowerFrequency < cc->m_lowerFrequency)
        {
          std::cout << "BWP part is out of the CC bounds" << std::endl;
          return false;
        }
      totalBandwidth += bwp->m_channelBandwidth;
    }

  if (totalBandwidth > cc->m_channelBandwidth)
    {
      std::cout << "Aggregated BWP is larger than carrier bandwidth" << std::endl;
      return false;
    }

  // Third check: BWPs shall not overlap in frequency
  uint16_t numBwps = cc->m_bwp.size ();
  for (uint16_t a = 0; a < numBwps - 1; a++)
    {
      if (cc->m_bwp.at (a)->m_higherFrequency > cc->m_bwp.at (a + 1)->m_lowerFrequency)
        {
          std::cout << "BWPs shall not overlap" << std::endl;
          return false;
        }
    }

  return true;
}

}  // namespace ns3
