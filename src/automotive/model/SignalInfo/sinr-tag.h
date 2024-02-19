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
 *  Diego Gasco, Politecnico di Torino (diego.gasco@polito.it, diego.gasco99@gmail.com)
*/

#ifndef SINR_TAG_H
#define SINR_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class Tag;

class SinrTag : public Tag
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Create a SinrTag with the default RSRP 0
   */
  SinrTag ();

  TypeId GetInstanceTypeId (void) const override;
  uint32_t GetSerializedSize (void) const override;
  void Serialize (TagBuffer i) const override;
  void Deserialize (TagBuffer i) override;
  void Print (std::ostream &os) const override;

  /**
   * Set the SINR to the given value.
   *
   * \param sinr the value of the SINR to set
   */
  void Set (double sinr);
  /**
   * Return the SINR value.
   *
   * \return the SINR value
   */
  double Get (void) const;


private:
  double m_sinr;  //!< SINR value
};

}

#endif /* SINR_TAG_H */
