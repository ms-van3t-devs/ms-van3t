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
#include <ns3/nr-gnb-phy.h>

/**
 * \file nr-lte-pattern-generation.cc
 * \ingroup test
 *
 * \brief The test considers the function NrGnbPhy::GenerateStructuresFromPattern
 * and checks that the output of that function is equal to the one pre-defined.
 * Test includes also the Harq feedback indication.
 */
namespace ns3 {

/**
 * \ingroup test
 * \brief TestSched testcase
 */
class LtePatternTestCase : public TestCase
{
public:

  /**
   * \brief The result in a single struct
   */
  struct Result
  {
    std::map<uint32_t, std::vector<uint32_t> > m_toSendDl;
    std::map<uint32_t, std::vector<uint32_t> > m_toSendUl;
    std::map<uint32_t, std::vector<uint32_t> > m_generateDl;
    std::map<uint32_t, std::vector<uint32_t> > m_generateUl;
    std::map<uint32_t, uint32_t> m_dlHarqFb;
  };


  /**
   * \brief The harqResult in a single struct
   */
  struct HarqResult
  {
    std::map<uint32_t, uint32_t> m_dlHarq;
  };

  /**
   * \brief Create LtePatternTestCase
   * \param name Name of the test
   */
  LtePatternTestCase (const std::string &name)
    : TestCase (name)
  {}

  /**
   * \brief Check if two maps are equal
   * \param a first map
   * \param b second map
   */
  void CheckMap (const std::map<uint32_t, std::vector<uint32_t> > &a,
                 const std::map<uint32_t, std::vector<uint32_t> > &b);

  /**
   * \brief Check if two maps of the Harq indication are equal
   * \param a first map
   * \param b second map
   */
  void CheckHarqMap (const std::map<uint32_t, uint32_t> &a,
                     const std::map<uint32_t, uint32_t> &b);

  /**
   * \brief Check if two vectors are equal
   * \param a first vector
   * \param b second vector
   */
  void CheckVector (const std::vector<uint32_t> &a,
                    const std::vector<uint32_t> &b);

private:
  virtual void DoRun (void) override;
  /**
   * \brief Test the output of PHY for a pattern, and compares it to the input
   * \param pattern The pattern to test
   * \param result The theoretical result
   */
  void TestPattern (const std::vector<LteNrTddSlotType> &pattern, const Result &result);

  /**
   * \brief Print the map
   * \param str the map to print
   */
  void Print (const std::map<uint32_t, std::vector<uint32_t> > &str);

  /**
   * \brief Print the Harq feedback map
   * \param str the map to print
   */
  void PrintHarq (const std::map<uint32_t, uint32_t> &str);

