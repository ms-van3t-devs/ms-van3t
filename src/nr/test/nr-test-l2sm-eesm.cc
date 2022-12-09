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
#include <ns3/nr-eesm-error-model.h>
#include <ns3/enum.h>
#include <ns3/nr-eesm-cc-t1.h>
#include <ns3/nr-eesm-cc-t2.h>
#include <ns3/nr-eesm-ir-t1.h>
#include <ns3/nr-eesm-ir-t2.h>
/**
 * \file nr-test-l2sm-eesm.cc
 * \ingroup test
 *
 * \brief This test validates specific functions of the NR PHY abstraction model.
 * The test checks two issues: 1) LDPC base graph (BG) selection works properly, and 2)
 * BLER values are properly obtained from the BLER-SINR look up tables for different
 * block sizes, MCS Tables, BG types, and SINR values.
 *
 */
namespace ns3 {

/**
 * \brief NrL2smEesm testcase
 */
class NrL2smEesmTestCase : public TestCase
{
public:
  NrL2smEesmTestCase (const std::string &name) : TestCase (name) { }

  /**
   * \brief Destroy the object instance
   */
  virtual ~NrL2smEesmTestCase () override {}

private:
  virtual void DoRun (void) override;

  void TestMappingSinrBler1 (const Ptr<NrEesmErrorModel> &em);
  void TestMappingSinrBler2 (const Ptr<NrEesmErrorModel> &em);
  void TestBgType1 (const Ptr<NrEesmErrorModel> &em);
  void TestBgType2 (const Ptr<NrEesmErrorModel> &em);

  void TestEesmCcTable1 ();
  void TestEesmCcTable2 ();
  void TestEesmIrTable1 ();
  void TestEesmIrTable2 ();
};

void
NrL2smEesmTestCase::TestBgType1 (const Ptr<NrEesmErrorModel> &em)
{
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 18), NrEesmErrorModel::SECOND,
                         "TestBgType1-a: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3900, 18), NrEesmErrorModel::FIRST,
                         "TestBgType1-b: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (200, 18), NrEesmErrorModel::SECOND,
                         "TestBgType1-c: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (4000, 0), NrEesmErrorModel::SECOND,
                         "TestBgType1-d: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 28), NrEesmErrorModel::FIRST,
                         "TestBgType1-e: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 2), NrEesmErrorModel::SECOND,
                         "TestBgType2-f: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 16), NrEesmErrorModel::SECOND,
                         "TestBgType2-g: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3900, 14), NrEesmErrorModel::FIRST,
                         "TestBgType2-h: The calculated value differs from the 3GPP base graph selection algorithm.");
}

void
NrL2smEesmTestCase::TestBgType2 (const Ptr<NrEesmErrorModel> &em)
{
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 18), NrEesmErrorModel::FIRST,
                         "TestBgType2-a: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3900, 18), NrEesmErrorModel::FIRST,
                         "TestBgType2-b: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (200, 18), NrEesmErrorModel::SECOND,
                         "TestBgType2-c: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (4000, 0), NrEesmErrorModel::SECOND,
                         "TestBgType2-d: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 27), NrEesmErrorModel::FIRST,
                         "TestBgType2-e: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 2), NrEesmErrorModel::SECOND,
                         "TestBgType2-f: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 16), NrEesmErrorModel::FIRST,
                         "TestBgType2-g: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3900, 14), NrEesmErrorModel::FIRST,
                         "TestBgType2-h: The calculated value differs from the 3GPP base graph selection algorithm.");
}

typedef std::tuple<double, uint8_t, uint32_t, double> MappingTable;

static std::vector<MappingTable> resultTable1 = {
  // sinr (lineal), mcs, cbsize, result

  // MCS 18, all CBS in continuation use BGtype2
  // CBS=3200, in table corresponds to 3104
  MappingTable { 19.95,  18, 3200, 0.023 },      // sinr 13 db
  MappingTable { 15.849, 18, 3200, 0.7567365 },  // sinr 12 db
  MappingTable { 10,     18, 3200, 1.00 },       // sinr 10 db
  // CBS=3500, in table corresponds to 3496
  MappingTable { 19.95,  18, 3500, 0.0735 },     // sinr 13 db
  MappingTable { 15.849, 18, 3500, 0.7908951 },  // sinr 12 db
  MappingTable { 10,     18, 3500, 1.00 },       // sinr 10 db

  // MCS 14, all CBS in continuation use BGtype1
  // CBS=3900, in table corresponds to 3840
  MappingTable { 8.9125, 14, 3900, 0.3225703 },  // sinr 9.5db
  MappingTable { 7.9433, 14, 3900, 0.8827055 },  // sinr 9 db
  MappingTable { 6.3095, 14, 3900, 1.00 },       // sinr 8 db
  // CBS=6300, in table corresponds to 6272
  MappingTable { 8.9125, 14, 6300, 0.0237 },     // sinr 9.5db
  MappingTable { 7.9433, 14, 6300, 0.9990385 },  // sinr 9 db
  MappingTable { 6.3095, 14, 6300, 1.00 }        // sinr 8 db

};
static std::vector<MappingTable> resultTable2 = {
  // sinr (lineal), mcs, cbsize, result

  // MCS 11, all CBS in continuation use BGtype2
  // CBS=3200, in table corresponds to 3104
  MappingTable { 19.95,  11, 3200, 0.023 },      // sinr 13 db
  MappingTable { 15.849, 11, 3200, 0.7567365 },  // sinr 12 db
  MappingTable { 10,     11, 3200, 1.00 },       // sinr 10 db
  // CBS=3500, in table corresponds to 3496
  MappingTable { 19.95,  11, 3500, 0.0735 },     // sinr 13 db
  MappingTable { 15.849, 11, 3500, 0.7908951 },  // sinr 12 db
  MappingTable { 10,     11, 3500, 1.00 },       // sinr 10 db

  // MCS 8, all CBS in continuation use BGtype1
  // CBS=3900, in table corresponds to 3840
  MappingTable { 8.9125, 8, 3900, 0.3225703 },  // sinr 9.5db
  MappingTable { 7.9433, 8, 3900, 0.8827055 },  // sinr 9 db
  MappingTable { 6.3095, 8, 3900, 1.00 },       // sinr 8 db
  // CBS=6300, in table corresponds to 6272
  MappingTable { 8.9125, 8, 6300, 0.0237 },     // sinr 9.5db
  MappingTable { 7.9433, 8, 6300, 0.9990385 },  // sinr 9 db
  MappingTable { 6.3095, 8, 6300, 1.00 }        // sinr 8 db

};

