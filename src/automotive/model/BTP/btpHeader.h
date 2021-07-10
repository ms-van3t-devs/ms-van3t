#ifndef BTPHEADER_H
#define BTPHEADER_H

#include <stdint.h>
#include <string>
#include "ns3/header.h"

namespace ns3
{
  class btpHeader : public Header
  {
    public:
      btpHeader();
      ~btpHeader();

      /**
       * \param port the destination port for this BTPHeader
       */
      void SetDestinationPort (uint16_t port);
      /**
       * \param port The source port for this BTPHeader
       */
      void SetSourcePort (uint16_t port);
      /**
       * \param port the destination port for this BTPHeader
       */
      void SetDestinationPortInfo (uint16_t portInfo);
      /**
       * \return The source port for this BTPHeader
       */
      uint16_t GetSourcePort (void) const;
      /**
       * \return the destination port for this BTPHeader
       */
      uint16_t GetDestinationPort (void) const;
      /**
       * \return the destination port for this BTPHeader
       */
      uint16_t GetDestinationPortInfo (void) const;

      static TypeId GetTypeId (void);
      virtual TypeId GetInstanceTypeId (void) const;
      virtual void Print (std::ostream &os) const;
      virtual uint32_t GetSerializedSize (void) const;
      virtual void Serialize (Buffer::Iterator start) const;
      virtual uint32_t Deserialize (Buffer::Iterator start);

    private:
      uint16_t m_destinationPort; //!< Destination port
      uint16_t m_source_destInfo; //!< Source port/Destination port info
  };

}
#endif // BTPHEADER_H
