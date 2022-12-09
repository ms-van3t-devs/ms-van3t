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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/eps-bearer-tag.h"
#include "ns3/config-store-module.h"
#include "ns3/nr-module.h"
#include "ns3/stats-module.h"
#include <ns3/sqlite-output.h>
#include "ns3/antenna-module.h"

const uint32_t DB_ATTEMPT_LIMIT = 20; // how many times to try to perform DB query before giving up, we dont want to enter to a infinite loop

using namespace ns3;

/**
 * \ingroup examples
 * \file realistic-beamforming.cc
 * \brief Simulation script for the realistic beamforming evaluation.
 * Simulation allows to configure various parameters out of which the
 * most important are:
 * - distance (by configuring deltaX and deltaY parameters). Distance
 * between transmitter and the receiver is an important parameter
 * (since we want to evaluate how the distance impact the selection of
 * the correct beam). Distance will be configured with deltaX and delta Y
 * simulation parameters that define the relative position of UE with
 * respect to gNB's position.
 * - type of the beamfoming methods (because we want to obtain results
 * for both ideal beamforming algorithm and realistic beamforming
 * algorithm. Expected behavior is that as the distance increases that
 * the error in estimating channel increases, thus realistic beamforming
 * algorithm makes more mistakes when selecting the correct beams at the
 * transmiter and the receiver).
 * -rngRun - random run number that will allow us to run many simulations
 * and to average the results
 *
 * The topology is very simple, consists of a single gNB and UE.
 *
 * <pre>
 *
 *                                                   + UE
 *                                                   |
 *                                                   |
 *                                                deltaY
 *                                                   |
 *                                                   |
 *                                                   |
 *   gNB+  ------------deltaX-------------------------
 * </pre>
 *
 *
 * The output files contain SINR values. File name starts with the prefix
 * "sinrs" which is followed by the ${simTag} if provided or by default with
 * a string that briefly describes the configuration parameters
 * that are being set in the specific simulation execution.
 *
 * The output file is created by default in the root project directory if not
 * configured differently by setting resultsDirPath parameter of the Run()
 * function.
 * To run this simulation you can use for example the following command:
 *
 * ./ns3 run cttc-realistic-beamforming '--command=%s
 *    --deltaX=300
 *    --deltaY=0
 *    --algType=Real
 *    --rngRun=1
 *    --numerology=0
 *    --gnbAntenna=3gpp
 *    --ueAntenna=3gpp
 *    --uePower=10
 *    --scenario=Uma
 *    --simTag=-sim-campaign-algReal-rng1-mu0-gNb3gpp-ue3gpp
 *    --resultsDir=/tmp/results/realistic-beamforming/
 *    --dbName=realistic_beamforming.db
 *    --tableName=results'
 *
 * To check all the parameters that are available you can always use:
 *
 * ./ns3 run "cttc-realistic-beamforming --PrintHelp"
 *
 */


NS_LOG_COMPONENT_DEFINE ("CttcRealisticBeamforming");

/**
 * \brief Main class
 */
class CttcRealisticBeamforming
{

public:

  enum BeamformingMethod
  {
    IDEAL,
    REALISTIC,
  };

  /**
   * \brief This function converts a linear SINR value that is encapsulated in
   * params structure to dBs, and then it prints the dB value to an output file
   * containing SINR values.
   * @param params RxPacketTraceParams structure that contains different
   * attributes that define the reception of the packet
   *
   */
  void UeReception (RxPacketTraceParams params);

