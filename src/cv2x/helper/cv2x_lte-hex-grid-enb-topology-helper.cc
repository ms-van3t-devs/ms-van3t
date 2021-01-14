/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by: NIST
 */


#include "cv2x_lte-hex-grid-enb-topology-helper.h"
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/cv2x_epc-helper.h>
#include <ns3/angles.h>
#include <ns3/random-variable-stream.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/cv2x_lte-ue-net-device.h>
#include <ns3/cv2x_lte-ue-phy.h>
#include <ns3/cv2x_lte-spectrum-phy.h>
#include <ns3/buildings-module.h>
#include <iostream>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteHexGridEnbTopologyHelper");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteHexGridEnbTopologyHelper);

cv2x_LteHexGridEnbTopologyHelper::cv2x_LteHexGridEnbTopologyHelper ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteHexGridEnbTopologyHelper::~cv2x_LteHexGridEnbTopologyHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId cv2x_LteHexGridEnbTopologyHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_LteHexGridEnbTopologyHelper")
    .SetParent<Object> ()
    .AddConstructor<cv2x_LteHexGridEnbTopologyHelper> ()
    .AddAttribute ("InterSiteDistance",
                   "The distance [m] between nearby sites",
                   DoubleValue (500),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_d),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SectorOffset",
                   "The offset [m] in the position for the node of each sector with respect "
		   "to the center of the three-sector site",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_offset),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SiteHeight",
                   "The height [m] of each site",
                   DoubleValue (30),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_siteHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("UeHeight",
                   "The height [m] of each UE",
                   DoubleValue (1.5),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_ueHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NumberOfRings",
                   "The number of rings in the grid",
                   DoubleValue (3),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_nbRings),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MinX", "The x coordinate where the hex grid starts.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_xMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinY", "The y coordinate where the hex grid starts.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_yMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("GridWidth", "The number of sites in even rows (odd rows will have one additional site).",
                   UintegerValue (1),
                   MakeUintegerAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_gridWidth),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CenterX", "The x coordinate of the center of the hex grid.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_xCenter),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CenterY", "The y coordinate of the center of the hex grid.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_yCenter),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinDistance", "The minimum distance between eNodeB and UE.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_minDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinEnodeb2HotspotDistance", "The minimum distance between eNodeB and hotspot.",
                   DoubleValue (75.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_minEnodeb2HotspotDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinHotspot2HotspotDistance", "The minimum distance between hotspot centers.",
                   DoubleValue (80.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_minHotspot2HotspotDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("HotspotRange", "The radius of the hotspot.",
                   DoubleValue (40.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_hotspotRange),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinEnodeb2BuildingDistance", "The minimum distance between the center of the building and the eNodeB.",
                   DoubleValue (100.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_minEnodeb2BuildingDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinUe2UeDistanceInBuilding", "The minimum distance between 2 UEs inside a building.",
                   DoubleValue (3.0),
                   MakeDoubleAccessor (&cv2x_LteHexGridEnbTopologyHelper::m_minUe2UeDistanceInBuilding),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

void
cv2x_LteHexGridEnbTopologyHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}


void 
cv2x_LteHexGridEnbTopologyHelper::SetLteHelper (Ptr<cv2x_LteHelper> h)
{
  NS_LOG_FUNCTION (this << h);
  m_lteHelper = h;
}

void 
cv2x_LteHexGridEnbTopologyHelper::SetNbRings (int ring)
{
  NS_ASSERT (ring >0);
  m_nbRings = ring;
}
  
void 
cv2x_LteHexGridEnbTopologyHelper::SetCenter (double x, double y)
{
  m_xCenter = x;
  m_yCenter = y;    
}

void 
cv2x_LteHexGridEnbTopologyHelper::SetInterSiteDistance (double d)
{
  NS_ASSERT (d > 0);
  m_d = d;
}

void
cv2x_LteHexGridEnbTopologyHelper::SetSiteHeight (double height)
{
  NS_ASSERT (height > 0);
  m_siteHeight = height;
}

void
cv2x_LteHexGridEnbTopologyHelper::SetUeHeight (double height)
{
  NS_ASSERT (height > 0);
  m_ueHeight = height;
}

void 
cv2x_LteHexGridEnbTopologyHelper::SetMinimumDistance (double m)
{
  if(m >= m_d/2)
  {
    NS_FATAL_ERROR ("UE minimum distance from eNB must be less than ISD/2= " << m_d/2);
  }
  m_minDistance = m;
}

NetDeviceContainer 
cv2x_LteHexGridEnbTopologyHelper::SetPositionAndInstallEnbDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer enbDevs;
  const double xydfactor = std::sqrt (0.75);
  double yd = xydfactor*m_d;
  for (uint32_t n = 0; n < c.GetN (); ++n)
    {
      uint32_t currentSite = n / 3; 
      uint32_t biRowIndex = (currentSite / (m_gridWidth + m_gridWidth + 1));
      uint32_t biRowRemainder = currentSite % (m_gridWidth + m_gridWidth + 1);
      uint32_t rowIndex = biRowIndex*2;
      uint32_t colIndex = biRowRemainder; 
      if (biRowRemainder >= m_gridWidth)
	{
	  ++rowIndex;
	  colIndex -= m_gridWidth;
	}
      NS_LOG_LOGIC ("node " << n << " site " << currentSite 
		    << " rowIndex " << rowIndex 
		    << " colIndex " << colIndex 
		    << " biRowIndex " << biRowIndex
		    << " biRowRemainder " << biRowRemainder);
      double y = m_yMin + yd * rowIndex;
      double x;
      double antennaOrientation;
      if ((rowIndex % 2) == 0) 
	{
	  x = m_xMin + m_d * colIndex;
	}
      else // row is odd
	{
	  x = m_xMin -(0.5*m_d) + m_d * colIndex;
	}
      
      switch (n%3)
	{
	case 0:
	  antennaOrientation = 0;
	  x += m_offset;
	  m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue (1));
	  break;
	  
	case 1:
	  antennaOrientation = 120;
	  x -= m_offset/2.0;
	  y += m_offset*xydfactor;
	  m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue (2));
	  break;
	  
	case 2:
	  antennaOrientation = -120;
	  x -= m_offset/2.0;
	  y -= m_offset*xydfactor;
	  m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue (3));
	  break;
	
          // no default, n%3 = 0, 1, 2
	}
      Ptr<Node> node = c.Get (n);
      Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
      Vector pos (x, y, m_siteHeight);
      NS_LOG_LOGIC ("node " << n << " at " << pos << " antennaOrientation " << antennaOrientation);
      mm->SetPosition (Vector (x, y, m_siteHeight));
      m_lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (antennaOrientation));
      enbDevs.Add (m_lteHelper->InstallEnbDevice (node));
    }
  return enbDevs;
}

