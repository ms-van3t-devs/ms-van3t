/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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

 * Edited by Marco Malinverno, Politecnico di Torino (marco.malinverno@polito.it)
*/
#include <errno.h>

#include "appClient.h"

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("appClient");

  NS_OBJECT_ENSURE_REGISTERED(appClient);

  TypeId
  appClient::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::appClient")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<appClient> ()
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&appClient::m_client),
            MakePointerChecker<TraciClient> ());
        return tid;
  }

  appClient::appClient ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
  }

  appClient::~appClient ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  appClient::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  appClient::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);
    m_id = m_client->GetVehicleId (this->GetNode ());
  }

  void
  appClient::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  appClient::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  appClient::receiveDENM (int speedmode)
  {
    if (speedmode==0)
      {
        m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, 6.94);
      }
    else if (speedmode==1)
      {
        m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, 30.00);
      }
  }


}





