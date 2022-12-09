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
#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <fstream>
#include <ns3/core-module.h>
#include "ue-to-ue-pkt-txrx-output-stats.h"

namespace ns3 {

UeToUePktTxRxOutputStats::UeToUePktTxRxOutputStats ()
{
}

void
UeToUePktTxRxOutputStats::SetDb (SQLiteOutput *db, const std::string &tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  ret = db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "timeSec DOUBLE NOT NULL, "
                      "txRx TEXT NOT NULL,"
                      "nodeId INTEGER NOT NULL,"
                      "imsi INTEGER NOT NULL,"
                      "pktSizeBytes INTEGER NOT NULL,"
                      "srcIp TEXT NOT NULL,"
                      "srcPort INTEGER NOT NULL,"
                      "dstIp TEXT NOT NULL,"
                      "dstPort INTEGER NOT NULL,"
                      "pktSeqNum INTEGER NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");

  NS_ABORT_UNLESS (ret);

  UeToUePktTxRxOutputStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                         RngSeedManager::GetRun (), tableName);
}

void
UeToUePktTxRxOutputStats::Save (const std::string txRx, const Address &localAddrs,
                                uint32_t nodeId, uint64_t imsi, uint32_t pktSize,
                                const Address &srcAddrs, const Address &dstAddrs, uint32_t seq)
{
  m_pktCache.emplace_back (UePacketResultCache (Simulator::Now ().GetNanoSeconds () / (double) 1e9,
                                                txRx, localAddrs,
                                                nodeId, imsi, pktSize,
                                                srcAddrs, dstAddrs,
                                                seq));

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_pktCache.size () * sizeof (UePacketResultCache) > 1000000)
    {
      WriteCache ();
    }
}

void
UeToUePktTxRxOutputStats::EmptyCache ()
{
  WriteCache ();
}

