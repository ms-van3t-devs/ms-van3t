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
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "ns3/asn_utils.h"
#include <chrono>
#include <cmath>

namespace ns3 {
  long compute_timestampIts (bool real_time)
  {
    if (real_time)
      {
        /* To get millisec since  2004-01-01T00:00:00:000Z */
        auto time = std::chrono::system_clock::now(); // get the current time
        auto since_epoch = time.time_since_epoch(); // get the duration since epoch
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

        long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
        return elapsed_since_2004;
      }
    else
        return Simulator::Now ().GetMilliSeconds ();

  }

  double haversineDist(double lat_a, double lon_a, double lat_b, double lon_b) {
      // 12742000 is the mean Earth radius (6371 km) * 2 * 1000 (to convert from km to m)
      return 12742000.0*asin(sqrt(sin(DEG_2_RAD(lat_b-lat_a)/2)*sin(DEG_2_RAD(lat_b-lat_a)/2)+cos(DEG_2_RAD(lat_a))*cos(DEG_2_RAD(lat_b))*sin(DEG_2_RAD(lon_b-lon_a)/2)*sin(DEG_2_RAD(lon_b-lon_a)/2)));
  }

  uint8_t
  setByteMask(uint8_t mask)
  {
    uint8_t out = 0;

    for (int i = 0; i < 8; i++) {
        out <<= 1;
        if (mask & 1)
            out |= 1;
        mask >>= 1;
    }
    return out;
  }
  uint8_t
  setByteMask(uint16_t mask, unsigned int i)
  {
    uint8_t  out = 0;
    uint8_t in = (mask >> (i*8)) & 0x000000FF;

    for (int j = 0; j < 8; j++) {
        out <<= 1;
        if (in & 1)
            out |= 1;
        in >>= 1;
    }
    return out;
  }
  uint8_t
  setByteMask(uint32_t mask, unsigned int i)
  {
    uint8_t  out = 0;
    uint8_t in = (mask >> (i*8)) & 0x000000FF;

    for (int j = 0; j< 8; j++) {
        out <<= 1;
        if (in & 1)
            out |= 1;
        in >>= 1;
    }
    return out;
  }

  uint8_t
  getFromMask(uint8_t mask)
  {
    uint8_t out = 0;

    for (int i = 0; i < 8; i++) {
        out <<= 1;
        if (mask & 1)
            out |= 1;
        mask >>= 1;
    }
    return out;
  }
}
