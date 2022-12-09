/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Lawrence Livermore National Laboratory
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
 *   Author:  Peter D. Barnes, Jr. <barnes26@llnl.gov>
 *
 */
#include "file-scenario-helper.h"
#include <ns3/core-module.h>
#include <ns3/mobility-module.h>

#include <cmath>  // M_PI (but non-standard)
#include <sstream>

using namespace ns3;

/** Unnamed namespace */
namespace {

/**
 * \brief Creates a GNUPLOT with the deployment including base stations
 * (BS) and user terminals (UT). Positions and cell
 * radius must be given in meters
 *
 * \param sitePositioner Site position allocator
 * \param utPositioner Vector of user terminals positions
 * \param numSectors the plot deployment sector parameter
 * \param maxRadius the plot deployment max radius
 * \param effIsd the plot deployment ISD
 */
static void
PlotDeployment (const Ptr<const ListPositionAllocator> &sitePositioner,
                const Ptr<const ListPositionAllocator> &utPositioner,
                const std::size_t numSectors,
                const double maxRadius,
                const double effIsd)
{
  std::size_t numSites = sitePositioner->GetSize ();
  std::size_t numUts = utPositioner->GetSize ();
  std::size_t numCells = numSites * numSectors;
  
  NS_ASSERT (numSites);
  NS_ASSERT (numUts);
  NS_ASSERT (numSectors > 0);
  NS_ASSERT (maxRadius > 0);
  NS_ASSERT (effIsd < maxRadius);

  // Try to open a new GNUPLOT file
  std::ofstream topologyOutfile;
  std::string topologyFileRoot = "./list-topology";
  std::string topologyFileName = topologyFileRoot + ".gnuplot";
  topologyOutfile.open (topologyFileName.c_str (),
                        std::ios_base::out | std::ios_base::trunc);
  if (!topologyOutfile.is_open ())
    {
      NS_ABORT_MSG ("Can't open " << topologyFileName);
    }

  topologyOutfile << "set term pdf" << std::endl;
  topologyOutfile << "set output \"" << topologyFileRoot << ".pdf\"" << std::endl;
  topologyOutfile << "set style arrow 1 linecolor \"red\" linewidth 2 head filled" << std::endl;
  //  topologyOutfile << "set autoscale" << std::endl;

  // Extent of the plot, 5% margin all around
  std::size_t radius = maxRadius * 1.05;  
  topologyOutfile << "set xrange [-" << radius << ":" << radius <<"]" << std::endl;
  topologyOutfile << "set yrange [-" << radius << ":" << radius <<"]" << std::endl;

  // Arrow style
  /** \todo: Need to recalculate ranges if the scenario origin is different to (0,0) */

  // Plo the UEs first, so the sector arrows are on top
  for (std::size_t utId = 0; utId < numUts; ++utId)
    {
      Vector utPos = utPositioner->GetNext ();
      // set label at xPos, yPos, zPos "" point pointtype 7 pointsize 2
      topologyOutfile << "set label at " << utPos.x << " , " << utPos.y <<
          " point pointtype 7 pointsize 0.5 center" << std::endl;
    }

  // Control the arrow length that indicates
  // the orientation of the sectorized antenna.
  // Set it to 1/3 the ISD:    x --->     <--- x
  double arrowLength = effIsd;
    
  const double sectorSize = 2 * M_PI / numSectors;
  std::size_t cellId = 0;
  
  for (std::size_t siteId = 0; siteId < numSites; ++siteId)
    {
      Vector site = sitePositioner->GetNext ();

      for (std::size_t sector = 0; sector < numSectors; ++sector)
        {
          double angle = 0.0;
          if (numSectors > 1)
            {
              std::size_t sector = cellId % numSectors;
              // 0'th sector spans from [0, sectorSize] in degrees,
              // centered at sectorSize / 2
              angle = sectorSize * (sector + 0.5);
            }
          double rx = arrowLength * std::cos (angle);
          double ry = arrowLength * std::sin (angle);
          
          topologyOutfile << "set arrow " << cellId + 1
                          << " from " << site.x << "," << site.y
                          << " rto " << rx << "," << ry
                          << " arrowstyle 1\n";

          topologyOutfile << "set label " << cellId + 1
                          << " \"" << (cellId + 1)
                          << "\" at " << site.x << "," << site.y
                          << " center" << std::endl;
          ++cellId;
        }
    }
  NS_ASSERT (cellId == numCells);

   topologyOutfile << "unset key" << std::endl; // Disable plot legends
   topologyOutfile << "plot 1/0" << std::endl;  // Need to plot a function

}  // PlotDeployment ()

}  // unnamed namespace