void
UeToUePktTxRxOutputStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  std::ostringstream oss;

  for (const auto & v : m_pktCache)
    {
      std::string srcStr;
      std::string dstStr;
      sqlite3_stmt *stmt;
      m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
      oss.str ("");
      ret = m_db->Bind (stmt, 1, v.timeSec);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 2, v.txRx);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 3, v.nodeId);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 4, static_cast<uint32_t> (v.imsi));
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 5, v.pktSize);
      NS_ABORT_UNLESS (ret);
      if (InetSocketAddress::IsMatchingType (v.srcAddrs))
        {
          oss << InetSocketAddress::ConvertFrom (v.srcAddrs).GetIpv4 ();
          if (!oss.str ().compare ("0.0.0.0")) //srcAddrs not set
            {
              std::ostringstream ip;
              ip << Ipv4Address::ConvertFrom (v.localAddrs);
              srcStr = ip.str ();
              ret = m_db->Bind (stmt, 6, srcStr);
              NS_ABORT_UNLESS (ret);
              ret = m_db->Bind (stmt, 7, InetSocketAddress::ConvertFrom (v.srcAddrs).GetPort ());
              NS_ABORT_UNLESS (ret);
              ip.str ("");
              ip << InetSocketAddress::ConvertFrom (v.dstAddrs).GetIpv4 ();
              dstStr = ip.str ();
              ret = m_db->Bind (stmt, 8, dstStr);
              NS_ABORT_UNLESS (ret);
              ret = m_db->Bind (stmt, 9, InetSocketAddress::ConvertFrom (v.dstAddrs).GetPort ());
              NS_ABORT_UNLESS (ret);
              ret = m_db->Bind (stmt, 10, v.seq);
              NS_ABORT_UNLESS (ret);
            }
          else
            {
              oss.str ("");
              oss << InetSocketAddress::ConvertFrom (v.dstAddrs).GetIpv4 ();
              if (!oss.str ().compare ("0.0.0.0")) //dstAddrs not set
                {
                  std::ostringstream ip;
                  ip << InetSocketAddress::ConvertFrom (v.srcAddrs).GetIpv4 ();
                  srcStr = ip.str ();
                  ret = m_db->Bind (stmt, 6, srcStr);
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 7, InetSocketAddress::ConvertFrom (v.srcAddrs).GetPort ());
                  NS_ABORT_UNLESS (ret);
                  ip.str ("");
                  ip << Ipv4Address::ConvertFrom (v.localAddrs);
                  dstStr = ip.str ();
                  ret = m_db->Bind (stmt, 8, dstStr);
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 9, InetSocketAddress::ConvertFrom (v.dstAddrs).GetPort ());
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 10, v.seq);
                  NS_ABORT_UNLESS (ret);
                }
              else
                {
                  std::ostringstream ip;
                  ip << InetSocketAddress::ConvertFrom (v.srcAddrs).GetIpv4 ();
                  srcStr = ip.str ();
                  ret = m_db->Bind (stmt, 6, srcStr);
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 7, InetSocketAddress::ConvertFrom (v.srcAddrs).GetPort ());
                  NS_ABORT_UNLESS (ret);
                  ip.str ("");
                  ip << InetSocketAddress::ConvertFrom (v.dstAddrs).GetIpv4 ();
                  dstStr = ip.str ();
                  ret = m_db->Bind (stmt, 8, dstStr);
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 9, InetSocketAddress::ConvertFrom (v.dstAddrs).GetPort ());
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 10, v.seq);
                  NS_ABORT_UNLESS (ret);
                }
            }
        }
      else if (Inet6SocketAddress::IsMatchingType (v.srcAddrs))
        {
          oss << Inet6SocketAddress::ConvertFrom (v.srcAddrs).GetIpv6 ();
          if (!oss.str ().compare ("::"))   //srcAddrs not set
            {
              std::ostringstream ip;
              ip << Ipv6Address::ConvertFrom (v.localAddrs);
              srcStr = ip.str ();
              ret = m_db->Bind (stmt, 6, srcStr);
              NS_ABORT_UNLESS (ret);
              ret = m_db->Bind (stmt, 7, Inet6SocketAddress::ConvertFrom (v.srcAddrs).GetPort ());
              NS_ABORT_UNLESS (ret);
              ip.str ("");
              ip << Inet6SocketAddress::ConvertFrom (v.dstAddrs).GetIpv6 ();
              dstStr =  ip.str ();
              ret = m_db->Bind (stmt, 8, dstStr);
              NS_ABORT_UNLESS (ret);
              ret = m_db->Bind (stmt, 9, Inet6SocketAddress::ConvertFrom (v.dstAddrs).GetPort ());
              NS_ABORT_UNLESS (ret);
              ret = m_db->Bind (stmt, 10, v.seq);
              NS_ABORT_UNLESS (ret);
            }
          else
            {
              oss.str ("");
              oss << Inet6SocketAddress::ConvertFrom (v.dstAddrs).GetIpv6 ();
              if (!oss.str ().compare ("::"))   //dstAddrs not set
                {
                  std::ostringstream ip;
                  ip << Inet6SocketAddress::ConvertFrom (v.srcAddrs).GetIpv6 ();
                  srcStr = ip.str ();
                  ret = m_db->Bind (stmt, 6, srcStr);
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 7, Inet6SocketAddress::ConvertFrom (v.srcAddrs).GetPort ());
                  NS_ABORT_UNLESS (ret);
                  ip.str ("");
                  ip << Ipv6Address::ConvertFrom (v.localAddrs);
                  dstStr =  ip.str ();
                  ret = m_db->Bind (stmt, 8, dstStr);
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 9, Inet6SocketAddress::ConvertFrom (v.dstAddrs).GetPort ());
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 10, v.seq);
                  NS_ABORT_UNLESS (ret);
                }
              else
                {
                  std::ostringstream ip;
                  ip << Inet6SocketAddress::ConvertFrom (v.srcAddrs).GetIpv6 ();
                  srcStr = ip.str ();
                  ret = m_db->Bind (stmt, 6, srcStr);
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 7, Inet6SocketAddress::ConvertFrom (v.srcAddrs).GetPort ());
                  NS_ABORT_UNLESS (ret);
                  ip.str ("");
                  ip << Inet6SocketAddress::ConvertFrom (v.dstAddrs).GetIpv6 ();
                  dstStr =  ip.str ();
                  ret = m_db->Bind (stmt, 8, dstStr);
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 9, Inet6SocketAddress::ConvertFrom (v.dstAddrs).GetPort ());
                  NS_ABORT_UNLESS (ret);
                  ret = m_db->Bind (stmt, 10, v.seq);
                  NS_ABORT_UNLESS (ret);
                }
            }
        }
      else
        {
          NS_FATAL_ERROR ("Unknown address type!");
        }

      ret = m_db->Bind (stmt, 11, RngSeedManager::GetSeed ());
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 12, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      NS_ABORT_UNLESS (ret);

      ret = m_db->SpinExec (stmt);
      NS_ABORT_UNLESS (ret);
    }

  m_pktCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ABORT_UNLESS (ret);
}

void
UeToUePktTxRxOutputStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
                                       uint32_t run, const std::string &table)
{
  bool ret;
  sqlite3_stmt *stmt;
  ret = p->SpinPrepare (&stmt, "DELETE FROM \"" + table + "\" WHERE SEED = ? AND RUN = ?;");
  NS_ABORT_IF (ret == false);
  ret = p->Bind (stmt, 1, seed);
  NS_ABORT_IF (ret == false);
  ret = p->Bind (stmt, 2, run);

  ret = p->SpinExec (stmt);
  NS_ABORT_IF (ret == false);
}

} // namespace ns3
