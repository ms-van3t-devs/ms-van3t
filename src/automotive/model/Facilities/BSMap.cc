/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

 * Created by:
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "ns3/BSMap.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("BSMap");

  BSMap::BSMap()
  {
      m_internal_bsvector.clear();
  }

  void
  BSMap::add(Ptr<BSContainer> bscontainer)
  {
    if(bscontainer==nullptr)
    {
      NS_FATAL_ERROR("Fatal error. Called BSMap::add() with a null pointer.");
    }

    StationID_t station_id=bscontainer->getStationID();

    m_internal_bsvector[station_id] = bscontainer;
  }

  void
  BSMap::remove(StationID_t station_id)
  {
    m_internal_bsvector.erase(station_id);
  }

  Ptr<BSContainer>
  BSMap::get(StationID_t station_id)
  {
    if(m_internal_bsvector.count(station_id)>0) {
      return m_internal_bsvector[station_id];
    } else {
      return nullptr;
    }
  }

  Ptr<BSContainer>
  BSMap::get(StationID_t station_id, bool &found)
  {
    if(m_internal_bsvector.count(station_id)>0) {
      return m_internal_bsvector[station_id];
      found = true;
    } else {
      found = false;
      return nullptr;
    }
  }
}
