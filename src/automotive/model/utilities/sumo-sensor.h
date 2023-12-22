#ifndef SUMOSENSOR_H
#define SUMOSENSOR_H

#include "ns3/ldm-utils.h"
#include "ns3/phPoints.h"
#include "ns3/core-module.h"
#include "ns3/traci-client.h"
#include "ns3/vdpTraci.h"
#include "ns3/LDM.h"
#include <unordered_map>
#include <vector>
#include <random>
#include <shared_mutex>
#include <boost/geometry.hpp>

namespace ns3 {

  typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> point_type;

  using polygon_type = boost::geometry::model::polygon<point_type>;
  using linestring_type = boost::geometry::model::linestring<point_type>;

  class SUMOSensor : public Object
  {
  public:
    SUMOSensor();
    ~SUMOSensor();

    void setStationID(std::string id){m_id=id;m_stationID=std::stol(id.substr (3));}
    void setTraCIclient(Ptr<TraciClient> client){m_client=client;m_event_updateDetectedObjects = Simulator::Schedule(MilliSeconds (100),&SUMOSensor::updateDetectedObjects,this);}
    void setVDP(VDP* vdp) {m_vdp=vdp;}
    void setSensorRange(double sensorRange){m_sensorRange = sensorRange;}
    void updateDetectedObjects();

    void setLDM(Ptr<LDM> ldm){m_LDM = ldm;}
    libsumo::TraCIPosition boost2TraciPos(point_type point_type);

    void cleanup();

  private:
        //Compute defining points of a vehicle with StationID id
        vehiclePoints_t adjust(std::string id);
        //Create gaussian noise for distance sensor measurements
        double distance_noise();

        //TraCI client pointer
        Ptr<TraciClient> m_client; //!< TraCI client

        uint64_t m_stationID;
        std::string m_id;
        VDP* m_vdp;

        Ptr<LDM> m_LDM;

        EventId m_event_updateDetectedObjects;

        double m_sensorRange;

        const double m_mean = 0.0;
        const double m_stddev_distance = 1.0; // meters
        const double m_stddev_angle = 0.57; //degrees
        const double m_stddev_speed = 0.45; // m/s
        double m_avg_dwell = 0.0;
        int m_dwell_count = 0;

        std::default_random_engine m_generator;
  };
}
#endif // SUMOSENSOR_H
