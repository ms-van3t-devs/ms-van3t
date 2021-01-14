/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef CV2X_LTE_HEX_GRID_ENB_TOPOLOGY_HELPER_H
#define CV2X_LTE_HEX_GRID_ENB_TOPOLOGY_HELPER_H

#include <ns3/cv2x_lte-helper.h>
#include <ns3/buildings-helper.h>

namespace ns3 {

/**
 * \ingroup lte
 *
 * This helper class allows to easily create a topology with eNBs
 * grouped in three-sector sites laid out on an hexagonal grid. The
 * layout is done row-wise. 
 *
 */
class cv2x_LteHexGridEnbTopologyHelper : public Object
{
public:
  cv2x_LteHexGridEnbTopologyHelper (void);
  virtual ~cv2x_LteHexGridEnbTopologyHelper (void);

  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /** 
   * Set the cv2x_LteHelper to be used to actually create the EnbNetDevices
   *
   * \note if no cv2x_EpcHelper is ever set, then cv2x_LteHexGridEnbTopologyHelper will default
   * to creating an LTE-only simulation with no EPC, using cv2x_LteRlcSm as
   * the RLC model, and without supporting any IP networking. In other
   * words, it will be a radio-level simulation involving only LTE PHY
   * and MAC and the FF Scheduler, with a saturation traffic model for
   * the RLC.
   * 
   * \param h a pointer to the cv2x_EpcHelper to be used
   */
  void SetLteHelper (Ptr<cv2x_LteHelper> h);

  /**
   * Position the nodes on a hex grid and install the corresponding
   * EnbNetDevices with antenna boresight configured properly
   *
   * \param c the node container where the devices are to be installed
   *
   * \return the NetDeviceContainer with the newly created devices
   */
  NetDeviceContainer SetPositionAndInstallEnbDevice (NodeContainer c);

  /**
   * Drops UEs uniformely within the coverage of the three sector cells
   * \param ues the list of UEs
   */
  NetDeviceContainer DropUEsUniformlyPerSector (NodeContainer ues);
  
  /**
   * Drops UEs uniformely within the coverage of the three sector cells, leaving no cell gaps, and not exceeding cell boundaries.
   * \param ues the list of UEs
   */
  NetDeviceContainer DropUEsUniformlyPerSector2 (NodeContainer ues);

  /**
   * Drops UEs by creating hotspots (TR 36.843 Table A.2.1.1-1). 
   * \param ues the list of UEs
   */
  NetDeviceContainer DropUEsHotspot (NodeContainer ues);

  /**
   * Drops UEs by creating hotspots (TR 36.843 Table A.2.1.1-1). 
   * \param ues the list of UEs
   * \param buildings The vector of buildings 
   */
  NetDeviceContainer DropUEsIndoorOutdoor (NodeContainer ues, std::vector< Ptr<Building> > &buildings);

  /**
   * Computes the number of nodes (i.e. sites) needed to create a hex grid with the given 
   * number of rings. First ring contains 1 node, 2nd has 6 nodes, 3rd 
   * has 12 nodes...
   * \return The number of nodes to be created
   */
  uint32_t GetNumberOfNodes ();

  /**
   * Sets the number of ring in the hexagon grid
   * \param ring the number of ring (1 means a single site)
   */
  void SetNbRings (int ring);
  
  /**
   * Set the coordinates for the center of the grid
   * \param x The x coordinates in m
   * \param y The y coordinates in m
   */
  void SetCenter (double x, double y);
  
  /**
   * Set the distance between sites 
   * \param d The distance in m
   */
  void SetInterSiteDistance (double d);

  /**
   * Set the site height
   * \param height The site height in m
   */
  void SetSiteHeight (double height);

  /**
   * Set the UE height
   * \param height The UE height in m
   */
  void SetUeHeight (double height);

  /**
   * Set the minimum distance between eNodeB and UE
   * \param m The minimum distance in m
   */
  void SetMinimumDistance (double m);

  /**
   * Returns the closest position of the receiver from the transmitter using wrap around method
   * \param txPos Position of the transmitter
   * \param rxPos Position of the receiver
   * \return closest position in the wrap around topology
   */
  Vector GetClosestPositionInWrapAround (Vector txPos, Vector rxPos);
  
  /**
   * Installs new buildings in wrap-around locations
   * \param buildings The list of buildings in their normal positions
   * \return The new list of buildings with duplicates in wrap around positions
   */
  std::vector< Ptr<Building> > InstallWrapAroundBuildings (std::vector< Ptr<Building> > buildings);

  /**
   * Manual attachment of a set of UE devices to the network via the eNodeB with the highest
   * RSRP to that UE. Uses the wrap around method to find the closest version of each eNodeB,
   * then calculates and finds the highest RSRP using the link from those eNodeBs to the UE
   * \param lossModel A pointer to the propagation loss model to be used
   * \param ueDevices UE devices to connect
   * \param enbDevices eNB devices UEs can be connected to
   */
  void AttachWithWrapAround (Ptr<PropagationLossModel> lossModel, NetDeviceContainer ueDevices, NetDeviceContainer enbDevices);
  

private:
  /**
   * Pointer to cv2x_LteHelper object
   */
  Ptr<cv2x_LteHelper> m_lteHelper;

  /**
   * The offset [m] in the position for the node of each sector with
   * respect to the center of the three-sector site
   */
  double m_offset;

  /**
   * The distance [m] between nearby sites
   */
  double m_d;

  /**
   * The x coordinate where the hex grid starts
   */
  double m_xMin;

  /**
   * The y coordinate where the hex grid starts
   */
  double m_yMin;

  /**
   * The number of sites in even rows (odd rows will have
   * one additional site)
   */
  uint32_t m_gridWidth;

  /**
   * The height [m] of each site
   */
  uint32_t m_siteHeight;

  
  uint32_t m_nbRings;
  double m_xCenter;
  double m_yCenter;
  double m_minDistance;
  double m_ueHeight;
  double m_minEnodeb2HotspotDistance;
  double m_minHotspot2HotspotDistance;
  double m_hotspotRange;
  double m_minEnodeb2BuildingDistance;
  double m_minUe2UeDistanceInBuilding;

  //Checks that the position is within the hexagon
  bool IsInsideHex(double x_center, double y_center, double h, double x_pos, double y_pos, bool flatbottom);
  //Returns the distance between 2 points
  double GetDistance(double x1, double y1, double x2, double y2);
  //Returns the position of the nodes
  std::vector<Vector> GetNodePositions ();
  //Indicates when the given coordinates are within the distance of all the other posisions in the vector
  bool IsWithinDistance(double x, double y, std::vector<Vector> pos, double d);
  bool IsAtLeastDistance(double x, double y, std::vector<Vector> pos, double d);
  bool IsInsideBuilding(double x, double y, double b_x, double b_y);
  Ptr<Building> AddBuilding (double x, double y, double w, double l, double h);
  Ptr<Building> AddBuilding (double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);
};


} // namespace ns3



#endif // CV2X_LTE_HEX_GRID_ENB_TOPOLOGY_HELPER_H