std::vector<Vector>
cv2x_LteHexGridEnbTopologyHelper::GetNodePositions () {
  NS_LOG_FUNCTION (this);

  uint32_t currentRing = 0;
  uint32_t sitesInRing = 0;
  uint32_t currentSitesInRing = 0;
  double x_site = 0;
  double y_site = 0;
  std::vector<Vector> positions;

  for (uint32_t n = 0; n < GetNumberOfNodes() ;)
    {      
      //Compute site location
      if (currentSitesInRing == sitesInRing) 
        {
          //we completed the ring, move to the next one
          currentRing++;
          sitesInRing = currentRing==1?1:(currentRing-1)*6;
          currentSitesInRing = 0;
          x_site = 0;
          y_site = (currentRing -1) * m_d;
        } 
      
      for (uint32_t m = 0; m < 3; ++m,++n) 
        {              
          uint32_t currentSector = n % 3;    

          double antennaOrientation;
      
          //Compute antenna orientation and position      
          antennaOrientation = 30 + 120 * currentSector;
          Vector pos (x_site, y_site, antennaOrientation);
          NS_LOG_LOGIC ("node " << n << " at " << pos << " antennaOrientation " << antennaOrientation);
          positions.push_back (pos);
        }

      if (currentRing > 1) {
        switch (currentSitesInRing/(currentRing-1)) {
        case 0:
          y_site -= m_d /2;
          x_site -= m_d * std::cos(DegreesToRadians(30));
          break;
        case 1:
          y_site -= m_d;
          break;
        case 2:
          y_site -= m_d /2;
          x_site += m_d * std::cos(DegreesToRadians(30));
          break;
        case 3:
          y_site += m_d /2;
          x_site += m_d * std::cos(DegreesToRadians(30));
          break;
        case 4:
          y_site += m_d;
          break;
        case 5:
          y_site += m_d /2;
          x_site -= m_d * std::cos(DegreesToRadians(30));
          break;
        default:
          //no default
          break;
        }
      }
      //increment counters of sites in ring
      currentSitesInRing++; 
    }
  return positions;
}


