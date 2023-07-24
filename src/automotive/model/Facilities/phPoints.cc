#include "phPoints.h"
#include <iostream>

namespace ns3 {


  PHpoints::PHpoints()
  {
    m_max_size = 10;
    m_size = 0;
    m_PHpoints = std::map<uint64_t, PHData_t>();
  }

  void
  PHpoints::deleteLast()
  {
    if(m_PHpoints.begin () != m_PHpoints.end())
      m_PHpoints.erase (m_PHpoints.begin ());
  }

  void
  PHpoints::insert(vehicleData_t newData, uint64_t station_id)
  {
    PHData newPoint = {0};
    if((unsigned int)m_PHpoints.size () == 10)
    {
      m_PHpoints.erase (m_PHpoints.begin ());
    }
    newPoint.detected = newData.detected;
    newPoint.lat = newData.lat;
    newPoint.lon = newData.lon;
    newPoint.heading = newData.heading;
    newPoint.speed_ms = newData.speed_ms;
    newPoint.stationID = newData.stationID;
    newPoint.timestamp = newData.age_us;
    if((newData.detected==true) && newData.perceivedBy.isAvailable ())
    {
      newPoint.perceivedBy = OptionalDataItem<long>(newData.perceivedBy.getData ());
    }
    else
      {
        newPoint.perceivedBy = OptionalDataItem<long>((long)station_id);
      }
    newPoint.CPMincluded =false;
    m_PHpoints[newData.timestamp_us] = newPoint;
  }
  PHData
  PHpoints::getLast()
  {
    return m_PHpoints.rbegin ()->second;
  }
  PHData
  PHpoints::getPrevious()
  {
    auto previous = m_PHpoints.end();
    previous--;
    previous--;
    return previous->second;
  }

  std::set<long>
  PHpoints::getAssocIDs()
  {
    std::set<long> retIDs = std::set<long>();
    //std::cout << "PHpoints getAssocIDs" << std::endl;
    for (auto it = m_PHpoints.begin (); it != m_PHpoints.end (); it++)
      {
        if(it->second.perceivedBy.isAvailable ())
          retIDs.insert(it->second.perceivedBy.getData ());
      }
    return retIDs;
  }

}
