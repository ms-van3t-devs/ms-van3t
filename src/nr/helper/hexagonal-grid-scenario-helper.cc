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
#include "hexagonal-grid-scenario-helper.h"
#include <ns3/double.h>
#include <ns3/mobility-helper.h>
#include "ns3/constant-velocity-mobility-model.h"
#include <cmath>

namespace ns3 {

HexagonalGridScenarioHelper::HexagonalGridScenarioHelper ()
{
  m_r = CreateObject<UniformRandomVariable> ();
  m_theta = CreateObject<UniformRandomVariable> ();
}

HexagonalGridScenarioHelper::~HexagonalGridScenarioHelper ()
{
}

const double distTo2ndRing = std::sqrt (3);
const double distTo4thRing = std::sqrt (7);
// Site positions in terms of distance and angle w.r.t. the central site
std::vector<double> HexagonalGridScenarioHelper::siteDistances {0,
                                                                1, 1, 1, 1, 1, 1,
                                                                distTo2ndRing, distTo2ndRing, distTo2ndRing, distTo2ndRing, distTo2ndRing, distTo2ndRing,
                                                                2, 2, 2, 2, 2, 2,
                                                                distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing, distTo4thRing,
                                                                3, 3, 3, 3, 3, 3
};

/*
 * Site angles w.r.t. the central site center.
 *
 * Note that the angles in the following vector are when looking the deployment in which hexagons are oriented in the following way:
 *
 *    ^               ______
 *    |              /      \
 *    |       ______/        \
 *    |      /      \        /
 *  y |     /        \______/
 *    |     \        /      \
 *    |      \______/        \
 *    |             \        /
 *    |              \______/
 *    ------------------------>
 *          x
 *
 *  This is important to note because the gnuplot function of this the HexagonalGridScenarioHelper plots hexagon in different orientation pointing towards top-bottom, e.g.:
 *
 *     /\
 *   /    \
 *  |      |
 *  |      |
 *   \    /
 *     \/
 *
*/

// the angle of the first hexagon of the fourth ring in the first quadrant
const double ang4thRingAlpha1 = atan2 (1, (3 * sqrt (3))) * (180 / M_PI);
// the angle of the second hexagon of the fourth ring in the first quadrant
const double ang4thRingAlpha2 = 90 - atan2 (sqrt (3), 2) * (180 / M_PI);
// the angle of the third hexagon of the fourth ring in the first quadrant
const double ang4thRingAlpha3 = 90 - atan2 (3 , (5 * sqrt(3))) * (180 / M_PI);

std::vector<double> HexagonalGridScenarioHelper::siteAngles {0, // 0 ring
                                                             30, 90, 150, 210, 270, 330, // 1. ring
                                                              0, 60, 120, 180, 240, 300, // 2. ring
                                                             30, 90, 150, 210, 270, 330, // 3. ring
                                                             ang4thRingAlpha1, ang4thRingAlpha2, ang4thRingAlpha3, // 4. ring 1. quadrant
                                                             180 - ang4thRingAlpha3, 180 - ang4thRingAlpha2, 180 - ang4thRingAlpha1, // 4. ring 2. quadrant
                                                             180 + ang4thRingAlpha1, 180 + ang4thRingAlpha2, 180 + ang4thRingAlpha3, // 4. ring 3. quadrant
                                                             - ang4thRingAlpha3, - ang4thRingAlpha2,  -ang4thRingAlpha1,// 4. ring 4. quadrant
                                                             30, 90, 150, 210, 270, 330  // 5. ring
};

/**
 * \brief Creates a GNUPLOT with the hexagonal deployment including base stations
 * (BS), their hexagonal cell areas and user terminals (UT). Positions and cell
 * radius must be given in meters
 *
 * \param sitePosVector Vector of site positions
 * \param cellCenterVector Vector of cell center positions
 * \param utPosVector Vector of user terminals positions
 * \param cellRadius Hexagonal cell radius in meters
 */
static void
PlotHexagonalDeployment (const Ptr<const ListPositionAllocator> &sitePosVector,
                         const Ptr<const ListPositionAllocator> &cellCenterVector,
                         const Ptr<const ListPositionAllocator> &utPosVector,
                         double cellRadius)
{
  uint16_t numCells = cellCenterVector->GetSize ();
  uint16_t numSites = sitePosVector->GetSize ();
  uint16_t numSectors = numCells / numSites;
  uint16_t numUts = utPosVector->GetSize ();
  NS_ASSERT_MSG (numCells > 0, "no cells");
  NS_ASSERT_MSG (numSites > 0, "no sites");
  NS_ASSERT_MSG (numUts > 0,   "no uts");

  // Try to open a new GNUPLOT file
  std::ofstream topologyOutfile;
  std::string topologyFileRoot = "./hexagonal-topology";
  std::string topologyFileName = topologyFileRoot + ".gnuplot";
  topologyOutfile.open (topologyFileName.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!topologyOutfile.is_open ())
    {
      NS_ABORT_MSG ("Can't open " << topologyFileName);
    }

  topologyOutfile << "set term pdf" << std::endl;
  topologyOutfile << "set output \"" << topologyFileRoot << ".pdf\"" << std::endl;
  topologyOutfile << "set style arrow 1 lc \"black\" lt 1 head filled" << std::endl;
//  topologyOutfile << "set autoscale" << std::endl;

  uint16_t margin = (12 * cellRadius) + 1;  //!< This is the farthest hexagonal vertex from the cell center
  topologyOutfile << "set xrange [-" << margin << ":" << margin <<"]" << std::endl;
  topologyOutfile << "set yrange [-" << margin << ":" << margin <<"]" << std::endl;
  //FIXME: Need to recalculate ranges if the scenario origin is different to (0,0)

  double arrowLength = cellRadius/4.0;  //<! Control the arrow length that indicates the orientation of the sectorized antenna
  std::vector<double> hx {0.0,-0.5,-0.5,0.0,0.5,0.5,0.0};   //<! Hexagon vertices in x-axis
  std::vector<double> hy {-1.0,-0.5,0.5,1.0,0.5,-0.5,-1.0}; //<! Hexagon vertices in y-axis
  Vector sitePos;

  for (uint16_t cellId = 0; cellId < numCells; ++cellId)
    {
      Vector cellPos = cellCenterVector->GetNext ();
      double angleDeg = 30 + 120 * (cellId % 3);
      double angleRad = angleDeg * M_PI / 180;
      double x, y;

      if (cellId % numSectors == 0)
        {
          sitePos = sitePosVector->GetNext ();
        }
      topologyOutfile << "set arrow " << cellId + 1 << " from " << sitePos.x
          << "," << sitePos.y << " rto " << arrowLength * std::cos(angleRad)
      << "," << arrowLength * std::sin(angleRad) << " arrowstyle 1 \n";

      // Draw the hexagon arond the cell center
      topologyOutfile << "set object " << cellId + 1 << " polygon from \\\n";

      for (uint16_t vertexId = 0; vertexId <= 6; ++vertexId)
        {
          // angle of the vertex w.r.t. y-axis
          x = cellRadius * std::sqrt(3.0) * hx.at (vertexId) + cellPos.x;
          y = cellRadius * hy.at (vertexId) + cellPos.y;
          topologyOutfile << x << ", " << y;
          if (vertexId == 6)
            {
              topologyOutfile << " front fs empty \n";
            }
          else
            {
              topologyOutfile << " to \\\n";
            }
        }

      topologyOutfile << "set label " << cellId + 1 << " \"" << (cellId + 1) <<
          "\" at " << cellPos.x << " , " << cellPos.y << " center" << std::endl;

    }

  for (uint16_t utId = 0; utId < numUts; ++utId)
    {
      Vector utPos = utPosVector->GetNext ();
//      set label at xPos, yPos, zPos "" point pointtype 7 pointsize 2
      topologyOutfile << "set label at " << utPos.x << " , " << utPos.y <<
          " point pointtype 7 pointsize 0.2 center" << std::endl;
    }

   topologyOutfile << "unset key" << std::endl; //!< Disable plot legends
   topologyOutfile << "plot 1/0" << std::endl;  //!< Need to plot a function

}

void
HexagonalGridScenarioHelper::SetNumRings (uint8_t numRings)
{
  NS_ABORT_MSG_IF(numRings > 5, "Unsupported number of outer rings (Maximum is 5");

  m_numRings = numRings;

  /*
   * 0 rings = 1 + 6 * 0 = 1 site
   * 1 rings = 1 + 6 * 1 = 7 sites
   * 2 rings = 1 + 6 * 2 = 13 sites
   * 3 rings = 1 + 6 * 3 = 19 site5
   * 4 rings = 1 + 6 * 5 = 31 sites
   * 5 rings = 1 + 6 * 6 = 37 sites
   */
  switch (numRings)
  {
    case 0:
      m_numSites = 1;
      break;
    case 1:
      m_numSites = 7;
      break;
    case 2:
      m_numSites = 13;
      break;
    case 3:
      m_numSites = 19;
      break;
    case 4:
      m_numSites = 31;
      break;
    case 5:
      m_numSites = 37;
      break;
  }
  SetSitesNumber (m_numSites);
}

double
HexagonalGridScenarioHelper::GetHexagonalCellRadius () const
{
  return m_hexagonalRadius;
}

Vector
HexagonalGridScenarioHelper::GetHexagonalCellCenter (const Vector &sitePos,
                                                     uint16_t cellId) const
{
  Vector center (sitePos);

  auto sectors = GetNumSectorsPerSite ();
  switch (sectors)
  {
  case 0:
      NS_ABORT_MSG ("Number of sectors has not been defined");
      break;

  case 1:
      break;

  case 3:
      switch (GetSectorIndex (cellId))
        {
        case 0:
          center.x += m_hexagonalRadius * std::sqrt (0.75);
          center.y += m_hexagonalRadius / 2;
          break;
          
        case 1:
          center.x -= m_hexagonalRadius * std::sqrt (0.75);
          center.y += m_hexagonalRadius / 2;
          break;

        case 2:
          center.y -= m_hexagonalRadius;
          break;

        default:
          NS_ABORT_MSG ("Unknown sector number: " << GetSectorIndex (cellId));
        }
      break;

    default:
      NS_ABORT_MSG("Unsupported number of sectors");
      break;
  }

  return center;
}


void
HexagonalGridScenarioHelper::CreateScenario ()
{
  m_hexagonalRadius = m_isd / 3;
  
  m_bs.Create (m_numBs);
  m_ut.Create (m_numUt);

  NS_ASSERT (m_isd > 0);
  NS_ASSERT (m_numRings < 6);
  NS_ASSERT (m_hexagonalRadius > 0);
  NS_ASSERT (m_bsHeight >= 0.0);
  NS_ASSERT (m_utHeight >= 0.0);
  NS_ASSERT (m_bs.GetN () > 0);
  NS_ASSERT (m_ut.GetN () > 0);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> bsPosVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> bsCenterVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> sitePosVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> utPosVector = CreateObject<ListPositionAllocator> ();

  // BS position
  for (uint16_t cellId = 0; cellId < m_numBs; cellId++)
    {
      uint16_t siteIndex = GetSiteIndex (cellId);
      Vector sitePos (m_centralPos);
      const double dist = siteDistances.at(siteIndex);
      const double angleRad = siteAngles.at(siteIndex) * M_PI / 180;
      sitePos.x += m_isd * dist * cos(angleRad);
      sitePos.y += m_isd * dist * sin(angleRad);
      sitePos.z = m_bsHeight;

      if (GetSectorIndex (cellId) == 0)
        {
          sitePosVector->Add (sitePos);
        }

      // FIXME: Until sites can have more than one antenna array, it is necessary to apply some distance offset from the site center (gNBs cannot have the same location)
      Vector bsPos = GetAntennaPosition (sitePos, cellId);

      bsPosVector->Add (bsPos);

      // Store cell center position for plotting the deployment
      Vector cellCenterPos = GetHexagonalCellCenter (bsPos, cellId);
      bsCenterVector->Add (cellCenterPos);

      //What about the antenna orientation? It should be dealt with when installing the gNB
    }

  // To allocate UEs, I need the center of the hexagonal cell.
  // Allocate UE around the disk of radius isd/3, the diameter of a the
  // hexagon representing the footprint of a single sector.
  // Reduce this radius by the min BS-UT distance, to respect that standoff
  // at the one corner of the sector hexagon where the sector antenna lies.
  // This results in UTs uniformly distributed in a disc centered on
  // the sector hexagon; there are no UTs near the vertices of the hexagon.
  // Spread UEs inside the inner hexagonal radius
  // Need to weight r to get uniform in the sector hexagon
  // See https://stackoverflow.com/questions/5837572
  // Set max = radius^2 here, then take sqrt below
  const double outerR = m_hexagonalRadius * std::sqrt(3) / 2 - m_minBsUtDistance;
  m_r->SetAttribute ("Min", DoubleValue (0));
  m_r->SetAttribute ("Max", DoubleValue (outerR * outerR));
  m_theta->SetAttribute ("Min", DoubleValue (-1.0 * M_PI));
  m_theta->SetAttribute ("Max", DoubleValue (M_PI));

  // UT position
  
  for (uint32_t utId = 0; utId < m_ut.GetN(); ++utId)
    {
      double d = std::sqrt (m_r->GetValue ());
      double t = m_theta->GetValue ();

      // Vector utPos (cellCenterPos);
      Vector utPos (bsCenterVector->GetNext ());
      utPos.x += d * cos (t);
      utPos.y += d * sin (t);
      utPos.z = m_utHeight;
      
      utPosVector->Add (utPos);
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (bsPosVector);
  mobility.Install (m_bs);

  mobility.SetPositionAllocator (utPosVector);
  mobility.Install (m_ut);

  PlotHexagonalDeployment (sitePosVector, bsCenterVector, utPosVector, m_hexagonalRadius);

}

void
HexagonalGridScenarioHelper::CreateScenarioWithMobility (const Vector &speed, double percentage)
{
  m_hexagonalRadius = m_isd / 3;

  m_bs.Create (m_numBs);
  m_ut.Create (m_numUt);

  NS_ASSERT (m_isd > 0);
  NS_ASSERT (m_numRings < 6);
  NS_ASSERT (m_hexagonalRadius > 0);
  NS_ASSERT (m_bsHeight >= 0.0);
  NS_ASSERT (m_utHeight >= 0.0);
  NS_ASSERT (m_bs.GetN () > 0);
  NS_ASSERT (m_ut.GetN () > 0);
  NS_ASSERT_MSG (percentage >=0 || percentage <=1, "Percentage must between 0"
                                                   " and 1");

  MobilityHelper mobility;
  MobilityHelper ueMobility;
  Ptr<ListPositionAllocator> bsPosVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> bsCenterVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> sitePosVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> utPosVector = CreateObject<ListPositionAllocator> ();

  // BS position
  for (uint16_t cellId = 0; cellId < m_numBs; cellId++)
    {
      uint16_t siteIndex = GetSiteIndex (cellId);
      Vector sitePos (m_centralPos);
      const double dist = siteDistances.at(siteIndex);
      const double angleRad = siteAngles.at(siteIndex) * M_PI / 180;
      sitePos.x += m_isd * dist * cos(angleRad);
      sitePos.y += m_isd * dist * sin(angleRad);
      sitePos.z = m_bsHeight;

      if (GetSectorIndex (cellId) == 0)
        {
          sitePosVector->Add (sitePos);
        }

      // FIXME: Until sites can have more than one antenna array, it is necessary to apply some distance offset from the site center (gNBs cannot have the same location)
      Vector bsPos = GetAntennaPosition (sitePos, cellId);

      bsPosVector->Add (bsPos);

      // Store cell center position for plotting the deployment
      Vector cellCenterPos = GetHexagonalCellCenter (bsPos, cellId);
      bsCenterVector->Add (cellCenterPos);

      //What about the antenna orientation? It should be dealt with when installing the gNB
    }

  // To allocate UEs, I need the center of the hexagonal cell.
  // Allocate UE around the disk of radius isd/3, the diameter of a the
  // hexagon representing the footprint of a single sector.
  // Reduce this radius by the min BS-UT distance, to respect that standoff
  // at the one corner of the sector hexagon where the sector antenna lies.
  // This results in UTs uniformly distributed in a disc centered on
  // the sector hexagon; there are no UTs near the vertices of the hexagon.
  // Spread UEs inside the inner hexagonal radius
  // Need to weight r to get uniform in the sector hexagon
  // See https://stackoverflow.com/questions/5837572
  // Set max = radius^2 here, then take sqrt below
  const double outerR = m_hexagonalRadius * std::sqrt(3) / 2 - m_minBsUtDistance;
  m_r->SetAttribute ("Min", DoubleValue (0));
  m_r->SetAttribute ("Max", DoubleValue (outerR * outerR));
  m_theta->SetAttribute ("Min", DoubleValue (-1.0 * M_PI));
  m_theta->SetAttribute ("Max", DoubleValue (M_PI));

  // UT position

  uint32_t numUesWithRandomUtHeight = 0;
  if (percentage != 0)
    {
      numUesWithRandomUtHeight = percentage * m_ut.GetN ();
    }

  for (uint32_t utId = 0; utId < m_ut.GetN (); ++utId)
    {
      double d = std::sqrt (m_r->GetValue ());
      double t = m_theta->GetValue ();

      // Vector utPos (cellCenterPos);
      Vector utPos (bsCenterVector->GetNext ());
      utPos.x += d * cos (t);
      utPos.y += d * sin (t);

      if (numUesWithRandomUtHeight > 0)
        {
          Ptr<UniformRandomVariable> uniformRandomVariable = CreateObject<UniformRandomVariable> ();
          double Nfl = uniformRandomVariable->GetValue (4, 8);
          double nfl = uniformRandomVariable->GetValue (1, Nfl);
          utPos.z = 3 * (nfl - 1) + 1.5;

          numUesWithRandomUtHeight--;
        }
      else
        {
          utPos.z = m_utHeight;
        }

      utPosVector->Add (utPos);
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (bsPosVector);
  mobility.Install (m_bs);

  //mobility.SetPositionAllocator (utPosVector);
  //mobility.Install (m_ut);

  ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  ueMobility.SetPositionAllocator (utPosVector);
  ueMobility.Install (m_ut);

  for (uint32_t i = 0; i < m_ut.GetN (); i++)
    {
      m_ut.Get (i)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (speed);
    }


  PlotHexagonalDeployment (sitePosVector, bsCenterVector, utPosVector, m_hexagonalRadius);

}

int64_t
HexagonalGridScenarioHelper::AssignStreams (int64_t stream)
{
  m_r->SetStream (stream);
  m_theta->SetStream (stream + 1);
  return 2;
}

} // namespace ns3