  bool m_verbose = false; //!< Print the generated structure
};


void
LtePatternTestCase::DoRun ()
{
  auto one = {LteNrTddSlotType::DL,
              LteNrTddSlotType::S,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::DL,
              LteNrTddSlotType::DL,
              LteNrTddSlotType::S,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::DL,};

  Result a = { {
                 { 0, {0, } },
                 { 1, {0, } },
                 { 4, {0, } },
                 { 5, {0, } },
                 { 6, {0, } },
                 { 9, {0, } },
               },
               {
                 { 0, {2, } },
                 { 1, {2, } },
                 { 5, {2, } },
                 { 6, {2, } },
               },
               {
                 { 2, {2, } },
                 { 3, {2, } },
                 { 4, {2, } },
                 { 7, {2, } },
                 { 8, {2, } },
                 { 9, {2, } },
               },
               {
                 { 3, {4, } },
                 { 4, {4, } },
                 { 8, {4, } },
                 { 9, {4, } },
               },
               {
                 {0, 7},
                 {1, 6},
                 {4, 4},
                 {5, 7},
                 {6, 6},
                 {9, 4},
               }};

  TestPattern (one, a);

  Result b = {
    {
      { 0, {0, } },
      { 1, {0, } },
      { 3, {0, } },
      { 4, {0, } },
      { 5, {0, } },
      { 6, {0, } },
      { 8, {0, } },
      { 9, {0, } },
    },
    {
      { 0, {2, } },
      { 5, {2, } },
    },
    {
      { 1, {2, } },
      { 2, {2, } },
      { 3, {2, } },
      { 4, {2, } },
      { 6, {2, } },
      { 7, {2, } },
      { 8, {2, } },
      { 9, {2, } },
    },
    {
      { 3, {4, } },
      { 8, {4, } },
    },
    {
      {0, 7},
      {1, 6},
      {3, 4},
      {4, 8},
      {5, 7},
      {6, 6},
      {8, 4},
      {9, 8},
    }
  };
  auto two = {LteNrTddSlotType::DL,
              LteNrTddSlotType::S,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::DL,
              LteNrTddSlotType::DL,
              LteNrTddSlotType::DL,
              LteNrTddSlotType::S,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::DL,
              LteNrTddSlotType::DL,};

  TestPattern (two, b);

  Result c = {
    {
      { 0, {0, } },
      { 1, {0, } },
      { 5, {0, } },
      { 6, {0, } },
      { 7, {0, } },
      { 8, {0, } },
      { 9, {0, } },
    },
    {
      { 0, {2, } },
      { 1, {2, 3, } },
    },
    {
      { 3, {2, } },
      { 4, {2, } },
      { 5, {2, } },
      { 6, {2, } },
      { 7, {2, } },
      { 8, {2, } },
      { 9, {2, } },
    },
    {
      { 8, {4, } },
      { 9, {4, 5, } },
    },
    {
      {0, 4},
      {1, 11},
      {5, 7},
      {6, 6},
      {7, 5},
      {8, 4},
      {9, 4},
    }
  };
  auto three = {LteNrTddSlotType::DL,
                LteNrTddSlotType::S,
                LteNrTddSlotType::UL,
                LteNrTddSlotType::UL,
                LteNrTddSlotType::UL,
                LteNrTddSlotType::DL,
                LteNrTddSlotType::DL,
                LteNrTddSlotType::DL,
                LteNrTddSlotType::DL,
                LteNrTddSlotType::DL,};

  TestPattern (three, c);

  Result d = {
    {
      { 0, {0, } },
      { 1, {0, } },
      { 4, {0, } },
      { 5, {0, } },
      { 6, {0, } },
      { 7, {0, } },
      { 8, {0, } },
      { 9, {0, } },
    },
    {
      { 0, {2, } },
      { 1, {2, } },
    },
    {
      { 2, {2, } },
      { 3, {2, } },
      { 4, {2, } },
      { 5, {2, } },
      { 6, {2, } },
      { 7, {2, } },
      { 8, {2, } },
      { 9, {2, } },
    },
    {
      { 8, {4, } },
      { 9, {4, } },
    },
    {
      {0, 12},
      {1, 11},
      {4, 8},
      {5, 7},
      {6, 6},
      {7, 5},
      {8, 4},
      {9, 4},
    }
  };
  auto four = {LteNrTddSlotType::DL,
               LteNrTddSlotType::S,
               LteNrTddSlotType::UL,
               LteNrTddSlotType::UL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,};

  TestPattern (four, d);

  Result e = {
    {
      { 0, {0, } },
      { 1, {0, } },
      { 3, {0, } },
      { 4, {0, } },
      { 5, {0, } },
      { 6, {0, } },
      { 7, {0, } },
      { 8, {0, } },
      { 9, {0, } },
    },
    {
      { 0, {2, } },
    },
    {
      { 1, {2, } },
      { 2, {2, } },
      { 3, {2, } },
      { 4, {2, } },
      { 5, {2, } },
      { 6, {2, } },
      { 7, {2, } },
      { 8, {2, } },
      { 9, {2, } },
    },
    {
      { 8, {4, } },
    },
    {
      {0, 12},
      {1, 11},
      {3, 9},
      {4, 8},
      {5, 7},
      {6, 6},
      {7, 5},
      {8, 4},
      {9, 13},
    }
  };
  auto five = {LteNrTddSlotType::DL,
               LteNrTddSlotType::S,
               LteNrTddSlotType::UL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::DL,};

  TestPattern (five, e);

  Result f = {
    {
      { 0, {0, } },
      { 1, {0, } },
      { 5, {0, } },
      { 6, {0, } },
      { 9, {0, } },
    },
    {
      { 0, {2, } },
      { 1, {2, 3, } },
      { 5, {2, } },
      { 6, {2, } },
    },
    {
      { 3, {2, } },
      { 4, {2, } },
      { 7, {2, } },
      { 8, {2, } },
      { 9, {2, } },
    },
    {
      { 3, {4, } },
      { 4, {4, } },
      { 8, {4, } },
      { 9, {4, 5, } },
    },
    {
      {0, 4},
      {1, 6},
      {5, 7},
      {6, 6},
      {9, 4},
    }
  };
  auto six = {LteNrTddSlotType::DL,
              LteNrTddSlotType::S,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::DL,
              LteNrTddSlotType::S,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::UL,
              LteNrTddSlotType::DL,};
  TestPattern (six, f);

  Result g = {
    {
      { 0, {0, } },
      { 1, {0, } },
      { 5, {0, } },
      { 6, {0, } },
    },
    {
      { 0, {2, } },
      { 1, {2, 3, } },
      { 5, {2, } },
      { 6, {2, 3, } },
    },
    {
      { 3, {2, } },
      { 4, {2, } },
      { 8, {2, } },
      { 9, {2, } },
    },
    {
      { 3, {4, } },
      { 4, {4, 5, } },
      { 8, {4, } },
      { 9, {4, 5, } },
    },
    {
      {0, 4},
      {1, 6},
      {5, 4},
      {6, 6},
    }
  };
  auto zero = {LteNrTddSlotType::DL,
               LteNrTddSlotType::S,
               LteNrTddSlotType::UL,
               LteNrTddSlotType::UL,
               LteNrTddSlotType::UL,
               LteNrTddSlotType::DL,
               LteNrTddSlotType::S,
               LteNrTddSlotType::UL,
               LteNrTddSlotType::UL,
               LteNrTddSlotType::UL,};

  TestPattern (zero, g);

  Result k = {
    {
      { 0, {0, } },
      { 1, {0, } },
    },
    {
      { 0, {2, } },
      { 1, {5, 2, 3} },
    },
    {
      { 3, {2, } },
      { 4, {2, } },
    },
    {
      { 3, {4, } },
      { 4, {4, 5, 7} },
    },
    {
      {0, 4},
      {1, 5},
    }
  };
  auto seven = {LteNrTddSlotType::DL,
                LteNrTddSlotType::F,
                LteNrTddSlotType::UL,
                LteNrTddSlotType::UL,
                LteNrTddSlotType::UL};

  TestPattern (seven, k);

  Result h = { {
                 { 0, {0, } },
                 { 1, {0, } },
                 { 2, {0, } },
                 { 3, {0, } },
                 { 4, {0, } },
                 { 5, {0, } },
                 { 6, {0, } },
                 { 7, {0, } },
                 { 8, {0, } },
                 { 9, {0, } },
               },
               {
                 { 0, {2, } },
                 { 1, {2, } },
                 { 2, {2, } },
                 { 3, {2, } },
                 { 4, {2, } },
                 { 5, {2, } },
                 { 6, {2, } },
                 { 7, {2, } },
                 { 8, {2, } },
                 { 9, {2, } },
               },
               {
                 { 0, {2, } },
                 { 1, {2, } },
                 { 2, {2, } },
                 { 3, {2, } },
                 { 4, {2, } },
                 { 5, {2, } },
                 { 6, {2, } },
                 { 7, {2, } },
                 { 8, {2, } },
                 { 9, {2, } },
               },
               {
                 { 0, {4, } },
                 { 1, {4, } },
                 { 2, {4, } },
                 { 3, {4, } },
                 { 4, {4, } },
                 { 5, {4, } },
                 { 6, {4, } },
                 { 7, {4, } },
                 { 8, {4, } },
                 { 9, {4, } },
               },
               {
                 {0, 4},
                 {1, 4},
                 {2, 4},
                 {3, 4},
                 {4, 4},
                 {5, 4},
                 {6, 4},
                 {7, 4},
                 {8, 4},
                 {9, 4}
               }};

  auto nr = {LteNrTddSlotType::F,
             LteNrTddSlotType::F,
             LteNrTddSlotType::F,
             LteNrTddSlotType::F,
             LteNrTddSlotType::F,
             LteNrTddSlotType::F,
             LteNrTddSlotType::F,
             LteNrTddSlotType::F,
             LteNrTddSlotType::F,
             LteNrTddSlotType::F,};

  TestPattern (nr, h);


  Result l = { {
                 { 0, {0, } },
                 { 1, {0, } },
                 { 2, {0, } },
                 { 3, {0, } },
                 { 4, {0, } },
               },
               {
                 { 0, {2, } },
                 { 1, {2, } },
                 { 2, {2, } },
                 { 3, {2, } },
                 { 4, {2, 3, 4, 5, 6, 7, } },
               },
               {
                 { 0, {2, } },
                 { 1, {2, } },
                 { 2, {2, } },
                 { 10, {2, } },
                 { 11, {2, } },
               },
               {
                 { 0, {4, } },
                 { 1, {4, } },
                 { 2, {4, 5, 6, 7, 8, 9, } },
                 { 10, {4, } },
                 { 11, {4, } },
               },
               {
                 {0, 4},
                 {1, 4},
                 {2, 4},
                 {3, 4},
                 {4, 4},
               }};

  auto twelve = {LteNrTddSlotType::DL,
                 LteNrTddSlotType::DL,
                 LteNrTddSlotType::F,
                 LteNrTddSlotType::F,
                 LteNrTddSlotType::F,
                 LteNrTddSlotType::UL,
                 LteNrTddSlotType::UL,
                 LteNrTddSlotType::UL,
                 LteNrTddSlotType::UL,
                 LteNrTddSlotType::UL,
                 LteNrTddSlotType::UL,
                 LteNrTddSlotType::UL,};

  TestPattern (twelve,l);

  Result m = { {
                 { 0, {0, } },
                 { 1, {0, } },
                 { 2, {0, } },
                 { 3, {0, } },
                 { 4, {0, } },
                 { 5, {0, } },
                 { 6, {0, } },
                 { 7, {0, } },
                 { 8, {0, } },
                 { 9, {0, } },
               },
               {
                 { 0, {2, } },
                 { 1, {2, } },
                 { 2, {2, } },
                 { 3, {2, } },
                 { 4, {2, } },
                 { 5, {2, } },
                 { 6, {2, } },
                 { 7, {2, } },
                 { 8, {2, } },
                 { 9, {2, } },
               },
               {
                 { 0, {2, } },
                 { 1, {2, } },
                 { 2, {2, } },
                 { 3, {2, } },
                 { 4, {2, } },
                 { 5, {2, } },
                 { 6, {2, } },
                 { 7, {2, } },
                 { 8, {2, } },
                 { 9, {2, } },
               },
               {},
               {
                 {0, 4},
                 {1, 4},
                 {2, 4},
                 {3, 4},
                 {4, 4},
                 {5, 4},
                 {6, 4},
                 {7, 4},
                 {8, 4},
                 {9, 4},
               }};

  auto thirtheen = {
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
    LteNrTddSlotType::DL,
  };

  TestPattern (thirtheen, m);
}

void
LtePatternTestCase::Print (const std::map<uint32_t, std::vector<uint32_t> > &str)
{
  std::cout << "{" << std::endl;
  for (const auto & v : str)
    {
      std::cout << " { " << v.first << ", {";
      for (const auto & i : v.second)
        {
          std::cout << i << ", ";
        }
      std::cout << "} }," << std::endl;
    }
  std::cout << "}" << std::endl;
}

void
LtePatternTestCase::PrintHarq (const std::map<uint32_t, uint32_t> &str)
{
  std::cout << "{" << std::endl;
  for (const auto & v : str)
    {
      std::cout << " { " << v.first << ", ";
      std::cout << v.second;
      std::cout << "}" << std::endl;
    }
  std::cout << "}" << std::endl;
}


void
LtePatternTestCase::CheckVector (const std::vector<uint32_t> &a,
                                 const std::vector<uint32_t> &b)
{
  NS_TEST_ASSERT_MSG_EQ (a.size (), b.size (), "Two vectors have different length");
  for (uint32_t i = 0; i < a.size (); ++i)
    {
      NS_TEST_ASSERT_MSG_EQ (a[i], b[i], "Values in vector diffes");
    }
}

void
LtePatternTestCase::CheckMap (const std::map<uint32_t, std::vector<uint32_t> > &a,
                              const std::map<uint32_t, std::vector<uint32_t> > &b)
{
  NS_TEST_ASSERT_MSG_EQ (a.size (), b.size (), "Two maps have different length");

  for (const std::pair<const uint32_t, std::vector<uint32_t> > & v : a)
    {
      CheckVector (a.at (v.first), b.at (v.first));
    }
}

void
LtePatternTestCase::CheckHarqMap (const std::map<uint32_t, uint32_t> &a,
                                  const std::map<uint32_t, uint32_t> &b)
{
  NS_TEST_ASSERT_MSG_EQ (a.size (), b.size (), "Two HARQ maps have different length");

  for (const auto & element : a)
    {
      NS_TEST_ASSERT_MSG_EQ (element.second, b.at (element.first), "A value in A is different from the value for the same key in B");
    }

}


void
LtePatternTestCase::TestPattern (const std::vector<LteNrTddSlotType> &pattern,
                                 const Result &result)
{
  std::map<uint32_t, std::vector<uint32_t> > toSendDl;
  std::map<uint32_t, std::vector<uint32_t> > toSendUl;
  std::map<uint32_t, std::vector<uint32_t> > generateDl;
  std::map<uint32_t, std::vector<uint32_t> > generateUl;
  std::map<uint32_t, uint32_t> dlHarqFb;

  NrGnbPhy::GenerateStructuresFromPattern (pattern, &toSendDl, &toSendUl, &generateDl, &generateUl, &dlHarqFb, 0, 2, 4, 2);

  if (m_verbose)
    {
      std::cout << std::endl << "PATTERN to test: ";
      for (const auto & v : pattern)
        {
          std::cout << v << " ";
        }
      std::cout << std::endl;
    }

  if (m_verbose)
    {
      std::cout << "To Send DL theoretic:" << std::endl;
      Print (toSendDl);
      std::cout << "To Send DL result:" << std::endl;
      Print (result.m_toSendDl);
    }
  CheckMap (toSendDl, result.m_toSendDl);

  if (m_verbose)
    {
      std::cout << "To Send UL theoretic:" << std::endl;
      Print (toSendUl);
      std::cout << "To Send UL result:" << std::endl;
      Print (result.m_toSendUl);
    }

  CheckMap (toSendUl, result.m_toSendUl);

  if (m_verbose)
    {
      std::cout << "Generate DL theoretic:" << std::endl;
      Print (generateDl);
      std::cout << "Generate DL result:" << std::endl;
      Print (result.m_generateDl);
    }

  CheckMap (generateDl, result.m_generateDl);

  if (m_verbose)
    {
      std::cout << "Generate UL theoretic:" << std::endl;
      Print (generateUl);
      std::cout << "Generate UL result:" << std::endl;
      Print (result.m_generateUl);
    }

  CheckMap (generateUl, result.m_generateUl);

  if (m_verbose)
    {
      std::cout << "HarqFB theoretic:" << std::endl;
      PrintHarq (dlHarqFb);
      std::cout << "HarqFB result:" << std::endl;
      PrintHarq (result.m_dlHarqFb);
    }

  CheckHarqMap (dlHarqFb, result.m_dlHarqFb);
}

/**
 * \brief The NrLtePatternTestSuite class
 */
class NrLtePatternTestSuite : public TestSuite
{
public:
  NrLtePatternTestSuite () : TestSuite ("nr-lte-pattern-generation", UNIT)
  {
    AddTestCase (new LtePatternTestCase ("LTE TDD Pattern test"), QUICK);
  }
};

static NrLtePatternTestSuite nrLtePatternTestSuite; //!< Pattern test suite

}  // namespace ns3