namespace ns3 {


FileScenarioHelper::~FileScenarioHelper (void)
{
}

void
FileScenarioHelper::Add (const std::string filePath,
                         char delimiter /* = ',' */)
{
  if (! m_bsPositioner)
    {
      m_bsPositioner = CreateObject<ListPositionAllocator> ();
    }
  m_bsPositioner->Add (filePath, m_bsHeight, delimiter);
  auto numSites = m_bsPositioner->GetSize ();
  SetSitesNumber (numSites);
}

void
FileScenarioHelper::CheckScenario (const char * where) const
{
  NS_ASSERT_MSG (m_scenarioCreated,
                 "Must Add() position file and CreateScenario() "
                 "before calling " << where);
}

Vector
FileScenarioHelper::GetSitePosition (std::size_t cellId) const
{
  CheckScenario (__FUNCTION__);
  
  auto node = m_bs.Get (cellId);
  auto mob = node->GetObject<MobilityModel> ();
  return mob->GetPosition ();
}

void
FileScenarioHelper::CreateScenario ()
{
  NS_ASSERT_MSG (m_bsPositioner,
                 "Must Add() a position file before CreateScenario()");
  
  //NS_ASSERT_MSG (m_numSites > 0,
      //           "Must have at least one site location in the position file.");
  
  NS_ASSERT_MSG (m_sectorization != NONE,
                 "Must SetSectorization() before CreateScenario()");
  NS_ASSERT_MSG (m_bsHeight >= 0.0,
                 "Must SetBsHeight() before CreateScenario()");
  NS_ASSERT_MSG (m_utHeight >= 0.0,
                 "Must SetUtHeight() before CreateScenario()");

  auto sectors = GetNumSectorsPerSite ();
  std::cout << "      creating BS" << std::endl;
  m_bs.Create (m_numBs);
  std::cout << "      creating UE" << std::endl;
  m_ut.Create (m_numUt);

  // Accumulate maxRadius and effIsd
  double maxRadius = 0;
  // For effective ISD we have to iterate over all pairs of towers :(
  // so cache the positions from m_bsPositioner
  std::cout << "      reserving sitePositions" << std::endl;
  std::vector<Vector> sitePositions;
  sitePositions.reserve (m_numSites);
  
  // Position the BS cells
  std::cout << "      creating bsPositions" << std::endl;
  Ptr<ListPositionAllocator> bsPositions = CreateObject<ListPositionAllocator> ();
  std::size_t cellId = 0;
  for (std::size_t siteId = 0; siteId < m_numSites; ++siteId)
    {
      Vector site = m_bsPositioner->GetNext ();
      sitePositions.push_back (site);

      maxRadius = std::max (maxRadius, std::abs(site.x));
      maxRadius = std::max (maxRadius, std::abs(site.y));

      std::cout << "        sectors for site " << siteId << std::endl;
      for (std::size_t sector = 0; sector < sectors; ++sector)
        {
          Vector bs {site};
          double angle = GetAntennaOrientationRadians(cellId);
          bs.x += m_antennaOffset * cos (angle);
          bs.y += m_antennaOffset * sin (angle);

          bsPositions->Add (bs);
          ++cellId;

        }  // for cell

    }  // for site
  NS_ASSERT_MSG (cellId == m_numBs,
                 "Computed positions for " << cellId <<
                 " cells, expected " << m_numBs);


  std::cout << "      BS mobility" << std::endl;
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (m_bsPositioner);
  mobility.Install (m_bs);
  
  // Compute effective ISD
  double effIsd = 0;
  std::cout << "      computing average Isd..." << std::flush;
  for (auto v = sitePositions.begin ();
       v != sitePositions.end ();
       ++v)
    {
      auto w = v;
      ++w;
      for ( ; w < sitePositions.end(); ++w)
        {
          NS_ASSERT (w != v);
          double range = (*w - *v).GetLength ();
          effIsd += range;

        }  // for w : sitePositions

    }  // for v : sitePositions
  effIsd /= m_numSites * (m_numSites - 1) / 2;
  std::cout << effIsd << std::endl;
  

  // Position the UEs uniformly in the sector annulus
  std::cout << "      UE positions" << std::endl;
  // equivalent for a hexagonal laydown:
  // between m_minBsUtDistance and m_isd / sqrt (3)
  std::cout << "        radius rng..." << std::flush;
  Ptr<UniformRandomVariable> r = CreateObject<UniformRandomVariable> ();
  std::cout << "setting stream..." << std::flush;
  r->SetStream (RngSeedManager::GetNextStreamIndex ());
  std::cout << "done" << std::endl;
  const double outerR =std::min (effIsd, m_isd); // / std::sqrt(3);
  NS_ASSERT (m_minBsUtDistance < outerR);
  // Need to weight r to get uniform in the sector wedge
  // See https://stackoverflow.com/questions/5837572
  r->SetAttribute ("Min", DoubleValue (m_minBsUtDistance * m_minBsUtDistance));
  r->SetAttribute ("Max", DoubleValue (outerR * outerR));
  std::cout << "[" << m_minBsUtDistance << "," << outerR << "]" << std::endl;
  std::cout << "        using effIsd " << outerR << std::endl;

  std::cout << "        theta rng..." << std::flush;
  Ptr<UniformRandomVariable> theta = CreateObject<UniformRandomVariable> ();
  std::cout << "setting stream..." << std::flush;
  theta->SetStream (RngSeedManager::GetNextStreamIndex ());
  std::cout << "done" << std::endl;

  Ptr<ListPositionAllocator> utPositioner = CreateObject<ListPositionAllocator> ();;  
  for (uint32_t utId = 0; utId < m_numUt; ++utId)
    {
      auto cellId = GetCellIndex (utId);
      auto siteId = GetSiteIndex (cellId);
      Vector cellPos = sitePositions[siteId];
      
      // Need to weight r to get uniform in the sector wedge
      // See https://stackoverflow.com/questions/5837572
      double d = std::sqrt (r->GetValue ());
      double boreSight = GetAntennaOrientationRadians (cellId);
      double halfWidth = 2 * M_PI / sectors / 2;
      theta->SetAttribute ("Min", DoubleValue (boreSight - halfWidth));
      theta->SetAttribute ("Max", DoubleValue (boreSight + halfWidth));
      double t = theta->GetValue ();
      
      Vector utPos (cellPos);
      utPos.x += d * cos (t);
      utPos.y += d * sin (t);
      utPos.z = m_utHeight;
      
      utPositioner->Add (utPos);
    }

  std::cout << "      UE mobility" << std::endl;
  mobility.SetPositionAllocator (utPositioner);
  mobility.Install (m_ut);
  
  std::cout << "      plot deployment" << std::endl;
  PlotDeployment (m_bsPositioner, utPositioner, sectors, maxRadius, outerR);

  m_scenarioCreated = true;
}

} // namespace ns3
