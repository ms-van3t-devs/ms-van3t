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
 *  Giuseppe Avino, Politecnico di Torino (giuseppe.avino@polito.it)
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "gps-tc-helper.h"
#include <iomanip>
#include <climits>

extern "C" {
    #include "ns3/utmups.h"
}

#define UNAVAIL_IDX_ACCEL -INT_MAX

namespace {
    std::vector<std::string> getNextLineAndSplitIntoTokens(std::string row)
    {
        std::vector<std::string>   result;

        std::stringstream          lineStream(row);
        std::string                cell;

        while(std::getline(lineStream,cell, ','))
        {
            result.push_back(cell);
        }
        // This checks for a trailing comma with no data after it
        if (!lineStream && cell.empty())
        {
            // If there was a trailing comma then add an empty element
            result.push_back("");
        }

        return result;
    }
}
namespace ns3 {
    NS_LOG_COMPONENT_DEFINE("GPSTraceClientHelper");

    GPSTraceClientHelper::GPSTraceClientHelper()
    {
        m_verbose=false;
        m_vehicle_vis_ptr=nullptr;

        // Default column names in the CSV file
        m_col_name_vehid = "agent_id";
        m_col_type_agent = "agent_type";
        m_col_name_tstamp = "timeStamp_posix";
        m_col_name_lat = "latitude_deg";
        m_col_name_lon = "longitude_deg";
        m_col_name_speed = "speed_ms";
        m_col_name_heading_deg = "heading_deg";
        m_col_name_accel = "accel_ms2";
        m_tstamp_is_date = false;
        m_date_format = "";
        m_heading_180 = false;
        m_start_date = {.tm_year=2004,.tm_mon=1,.tm_mday=1};
        m_interpolate = false;
    }

    std::tuple<double,double> GPSTraceClientHelper::getAverageLatLon(std::ifstream &inFileStream)
    {
        std::string line;
        std::vector<std::string> result;

        int idx_lat = -1;
        int idx_lon = -1;

        double lat0=0;
        double lon0=0;

        // Get the lat and lon indeces from the header
        std::getline(inFileStream, line);
        std::istringstream header_iss(line);
        result = getNextLineAndSplitIntoTokens(line);

        for(unsigned int i=0;i<result.size ();i++)
        {
            if(result[i] == m_col_name_lat)
            {
                idx_lat = i;
            }
            else if(result[i] == m_col_name_lon)
            {
                idx_lon = i;
            }
        }

        if(idx_lat == -1 ||
           idx_lon == -1)
          {
            if(m_vehicle_vis_ptr!=nullptr)
            {
              m_vehicle_vis_ptr->terminateServer ();
            }

            NS_FATAL_ERROR ("Error while reading the GPS Trace file. Some columns appear to be missing. Please check the file format.");
          }


        // Compute the reference longitude (lon0) to center the Transverse Mercator projection on
        // It is computed as the average between all the longitude values in the trace file, in order
        // to try to minimize the overall average error due to the projection
        int linecount=0;
        while (std::getline(inFileStream, line))
        {
            std::istringstream iss(line);
            result = getNextLineAndSplitIntoTokens(line);

            double curr_lat=std::stod(result[idx_lat]);
            double curr_lon=std::stod(result[idx_lon]);

            linecount++;

            // Computing the "running" average as we get samples, instead of summing up a very large value and then dividing by linecount
            lat0+=(curr_lat-lat0)/linecount;
            lon0+=(curr_lon-lon0)/linecount;
        }

        // "Rewind" the file, in order to read it if needed
        inFileStream.clear();
        inFileStream.seekg(0);

        return std::tuple<double,double>(lat0,lon0);
    }

