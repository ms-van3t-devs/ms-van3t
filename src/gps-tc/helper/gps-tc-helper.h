/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef GPS_TC_HELPER_H
#define GPS_TC_HELPER_H

#include "ns3/gps-tc.h"
#include "ns3/vehicle-visualizer-module.h"

#define GPS_TC_MAP_ITERATOR(mapname,itername) for(std::map<std::string,GPSTraceClient*>::iterator itername=mapname.begin(); itername!=mapname.end(); ++itername)
#define GPS_TC_IT_OBJECT(itername) itername->second

namespace ns3 {

class GPSTraceClientHelper
{
    public:
      GPSTraceClientHelper();

      std::map<std::string,GPSTraceClient*> createTraceClientsFromCSV(std::string filepath);
      void setVehicleVisualizer(Ptr<vehicleVisualizer> vehVis) {m_vehicle_vis_ptr=vehVis;}

      // Set or unset a verbose mode
      // Default: false (verbose mode not activated)
      void setVerbose(bool verbose) {
        m_verbose=verbose;
      }

    private:
      bool m_verbose;

      Ptr<vehicleVisualizer> m_vehicle_vis_ptr;

};

}

#endif /* GPS_TC_HELPER_H */

