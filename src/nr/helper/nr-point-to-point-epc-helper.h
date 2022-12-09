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
#ifndef NR_POINT_TO_POINT_EPC_HELPER_H
#define NR_POINT_TO_POINT_EPC_HELPER_H

#include <ns3/point-to-point-epc-helper.h>

namespace ns3 {

  class LteSlTft;

/**
 * \ingroup helper
 *
 * \brief Create an EPC network with PointToPoint links
 *
 * The class is based on the LTE version. The usage is, in most of the cases,
 * automatic inside the NrHelper. All the user has to do, is:
 *
\verbatim
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  ...
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  nrHelper->SetEpcHelper (epcHelper);
\endverbatim
 *
 * This helper will then used to create the links between the GNBs and the EPC.
 * All links will be point-to-point, with some properties.
 * The user can set the point-to-point links properties by using:
 *
\verbatim
  epcHelper->SetAttribute ("AttributeName", UintegerValue (10));
\endverbatim
 *
 * And these attribute will be valid for all the code that follows the
 * SetAttribute call. The list of attributes can be seen in the class
 * PointToPointEpcHelper in the ns-3 code base.
 *
 * \section p2p_epc_pgw Obtaining the PGW node
 *
 * You can obtain the pointer to the PGW node by doing:
\verbatim
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
\endverbatim
 *
 * After that, you would probably want to setup a network between the PGW and
 * your remote nodes, to create your topology. As example, there is the code
 * that setup a point to point link between the PGW and a single remote node:
 *
\verbatim
  // Create our remote host
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);

  // Install internet stack on the remote host
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // connect a remoteHost to pgw
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  // Here is the routing part.. please note that UEs will always be in the
  // class 7.0.0.0
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
\endverbatim
 *
 * \section p2p_epc_ipv4 Assigning IPV4 addresses
 *
 * Another important thing that this helper can do is assigning automatically
 * the IPv4 addresses to the UE:
 *
\verbatim
  NetDeviceContainer netDeviceContainerForUe = ...;
  Ipv4InterfaceContainer ueLowLatIpIface = epcHelper->AssignUeIpv4Address (netDeviceContainerForUe);
\endverbatim
 *
 * And, of course, you would like to set the default routing for the UE,
 * which is the address of the EPC:
 *
\verbatim
  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueContainer.GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueContainer.Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
\endverbatim
 *
 * For everthing else, please see also the NrHelper documentation.
 *
 * \see PointToPointEpcHelper
 */
class NrPointToPointEpcHelper : public PointToPointEpcHelper
{
public:
  /**
   * \brief Constructor
   */
  NrPointToPointEpcHelper ();

  /**
   * \brief Destructor
   */
  virtual ~NrPointToPointEpcHelper () override;

  // inherited from Object
  /**
   *  \brief Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

protected:
  virtual void DoAddX2Interface (const Ptr<EpcX2> &gnb1X2, const Ptr<NetDevice> &gnb1NetDev,
                                 const Ipv4Address &gnb1X2Address,
                                 const Ptr<EpcX2> &gnb2X2, const Ptr<NetDevice> &gnb2NetDev,
                                 const Ipv4Address &gnb2X2Address) const override;
  virtual void DoActivateEpsBearerForUe (const Ptr<NetDevice> &ueDevice,
                                         const Ptr<EpcTft> &tft,
                                         const EpsBearer &bearer) const override;

//NR SL
public:
  /**
   * \brief Activate NR sidelink bearer
   *
   * \param ueDevice The device of the UE
   * \param slTft The sidelink traffic flow template for the new bearer
   */
  void ActivateNrSlBearerForUe (const Ptr<NetDevice> &ueDevice, const Ptr<LteSlTft> &slTft) const;

};

} // namespace ns3

#endif // NR_POINT_TO_POINT_EPC_HELPER_H
