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
#include <ns3/sfnsf.h>

/**
 * \file test-sfnsf.cc
 * \ingroup test
 *
 * \brief Unit-testing for the frame/subframe/slot numbering, along with the
 * numerology. The test checks that the normalized slot number equals a
 * monotonically-increased integer, for every numerology.
 */
namespace ns3 {


class TestSfnSfTestCase : public TestCase
{
public:
  TestSfnSfTestCase (uint16_t num, const std::string &name)
    : TestCase (name),
      m_numerology (num)
  {}

private:
  virtual void DoRun (void) override;
  uint16_t m_numerology {0};
};


void
TestSfnSfTestCase::DoRun ()
{
  SfnSf sfn (0,0,0, m_numerology);

  for (uint32_t i = 0; i < 9999; ++i)
    {
      NS_TEST_ASSERT_MSG_EQ (sfn.Normalize (), i, "Mm");
      sfn.Add (1);
    }
}

class TestSfnSf : public TestSuite
{
public:
  TestSfnSf () : TestSuite ("nr-test-sfnsf", UNIT)
  {
    AddTestCase (new TestSfnSfTestCase (0, "SfnSf TestAdd with num 2"), QUICK);
    AddTestCase (new TestSfnSfTestCase (1, "SfnSf TestAdd with num 2"), QUICK);
    AddTestCase (new TestSfnSfTestCase (2, "SfnSf TestAdd with num 2"), QUICK);
    AddTestCase (new TestSfnSfTestCase (3, "SfnSf TestAdd with num 2"), QUICK);
    AddTestCase (new TestSfnSfTestCase (4, "SfnSf TestAdd with num 2"), QUICK);
  }
};

static TestSfnSf testSfnSf; //!< SfnSf test

}  // namespace ns3
