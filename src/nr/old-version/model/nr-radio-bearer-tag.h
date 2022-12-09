/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef NR_RADIO_BEARER_TAG_H
#define NR_RADIO_BEARER_TAG_H


#include "ns3/tag.h"

namespace ns3 {

class Tag;


/**
 * \ingroup spectrum
 *
 * \brief Tag used to define the RNTI and LC id for each MAC packet trasmitted
 */
class NrRadioBearerTag : public Tag
{
public:
  /**
   * \brief Get the object TypeId
   * \return the object type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the InstanceTypeId
   * \return the TypeId of the instance
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create an empty NrRadioBearerTag
   */
  NrRadioBearerTag ();

  /**
   * Create a NrRadioBearerTag with the given RNTI and LC id
   */
  NrRadioBearerTag (uint16_t  rnti, uint8_t lcId, uint32_t size);

  /**
  * Create a NrRadioBearerTag with the given RNTI, LC id and layer
  */
  NrRadioBearerTag (uint16_t  rnti, uint8_t lcId, uint32_t size, uint8_t layer);

  /**
   * Set the RNTI to the given value.
   *
   * \param rnti the value of the RNTI to set
   */
  void SetRnti (uint16_t rnti);

  /**
   * Set the LC id to the given value.
   *
   * \param lcid the value of the RNTI to set
   */
  void SetLcid (uint8_t lcid);

  /**
  * Set the layer id to the given value.
  *
  * \param layer the value of the layer to set
  */
  void SetLayer (uint8_t layer);

  /**
  * Set the size of the RLC PDU in bytes.
  *
  * \param size the size of the RLC PDU in bytes
  */
  void SetSize (uint32_t size);

  // inherited
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual uint32_t GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

  /**
   * \brief Get the Rnti
   * \return the RNTI
   */
  uint16_t GetRnti (void) const;
  /**
   * \brief Get the Lcid
   * \return the LCID
   */
  uint8_t GetLcid (void) const;
  /**
   * \brief Get the Layer
   * \return the layer (?)
   */
  uint8_t GetLayer (void) const;
  /**
   * \brief Get Size
   * \return size in bytes of RLC PDU
   */
  uint32_t GetSize (void) const;

private:
  uint16_t m_rnti;  //!< RNTI
  uint8_t m_lcid;   //!< LCID
  uint8_t m_layer;  //!< Layer
  uint32_t m_size;  //!< Size in bytes of RLC PDU
};

} // namespace ns3

#endif /* NR_RADIO_BEARER_TAG_H */