void
NrL2smEesmTestCase::TestMappingSinrBler1 (const Ptr<NrEesmErrorModel> &em)
{
  for (auto result : resultTable1)
    {
      NS_TEST_ASSERT_MSG_EQ (em->MappingSinrBler (std::get<0> (result),
                                                  std::get<1> (result),
                                                  std::get<2> (result)),
                             std::get<3> (result),
                             "TestMappingSinrBler1: The calculated value differs from "
                             " the SINR-BLER table. SINR=" << std::get<0> (result) <<
                             " MCS " << static_cast<uint32_t> (std::get<1> (result)) <<
                             " CBS " << std::get<2> (result));
    }

}
void
NrL2smEesmTestCase::TestMappingSinrBler2 (const Ptr<NrEesmErrorModel> &em)
{
  for (auto result : resultTable2)
    {
      NS_TEST_ASSERT_MSG_EQ (em->MappingSinrBler (std::get<0> (result),
                                                  std::get<1> (result),
                                                  std::get<2> (result)),
                             std::get<3> (result),
                             "TestMappingSinrBler2: The calculated value differs from "
                             " the SINR-BLER table. SINR=" << std::get<0> (result) <<
                             " MCS " << static_cast<uint32_t> (std::get<1> (result)) <<
                             " CBS " << std::get<2> (result));
    }

}
void
NrL2smEesmTestCase::TestEesmCcTable1 ()
{
  // Create an object of type NrEesmCcT1 and cast it to NrEesmErrorModel
  Ptr<NrEesmErrorModel> em = CreateObject <NrEesmCcT1> ();

  // Check that the object was created
  bool ret = em == nullptr;
  NS_TEST_ASSERT_MSG_EQ (ret, false, "Could not create NrEesmCcT1 object");

  // Test here the functions:
  TestBgType1 (em);
  TestMappingSinrBler1 (em);
}

void
NrL2smEesmTestCase::TestEesmCcTable2 ()
{
  // Create an object of type NrEesmCcT2 and cast it to NrEesmErrorModel
  Ptr<NrEesmErrorModel> em = CreateObject <NrEesmCcT2> ();

  // Check that the object was created
  bool ret = em == nullptr;
  NS_TEST_ASSERT_MSG_EQ (ret, false, "Could not create NrEesmCcT2 object");

  // Test here the functions:
  TestBgType2 (em);
  TestMappingSinrBler2 (em);
}

void
NrL2smEesmTestCase::TestEesmIrTable1 ()
{
  // Create an object of type NrEesmIrT1 and cast it to NrEesmErrorModel
  Ptr<NrEesmErrorModel> em = CreateObject <NrEesmIrT1> ();

  // Check that the object was created
  bool ret = em == nullptr;
  NS_TEST_ASSERT_MSG_EQ (ret, false, "Could not create NrEesmIrT1 object");

  // Test here the functions:
  TestBgType1 (em);
  TestMappingSinrBler1 (em);
}

void
NrL2smEesmTestCase::TestEesmIrTable2 ()
{
  // Create an object of type NrEesmIrT2 and cast it to NrEesmErrorModel
  Ptr<NrEesmErrorModel> em = CreateObject <NrEesmIrT2> ();

  // Check that the object was created
  bool ret = em == nullptr;
  NS_TEST_ASSERT_MSG_EQ (ret, false, "Could not create NrEesmIrT2 object");

  // Test here the functions:
  TestBgType2 (em);
  TestMappingSinrBler2 (em);
}

void
NrL2smEesmTestCase::DoRun ()
{
  TestEesmCcTable1 ();
  TestEesmCcTable2 ();
  TestEesmIrTable1 ();
  TestEesmIrTable2 ();
}

class NrTestL2smEesm : public TestSuite
{
public:
  NrTestL2smEesm () : TestSuite ("nr-test-l2sm-eesm", UNIT)
  {
    AddTestCase (new NrL2smEesmTestCase ("First test"), QUICK);
  }
};

static NrTestL2smEesm NrTestL2smEesmTestSuite; //!< Nr test suite

}  // namespace ns3