  /**
   * \brief Function that will save the configuration parameters to be used later for
   * printing the results into the files.
   *
   * @param deltaX delta that will be used to determine X coordinate of UE wrt to gNB X coordinate
   * @param deltaY delta that will be used to determine Y coordinate of UE wrt to gNB Y coordinate
   * @param beamforming beamforming type: Ideal or Real
   * @param realTriggerEvent if realistic beamforming is used it defines the trigger event
   * @param idealPeriodicity if the ideal beamforming is used it defines the periodicity
   * @param rngRun rngRun number that will be used to run the simulation
   * @param numerology The numerology
   * @param gnbAntenna antenna model to be used by gNB device, can be Iso or directional 3gpp
   * @param ueAntenna antenna model to be used by UE device, can be Iso directional 3gpp
   * @param scenario deployment scenario, can be Uma, Umi, Inh, Rma
   * @param uePower uePower
   * @param resultsDirPath results directory path
   * @param tag A tag that contains some simulation run specific values in order
   * to be able to distinguish the results file for different runs for different
   * parameters configuration
   */
  void Configure (double deltaX, double deltaY, BeamformingMethod beamforming,
                  RealisticBfManager::TriggerEvent realTriggerEvent,
                  uint32_t idealPeriodicity, uint64_t rngRun, uint16_t numerology,
                  std::string gnbAntenna, std::string ueAntenna,
                  std::string scenario, double uePower,
                  std::string resultsDirPath, std::string tag,
                  std::string dbName, std::string tableName);

  /**
   * \brief Function that will actually configure all the simulation parameters,
   * topology and run the simulation by using the parameters that are being
   * configured for the specific run.
   */
  void RunSimulation ();
  /**
   * \brief Destructor that closes the output file stream and finished the
   *  writing into the files.
   */
  ~CttcRealisticBeamforming ();
  /**
  * \brief Creates a string tag that contains some simulation run specific values in
  * order to be able to distinguish the results files for different runs for
  * different parameters.
  */
  std::string BuildTag ();
  /**
   * \brief
   * Prepare files for the output of the results
   */
  void PrepareOutputFiles ();
  /**
   * \brief
   * Print the statistics to the output files
   */
  void PrintResultsToFiles ();
  /**
   * \brief
   * Create traffic applications
   */
  void CreateDlTrafficApplications (ApplicationContainer& serverAppDl, ApplicationContainer& clientAppDl,
                                    NodeContainer& ueNode, Ptr<Node> remoteHost, NetDeviceContainer ueNetDev,
                                    Ipv4InterfaceContainer& ueIpIface);

  /**
   * \brief Prepare the database to print the results, e.g., open it, and
   * create the necessary table if it does not exist.
   * Method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "(Sinr DOUBLE NOT NULL, "
   * - "Distance DOUBLE NULL,"
   *   "DeltaX DOUBLE NULL,
   *   "DeltaY DOUBLE NULL,
   *   "BeamformingType TEXT NOT NULL,"
   *   "RngRun INTEGER NOT NULL,"
   *   "numerology INTEGER NOT NULL,"
   *   "gNBAntenna TEXT NOT NULL,"
   *   "ueAntenna TEXT NOT NULL,"
   *   "uePower INTEGER NOT NULL,"
   *   "scenario TEXT NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * NOTE: If the database already contains a table with
   * the same name, this method will clean existing values
   * with the same Run value.
   */
  void PrepareDatabase ();

  /**
   * \brief Insert results to the table in database.
   */
  void PrintResultsToDatabase ();

  /**
   * \brief Delete results entry from database if already exist
   */
  void DeleteFromDatabaseIfAlreadyExist ();

private:

  //output file streams
  std::ofstream m_outSinrFile;         //!< the output file stream for the SINR file in linear scale

  // database related attributes
  sqlite3 *m_db { nullptr };                          //!< DB pointer
  std::string m_tableName {"results"};                //!< Table name
  std::string m_dbName {"realistic_beamforming.db"};  //!< Database name

  // statistics collecting object
  MinMaxAvgTotalCalculator<double> m_sinrStats;  //!< the statistics calculator for SINR values

  // main simulation parameters that is expected that user will change often
  double m_deltaX {1};
  double m_deltaY {1};
  BeamformingMethod m_beamforming {IDEAL};
  uint32_t m_rngRun {1};
  RealisticBfManager::TriggerEvent m_realTriggerEvent {RealisticBfManager::SRS_COUNT};
  uint32_t m_idealPeriodicity {0}; // ideal beamforming periodicity in the number of milli seconds
  uint16_t m_numerology {0};
  std::string m_gnbAntennaModel {"Iso"};
  std::string m_ueAntennaModel {"Iso"};
  std::string m_scenario {"Uma"};
  std::string m_resultsDirPath {""};
  std::string m_tag {""};
  double m_gNbHeight {25};
  double m_gNbTxPower {35};
  BandwidthPartInfo::Scenario m_deployScenario {BandwidthPartInfo::UMa};
  double m_ueTxPower {23};