NetDeviceContainer
cv2x_LteHexGridEnbTopologyHelper::DropUEsUniformlyPerSector (NodeContainer ues)
{
  NS_LOG_FUNCTION (this);

  //make sure there will be equal number of UEs per cell
  NS_ASSERT ( ues.GetN () % GetNumberOfNodes()==0);

  //Storage for return value
  NetDeviceContainer ueDevs;

  uint32_t nbUesPerSector = ues.GetN () / GetNumberOfNodes();
  uint32_t currentUe = 0;
  Ptr<UniformRandomVariable> rndPos = CreateObject<UniformRandomVariable> ();

  //Get node position and antenna orientation
  std::vector<Vector> eNodeBpos = GetNodePositions ();
  //Loop through each cell
  for (uint32_t n = 0; n < eNodeBpos.size(); n++)
    {  
      //position uses z for the antenna direction
      Vector pos = eNodeBpos.at(n);
      double x_sector, y_sector;
      //Compute center of sector hexagon
      //This hexagon has center sector center, m_d/3
      x_sector = pos.x + m_d / 3 * std::cos(DegreesToRadians(pos.z));
      y_sector = pos.y + m_d / 3 * std::sin(DegreesToRadians(pos.z));
      NS_LOG_LOGIC ("sector_center " << n << " at " << x_sector << ":" << y_sector);
      
      //Drop users in the sector coverage
      for (uint32_t u = 0 ; u < nbUesPerSector ; u++) {
        double x_pos, y_pos;
        do 
          {
            //pick random x and y
            x_pos = pos.x + rndPos->GetValue (- m_d, m_d);
            y_pos = pos.y + rndPos->GetValue (- m_d, m_d);
          }
        while (!(IsInsideHex(x_sector, y_sector, m_d/3*std::cos(DegreesToRadians(30)), x_pos, y_pos, false)) || GetDistance(pos.x, pos.y, x_pos, y_pos) < m_minDistance);
        // do while( Not in the intersection of cell_hex with sector_hex, or distance < m_minDistance)
	
        //assign position to node
        Vector pos (x_pos, y_pos, m_ueHeight);        
        Ptr<Node> node = ues.Get (currentUe);
        Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
        NS_LOG_LOGIC ("ue " << u << " at " << pos);
        mm->SetPosition (Vector (x_pos, y_pos, m_ueHeight));
        ueDevs.Add (m_lteHelper->InstallUeDevice (node));
        
        //increment UE counter
        currentUe ++;
      }       

    }
  return ueDevs;
}

NetDeviceContainer
cv2x_LteHexGridEnbTopologyHelper::DropUEsUniformlyPerSector2 (NodeContainer ues)
{
  NS_LOG_FUNCTION (this);

  //make sure there will be equal number of UEs per cell
  NS_ASSERT ( ues.GetN () % GetNumberOfNodes()==0);

  //Storage for return value
  NetDeviceContainer ueDevs;

  uint32_t nbUesPerSector = ues.GetN () / GetNumberOfNodes();
  uint32_t currentUe = 0;
  Ptr<UniformRandomVariable> rndPos = CreateObject<UniformRandomVariable> ();

  //Get node position and antenna orientation
  std::vector<Vector> eNodeBpos = GetNodePositions ();
  //Loop through each cell
  for (uint32_t n = 0; n < eNodeBpos.size(); n++)
    {  
      //position uses z for the antenna direction
      Vector pos = eNodeBpos.at(n);
      double x_sector, y_sector;
      //Compute center of sector hexagon
      //This hexagon has center on cell flat edge
      x_sector = pos.x + m_d / 2 * std::cos(DegreesToRadians(pos.z));
      y_sector = pos.y + m_d / 2 * std::sin(DegreesToRadians(pos.z));
      NS_LOG_LOGIC ("sector_center " << n << " at " << x_sector << ":" << y_sector);
      
      //Drop users in the sector coverage
      for (uint32_t u = 0 ; u < nbUesPerSector ; u++) {
        double x_pos, y_pos;
        do 
          {
            //pick random x and y
            x_pos = pos.x + rndPos->GetValue (- m_d, m_d);
            y_pos = pos.y + rndPos->GetValue (- m_d, m_d);
          }
        while (!(IsInsideHex(pos.x, pos.y, m_d/2, x_pos, y_pos, true) && IsInsideHex(x_sector, y_sector, m_d/2*std::cos(DegreesToRadians(30)), x_pos, y_pos, false)) || GetDistance(pos.x, pos.y, x_pos, y_pos) < m_minDistance);
        // do while( Not in the intersection of cell_hex with sector_hex, or distance < m_minDistance)
				
        //assign position to node
        Vector pos (x_pos, y_pos, m_ueHeight);        
        Ptr<Node> node = ues.Get (currentUe);
        Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
        NS_LOG_LOGIC ("ue " << u << " at " << pos);
        mm->SetPosition (Vector (x_pos, y_pos, m_ueHeight));
		ueDevs.Add (m_lteHelper->InstallUeDevice (node));
        
        //increment UE counter
        currentUe ++;
      }       

    }
  return ueDevs;
}

