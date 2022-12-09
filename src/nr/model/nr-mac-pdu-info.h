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
#ifndef NR_MAC_PDU_INFO_H
#define NR_MAC_PDU_INFO_H

#include "nr-phy-mac-common.h"

namespace ns3 {

/**
 * \ingroup ue-mac
 * \ingroup gnb-mac
 * \brief Used to track the MAC PDU with the slot in which has to go, and the DCI
 * that generated it
 *
 */
struct NrMacPduInfo
{
  /**
   * \brief Construct a NrMacPduInfo
   * \param sfn SfnSf of the PDU
   * \param dci DCI of the PDU
   */
  NrMacPduInfo (SfnSf sfn, std::shared_ptr<DciInfoElementTdma> dci) :
    m_sfnSf (sfn), m_dci (dci)
  {
    for (const auto &it:dci->m_tbSize)
      {
        m_maxBytes += it;
      }
  }

  SfnSf m_sfnSf;                  //!< SfnSf of the PDU
  std::shared_ptr<DciInfoElementTdma> m_dci; //!< The DCI
  uint32_t m_used {0};  //!< Bytes sent down to PHY for this PDU
  uint32_t m_maxBytes {0};
};

} // namespace ns3
#endif // NR_MAC_PDU_INFO_H