  // simulation parameters that are not expected to be changed often by the user
  Time m_simTime = MilliSeconds (150);
  Time m_udpAppStartTimeDl = MilliSeconds (100);
  Time m_udpAppStopTimeDl = MilliSeconds (150);
  uint32_t m_packetSize = 1000;
  DataRate m_udpRate = DataRate ("1kbps");
  double m_centralFrequency = 28e9;
  double m_bandwidth = 100e6;
  double m_ueHeight = 1.5; // UE antenna height is 1.5 meters


  const uint8_t m_numCcPerBand = 1;
  double m_gNbX = 0;
  double m_gNbY = 0;
};

/**
 * Function that creates the output file name for the results.
 * @param directoryName Directory name
 * @param filePrefix The prefix for the file name, e.g. sinr
 * @param tag A tag that contains some simulation run specific values in order to be
 * able to distinguish the results file for different runs for different parameters
 * configuration
 * @return returns The full path file name string
 */
std::string
BuildFileNameString (std::string directoryName, std::string filePrefix, std::string tag)
{
  std::ostringstream oss;
  oss << directoryName << "/" << filePrefix << tag;
  return oss.str ();
}

std::string
CttcRealisticBeamforming:: BuildTag ()
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;
  std::string algorithm = (m_beamforming == CttcRealisticBeamforming::IDEAL) ? "I" : "R";
  double distance2D = sqrt (m_deltaX * m_deltaX + m_deltaY * m_deltaY);

  oss << "-"   << algorithm   <<
    "-d" << distance2D <<
    "-mu" << m_numerology <<
    "-gnb" << m_gnbAntennaModel  <<
    "-ue" << m_ueAntennaModel <<
    "-uePow" << m_ueTxPower <<
    "-scenario" << m_scenario;

  return oss.str ();
}


void
CttcRealisticBeamforming::PrepareOutputFiles ()
{
  NS_LOG_FUNCTION (this);
  // If simulation tag is not provided create one, user can provide his own tag through the command line
  if (m_tag == "")
    {
      m_tag = BuildTag ();
    }
  std::string fileSinr = BuildFileNameString ( m_resultsDirPath, "sinrs", m_tag);

  m_outSinrFile.open (fileSinr.c_str (), std::ios_base::app);
  m_outSinrFile.setf (std::ios_base::fixed);
  NS_ABORT_MSG_IF (!m_outSinrFile.is_open (), "Can't open file " << fileSinr);
}