NetDeviceContainer
cv2x_LteHexGridEnbTopologyHelper::DropUEsHotspot (NodeContainer ues)
{
  NS_LOG_FUNCTION (this);

  //make sure there will be equal number of UEs per cell
  NS_ASSERT ( ues.GetN () % GetNumberOfNodes()==0);

  //Storage for return value
  NetDeviceContainer ueDevs;

  uint32_t nbUesPerSector = ues.GetN () / GetNumberOfNodes();
  uint32_t nbUesHotspot = 2 * nbUesPerSector / 3;
  uint32_t currentUe = 0;
  Ptr<UniformRandomVariable> rndPos = CreateObject<UniformRandomVariable> ();

  //Get node position and antenna orientation
  std::vector<Vector> eNodeBpos = GetNodePositions ();
  std::vector<Vector> hotspotpos;
  //Loop through each cell
  for (uint32_t n = 0; n < eNodeBpos.size(); n++)
    {  
      //position uses z for the antenna direction
      Vector pos = eNodeBpos.at(n);
      double x_sector, y_sector;
      double x_pos, y_pos;                     
      //Compute center of sector hexagon
      //This hexagon has center on cell flat edge
      x_sector = pos.x + m_d / 2 * std::cos(DegreesToRadians(pos.z));
      y_sector = pos.y + m_d / 2 * std::sin(DegreesToRadians(pos.z));
      NS_LOG_LOGIC ("sector_center " << n << " at " << x_sector << ":" << y_sector);
      
      //Randomly select an area within each cell area
      do 
        {
          //pick random x and y
          x_pos = pos.x + rndPos->GetValue (- m_d, m_d);
          y_pos = pos.y + rndPos->GetValue (- m_d, m_d);
        }
      while (!(IsInsideHex(pos.x, pos.y, m_d/2, x_pos, y_pos, true) && IsInsideHex(x_sector, y_sector, m_d/2*std::cos(DegreesToRadians(30)), x_pos, y_pos, false)) || GetDistance(pos.x, pos.y, x_pos, y_pos) < m_minEnodeb2HotspotDistance || !IsAtLeastDistance(x_pos, y_pos, hotspotpos, m_minHotspot2HotspotDistance));
      // do while( Not in the intersection of cell_hex with sector_hex, or distance < m_minDistance, or min distance to other hotspots not meet)
      Vector hotspotPosition (x_pos, y_pos, 0);
      NS_LOG_LOGIC ("hotspot " << hotspotpos.size() << " at " << hotspotPosition);
      hotspotpos.push_back (hotspotPosition);   

      //Randomly and uniformly drop 2/3 UEs within 40 m of the selected area
      for (uint32_t u = 0 ; u < nbUesHotspot ; u++) 
        {
          do 
            {
              //pick random x and y
              x_pos = hotspotPosition.x + rndPos->GetValue (- m_hotspotRange, m_hotspotRange);
              y_pos = hotspotPosition.y + rndPos->GetValue (- m_hotspotRange, m_hotspotRange);
            }
          while (GetDistance(hotspotPosition.x, hotspotPosition.y, x_pos, y_pos) > m_hotspotRange);
          // do while( distance > m_hotpotRange)
				
          //assign position to node
          Vector pos (x_pos, y_pos, m_ueHeight);        
          Ptr<Node> node = ues.Get (currentUe);
          Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
          NS_LOG_LOGIC ("ue " << u << " at " << pos);
          mm->SetPosition (Vector (x_pos, y_pos, m_ueHeight));
          ueDevs.Add (m_lteHelper->InstallUeDevice (node));
        
          //increment UE counter
          currentUe ++;
        }       
      //Randomly and uniformly drop the remaining 1/3 UEs to the entire macro geographical area of the given macro cell
      for (uint32_t u = nbUesHotspot ; u < nbUesPerSector ; u++) 
        {
          do 
            {
              //pick random x and y
              x_pos = pos.x + rndPos->GetValue (- m_d, m_d);
              y_pos = pos.y + rndPos->GetValue (- m_d, m_d);
            }
          while (!(IsInsideHex(pos.x, pos.y, m_d/2, x_pos, y_pos, true) && IsInsideHex(x_sector, y_sector, m_d/2*std::cos(DegreesToRadians(30)), x_pos, y_pos, false)) || GetDistance(pos.x, pos.y, x_pos, y_pos) < m_minDistance);
          // do while( Not in the intersection of cell_hex with sector_hex, or distance < m_minDistance)
				
          //assign position to node
          Vector pos (x_pos, y_pos, m_ueHeight);        
          Ptr<Node> node = ues.Get (currentUe);
          Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
          NS_LOG_LOGIC ("ue " << u << " at " << pos);
          mm->SetPosition (Vector (x_pos, y_pos, m_ueHeight));
          ueDevs.Add (m_lteHelper->InstallUeDevice (node));
        
          //increment UE counter
          currentUe ++;
        }       

    }
  return ueDevs;
}

