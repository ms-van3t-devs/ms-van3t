/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Alexander Krotov
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
 * Author: Alexander Krotov <krotov@iitp.ru>
 *
 */

#ifndef CV2X_LTE_TEST_SECONDARY_CELL_SELECTION_H
#define CV2X_LTE_TEST_SECONDARY_CELL_SELECTION_H

#include <ns3/test.h>
#include <ns3/nstime.h>
#include <ns3/node-container.h>
#include <ns3/vector.h>
#include <ns3/cv2x_lte-ue-rrc.h>
#include <vector>

using namespace ns3;

/**
 * \brief Test suite for executing the secondary cell selection test cases.
 *
 * \sa ns3::cv2x_LteSecondaryCellSelectionTestCase
 */
class cv2x_LteSecondaryCellSelectionTestSuite : public TestSuite
{
public:
  cv2x_LteSecondaryCellSelectionTestSuite ();
};




/**
 * \ingroup lte
 *
 * \brief Testing the initial cell selection procedure by UE at IDLE state in
 *        the beginning of simulation with multiple component carriers.
 */
class cv2x_LteSecondaryCellSelectionTestCase : public TestCase
{
public:
  /**
   * \brief Creates an instance of the initial cell selection test case.
   * \param name name of this test
   * \param isIdealRrc if true, simulation uses Ideal RRC protocol, otherwise
   *                   simulation uses Real RRC protocol
   * \param rngRun the number of run to be used by the random number generator
   * \param numberOfComponentCarriers number of component carriers
   */
  cv2x_LteSecondaryCellSelectionTestCase (std::string name, bool isIdealRrc, uint64_t rngRun, uint8_t numberOfComponentCarriers);

  virtual ~cv2x_LteSecondaryCellSelectionTestCase ();

private:
  /**
   * \brief Setup the simulation according to the configuration set by the
   *        class constructor, run it, and verify the result.
   */
  virtual void DoRun ();

  /**
   * \brief State transition callback function
   * \param context the context string
   * \param imsi the IMSI
   * \param cellId the cell ID
   * \param rnti the RNTI
   * \param oldState the old state
   * \param newState the new state
   */
  void StateTransitionCallback (std::string context, uint64_t imsi,
                                uint16_t cellId, uint16_t rnti,
                                cv2x_LteUeRrc::State oldState, cv2x_LteUeRrc::State newState);
  /**
   * \brief Initial cell selection end ok callback function
   * \param context the context string
   * \param imsi the IMSI
   * \param cellId the cell ID
   */
  void InitialSecondaryCellSelectionEndOkCallback (std::string context, uint64_t imsi,
                                          uint16_t cellId);
  /**
   * \brief Connection established callback function
   * \param context the context string
   * \param imsi the IMSI
   * \param cellId the cell ID
   * \param rnti the RNTI
   */
  void ConnectionEstablishedCallback (std::string context, uint64_t imsi,
                                      uint16_t cellId, uint16_t rnti);

  bool m_isIdealRrc; ///< whether the LTE is configured to use ideal RRC
  uint64_t m_rngRun; ///< rng run
  uint8_t m_numberOfComponentCarriers; ///< number of component carriers

  /// The current UE RRC state.
  std::map<uint64_t, cv2x_LteUeRrc::State> m_lastState;

}; // end of class cv2x_LteSecondaryCellSelectionTestCase

#endif /* CV2X_LTE_TEST_SECONDARY_CELL_SELECTION_H */
