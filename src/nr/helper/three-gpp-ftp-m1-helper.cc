/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
/*
 *  Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "three-gpp-ftp-m1-helper.h"
#include <ns3/packet-sink-helper.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ThreeGppFtpM1Helper");

ThreeGppFtpM1Helper::ThreeGppFtpM1Helper (ApplicationContainer* serverApps, ApplicationContainer* clientApps,
                                          NodeContainer* serverNodes, NodeContainer* clientNodes,
                                          Ipv4InterfaceContainer* serversIps)
{
  NS_LOG_FUNCTION (this);
  m_serverApps = serverApps;
  m_clientApps = clientApps;
  m_serverNodes = serverNodes;
  m_clientNodes = clientNodes;
  m_serversIps = serversIps;
}

ThreeGppFtpM1Helper::ThreeGppFtpM1Helper (void)
{
  NS_LOG_FUNCTION (this);
}

ThreeGppFtpM1Helper::~ThreeGppFtpM1Helper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId
ThreeGppFtpM1Helper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ThreeGppFtpM1Helper")
                      .SetParent<Object> ()
                      .AddConstructor<ThreeGppFtpM1Helper> ()
  ;
  return tid;
}

void
ThreeGppFtpM1Helper::DoConfigureFtpServers (void)
{
  NS_LOG_FUNCTION (this);
  Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), m_port));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", apLocalAddress);
  *m_serverApps = packetSinkHelper.Install (*m_serverNodes);
  m_serverApps->Start (m_serverStartTime);
}

void
ThreeGppFtpM1Helper::DoConfigureFtpClients (void)
{
  NS_LOG_FUNCTION (this);
  uint32_t ftpSegSize = 1448; // bytes
  FileTransferHelper ftpHelper ("ns3::UdpSocketFactory", Address ());
  ftpHelper.SetAttribute ("SendSize", UintegerValue (ftpSegSize));
  ftpHelper.SetAttribute ("FileSize", UintegerValue (m_ftpFileSize));

  for (uint32_t i = 0; i < m_serversIps->GetN (); i++)
    {
      Ipv4Address ipAddress = m_serversIps->GetAddress (i, 0);
      AddressValue remoteAddress (InetSocketAddress (ipAddress, m_port));
      ftpHelper.SetAttribute ("Remote", remoteAddress);
      m_clientApps->Add (ftpHelper.Install (*m_clientNodes));

      // Seed the ARP cache by pinging early in the simulation
      // This is a workaround until a static ARP capability is provided
      V4PingHelper ping (ipAddress);
      m_pingApps.Add (ping.Install (*m_clientNodes));
    }

  m_clientApps->Start (m_clientStartTime + Seconds (m_startJitter->GetValue ()));
  // Add one or two pings for ARP at the beginning of the simulation
  m_pingApps.Start (Seconds (0.300) + Seconds (m_startJitter->GetValue ()));
  m_pingApps.Stop (Seconds (0.500));
}

void
ThreeGppFtpM1Helper::DoStartFileTransfer (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_lastClient >= 0 && m_lastClient < m_clientApps->GetN ());
  Ptr<Application> app = m_clientApps->Get (m_lastClient);
  NS_ASSERT (app);
  Ptr<FileTransferApplication> fileTransfer = DynamicCast <FileTransferApplication> (app);
  NS_ASSERT (fileTransfer);
  fileTransfer->SendFile ();

  m_lastClient += 1;
  if (m_lastClient == m_clientApps->GetN ())
    {
      m_lastClient = 0;
    }
  Simulator::Schedule (DoGetNextTime (), &ThreeGppFtpM1Helper::DoStartFileTransfer, this);
}


void
ThreeGppFtpM1Helper::Configure (uint16_t port, Time serverStartTime,
                                Time clientStartTime, Time clientStopTime,
                                double ftpLambda, uint32_t ftpFileSize)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (m_boolConfigured, "Already configured FTP M1 helper.");
  NS_ABORT_MSG_IF ( m_serverNodes->GetN () == 0 || m_clientNodes->GetN() == 0 || m_serversIps->GetN () == 0,
                    "Server and/or client nodes or IP server interfaces not set.");
  m_port = port;
  m_clientStartTime = clientStartTime;
  m_clientStopTime = clientStopTime;
  m_ftpLambda = ftpLambda;
  m_ftpFileSize = ftpFileSize;
  m_serverStartTime = serverStartTime;
  m_boolConfigured = true;

  m_ftpArrivals = CreateObject<ExponentialRandomVariable> ();
  m_ftpArrivals->SetAttribute ("Mean", DoubleValue (1 / m_ftpLambda));
  // Randomly distribute the start times across 100ms interval
  m_startJitter = CreateObject<UniformRandomVariable> ();
  m_startJitter->SetAttribute ("Max", DoubleValue (0.100));
}


void
ThreeGppFtpM1Helper::Start (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_boolConfigured);

  DoConfigureFtpServers ();
  DoConfigureFtpClients ();

  // Start file transfer arrival process in both networks
  Simulator::Schedule (m_clientStartTime + DoGetNextTime (), &ThreeGppFtpM1Helper::DoStartFileTransfer, this);
}

Time
ThreeGppFtpM1Helper::DoGetNextTime (void) const
{
  return Seconds (m_ftpArrivals->GetValue ());
}

}//end ns-3 namespace
