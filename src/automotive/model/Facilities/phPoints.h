#ifndef PHPOINTS_H
#define PHPOINTS_H

#include "ns3/ldm-utils.h"
namespace ns3 {
class PHpoints
{
public:
  PHpoints();
  void setMaxSize (unsigned int size);
  std::map<uint64_t, PHData_t> getPHpoints(){return m_PHpoints;}
  PHData getLast();
  PHData getPrevious();
  long getSize(){return (long) m_PHpoints.size ();}
  void insert(vehicleData_t newData, uint64_t station_id);
  void setCPMincluded(){auto it = m_PHpoints.end ();it--;it->second.CPMincluded = true;}
  std::set<long> getAssocIDs();
  uint64_t getLastTS(){return m_PHpoints.rbegin ()->first;}
  void deleteLast();


private:
  std::map<uint64_t, PHData_t> m_PHpoints;
  unsigned int m_max_size;
  unsigned int m_size;
  uint64_t m_oldest;
};
}
#endif // PHPOINTS_H
