/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Georgia Institute of Technology
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
 * Author: George F. Riley <riley@ece.gatech.edu>
 */

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "file-transfer-application.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FileTransferApplication");

NS_OBJECT_ENSURE_REGISTERED (FileTransferApplication);

TypeId
FileTransferApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FileTransferApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications") 
    .AddConstructor<FileTransferApplication> ()
    .AddAttribute ("SendSize", "The number of bytes to write per socket send",
                   UintegerValue (512),
                   MakeUintegerAccessor (&FileTransferApplication::m_sendSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&FileTransferApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("FileSize",
                   "The total number of bytes to send. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&FileTransferApplication::m_fileSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&FileTransferApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&FileTransferApplication::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}


FileTransferApplication::FileTransferApplication ()
  : m_socket (0),
    m_connected (false),
    m_totBytes (0)
{
  NS_LOG_FUNCTION (this);
}

FileTransferApplication::~FileTransferApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
FileTransferApplication::SetFileSize (uint32_t fileSize)
{
  NS_LOG_FUNCTION (this << fileSize);
  m_fileSize = fileSize;
}

uint32_t
FileTransferApplication::GetTotalBytes (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totBytes;
}

Ptr<Socket>
FileTransferApplication::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

void
FileTransferApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

bool
FileTransferApplication::SendFile (void)
{
  NS_LOG_FUNCTION (this);
  if (m_socket)
    {
      NS_LOG_FUNCTION ("Socket exists; ignoring request");
      return false;
    }
  m_totBytes = 0;
  m_socket = Socket::CreateSocket (GetNode (), m_tid);

  if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind6 ();
    }
  else if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }

  m_socket->Connect (m_peer);
  m_socket->ShutdownRecv ();
  m_socket->SetConnectCallback (
    MakeCallback (&FileTransferApplication::ConnectionSucceeded, this),
    MakeCallback (&FileTransferApplication::ConnectionFailed, this));
  m_socket->SetSendCallback (
    MakeCallback (&FileTransferApplication::DataSend, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&FileTransferApplication::CloseSucceeded, this),
    MakeCallback (&FileTransferApplication::CloseFailed, this));
  NS_LOG_LOGIC ("FileTransferApplication: Starting file transfer of size " << m_fileSize << " at time " << Simulator::Now ().GetSeconds ());
  if (m_connected || m_tid == UdpSocketFactory::GetTypeId ())
    {
      SendData ();
    }
  return true;
}

// Application Methods
void FileTransferApplication::StartApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
}

void FileTransferApplication::StopApplication (void) // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_connected = false;
    }
  else
    {
      NS_LOG_WARN ("FileTransferApplication found null socket to close in StopApplication");
    }
}

// Private helpers

void FileTransferApplication::SendData (void)
{
  NS_LOG_FUNCTION (this);

  while (m_fileSize == 0 || m_totBytes < m_fileSize)
    { // Time to send more
      uint32_t toSend = m_sendSize;
      // Make sure we don't send too many
      if (m_fileSize > 0)
        {
          toSend = std::min (m_sendSize, m_fileSize - m_totBytes);
        }
      NS_LOG_LOGIC ("sending packet at " << Simulator::Now ());
      Ptr<Packet> packet = Create<Packet> (toSend);
      m_txTrace (packet);
      int actual = m_socket->Send (packet);
      if (actual > 0)
        {
          m_totBytes += actual;
        }
      // We exit this loop when actual < toSend as the send side
      // buffer is full. The "DataSent" callback will pop when
      // some buffer space has freed ip.
      if ((unsigned)actual != toSend)
        {
          break;
        }
    }
  // Check if time to close (all sent)
  if (m_totBytes == m_fileSize && (m_connected || m_tid == UdpSocketFactory::GetTypeId ()))
    {
      NS_LOG_LOGIC ("FileTransferApplication closing");
      m_socket->Close ();
      m_connected = false;
    }
  if (m_tid == UdpSocketFactory::GetTypeId ())
    {
      m_socket = 0;
    }
}

void FileTransferApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("FileTransferApplication Connection succeeded");
  m_connected = true;
  SendData ();
}

void FileTransferApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("FileTransferApplication Connection failed");
  m_socket->Close ();
}

void FileTransferApplication::CloseSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("FileTransferApplication Close succeeded");
  m_socket = 0;
}

void FileTransferApplication::CloseFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("FileTransferApplication Close failed");
  m_socket = 0;
}


void FileTransferApplication::DataSend (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);

  if (m_connected || m_tid == UdpSocketFactory::GetTypeId ())
    { // Only send new data if the connection has completed
      NS_LOG_LOGIC ("FileTransferApplication DataSent callback triggers new SendData() call");
      Simulator::ScheduleNow (&FileTransferApplication::SendData, this);
    }
}



} // Namespace ns3
