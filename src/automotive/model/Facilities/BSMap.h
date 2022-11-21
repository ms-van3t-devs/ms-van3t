#ifndef BSFAMAP_H
#define BSFAMAP_H

#include "ns3/BSContainer.h"
#include "unordered_map"

namespace ns3
{
  class BSMap: public Object
  {
    public:
      BSMap();

      void add(Ptr<BSContainer> bscontainer);
      void remove(StationID_t station_id);

      Ptr<BSContainer> get(StationID_t station_id);
      Ptr<BSContainer> get(StationID_t station_id, bool &found);
    private:
      std::unordered_map<StationID_t,Ptr<BSContainer>> m_internal_bsvector;
  };
}

#endif // BSMAP_H
