#ifndef BEACONHEADER_H
#define BEACONHEADER_H

#include <stdint.h>
#include <string>
#include <cstdint>
#include <cstring>
#include "utils.h"

  class BeaconHeader
  {
    public:
      BeaconHeader();
      ~BeaconHeader();

      void removeHeader(unsigned char * buffer);

      //Getters
      [[nodiscard]] GNlpv_t GetLongPositionV() const {return m_sourcePV;}

      //Setters
      void SetLongPositionV(GNlpv_t longPositionVector) {m_sourcePV = longPositionVector;}

      void addHeader(std::vector<unsigned char>& buffer);

    private:
      GNlpv_t m_sourcePV;  //!Source long position vector
      uint8_t m_reserved; //! aux variable for reading reserved fields
  };



#endif // BEACONHEADER_H
