#ifndef SHORTPOSITIONVECTOR_H
#define SHORTPOSITIONVECTOR_H

#include <stdint.h>
#include "ns3/gn-address.h"

namespace ns3 {

  typedef struct _shortPositionVector{
    GNAddress GnAddress; //! Address representation based on ipv6address
    uint32_t TST; //! Time at which lat and long were acquired by GeoAdhoc router
    int32_t latitude;
    int32_t longitude;
  }GNspv_t;

}


#endif // SHORTPOSITIONVECTOR_H
