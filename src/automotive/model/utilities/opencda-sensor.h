#ifndef OPENCDA_SENSOR_H
#define OPENCDA_SENSOR_H

#include "ns3/OpenCDAClient.h"
#include "ns3/ldm-utils.h"
#include "ns3/phPoints.h"
#include "ns3/core-module.h"
#include "ns3/LDM.h"
#include <unordered_map>
#include <vector>

namespace ns3 {

  class OpenCDASensor: public Object
  {
  public:
    OpenCDASensor();
    OpenCDASensor(Ptr<OpenCDAClient> opencda_client, std::string id);
    OpenCDASensor(Ptr<OpenCDAClient> opencda_client, int id);
    ~OpenCDASensor();

    void setStationID(std::string id){m_string_id=id;m_id=std::stoi(id);}
    void setStationID(int id){m_string_id=std::to_string (id);m_id=id;}
    void setOpenCDAClient(Ptr<OpenCDAClient> opencda_client){m_opencda_client=opencda_client;m_event_updateDetectedObjects = Simulator::Schedule(MilliSeconds (100),&OpenCDASensor::updateDetectedObjects,this);}
    void setVDP(VDP* vdp) {m_vdp=vdp;}

    void updateDetectedObjects();
    void setLDM(Ptr<LDM> ldm){m_LDM=ldm;}

  private:

    void insertObject(vehicleData_t vehData);

    Ptr<OpenCDAClient> m_opencda_client;
    std::string m_string_id;
    int m_id;
    Ptr<LDM> m_LDM;
    VDP* m_vdp;
    double m_avg_dwell = 0.0;
    int m_dwell_count = 0;
    EventId m_event_updateDetectedObjects;
    std::map<uint64_t,uint64_t> m_lastInserted;  //! Auxiliary map to avoid inserting the same perception in OpenCDA's LDM more than once
  };
}

#endif // OPENCDASENSOR