    std::map<std::string,GPSTraceClient*> GPSTraceClientHelper::createTraceClientsFromCSV(std::string filepath)
    {
      // Variables to read the file
      std::ifstream              inFile;
      std::string                line;
      std::vector<std::string>   result;

      // Variables to scan each row of the file
      std::string currentVehId;
      std::map<std::string,GPSTraceClient*> m_GPSTraceClient;

      // Field indeces
      int m_idx_vehid = -1;
      int m_idx_type = -1;
      int m_idx_tstamp = -1;
      int m_idx_lat = -1;
      int m_idx_lon = -1;
      int m_idx_speed = -1;
      int m_idx_heading_deg = -1;
      int m_idx_accel = -1;

      // GeographicLib (C adaptation) pre-initialized Transverse Mercator structure
      transverse_mercator_t tmerc=UTMUPS_init_UTM_TransverseMercator ();

      inFile.open(filepath);
      if (!inFile) {
          NS_FATAL_ERROR("Unable to open the file: "<<filepath);
      }

      std::tuple<double,double> latlon0 = getAverageLatLon (inFile);

      double lat0 = std::get<0>(latlon0);
      double lon0 = std::get<1>(latlon0);

      // Get the indeces of the different fields from the CSV header, i.e. find out
      // in which columns the needed data is saved inside the CSV file
      std::getline(inFile, line);
      std::istringstream header_iss(line);
      result = getNextLineAndSplitIntoTokens(line);

      for(unsigned int i=0;i<result.size ();i++)
      {
          if(result[i] == m_col_name_vehid)
          {
              m_idx_vehid = i;
          }
          else if (result[i] == m_col_type_agent)
            {
              m_idx_type = i;
            }
          else if(result[i] == m_col_name_tstamp)
          {
              m_idx_tstamp = i;
          }
          else if(result[i] == m_col_name_lat)
          {
              m_idx_lat = i;
          }
          else if(result[i] == m_col_name_lon)
          {
              m_idx_lon = i;
          }
          else if(result[i] == m_col_name_speed)
          {
              m_idx_speed = i;
          }
          else if(result[i] == m_col_name_heading_deg)
          {
              m_idx_heading_deg = i;
          }
          else if(result[i] == m_col_name_accel)
          {
              m_idx_accel = i;
          }
      }

      if(m_col_name_accel == "unavailable")
      {
        m_idx_accel = UNAVAIL_IDX_ACCEL;
      }

      // If one or more columns with needed data are not found inside the CSV file,
      // print an error and terminate the program (i.e. trigger a NS_FATAL_ERROR())
      if(m_idx_vehid == -1 ||
          m_idx_type == -1 ||
         m_idx_tstamp == -1 ||
         m_idx_lat == -1 ||
         m_idx_lon == -1 ||
         m_idx_speed == -1 ||
         m_idx_heading_deg == -1 ||
         m_idx_accel == -1)
        {
          if(m_vehicle_vis_ptr!=nullptr)
          {
            m_vehicle_vis_ptr->terminateServer ();
          }

          NS_FATAL_ERROR ("Error while reading the GPS Trace file. Some columns appear to be missing. Please check the file format.");
        }

      double min_tm_x=DBL_MAX;
      double min_tm_y=DBL_MAX;

      // Read the file line by line and, for each line, gather the values stored inside each column (stored inside "result")
      while (std::getline(inFile, line))
      {
          std::istringstream iss(line);
          result = getNextLineAndSplitIntoTokens(line);

          // Get the vehicle id
          currentVehId = result[m_idx_vehid];
          // Check if the element is present in the map m_GPSTraceClient
          if ( m_GPSTraceClient.find(currentVehId) == m_GPSTraceClient.end() ) {
            // Not found - need to create the entry for such a vector
            std::string type = result[m_idx_type];
            if (type != "car" && type != "vru")
              {
                NS_FATAL_ERROR ("Only car and vru types are supported. Please check the file format.");
              }
            GPSTraceClient* gpsclient = new GPSTraceClient(currentVehId, type);
            gpsclient->setLat0 (lat0);
            gpsclient->setLon0 (lon0);
            if(m_vehicle_vis_ptr!=nullptr && m_vehicle_vis_ptr->isConnected())
            {
              gpsclient->setVehicleVisualizer (m_vehicle_vis_ptr);
            }
            m_GPSTraceClient.insert(std::make_pair(currentVehId, gpsclient));
          }

          double lat=std::stod(result[m_idx_lat]);
          double lon=std::stod(result[m_idx_lon]);
          double tm_x,tm_y,tm_gamma,tm_kappa;

          // Project lat and lon to a Cartesian Plane using Transverse Mercator
          TransverseMercator_Forward (&tmerc,lon0,lat,lon,&tm_x,&tm_y,&tm_gamma,&tm_kappa);

          m_GPSTraceClient[currentVehId]->setLon0(lon0); // To allow later on to perform TransverseMercator_forward

          //std::cout<<"Converted ("<<lat<<","<<lon<<") into ("<<tm_x<<","<<tm_y<<") [reflon:"<<lon0<<"]"<<std::endl;

          /* Save the info of this line of CSV */
          // String which contains a POSIX timestamp, in seconds since epoch (floating point to include also milli or microseconds)
          std::string tstamp;

          // Convert a date format into a timestamp format, if needed
          if(m_tstamp_is_date == true)
          {
              int millisec = 0;
              double tstamp_dbl;

              // Look if milliseconds are specified (%MSEC can be specified only at the end of the string)
              std::size_t found = m_date_format.find("%MSEC");
              if(found != std::string::npos)
              {
                  // %MSEC can only be specified at the end of the date format string
                  if(m_date_format.size () -1 > found + 4)
                  {
                      NS_FATAL_ERROR("Error. Wrong timestamp string format. %MSEC can only be specified at the end of the format string.");
                  }

                  // Only milliseconds are specified -> just convert them directly from the value read from the CSV file in the timestamps column
                  if(found == 0)
                  {
                      millisec = std::stoi(result[m_idx_tstamp]);
                  }
                  // milliseconds are specified with something else -> extract the milliseconds from the string read from the CSV file
                  else
                  {
                    // Get the separator separating the milliseconds from the rest of the time/date
                    char separator = m_date_format.c_str ()[found - 1];

                    // Find the position og the separator at the end of the timestamp string read from the CSV file
                    std::size_t msec_pos = result[m_idx_tstamp].find_last_of (separator);

                    if(msec_pos == std::string::npos)
                    {
                        NS_FATAL_ERROR("Error. Wrong timestamp string format. Cannot match date format with CSV file data.");
                    }

                    // Extract and convert the milliseconds value from the string read from the CSV file
                    std::string msec_str = result[m_idx_tstamp].substr (msec_pos+1,result[m_idx_tstamp].size()-msec_pos-1);
                    millisec = std::stoi(msec_str);

                    // As we have read the milliseconds value, modify the string read from the CSV file by removing the milliseconds value (+ its separator)
                    result[m_idx_tstamp].erase(msec_pos,result[m_idx_tstamp].size()-msec_pos);
                  }
              }

              // If milliseconds are not specified alone (or are not specified at all), convert the date/time value into a timestamp
              if(found != 0)
              {
                std::tm tm = {};
                std::istringstream timess(result[m_idx_tstamp]);
                timess >> std::get_time(&tm,m_date_format.c_str ());

                if(timess.fail())
                {
                    NS_FATAL_ERROR ("Cannot convert a date in the CSV file to a timestamp format. Issue occurred with date: " << result[m_idx_tstamp]);
                }

                if(tm.tm_year==0)
                {
                    // Convert the values from the "human-readable" dateyear_t structure (i.e. years from 0, months 1-12, days 1-31)
                    // to the std::tm data format (i.e. years relative with respect to 1900, months 0-11, days 1-31)
                    tm.tm_year = m_start_date.tm_year-1900;
                    tm.tm_mon = m_start_date.tm_mon-1;
                    tm.tm_mday = m_start_date.tm_mday;
                }

                tstamp_dbl = ((double) mktime(&tm)) + ((double) millisec)/1000.0;
              }
              else
              {
                tstamp_dbl = ((double) millisec)/1000.0;
              }

              tstamp = std::to_string (tstamp_dbl);

              NS_LOG_INFO("Converted date " << result[m_idx_tstamp] << " to timestamp " << tstamp);
          }
          // If the timestamps are already provided as actual timestamps, in the CSV file, just set the "tstamp" string to the value
          // read from the CSV file
          else
          {
              tstamp = result[m_idx_tstamp];
          }

          m_GPSTraceClient[currentVehId]->setTimestamp(tstamp);

          m_GPSTraceClient[currentVehId]->setLat(result[m_idx_lat]);
          m_GPSTraceClient[currentVehId]->setLon(result[m_idx_lon]);

          m_GPSTraceClient[currentVehId]->setX(tm_x);
          m_GPSTraceClient[currentVehId]->setY(tm_y);

          m_GPSTraceClient[currentVehId]->setSpeedms(result[m_idx_speed]);

          // Only degrees between 0 and 360 degrees should be used for the heading in gps-tc
          std::string heading_str;

          // If the user specified that the format used in the CSV file implies heading values between -180 and 180 degees
          // perform the conversion to the range [0,360) before setting the values inside the GPS Trace Client object
          if(m_heading_180 == true)
          {
              double heading_dbl = std::stod(result[m_idx_heading_deg]);
              heading_str = std::to_string(heading_dbl+180.0);
          }
          else
          {
              heading_str = result[m_idx_heading_deg];
          }

          m_GPSTraceClient[currentVehId]->setheading(heading_str);

          if(m_idx_accel != UNAVAIL_IDX_ACCEL)
          {
            m_GPSTraceClient[currentVehId]->setAccelmsq(result[m_idx_accel]);
          }

          // Find the minimum x and y values
          if(tm_x<min_tm_x)
          {
              min_tm_x=tm_x;
          }

          if(tm_y<min_tm_y)
          {
              min_tm_y=tm_y;
          }
      }

      // Sort for each object the vector "vehiclesdata" according to the timestamp
      // Shift also the x,y coordinate in order to have the origin (0,0) at the minimum y and minimum y point
      for(std::map<std::string,GPSTraceClient*>::iterator it=m_GPSTraceClient.begin(); it!=m_GPSTraceClient.end(); ++it) {
          it->second->sortVehiclesdata();

          // If no acceleration is available in the CSV file, make gps-tc compute the acceleration values between each
          // couple of points, considering a constant acceleration (and computing it as delta(v)/delta(t))
          if(m_idx_accel == UNAVAIL_IDX_ACCEL)
          {
            it->second->generateAccelerationValues ();
          }

          // Perform the trace interpolation if required
          // By default, m_interpolate is false and if not explicitely required with setInterpolation(true),
          // no interpolation is performed and the trace is used as-is
          if(m_interpolate == true)
          {
            it->second->setInterpolationPoints(100);
          }

          it->second->shiftOrigin(min_tm_x,min_tm_y);
      }

      //*
      // PRINT ANY OBJECT AND ITS CORRESPONDING VECTOR *
      if(m_verbose==true) {
          for(std::map<std::string,GPSTraceClient*>::iterator it=m_GPSTraceClient.begin(); it!=m_GPSTraceClient.end(); ++it) {
              std::cout << "Key: " << it->first << std::endl;
              it->second->printVehiclesdata();
              std::cout << std::endl;
          }
      }

      if(m_vehicle_vis_ptr!=nullptr && m_vehicle_vis_ptr->isConnected())
      {
          int rval = m_vehicle_vis_ptr->sendMapDraw(lat0,lon0);
          if (rval<0)
          {
              NS_FATAL_ERROR("Error: cannot send the map coordinates to the vehicle visualizer.");
          }
      }
      //*/
      return m_GPSTraceClient;
    }


}

