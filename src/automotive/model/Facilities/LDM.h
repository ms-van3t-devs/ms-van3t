#ifndef LDM_H
#define LDM_H

#include "ns3/ldm-utils.h"
#include "ns3/phPoints.h"
#include "ns3/core-module.h"
#include "ns3/traci-client.h"
#include "ns3/vdpTraci.h"
#include <unordered_map>
#include <vector>
#include <random>
#include <shared_mutex>
#include <boost/geometry.hpp>
//#include <boost/optional/optional.hpp>


#define DB_CLEANER_INTERVAL_SECONDS 0.5
#define DB_DELETE_OLDER_THAN_SECONDS 1
namespace ns3 {


  typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> point_type;

  using polygon_type = boost::geometry::model::polygon<point_type>;
  using linestring_type = boost::geometry::model::linestring<point_type>;

class LDM : public Object
{
public:
  typedef enum {
          LDM_OK,
          LDM_UPDATED,
          LDM_ITEM_NOT_FOUND,
          LDM_MAP_FULL,
          LDM_UNKNOWN_ERROR
  } LDM_error_t;

  typedef struct {
          vehicleData_t vehData;
          PHpoints phData;
  } returnedVehicleData_t;

    LDM();
    ~LDM();

    /* This function clears the whole database (to be used only when the dabatase and its content is not going to be accessed again)*/
    void clear();
    /* This function inserts or updates a vehicle in the database
     * The vehicle data should be passed inside a vehicleData_t structure and vehicles are univocally identified by their stationID
     * This function returns LDMMAP_OK if a new vehicle has been inserted, LDMMAP_UPDATED is an existing vehicle entry has been updated,
     * LDMMAP_MAP_FULL if the database if full and the insert operation failed (this should never happen, in any case)*/
    LDM_error_t insert(vehicleData_t newVehicleData);

    /* This function removes from the database the vehicle entry with station ID == stationID
     * It returns LDMMAP_ITEM_NOT_FOUND if no vehicle with the given stationID was found for removal
     * It returns LDMMAP_OK if the vehicle entry was succesfully removed*/
    LDM_error_t remove(uint64_t stationID);

    /* This function returns the vehicle entry with station ID == stationID - the entry data is returned in retVehicleData
     * This function returns LDMMAP_ITEM_NOT_FOUND if no vehicle with the given stationID was found for removal, while
     * it returns LDMMAP_OK if the retVehicleData structure was properly filled with the requested vehicle data */
    LDM_error_t lookup(uint64_t stationID, returnedVehicleData_t &retVehicleData);

    /* This function returns a vector of vehicles, including their Path History points, located within a certain radius
     * centered on a given latitude and longitude
     * For the time being, this function should always return LDMMAP_OK (i.e. to understand if no vehicles are returned,
     * you should check the size of the selectedVehicles vector)*/
    LDM_error_t rangeSelect(double range_m, double lat, double lon, std::vector<returnedVehicleData_t> &selectedVehicles);

    /* This function is the same as the other method with the same name, but it will return all the vehicles around another
     * vehicle (which is also included in the returned vector), given it stationID
     * This function may return LDMMAP_ITEM_NOT_FOUND if the specified stationID is not stored inside the database */
    LDM_error_t rangeSelect(double range_m, uint64_t stationID, std::vector<returnedVehicleData_t> &selectedVehicles);

    /* This function updates the timestamp indicating the last time the given object has been included in a CPM */
    LDM_error_t updateCPMincluded(uint64_t stationID,uint64_t timestamp);

    // This function returns all Perceived Objects (POs) that are currently in the LDM, false if there are not POs in LDM
    bool getAllPOs(std::vector<returnedVehicleData_t> &selectedVehicles);
    // This function returns all Connected Vehicles (CVs) that are currently in the LDM, false if there are not CVs in LDM
    bool getAllCVs(std::vector<returnedVehicleData_t> &selectedVehicles);

    /* This function deletes from the database all the entries older than time_milliseconds ms
     * The entries are deleted if their age is > time_milliseconds ms if greater_equal == false,
     * or >= time_milliseconds ms if greater_equal == true
     * This function performs a full database read operation */
    void deleteOlderThan();

    /* This function is a combination of deleteOlderThan() and executeOnAllContents(), calling the open_fcn()
     * callback for every deleted entry */
    void deleteOlderThanAndExecute(double time_milliseconds,void (*oper_fcn)(uint64_t,void *),void *additional_args);

    /* This function can be used to write all the content of the database in a log file*/
    void writeAllContents();

    void cleanup();

    /* This function reads the whole database, and, for each entry, it executes the "oper_fcn" callback
     * This callback should return void (i.e. nothing) and have two arguments:
     * - a vehicleData_t structure, in which the data stored in each entry will be made available to the callback
     * - a void * pointer, by means of which possible additional arguments can be passed to the callback
     * The additional arguments to be passed to the callback, each time it is called, can be specified by setting
     * void *additional_args to a value different than "nullptr"
     * If additional_args == nullptr, also the second argument of each callback call will be nullptr */
    void executeOnAllContents(void (*oper_fcn)(vehicleData_t,void *),void *additional_args);

    int getCardinality() {return m_card;};

    void setStationID(std::string id){m_id=id;m_stationID=std::stol(id.substr (3));}

    void enableOutputFile(std::string id){m_logfile_file.open(id+"-LDM.txt",std::ofstream::trunc);
                                          m_csv_file.open(id+"-LDM.csv",std::ofstream::trunc);
                                          m_csv_file << "Time,Size,POs,AvgConf,AvgAcc,AvgAge,AvgDwell,AvgAssoc,CVs,AvgDist,MaxDist,AvgT2D,UnderPPrange,AvgPPDwell"<<std::endl;}

    void setTraCIclient(Ptr<TraciClient> client){m_client=client;}
    void setVDP(VDP* vdp) {m_vdp=vdp;}

    /*This function updates all the Perceived Object polygon's showing the current perception of them */
    void updatePolygons();
    void drawPolygon(vehicleData_t data);
    void enablePolygons(){m_polygons=true;m_event_updatePolygons = Simulator::Schedule(MilliSeconds (100),&LDM::updatePolygons,this);}

    libsumo::TraCIPosition boost2TraciPos(point_type point_type);

private:

	// Main database structure
	std::unordered_map<uint64_t,returnedVehicleData_t> m_LDM;
	// Database cardinality (number of entries stored in the database)
	uint64_t m_card;
	long m_count;
	//TraCI client pointer
	Ptr<TraciClient> m_client; //!< TraCI client

        uint64_t m_stationID;
        std::string m_id;
        std::ofstream m_logfile_file;
        std::ofstream m_csv_file;
        VDP* m_vdp;

        bool m_polygons;

        EventId m_event_deleteOlderThan;
        EventId m_event_writeContents;
        EventId m_event_updatePolygons;

	double m_avg_dwell = 0.0;
	int m_dwell_count = 0;


};
}
#endif // LDM_H
