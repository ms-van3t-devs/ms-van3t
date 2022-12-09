/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/command-line.h>
#include <ns3/show-progress.h>

#include "lena-lte-comparison.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  Parameters params;
  /*
   * From here, we instruct the ns3::CommandLine class of all the input parameters
   * that we may accept as input, as well as their description, and the storage
   * variable.
   */
  CommandLine cmd;

  cmd.AddValue ("scenario",
                "The urban scenario string (UMa,UMi,RMa)",
                params.scenario);
  cmd.AddValue ("numRings",
                "The number of rings around the central site",
                params.numOuterRings);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per cell or gNB in multiple-ue topology",
                params.ueNumPergNb);
  cmd.AddValue ("siteFile",
                "Path to file of tower coordinates (instead of hexagonal grid)",
                params.baseStationFile);
  cmd.AddValue ("useSiteFile",
                "If true, it will be used site file, otherwise it will be used "
                "numRings parameter to create scenario.",
                params.useSiteFile);
  cmd.AddValue ("appGenerationTime",
                "Duration applications will generate traffic.",
                params.appGenerationTime);
  cmd.AddValue ("numerologyBwp",
                "The numerology to be used (NR only)",
                params.numerologyBwp);
  cmd.AddValue ("pattern",
                "The TDD pattern to use",
                params.pattern);
  cmd.AddValue ("direction",
                "The flow direction (DL or UL)",
                params.direction);
  cmd.AddValue ("simulator",
                "The cellular network simulator to use: LENA or 5GLENA",
                params.simulator);
  cmd.AddValue ("technology",
                "The radio access network technology (LTE or NR)",
                params.radioNetwork);
  cmd.AddValue ("operationMode",
                "The network operation mode can be TDD or FDD",
                params.operationMode);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                params.simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                params.outputDir);
  cmd.AddValue ("errorModelType",
                "Error model type: ns3::NrEesmCcT1, ns3::NrEesmCcT2, ns3::NrEesmIrT1, ns3::NrEesmIrT2, ns3::NrLteMiErrorModel",
                params.errorModel);
  cmd.AddValue ("calibration",
                "disable a bunch of things to make LENA and NR_LTE comparable",
                params.calibration);
  cmd.AddValue ("trafficScenario",
                "0: saturation (80 Mbps/20 MHz), 1: latency (1 pkt of 12 bytes), 2: low-load (1 Mbps), 3: medium-load (20Mbps)",
                params.trafficScenario);
  cmd.AddValue ("scheduler",
                "PF: Proportional Fair, RR: Round-Robin",
                params.scheduler);
  cmd.AddValue ("bandwidth",
                "BW in MHz for each BWP (integer value): valid values are 20, 10, 5",
                params.bandwidthMHz);
  cmd.AddValue ("freqScenario",
                "0: NON_OVERLAPPING (each sector in different freq), 1: OVERLAPPING (same freq for all sectors)",
                params.freqScenario);
  cmd.AddValue ("downtiltAngle",
                "Base station antenna down tilt angle (deg)",
                params.downtiltAngle);
  cmd.AddValue ("enableUlPc",
                "Whether to enable or disable UL power control",
                params.enableUlPc);
  cmd.AddValue ("powerAllocation",
                "Power allocation can be a)UniformPowerAllocBw or b)UniformPowerAllocUsed.",
                params.powerAllocation);

  // Parse the command line
  cmd.Parse (argc, argv);
  params.Validate ();

  std::cout << params;

  ShowProgress spinner (Seconds (100));

  LenaLteComparison (params);

  return 0;
}