NetDeviceContainer
cv2x_LteHexGridEnbTopologyHelper::DropUEsIndoorOutdoor (NodeContainer ues, std::vector< Ptr<Building> > &buildings)
{
  NS_LOG_FUNCTION (this);

  //make sure there will be equal number of UEs per cell
  NS_ASSERT ( ues.GetN () % GetNumberOfNodes()==0);

  //Storage for return value
  NetDeviceContainer ueDevs;

  uint32_t nbUesPerSector = ues.GetN () / GetNumberOfNodes();
  uint32_t nbIndoorUes = /*2 * nbUesPerSector / 3*/ 0.8 * nbUesPerSector;
  uint32_t currentUe = 0;
  uint32_t totalIndoorUes = 0;
  Ptr<UniformRandomVariable> rndPos = CreateObject<UniformRandomVariable> ();

  //Get node position and antenna orientation
  std::vector<Vector> eNodeBpos = GetNodePositions ();
  std::vector<Vector> buildingpos;
  //Loop through each cell
  for (uint32_t n = 0; n < eNodeBpos.size(); n++)
    {  
      //position uses z for the antenna direction
      Vector pos = eNodeBpos.at(n);
      double x_sector, y_sector;
      double x_pos, y_pos;     
      std::vector<Vector> buildingsInCell;
      //Compute center of sector hexagon
      //This hexagon has center on cell flat edge
      x_sector = pos.x + m_d / 2 * std::cos(DegreesToRadians(pos.z));
      y_sector = pos.y + m_d / 2 * std::sin(DegreesToRadians(pos.z));
      NS_LOG_LOGIC ("sector_center " << n << " at " << x_sector << ":" << y_sector);
      
      //Drop 2 RRH buildings within each cell area. Each building is 50 m x 120 m x 6 m
      for (uint8_t nbuildings = 0; nbuildings < 2; nbuildings++)
        {
          do 
            {
              //pick random x and y
              x_pos = pos.x + rndPos->GetValue (- m_d, m_d);
              y_pos = pos.y + rndPos->GetValue (- m_d, m_d);
            }
          while (!(IsInsideHex(pos.x, pos.y, m_d/2, x_pos, y_pos, true) && IsInsideHex(x_sector, y_sector, m_d/2*std::cos(DegreesToRadians(30)), x_pos, y_pos, false)) || GetDistance(pos.x, pos.y, x_pos, y_pos) < m_minEnodeb2BuildingDistance || !IsAtLeastDistance(x_pos, y_pos, buildingpos, m_minEnodeb2BuildingDistance));
          // do while( Not in the intersection of cell_hex with sector_hex, or distance < m_minDistance, or min distance to other buildings and eNodeB not meet)
           
          // Ptr<Building> building = CreateObject<Building> ();
          // building->SetBoundaries (Box (x_pos-60.0, x_pos+60.0, y_pos-25.0, y_pos+25.0, 0.0, 6.0));
          // building->SetBuildingType (Building::Office);
          // building->SetExtWallsType (Building::ConcreteWithWindows);
          // building->SetNFloors (1);
          buildings.push_back (AddBuilding(x_pos, y_pos, 120.0, 50.0, 6.0));
      
          Vector buildingPosition (x_pos, y_pos, 0);
          NS_LOG_LOGIC ("building " << buildingpos.size() << " center at " << buildingPosition);
          buildingpos.push_back (buildingPosition); 
          buildingsInCell.push_back (buildingPosition);
        }


      //Randomly drop 2/3 UEs within buildings
      std::vector<Vector> indoorUes;
      for (uint32_t u = 0 ; u < nbIndoorUes ; u++) 
        {
          //randomly select the building to drop the UE
          uint32_t buildingIndex = rndPos->GetValue (0, 2) > 1 ? 1:0;
          NS_LOG_LOGIC ("Deploying UE in building " << buildingIndex);
          Vector buildingPosition = buildingsInCell[buildingIndex];
          do 
            {
              //pick random x and y
              x_pos = buildingPosition.x + rndPos->GetValue (-60, 60);
              y_pos = buildingPosition.y + rndPos->GetValue (-25, 25);
            }
          while (!IsAtLeastDistance(x_pos, y_pos, indoorUes, m_minUe2UeDistanceInBuilding));
          // do while( distance < minimum distance between UEs)
				
          //assign position to node
          Vector pos (x_pos, y_pos, m_ueHeight);        
          Ptr<Node> node = ues.Get (currentUe);
          Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
          NS_LOG_LOGIC ("ue " << u << " at " << pos);
          mm->SetPosition (Vector (x_pos, y_pos, m_ueHeight));
          ueDevs.Add (m_lteHelper->InstallUeDevice (node));
          indoorUes.push_back (pos);
        
          //increment UE counter
          currentUe ++;
        }  
     
      //Randomly and uniformly drop the remaining 1/3 UEs to the entire macro geographical area of the given macro cell and ensure 80 % of UEs end up being indoor
      for (uint32_t u = nbIndoorUes ; u < nbUesPerSector ; u++) 
        {
          do 
            {
              //pick random x and y
              x_pos = pos.x + rndPos->GetValue (- m_d, m_d);
              y_pos = pos.y + rndPos->GetValue (- m_d, m_d);
            }
          while (!(IsInsideHex(pos.x, pos.y, m_d/2, x_pos, y_pos, true) && IsInsideHex(x_sector, y_sector, m_d/2*std::cos(DegreesToRadians(30)), x_pos, y_pos, false)) 
                 || GetDistance(pos.x, pos.y, x_pos, y_pos) < m_minDistance 
                 || ( ( IsInsideBuilding (x_pos, y_pos, buildingsInCell[0].x, buildingsInCell[0].y) 
                        || IsInsideBuilding (x_pos, y_pos, buildingsInCell[1].x, buildingsInCell[1].y) )
                      && !IsAtLeastDistance(x_pos, y_pos, indoorUes, m_minUe2UeDistanceInBuilding) )
                 );
          // do while( Not in the intersection of cell_hex with sector_hex, or distance < m_minDistance or dropped in building and does not meet minimum distance between UE)
				
          //assign position to node
          Vector pos (x_pos, y_pos, m_ueHeight);        
          Ptr<Node> node = ues.Get (currentUe);
          Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
          NS_LOG_LOGIC ("ue " << u << " at " << pos);
          mm->SetPosition (Vector (x_pos, y_pos, m_ueHeight));
          ueDevs.Add (m_lteHelper->InstallUeDevice (node));
        
          //check if UE is inside building
          if (IsInsideBuilding (x_pos, y_pos, buildingsInCell[0].x, buildingsInCell[0].y) 
              || IsInsideBuilding (x_pos, y_pos, buildingsInCell[1].x, buildingsInCell[1].y))
            {
              indoorUes.push_back (pos);
            }

          //increment UE counter
          currentUe ++;
        }
      totalIndoorUes += indoorUes.size();
      NS_LOG_LOGIC ("Indoor UEs = " << indoorUes.size() << " ratio=" << 100*((double)indoorUes.size()/nbUesPerSector));
    }
  NS_LOG_LOGIC ("Total indoor UEs = " << totalIndoorUes << " ratio=" << 100*((double)totalIndoorUes/ues.GetN ()));
  return ueDevs;
}

