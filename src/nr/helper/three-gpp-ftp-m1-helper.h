/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
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
#ifndef THREE_GPP_FTP_M1_HELPER_H
#define THREE_GPP_FTP_M1_HELPER_H

#include <ns3/node.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/file-transfer-application.h>
#include <ns3/v4ping-helper.h>
#include <ns3/file-transfer-helper.h>

namespace ns3 {


  /**
* \ingroup helper
* \brief Helper for a correct setup of every FTP Model 1 applications
*
* This class can help you in setting up a simulation that is using
* an FTP Model 1 application.
*
* FTP Model 1 is described in section A.2.1.3.1 of TR36.814.
* In an operator network, files arrive for transfer according
* to a Poisson process with a lambda value that, for file transfers
* of 0.5 Mbytes, ranges from [0.5, 1, 1.5, 2, 2.5].
* As lambda increases, the traffic intensity increases;
* on average, every 1/lambda seconds, a new file arrival occurs.
* In each operator network, a separate file generation process
* governed by lambda is implemented, and all files are originated
* from a node in the backhaul network towards one of the UEs.
* So, for instance, if lambda equals 1, each second (on average)
* a UE will be picked at random in each operator network and a
* file transfer will be started towards it from the backhaul network node.
*
* According to this helper FTP application will cause files to be
* sent over UDP/IP, and its metrics can be measured at the IP layer.
*
* Traffic intensity can be modified by varying the FTP lambda
* value.
*
* This helper should be used in the following way:
*
* ...
* Ptr<ThreeGppFtpM1Helper> helper = CreateObject<ThreeGppFtpM1Helper> (serverApps, clientApps, servernNodes, clientNodes, serverIps);
* helper->Configure (port, serverStartTime, clientStartTime, clientStopTime, ftpLambda, ftpFileSize);
* helper->Start();
* ...
*
*/
class ThreeGppFtpM1Helper: public Object
{
public:

  /**
   * Default constructor
   */
  ThreeGppFtpM1Helper (void);
  /**
   * Constructor that should be called from user program in order to setup the FTP Model 1
   * traffic helper.
   * \param serverApps is a container of server applications. This helper will be adding client applications in this container.
   * \param clientApps is a container of client applications. This helper will be adding client applications to this container.
   * \param serverNodes are server nodes that will be considered for this FTP Mpdel 1 traffic
   * \param clientNodes are client nodes that will be considered fot this FTP Model 1 traffic
   * \param serversIps are the Ipv4 interfaces of the server nodes
   */
  ThreeGppFtpM1Helper (ApplicationContainer* serverApps, ApplicationContainer* clientApps,
                       NodeContainer* serverNodes, NodeContainer* clientNodes,
                       Ipv4InterfaceContainer* serversIps);
  /**
   * Destructor
   */
  virtual ~ThreeGppFtpM1Helper (void);

  /**
   * \brief GetTypeId
   * \return the type id of the object
   */
  static TypeId GetTypeId (void);

  /**
   * \brief To be called before Start() function.
   */
  void Configure (uint16_t port, Time serverStartTime,
                  Time clientStartTime, Time clientStopTime,
                  double ftpLambda, uint32_t ftpFileSize);
  /**
   * \brief Start to generate fille transfers according to FTP Model 1
   */
  void Start ();

private:

  /**
   * \brief Configures FTP servers
   */
  void DoConfigureFtpServers (void);
  /**
   * \brief Configures FTP clients
   */
  void DoConfigureFtpClients (void);
  /**
   * \brief Function that is called to start the next file transfer
   */
  void DoStartFileTransfer (void);
  /**
   * \brief Gets the next file transfer time
   */
  Time DoGetNextTime (void) const;

  uint32_t m_lastClient {0};//!<last client index, index is the index to access to a node in m_clientNodes container, used to remember which is the last served client
  Ptr<ExponentialRandomVariable> m_ftpArrivals;//!<random variable used to obtain the times for FTP Model 1 arrivals
  Ptr<UniformRandomVariable> m_startJitter;//!<random variable used to generate a jitter among start times, to not start all clients exactly at the same time
  uint16_t m_port {0};//!< port that will be used for all the applications
  Time m_clientStartTime {Seconds (0)};//!<start time of client applications
  Time m_clientStopTime {Seconds (0)};//!<stop time of client applications
  double m_ftpLambda {0.0};//!<ftp lambda as parameter of FTP Model 1 traffic
  uint32_t m_ftpFileSize {0};//!< file size
  Time m_serverStartTime {Seconds (0)};//!<server start time
  bool m_boolConfigured {false};//!<indicator that tells whether this helper is configured correctly
  ApplicationContainer* m_serverApps;//!<container of server applications
  ApplicationContainer* m_clientApps;//!<container of client applications
  NodeContainer* m_serverNodes;//!<container of server nodes
  NodeContainer* m_clientNodes;//!<container of client nodes
  Ipv4InterfaceContainer* m_serversIps;//!<container of IPv4 server interfaces
  ApplicationContainer m_pingApps;//!<container of ping apps
};


}; //end ns3 namespace
#endif // THREE_GPP_FTP_M1_HELPER_H