void
CttcRealisticBeamforming::PrepareDatabase ()
{
  NS_LOG_FUNCTION (this);

  int rc = sqlite3_open ((m_resultsDirPath + "/" + m_dbName).c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Failed to open DB");

  std::string cmd = "CREATE TABLE IF NOT EXISTS " + m_tableName + " ("
    "SINR DOUBLE NOT NULL, "
    "SINR_DB DOUBLE NOT NULL, "
    "Distance DOUBLE NOT NULL,"
    "DeltaX DOUBLE NOT NULL,"
    "DeltaY DOUBLE NOT NULL,"
    "BeamformingType TEXT NOT NULL,"
    "RngRun INTEGER NOT NULL,"
    "Numerology INTEGER NOT NULL,"
    "GnbAntenna TEXT NOT NULL,"
    "UeAntenna TEXT NOT NULL,"
    "UePower INTEGER NOT NULL,"
    "Scenario TEXT NOT NULL);";

  sqlite3_stmt *stmt;

  // prepare the statement for creating the table
  uint32_t attemptCount = 0;
  do
    {
      rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  // check if it went correctly
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not prepare correctly the statement for creating the table. Db error:" << sqlite3_errmsg (m_db)
                                                                                                                                           << "full command is: \n" << cmd);

  // execute a step operation on a statement until the result is ok or an error
  attemptCount = 0;
  do
    {
      rc = sqlite3_step (stmt);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  // check if it went correctly
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement for creating the table. Db error:" << sqlite3_errmsg (m_db));

  // finalize the statement until the result is ok or an error occures
  attemptCount = 0;
  do
    {
      rc = sqlite3_finalize (stmt);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  // check if it went correctly
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement for creating the table. Db error:" << sqlite3_errmsg (m_db));
}

void
CttcRealisticBeamforming::PrintResultsToDatabase ()
{
  NS_LOG_FUNCTION (this);

  DeleteFromDatabaseIfAlreadyExist ();

  sqlite3_stmt *stmt;
  std::string cmd = "INSERT INTO " + m_tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
  std::string beamformingType = (m_beamforming == IDEAL) ? "Ideal" : "Real";
  int rc;
  double distance2D = sqrt (m_deltaX * m_deltaX + m_deltaY * m_deltaY);

  // prepare the statement for creating the table
  uint32_t attemptCount = 0;
  do
    {
      rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  // check if it went correctly
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not prepare correctly the insert into the table statement. "
                       " Db error:" << sqlite3_errmsg (m_db) << ". The full command is: \n" << cmd);

  // add all parameters to the command
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 1, m_sinrStats.getMean ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 2, 10.0 * log10 (m_sinrStats.getMean ())) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 3, distance2D) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 4, m_deltaX) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 5, m_deltaY) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 6, beamformingType.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 7, m_rngRun) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 8, m_numerology) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 9, m_gnbAntennaModel.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 10, m_ueAntennaModel.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 11, m_ueTxPower) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 12, m_scenario.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);


  // finalize the command
  attemptCount = 0;
  do
    {
      rc = sqlite3_step (stmt);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  // check if it went correctly
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error:" << sqlite3_errmsg (m_db));
  attemptCount = 0;
  do
    {
      rc = sqlite3_finalize (stmt);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error:" << sqlite3_errmsg (m_db));

}

void
CttcRealisticBeamforming::DeleteFromDatabaseIfAlreadyExist ()
{
  sqlite3_stmt *stmt;
  std::string cmd = "DELETE FROM \"" + m_tableName + "\" WHERE "
    "deltaX == ? AND "                 // 1
    "deltaY == ? AND "                 //2
    "BeamformingType = ? AND "                //3
    "RngRun == ? AND "                //4
    "Numerology == ? AND "                //5
    "GnbAntenna = ? AND "                //6
    "UeAntenna = ? AND "                //7
    "UePower = ? AND "                //8
    "Scenario = ?;";                //9
  int rc;

  // prepare the statement for creating the table
  uint32_t attemptCount = 0;
  do
    {
      rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  // check if it went correctly
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not prepare correctly the delete statement. "
                       " Db error:" << sqlite3_errmsg (m_db) << ". The full command is: \n" << cmd);


  std::string beamformingType = (m_beamforming == IDEAL) ? "Ideal" : "Real";
  // add all parameters to the command
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 1, m_deltaX) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 2, m_deltaY) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 3, beamformingType.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 4, m_rngRun) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 5, m_numerology) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 6, m_gnbAntennaModel.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 7, m_ueAntennaModel.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 8, m_ueTxPower) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 9, m_scenario.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);


  // finalize the command
  attemptCount = 0;
  do
    {
      rc = sqlite3_step (stmt);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  // check if it went correctly
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error:" << sqlite3_errmsg (m_db));
  attemptCount = 0;
  do
    {
      rc = sqlite3_finalize (stmt);
      NS_ABORT_MSG_IF (++attemptCount == DB_ATTEMPT_LIMIT, "Waiting too much for sqlite3 database to be ready. "
                       "Check if you have the database/table open with another program. "
                       "If yes, close it before running again cttc-realistic-beamforming program.\n\n");
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error:" << sqlite3_errmsg (m_db));

}

void
CttcRealisticBeamforming::CreateDlTrafficApplications (ApplicationContainer& serverAppDl, ApplicationContainer& clientAppDl,
                                                       NodeContainer& ueNode, Ptr<Node> remoteHost, NetDeviceContainer ueNetDev,
                                                       Ipv4InterfaceContainer& ueIpIface)
{
  NS_LOG_FUNCTION (this);
  uint16_t dlPort = 1234;
  //Calculate UDP interval based on the packetSize and desired udp rate
  Time udpInterval = Time::FromDouble ((m_packetSize * 8) / static_cast<double> (m_udpRate.GetBitRate ()), Time::S);
  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverAppDl.Add (dlPacketSinkHelper.Install (ueNode));
  // Configure UDP downlink traffic
  for (uint32_t i = 0; i < ueNetDev.GetN (); i++)
    {
      UdpClientHelper dlClient (ueIpIface.GetAddress (i), dlPort);
      dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      dlClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
      dlClient.SetAttribute ("Interval", TimeValue (udpInterval)); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
      clientAppDl.Add (dlClient.Install (remoteHost));
    }

  // Start UDP server and client app, and configure stop time
  serverAppDl.Start (m_udpAppStartTimeDl);
  clientAppDl.Start (m_udpAppStartTimeDl);
  serverAppDl.Stop (m_udpAppStopTimeDl);
  clientAppDl.Stop (m_udpAppStopTimeDl);
}

/**
 * A callback function that redirects a call to the simulation setup instance.
 * @param simSetup A pointer to a simulation instance
 * @param params RxPacketTraceParams structure containing RX parameters
 */
void UeReceptionTrace (CttcRealisticBeamforming* simSetup, RxPacketTraceParams params)
{
  simSetup->UeReception (params);
}

void
CttcRealisticBeamforming::UeReception (RxPacketTraceParams params)
{
  m_sinrStats.Update (params.m_sinr); // we have to pass the linear value
}

CttcRealisticBeamforming::~CttcRealisticBeamforming ()
{
  // close the output results file
  m_outSinrFile.close ();

  // Failed to close the database
  int rc = SQLITE_FAIL;
  rc = sqlite3_close_v2 (m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Failed to close DB");
}

void
CttcRealisticBeamforming::PrintResultsToFiles ()
{
  NS_LOG_FUNCTION (this);
  m_outSinrFile << m_sinrStats.getMean () << std::endl;
}

void
CttcRealisticBeamforming::Configure (double deltaX, double deltaY, BeamformingMethod beamforming,
                                     RealisticBfManager::TriggerEvent realTriggerEvent,
                                     uint32_t idealPeriodicity, uint64_t rngRun,
                                     uint16_t numerology,
                                     std::string gNbAntennaModel, std::string ueAntennaModel,
                                     std::string scenario, double uePower,
                                     std::string resultsDirPath, std::string tag,
                                     std::string dbName, std::string tableName)

{
  NS_LOG_FUNCTION (this);
  m_deltaX = deltaX;
  m_deltaY = deltaY;
  m_beamforming = beamforming;
  m_rngRun = rngRun;
  m_realTriggerEvent = realTriggerEvent;
  m_idealPeriodicity = idealPeriodicity;
  m_numerology = numerology;
  m_gnbAntennaModel = gNbAntennaModel;
  m_ueAntennaModel = ueAntennaModel;
  m_scenario = scenario;
  m_ueTxPower = uePower;
  m_resultsDirPath = resultsDirPath;
  m_tag = tag;

  if (scenario == "Uma") // parameters based on TR 38.901 full calibration for RMa, UMa, UMi and InH_OfficeOpen, 30GHz band
    {
      m_gNbHeight = 25; // gNB antenna height
      m_gNbTxPower = 35;  // gNB transmit power
      m_deployScenario = BandwidthPartInfo::UMa;
    }
  else if (scenario == "UmaLos")
    {
      m_gNbHeight = 25;
      m_gNbTxPower = 35;
      m_deployScenario = BandwidthPartInfo::UMa_LoS;
    }
  else if (scenario == "UmaNlos")
    {
      m_gNbHeight = 25;
      m_gNbTxPower = 35;
      m_deployScenario = BandwidthPartInfo::UMa_nLoS;
    }
  else if (scenario == "Rma")
    {
      m_gNbHeight = 35;
      m_gNbTxPower = 35;
      m_deployScenario = BandwidthPartInfo::RMa;
    }
  else if (scenario == "RmaLos")
    {
      m_gNbHeight = 35;
      m_gNbTxPower = 35;
      m_deployScenario = BandwidthPartInfo::RMa_LoS;
    }
  else if (scenario == "RmaNlos")
    {
      m_gNbHeight = 35;
      m_gNbTxPower = 35;
      m_deployScenario = BandwidthPartInfo::RMa_nLoS;
    }
  else if (scenario == "Umi")
    {
      m_gNbHeight = 10;
      m_gNbTxPower = 35;
      m_deployScenario = BandwidthPartInfo::UMi_StreetCanyon;
    }
  else if (scenario == "UmiLos")
    {
      m_gNbHeight = 10;
      m_gNbTxPower = 35;
      m_deployScenario = BandwidthPartInfo::UMi_StreetCanyon_LoS;
    }
  else if (scenario == "UmiNlos")
    {
      m_gNbHeight = 10;
      m_gNbTxPower = 35;
      m_deployScenario = BandwidthPartInfo::UMi_StreetCanyon_nLoS;
    }
  else if (scenario == "Inh")
    {
      m_gNbHeight = 3;
      m_gNbTxPower = 24;
      m_deployScenario = BandwidthPartInfo::InH_OfficeOpen;
    }
  else if (scenario == "InhLos")
    {
      m_gNbHeight = 3;
      m_gNbTxPower = 24;
      m_deployScenario = BandwidthPartInfo::InH_OfficeOpen_LoS;
    }
  else if (scenario == "InhNlos")
    {
      m_gNbHeight = 3;
      m_gNbTxPower = 24;
      m_deployScenario = BandwidthPartInfo::InH_OfficeOpen_nLoS;
    }
  else
    {
      NS_ABORT_MSG ("Not supported scenario:" << scenario);
    }
}

void
CttcRealisticBeamforming::RunSimulation ()
{
  NS_LOG_FUNCTION (this);

  // Set simulation run number
  SeedManager::SetRun (m_rngRun);

  // Create gNB and UE nodes
  NodeContainer gNbNode;
  NodeContainer ueNode;
  gNbNode.Create (1);
  ueNode.Create (1);

  // Set positions
  Ptr<ListPositionAllocator> positions = CreateObject<ListPositionAllocator> ();
  positions->Add (Vector (m_gNbX, m_gNbY, m_gNbHeight));    //gNb will take this position
  positions->Add (Vector (m_gNbX + m_deltaX, m_gNbY + m_deltaY, m_ueHeight));   //UE will take this position
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positions);
  mobility.Install (gNbNode);
  mobility.Install (ueNode);

  // Create NR helpers: nr helper, epc helper, and beamforming helper
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();

  // Initialize beamforming
  Ptr<BeamformingHelperBase> beamformingHelper;

  if (m_beamforming == CttcRealisticBeamforming::IDEAL)
    {
      beamformingHelper = CreateObject<IdealBeamformingHelper> ();
      beamformingHelper->SetAttribute ("BeamformingPeriodicity", TimeValue (MilliSeconds (m_idealPeriodicity)));
      beamformingHelper->SetBeamformingMethod (CellScanBeamforming::GetTypeId ());
    }
  else if (m_beamforming == CttcRealisticBeamforming::REALISTIC)
    {
      beamformingHelper = CreateObject<RealisticBeamformingHelper> ();
      beamformingHelper->SetBeamformingMethod (RealisticBeamformingAlgorithm::GetTypeId ());
      // when realistic beamforming used, also realistic beam manager should be set
      // TODO, move this to NrHelper, so user sets BeamformingMethod calling NrHelper
      nrHelper->SetGnbBeamManagerTypeId (RealisticBfManager::GetTypeId ());
      nrHelper->SetGnbBeamManagerAttribute ("TriggerEvent", EnumValue (m_realTriggerEvent));
    }
  else
    {
      NS_ABORT_MSG ("Unknown beamforming type.");
    }
  nrHelper->SetBeamformingHelper (beamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  Config::SetDefault ("ns3::NrUePhy::EnableUplinkPowerControl", BooleanValue (false));

  /*
   * Configure the spectrum division: single operational band, containing single
   * component carrier, which contains a single bandwidth part.
   *
   * |------------------------Band-------------------------|
   * |-------------------------CC--------------------------|
   * |-------------------------BWP-------------------------|
   *
   */
  CcBwpCreator ccBwpCreator;
  // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates a single BWP per CC
  CcBwpCreator::SimpleOperationBandConf bandConf (m_centralFrequency,
                                                  m_bandwidth,
                                                  m_numCcPerBand,
                                                  m_deployScenario);
  // By using the configuration created, make the operation band
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
  nrHelper->InitializeOperationBand (&band);
  BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps ({band});

  // Configure antenna of gNb
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  //Antenna element type for gNBs
  if (m_gnbAntennaModel == "Iso")
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }

  // Configure antenna of UE
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  //Antenna element type for UEs
  if (m_ueAntennaModel == "Iso")
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }
  // configure schedulers
  nrHelper->SetSchedulerAttribute ("SrsSymbols", UintegerValue (1));

  // Install nr net devices
  NetDeviceContainer gNbDev = nrHelper->InstallGnbDevice (gNbNode, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNode, allBwps);

  int64_t randomStream = m_rngRun;
  randomStream += nrHelper->AssignStreams (gNbDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

  for (uint32_t i = 0; i < gNbDev.GetN (); i++)
    {
      nrHelper->GetGnbPhy (gNbDev.Get (i), 0)->SetAttribute ("Numerology", UintegerValue (m_numerology));
      nrHelper->GetGnbPhy (gNbDev.Get (i), 0)->SetAttribute ("TxPower", DoubleValue (m_gNbTxPower));
    }
  for (uint32_t j = 0; j < ueNetDev.GetN (); j++)
    {
      nrHelper->GetUePhy (ueNetDev.Get (j), 0)->SetAttribute ("TxPower", DoubleValue (m_ueTxPower));
    }

  // Update configuration
  for (auto it = gNbDev.Begin (); it != gNbDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  // Create the internet and install the IP stack on the UEs, get SGW/PGW and create a single RemoteHost
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);
  // connect a remoteHost to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

  // Configure routing
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNode);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // Set the default gateway for the UE
  for (uint32_t j = 0; j < ueNode.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach UE to gNB
  nrHelper->AttachToEnb (ueNetDev.Get (0), gNbDev.Get (0));

  // Install UDP downlink applications
  ApplicationContainer clientAppDl;
  ApplicationContainer serverAppDl;
  CreateDlTrafficApplications (clientAppDl, serverAppDl, ueNode, remoteHost, ueNetDev, ueIpIface);

  // Connect traces to our listener functions
  for (uint32_t i = 0; i < ueNetDev.GetN (); i++)
    {
      Ptr<NrSpectrumPhy> ue1SpectrumPhy = DynamicCast <NrUeNetDevice> (ueNetDev.Get (i))->GetPhy (0)->GetSpectrumPhy ();
      ue1SpectrumPhy->TraceConnectWithoutContext ("RxPacketTraceUe", MakeBoundCallback (&UeReceptionTrace, this));
      Ptr<NrInterference> ue1SpectrumPhyInterference = ue1SpectrumPhy->GetNrInterference ();
      NS_ABORT_IF (!ue1SpectrumPhyInterference);
    }

  Simulator::Stop (m_simTime);
  Simulator::Run ();
  Simulator::Destroy ();
}

int
main (int argc, char *argv[])
{
  // simulation configuration parameters
  double deltaX = 10.0;
  double deltaY = 10.0;
  std::string algType = "Real";
  std::string realTriggerEvent = "SrsCount"; // what will be the trigger event to update the beamforming vectors, only used when --algType="Real"
  uint32_t idealPeriodicity = 10; // how often will be updated the beamforming vectors, only used when --algType="Ideal", in [ms]
  uint64_t rngRun = 1;
  uint16_t numerology = 2;
  std::string gnbAntenna = "Iso";
  std::string ueAntenna = "Iso";
  double ueTxPower = 1;
  std::string scenario = "Uma";

  // parameters for saving the output
  std::string resultsDir = ".";
  std::string simTag = "";
  std::string dbName = "realistic-beamforming.db";
  std::string tableName = "results";

  CttcRealisticBeamforming::BeamformingMethod beamformingType;
  RealisticBfManager::TriggerEvent triggerEventEnum = RealisticBfManager::SRS_COUNT;
  CommandLine cmd;

  cmd.AddValue ("deltaX",
                "Determines X coordinate of UE wrt. to gNB X coordinate [meters].",
                deltaX);
  cmd.AddValue ("deltaY",
                "Determines Y coordinate of UE wrt. to gNB Y coordinate [meters].",
                deltaY);
  cmd.AddValue ("algType",
                "Algorithm type to be used. Can be: 'Ideal' or 'Real'.",
                algType);
  cmd.AddValue ("realTriggerEvent",
                "In the case of the realistic beafmorming (algType=\"Real\") it defines when the beamforming "
                "vectors will be updated: upon each SRS reception but with a certain delay, or after certain number of SRSs."
                "For the first option the parameter should be configured with 'DelayedUpdate' and for the "
                "second option the value to be configured is 'SrsCount'",
                realTriggerEvent);
  cmd.AddValue ("idealPeriodicity",
                "In the case of the ideal beamforminng (algType=\"Ideal\") it defines how often the "
                "beamforming vectors will be updated in milli seconds [ms].",
                idealPeriodicity);
  cmd.AddValue ("rngRun",
                "Rng run random number.",
                rngRun);
  cmd.AddValue ("numerology",
                "Numerology to be used.",
                numerology);
  cmd.AddValue ("gnbAntenna",
                "Configure antenna elements at gNb: Iso or 3gpp",
                gnbAntenna);
  cmd.AddValue ("ueAntenna",
                "Configure antenna elements at UE: Iso or 3gpp",
                ueAntenna);
  cmd.AddValue ("scenario",
                "Deployment scenario: Uma, Umi, Inh",
                scenario);
  cmd.AddValue ("uePower",
                "Tx power to be used by the UE [dBm].",
                ueTxPower);
  // output command line parameters
  cmd.AddValue ("resultsDir",
                "Directory where to store the simulation results.",
                resultsDir);
  cmd.AddValue ("simTag",
                "Tag to be appended to output filenames to distinguish simulation campaigns.",
                simTag);
  cmd.AddValue ("dbName",
                "Database name.",
                dbName);
  cmd.AddValue ("tableName",
                "Table name.",
                tableName);
  cmd.Parse (argc, argv);


  if (algType == "Ideal")
    {
      beamformingType = CttcRealisticBeamforming::IDEAL;
    }
  else if (algType == "Real")
    {
      beamformingType = CttcRealisticBeamforming::REALISTIC;

      if (realTriggerEvent == "SrsCount")
        {
          triggerEventEnum = RealisticBfManager::SRS_COUNT;
        }
      else if (realTriggerEvent == "DelayedUpdate")
        {
          triggerEventEnum = RealisticBfManager::DELAYED_UPDATE;
        }
      else
        {
          NS_ABORT_MSG ("Not supported trigger event for the realistic type of beamforming:" << algType);
        }

    }
  else
    {
      NS_ABORT_MSG ("Not supported value for algType:" << algType);
    }

  CttcRealisticBeamforming simpleBeamformingScenario;
  simpleBeamformingScenario.Configure (deltaX, deltaY, beamformingType, triggerEventEnum, idealPeriodicity,
                                       rngRun, numerology, gnbAntenna, ueAntenna, scenario, ueTxPower,
                                       resultsDir, simTag, dbName, tableName);
  simpleBeamformingScenario.PrepareDatabase ();
  simpleBeamformingScenario.PrepareOutputFiles ();
  simpleBeamformingScenario.RunSimulation ();
  simpleBeamformingScenario.PrintResultsToDatabase ();
  simpleBeamformingScenario.PrintResultsToFiles ();
}