uint32_t
cv2x_LteHexGridEnbTopologyHelper::GetNumberOfNodes () {
  NS_ASSERT (m_nbRings > 0);

  uint32_t nbNodes = 1;

  for (uint32_t i = 1 ; i < m_nbRings ; i++) {
    nbNodes += i*6;
  }

  return 3*nbNodes;
}


bool
cv2x_LteHexGridEnbTopologyHelper::IsInsideHex(double x_center, double y_center, double h, double x_pos, double y_pos, bool flatbottom)
{ 
  
  double q2x = std::abs(x_pos - x_center);         // transform the test point locally and to quadrant 2
  double q2y = std::abs(y_pos - y_center);         // transform the test point locally and to quadrant 2
  
  if(flatbottom)
  {
    double v = h;
	h = v / std::sqrt(3);
    //New conditions because our cell site hex is rotated 90 degrees, having a bottom flat side edge
	if (q2x > 2*h || q2y > v) return false;           // bounding test (since q2 is in quadrant 2 only 2 tests are needed)
	return 2 * v * h - v * q2x - h * q2y >= 0;
  }
  else
  {
	double v = h / std::sqrt(3);
	if (q2x > h || q2y > v*2) return false;           // bounding test (since q2 is in quadrant 2 only 2 tests are needed)
	return 2 * v * h - v * q2x - h * q2y >= 0;   // finally the dot product can be reduced to this due to the hexagon symmetry
  }	  
}

