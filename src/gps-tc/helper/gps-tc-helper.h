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
      typedef struct _dateyear {
          int tm_year;
          int tm_mon;
          int tm_mday;
      } dateyear_t;

      GPSTraceClientHelper();

      std::map<std::string,GPSTraceClient*> createTraceClientsFromCSV(std::string filepath);

      // Set or unset a verbose mode
      // Default: false (verbose mode not activated)
      void setVerbose(bool verbose) {
        m_verbose=verbose;
      }

      // These setter methods can be called to specify, to the GPS Trace Client Helper, which are the names of columns in the CSV trace file containing the needed information
      // They should always be called BEFORE using createTraceClientsFromCSV(), as they affect how the CSV GNSS trace file is read

      // This method can be used to set the name of the column containing the vehicle ID (default: "agent_id")
      void setVehicleIDColumnName(std::string col_name_vehid) {m_col_name_vehid = col_name_vehid;}
      // This method can be used to set the name of the column containing the timestamps (default: "timeStamp_posix")
      // The second argument can be either "true" or "false"
      // "true" means that the timestamps are not actual timestamps, but are dates and/or times, in a human-readable format
      // "false" means that the timestamps are already provided as POSIX timestamps, in seconds since the epoch (can also be floating point to specify milli/microseconds
      // If this value is set to "true", the user can also specify two additional arguments:
      //   "date_format", i.e. the format of the dates/times stored in the CSV files (which will be automatically converted into timestamps)
      //     The "date_format" syntax is the same as the one accepted by std::get_time() (https://en.cppreference.com/w/cpp/io/manip/get_time), with the addition of the %MSEC
      //     specifier, which can be used to read milliseconds at the end of the date (%MSEC can only be specified, with a proper separator, at the end of the date format string)
      //     The default date format is "%H:%M:%S.%MSEC" (dates/times with microseconds are still not supported)
      //   "start_date", if "date_format" does not specify any year-month-day date, this structure can be used to specify one date, which will be used to set all the computed
      //     timestamps. If a date is not specified and it is not found in the CSV file, 2004-01-01 will be used
      //     In any case, what really matters for the simulations are the differences between timestamps, thus potentially any valid date can be used without affecting the overall
      //     simulation, unless the gps-tc getTimestamp() method is used
      void setTimestampColumnName(std::string col_name_tstamp, bool is_date, std::string date_format = "%H:%M:%S.%MSEC", dateyear_t start_date = {.tm_year=2004,.tm_mon=1,.tm_mday=1}) {m_col_name_tstamp = col_name_tstamp; m_tstamp_is_date = is_date; m_date_format = date_format; m_start_date = start_date;}
      // This method can be used to set the name of the column containing the Latitude values in degrees (default: "latitude_deg")
      void setLatitudeColumnName(std::string col_name_lat) {m_col_name_lat = col_name_lat;}
      // This method can be used to set the name of the column containing the Longitude values in degrees (default: "longitude_deg")
      void setLongitudeColumnName(std::string col_name_lon) {m_col_name_lon = col_name_lon;}
      // This method can be used to set the name of the column containing the speed values in m/s (default: "speed_ms")
      void setSpeedColumnName(std::string col_name_speed) {m_col_name_speed = col_name_speed;}
      // This method can be used to set the name of the column containing the heading values in degrees, from 0 to 360 (default: "heading_deg")
      // If the second argument is specified and it is set to "true", the GPS Trace Client Helper will expect values from -180 to 180 degrees and then convert them to the range [0,360)
      void setHeadingColumnName(std::string col_name_heading_deg, bool heading_180 = false) {m_col_name_heading_deg = col_name_heading_deg; m_heading_180 = heading_180;}
      // This method can be used to set the name of the column containing the acceleration values in m/s^2 (default: "accel_ms2")
      // If "unavailable" is used as column name, no acceleration is available in the trace and it will be estimated as constant between couple of points, using delta(v)/delta(t)
      void setAccelerationColumnName(std::string col_name_accel) {m_col_name_accel = col_name_accel;}

      // This method can be used to activate the linear interpolation between real points, to have a more "smooth" trace
      // in case the CSV file does not contain data which is frequently updated
      // i.e. a trace coming from a device updating every 1 Hz can be "enhanced" to a trace with updates every around 100 ms
      // by means of linear interpolation between real trace points
      // When setInterpolation(true) is set, this helper will call setInterpolationPoints(100) for each generated GPSTraceClient object
      // (see gps-tc.h for more details)
      // As this function affects the procedure in reading the CSV file and generating the GPSTraceClient objects, it must be called
      // BEFORE calling createTraceClientsFromCSV()
      // IMPORTANT: keep in mind that this is a very simple and purely linear interpolation between actual GNSS data, which
      // will very probably not correspond to the physical reality between each couple of real points (in which, very probably,
      // the acceleration is not constant, nor the speed or the position variation; in the physical world, it would also be
      // impossible to have a position and a speed which both increase linearly between couple of points)
      // Thus, never use setInterpolation(true) when the physics involving speed and acceleration are involved, but rely
      // instead on the actual points given by the GNSS device only (or use another device providing more frequent updates)
      // It can be useful, instead, for applications in which only the actual position and heading of vehicles matters
      void setInterpolation(bool interpolate)  {m_interpolate = interpolate;}

      // Set a vehicle visualizer object
      // If a vehicle visualizer object is specified, the web-based vehicle visualizer will be used to diplay the vehicles on a map
      // This function shall be called BEFORE createTraceClientsFromCSV(), as it affect the procedure of generating the GPS Trace
      // Client objects of the vehicles
      void setVehicleVisualizer(Ptr<vehicleVisualizer> vehVis) {m_vehicle_vis_ptr=vehVis;}

      void SetInputMicroseconds(bool use_microseconds) {m_use_microseconds = use_microseconds;};

    private:
      std::tuple<double,double> getAverageLatLon(std::ifstream &inFileStream);
      bool m_verbose;

      Ptr<vehicleVisualizer> m_vehicle_vis_ptr;

      std::string m_col_name_vehid;
      std::string m_col_type_agent;
      std::string m_col_name_tstamp;
      std::string m_col_name_lat;
      std::string m_col_name_lon;
      std::string m_col_name_speed;
      std::string m_col_name_heading_deg;
      std::string m_col_name_accel;
      bool m_tstamp_is_date;
      bool m_heading_180;
      std::string m_date_format;
      dateyear_t m_start_date;

      bool m_interpolate;

      bool m_use_microseconds = false;

};

}

#endif /* GPS_TC_HELPER_H */

