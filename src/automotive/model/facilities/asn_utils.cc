#include "ns3/asn_utils.h"
#include <chrono>

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
}