double 
cv2x_LteHexGridEnbTopologyHelper::GetDistance(double x1, double y1, double x2, double y2)
{
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

bool 
cv2x_LteHexGridEnbTopologyHelper::IsWithinDistance(double x, double y, std::vector<Vector> positions, double d)
{
  bool ok = true;
  for (uint32_t n = 0; n < positions.size(); n++)
    {        
      Vector pos = positions.at(n);
      if (GetDistance (x, y, pos.x, pos.y) > d)
        { //at least one distance is too long
          ok = false;
          break; 
        }
    }
  return ok;
}

bool 
cv2x_LteHexGridEnbTopologyHelper::IsAtLeastDistance(double x, double y, std::vector<Vector> positions, double d)
{
  bool ok = true;
  for (uint32_t n = 0; n < positions.size(); n++)
    {        
      Vector pos = positions.at(n);
      if (GetDistance (x, y, pos.x, pos.y) < d)
        { //at least one distance is too short
          ok = false;
          break; 
        }
    }
  return ok;
}

bool
cv2x_LteHexGridEnbTopologyHelper::IsInsideBuilding(double x, double y, double b_x, double b_y)
{
  return (x < b_x + 60) && (x > b_x - 60) && (y < b_y + 25) && (y > b_y - 25);
}

Vector 
cv2x_LteHexGridEnbTopologyHelper::GetClosestPositionInWrapAround (Vector txPos, Vector rxPos)
{
  NS_LOG_FUNCTION (this << txPos << rxPos);
  if (m_nbRings < 1 || m_nbRings > 4) {
    NS_FATAL_ERROR("WRAP-AROUND : NO SUPPORT FOR TOPOLOGIES OTHER THAN 1 THROUGH 4 RINGS!");
  }
  double s = m_d / std::sqrt (3); //hexagon side length
  Vector minPos (rxPos.x, rxPos.y, rxPos.z);
  double minD = GetDistance (txPos.x, txPos.y, rxPos.x, rxPos.y);
  NS_LOG_DEBUG ("Default distance = " << minD);
  NS_LOG_DEBUG ("C1 center 0:0");
  NS_LOG_DEBUG ("C1 pos " << minPos << " distance=" << minD);
  //compute position in each of the 7 wrap around locations and return position with shortest distance to transmitter
 
  Vector pos (rxPos.x + (1.5*((2*m_nbRings)-1))*s, rxPos.y + 0.5 * m_d, rxPos.z);
  double d = GetDistance (txPos.x, txPos.y, pos.x, pos.y);
  NS_LOG_DEBUG ("C2 center " << (1.5*((2*m_nbRings)-1))*s << ":" << 0.5 * m_d);
  NS_LOG_DEBUG ("C2 pos " << pos << " distance=" << d);
  if (d < minD)
    {
      minD = d;
      minPos = pos;
    }
  
  pos.x= rxPos.x + (1.5*(m_nbRings-1))*s;
  pos.y= rxPos.y + (1+(1.5*(m_nbRings-1)))*m_d;
  d = GetDistance (txPos.x, txPos.y, pos.x, pos.y);
  NS_LOG_DEBUG ("C3 center " << (1.5*(m_nbRings-1))*s << ":" << (1+(1.5*(m_nbRings-1)))*m_d);
  NS_LOG_DEBUG ("C3 pos " << pos << " distance=" << d);
  if (d < minD)
    {
      minD = d;
      minPos = pos;
    }

  pos.x= rxPos.x - (1.5*m_nbRings)*s;
  pos.y= rxPos.y + (0.5+(1.5*(m_nbRings-1)))*m_d;
  d = GetDistance (txPos.x, txPos.y, pos.x, pos.y);
  NS_LOG_DEBUG ("C4 center " << (-1)*(1.5*m_nbRings)*s << ":" << (0.5+(1.5*(m_nbRings-1)))*m_d);
  NS_LOG_DEBUG ("C4 pos " << pos << " distance=" << d);
  if (d < minD)
    {
      minD = d;
      minPos = pos;
    }

  pos.x= rxPos.x - (1.5*((2*m_nbRings)-1))*s;
  pos.y= rxPos.y - 0.5*m_d;
  d = GetDistance (txPos.x, txPos.y, pos.x, pos.y);
  NS_LOG_DEBUG ("C5 center " << (-1)*(1.5*((2*m_nbRings)-1))*s << ":" << (-1)*0.5*m_d);
  NS_LOG_DEBUG ("C5 pos " << pos << " distance=" << d);
  if (d < minD)
    {
      minD = d;
      minPos = pos;
    }

  pos.x= rxPos.x - (1.5*(m_nbRings-1))*s;
  pos.y= rxPos.y - (1+(1.5*(m_nbRings-1)))*m_d;
  d = GetDistance (txPos.x, txPos.y, pos.x, pos.y);
  NS_LOG_DEBUG ("C6 center " << 0-(1.5*(m_nbRings-1))*s << ":" << (-1)*(1+(1.5*(m_nbRings-1)))*m_d);
  NS_LOG_DEBUG ("C6 pos " << pos << " distance=" << d);
  if (d < minD)
    {
      minD = d;
      minPos = pos;
    }

  pos.x= rxPos.x + (1.5*m_nbRings)*s;
  pos.y= rxPos.y - (0.5+(1.5*(m_nbRings-1)))*m_d;
  d = GetDistance (txPos.x, txPos.y, pos.x, pos.y);
  NS_LOG_DEBUG ("C7 center " << (1.5*m_nbRings)*s << ":" << (-1)*(0.5+(1.5*(m_nbRings-1)))*m_d);
  NS_LOG_DEBUG ("C7 pos " << pos << " distance=" << d);
  if (d < minD)
    {
      minD = d;
      minPos = pos;
    }

  NS_LOG_DEBUG ("Pos for min distance " << minPos << " distance=" << minD);

  return minPos;
}

std::vector< Ptr<Building> >
cv2x_LteHexGridEnbTopologyHelper::InstallWrapAroundBuildings (std::vector< Ptr<Building> > buildings)
{
  NS_ASSERT (0 < m_nbRings && m_nbRings < 5);
  double s = m_d / std::sqrt (3); //hexagon side length

  std::vector< Ptr<Building> > allBuildings;
  std::vector< Ptr<Building> >::iterator it;
  for (it=buildings.begin () ; it!=buildings.end(); it++)
    {
      Box b = (*it)->GetBoundaries();
      //add current building and 6 other copies in extended coverage
      allBuildings.push_back (*it); // no need to duplicate the building
      allBuildings.push_back (AddBuilding(b.xMin+(1.5*((2*m_nbRings)-1))*s, b.xMax+(1.5*((2*m_nbRings)-1))*s, b.yMin+0.5*m_d, b.yMax+0.5*m_d, b.zMin, b.zMax));
      allBuildings.push_back (AddBuilding(b.xMin+(1.5*(m_nbRings-1))*s, b.xMax+(1.5*(m_nbRings-1))*s, b.yMin+(1+(1.5*(m_nbRings-1)))*m_d, b.yMax+(1+(1.5*(m_nbRings-1)))*m_d, b.zMin, b.zMax));
      allBuildings.push_back (AddBuilding(b.xMin-(1.5*m_nbRings)*s, b.xMax-(1.5*m_nbRings)*s, b.yMin+(0.5+(1.5*(m_nbRings-1)))*m_d, b.yMax+(0.5+(1.5*(m_nbRings-1)))*m_d, b.zMin, b.zMax));
      allBuildings.push_back (AddBuilding(b.xMin-(1.5*((2*m_nbRings)-1))*s, b.xMax-(1.5*((2*m_nbRings)-1))*s, b.yMin-0.5*m_d, b.yMax-0.5*m_d, b.zMin, b.zMax));
      allBuildings.push_back (AddBuilding(b.xMin-(1.5*(m_nbRings-1))*s, b.xMax-(1.5*(m_nbRings-1))*s, b.yMin-(1+(1.5*(m_nbRings-1)))*m_d, b.yMax-(1+(1.5*(m_nbRings-1)))*m_d, b.zMin, b.zMax));
      allBuildings.push_back (AddBuilding(b.xMin+(1.5*m_nbRings)*s, b.xMax+(1.5*m_nbRings)*s, b.yMin-(0.5+(1.5*(m_nbRings-1)))*m_d, b.yMax-(0.5+(1.5*(m_nbRings-1)))*m_d, b.zMin, b.zMax));
    }
  return allBuildings;
}

void
cv2x_LteHexGridEnbTopologyHelper::AttachWithWrapAround (Ptr<PropagationLossModel> lossModel, NetDeviceContainer ueDevices, NetDeviceContainer enbDevices)
{
  Vector closestPos;
  Ptr<NetDevice> enbTemp;
  uint32_t bestEnb = 0;
  double rsrp = 0, bestRsrp = 0;
  for (uint32_t u = 0 ; u < ueDevices.GetN() ; ++u)
  {
    for (uint32_t e = 0 ; e < enbDevices.GetN() ; ++e)
    {
      // Find position of eNB closest to the UE (out of the eNB and its six wrap-around locations)
      closestPos = GetClosestPositionInWrapAround (ueDevices.Get(u)->GetNode()->GetObject<MobilityModel> ()->GetPosition(), enbDevices.Get(e)->GetNode()->GetObject<MobilityModel> ()->GetPosition());
      // Create a temporary copy of the eNB and set location to the found closest eNB
      // to act as an eNB in a wrap-around location, so RSRP can be calculated
      enbTemp = enbDevices.Get(e);
      enbTemp->GetNode()->GetObject<MobilityModel> ()->SetPosition (closestPos);
      // Calculate RSRP from eNB to UE
      rsrp = m_lteHelper->DoCalcRsrp (lossModel, 
          enbDevices.Get(e)->GetNode()->GetObject<cv2x_LteEnbPhy> ()->GetTxPower(), 
          enbDevices.Get(e)->GetNode()->GetObject<cv2x_LteEnbNetDevice> ()->GetPhy()->GetUlSpectrumPhy (), 
          ueDevices.Get(u)->GetNode()->GetObject<cv2x_LteUeNetDevice> ()->GetPhy()->GetUlSpectrumPhy ());
      if (rsrp > bestRsrp)
      {
        bestRsrp = rsrp;
        bestEnb = e;
      }
    }
    m_lteHelper->Attach( ueDevices.Get(u), enbDevices.Get(bestEnb) );
  }
}

Ptr<Building>
cv2x_LteHexGridEnbTopologyHelper::AddBuilding (double x, double y, double w, double l, double h)
{
  Ptr<Building> building = CreateObject<Building> ();
  building->SetBoundaries (Box (x-w/2, x+w/2, y-l/2, y+l/2, 0.0, h));
  building->SetBuildingType (Building::Office);
  building->SetExtWallsType (Building::ConcreteWithWindows);
  building->SetNFloors (1);
  return building;
}

Ptr<Building>
  cv2x_LteHexGridEnbTopologyHelper::AddBuilding (double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
{
  Ptr<Building> building = CreateObject<Building> ();
  building->SetBoundaries (Box (xmin, xmax, ymin, ymax, zmin, zmax));
  building->SetBuildingType (Building::Office);
  building->SetExtWallsType (Building::ConcreteWithWindows);
  building->SetNFloors (1);
  return building;
}

} // namespace ns3
 
