#ifndef GN_ADDRESS_H
#define GN_ADDRESS_H

#include "ns3/address.h"
#include "ns3/address-utils.h"
#include <stdint.h>
#include <cstring>

#include <ostream>

#include "ns3/attribute-helper.h"

namespace ns3
{
  class GNAddress
  {
  public:
    GNAddress();
    /**
     * \brief Constructs an Ipv6Address by using the input 8 bytes.
     * \param address the 64-bit address
     * \warning the parameter must point on a 8 bytes integer array!
     */
    GNAddress (uint8_t address[8]);
    /**
     * \brief Make the autoconfigured GeoNet address with Mac48Address.
     *
     * ETSI EN 302 636-4-1 [10.2.1.3]
     *
     * \param addr the MAC address (48 bits)
     * \return autoconfigured GeoNet address
     */
    static GNAddress MakeManagedconfiguredAddress (Mac48Address addr, uint8_t ITSType);

    /**
     * \brief Set an GNAddress by using the input 8 bytes.
     *
     * \param address the 64-bit address
     * \warning the parameter must point on a 8 bytes integer array!
     */
    void Set (uint8_t address[8]);

    /**
     * \brief Serialize this address to a 8-byte buffer.
     * \param buf the output buffer to which this address gets overwritten with this
     * GNAddress
     */
    void Serialize (uint8_t buf[8]) const;
    /**
     * \brief Deserialize this address.
     * \param buf buffer to read address from
     * \return an GNAddress
     */
    static GNAddress Deserialize (const uint8_t buf[8]);
    /**
     * \brief Convert to Address object
     */
    operator Address () const;
    /**
     * \param address a polymorphic address
     * \return a new GNAddress from the polymorphic address
     *
     * This function performs a type check and asserts if the
     * type of the input GNAddress.
     */
    static GNAddress ConvertFrom (const Address& address);
    /**
     * \brief Convert to an Address type
     * \return the Address corresponding to this object.
     */
    Address ConvertTo (void) const;

    uint8_t GetITSType(){return m_address[0]>>2;}
    Mac48Address GetLLAddress();
  private:
    /**
     * \brief Get the underlying address type (automatically assigned).
     *
     * \returns the address type
     */
    static uint8_t GetType (void);
    /**
     * \brief The address representation on 64 bits (8 bytes).
     */
    uint8_t m_address[8];

  };
}

#endif // GNAddress_H




