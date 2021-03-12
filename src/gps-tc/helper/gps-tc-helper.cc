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

extern "C" {
    #include "ns3/utmups.h"
}

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

    std::tuple<double,double> getAverageLatLon(std::ifstream &inFileStream, bool header)
    {
        std::string line;
        std::vector<std::string> result;

        double lat0=0;
        double lon0=0;

        // Skip the header
        if (header) {
           std::getline(inFileStream, line);
        }

        // Compute the reference longitude (lon0) to center the Transverse Mercator projection on
        // It is computed as the average between all the longitude values in the trace file, in order
        // to try to minimize the overall average error due to the projection
        int linecount=0;
        while (std::getline(inFileStream, line))
        {
            std::istringstream iss(line);
            result = getNextLineAndSplitIntoTokens(line);

            double curr_lat=std::stod(result[2]);
            double curr_lon=std::stod(result[3]);

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
}
namespace ns3 {
    NS_LOG_COMPONENT_DEFINE("GPSTraceClientHelper");

    GPSTraceClientHelper::GPSTraceClientHelper()
    {
        m_verbose=false;
        m_vehicle_vis_ptr=nullptr;
    }

    std::map<std::string,GPSTraceClient*> GPSTraceClientHelper::createTraceClientsFromCSV(std::string filepath)
    {
      // Variables to read the file
      std::ifstream              inFile;
      std::string                line;
      std::vector<std::string>   result;
      bool header = true;

      // Variables to scan each row of the file
      std::string currentVehId;
      std::map<std::string,GPSTraceClient*> m_GPSTraceClient;

      // GeographicLib (C adaptation) pre-initialized Transverse Mercator structure
      transverse_mercator_t tmerc=UTMUPS_init_UTM_TransverseMercator ();

      inFile.open(filepath);
      if (!inFile) {
          NS_FATAL_ERROR("Unable to open the file: "<<filepath);
      }

      std::tuple<double,double> latlon0=getAverageLatLon (inFile, header);
      double lat0 = std::get<0>(latlon0);
      double lon0 = std::get<1>(latlon0);

      // Skip the header
      if (header) {
         std::getline(inFile, line);
      }

      double min_tm_x=DBL_MAX;
      double min_tm_y=DBL_MAX;

      while (std::getline(inFile, line))
      {

          std::istringstream iss(line);
          result = getNextLineAndSplitIntoTokens(line);
          /* Print the first two elements of the vector *
          std::cout << result[0] << " - " << result[1] << std::endl;
          //*/
          /* Read the whole line
          for (std::vector<std::string>::const_iterator i = result.begin(); i != result.end(); ++i)
              //std::cout << *i << endl;
          */

          // Get the vehicle id
          currentVehId = result[0];
          // Check if the element is present in the map m_GPSTraceClient
          if ( m_GPSTraceClient.find(currentVehId) == m_GPSTraceClient.end() ) {
            // Not found - needed to create the entry for such a vector
            GPSTraceClient* gpsclient = new GPSTraceClient(currentVehId);
            gpsclient->setLat0 (lat0);
            gpsclient->setLon0 (lon0);
            if(m_vehicle_vis_ptr!=nullptr && m_vehicle_vis_ptr->isConnected())
            {
              gpsclient->setVehicleVisualizer (m_vehicle_vis_ptr);
            }
            m_GPSTraceClient.insert(std::make_pair(currentVehId, gpsclient));
          }

          double lat=std::stod(result[2]);
          double lon=std::stod(result[3]);
          double tm_x,tm_y,tm_gamma,tm_kappa;

          // Project lat and lon to a Cartesian Plane using Transverse Mercator
          TransverseMercator_Forward (&tmerc,lon0,lat,lon,&tm_x,&tm_y,&tm_gamma,&tm_kappa);

          m_GPSTraceClient[currentVehId]->setLon0(lon0); // To allow later on to perform TransverseMercator_forward

          //std::cout<<"Converted ("<<lat<<","<<lon<<") into ("<<tm_x<<","<<tm_y<<") [reflon:"<<lon0<<"]"<<std::endl;

          // Save the info of this line of csv
          m_GPSTraceClient[currentVehId]->setTimestamp(result[1]);
          m_GPSTraceClient[currentVehId]->setLat(result[2]);
          m_GPSTraceClient[currentVehId]->setLon(result[3]);
          //
          m_GPSTraceClient[currentVehId]->setX(tm_x);
          m_GPSTraceClient[currentVehId]->setY(tm_y);

          m_GPSTraceClient[currentVehId]->setSpeedms(result[4]);

          // Only degrees should be used for the heading in gps-tc ([5] is discarded as it contains the heading in radians)
          m_GPSTraceClient[currentVehId]->setheading(result[6]);
          m_GPSTraceClient[currentVehId]->setAccelmsq(result[7]);

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

