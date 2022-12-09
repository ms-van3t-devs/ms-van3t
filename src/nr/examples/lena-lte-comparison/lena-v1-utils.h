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
#ifndef LENA_V1_UTILS_H
#define LENA_V1_UTILS_H

#include <ns3/nr-module.h>
#include <ns3/hexagonal-grid-scenario-helper.h>
#include <ns3/lte-module.h>

#include "sinr-output-stats.h"


namespace ns3 {

class SinrOutputStats;
class PowerOutputStats;
class SlotOutputStats;

class LenaV1Utils
{
public:
  static void
  SetLenaV1SimulatorParameters (const double sector0AngleRad,
                                std::string scenario,
                                NodeContainer enbSector1Container,
                                NodeContainer enbSector2Container,
                                NodeContainer enbSector3Container,
                                NodeContainer ueSector1Container,
                                NodeContainer ueSector2Container,
                                NodeContainer ueSector3Container,
                                Ptr<PointToPointEpcHelper> &epcHelper,
                                Ptr<LteHelper> &lteHelper,
                                NetDeviceContainer &enbSector1NetDev,
                                NetDeviceContainer &enbSector2NetDev,
                                NetDeviceContainer &enbSector3NetDev,
                                NetDeviceContainer &ueSector1NetDev,
                                NetDeviceContainer &ueSector2NetDev,
                                NetDeviceContainer &ueSector3NetDev,
                                bool calibration,
                                bool enableUlPc,
                                SinrOutputStats *sinrStats,
                                PowerOutputStats *powerStats,
                                const std::string &scheduler,
                                uint32_t bandwidthMHz,
                                uint32_t freqScenario,
                                double downtiltAngle);

  static void
  ReportSinrLena (SinrOutputStats *stats, uint16_t cellId, uint16_t rnti, double power, double avgSinr, uint8_t bwpId);
  static void
  ReportPowerLena (PowerOutputStats *stats, uint16_t rnti, Ptr<SpectrumValue> txPsd);

};

} // namespace ns3

#endif // LENA_V1_UTILS_H
