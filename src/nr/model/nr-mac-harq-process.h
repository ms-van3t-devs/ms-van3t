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
#pragma once

#include <vector>
#include <ns3/nr-phy-mac-common.h>
#include <memory>

namespace ns3 {
/**
 * \ingroup scheduler
 * \brief The HarqProcess struct
 *
 * The struct represent a single HARQ process, identified by an ID
 * (unfortunately the ID is not stored in the struct, but somewhere else,
 * as it will be explained later).
 * The process has a status (HarqProcess::Status) and could be active or
 * inactive. Inside the process is stored a shared pointer to a DciInfoElementTdma,
 * which contains all the information for the retransmission of the data,
 * as well as the RLC PDU.
 *
 * The HarqProcess will be stored inside the class NrMacHarqVector, which
 * is a unordered map that maps the HARQ ID with the HARQ content (this struct).
 */
struct HarqProcess
{
  /**
   * \brief Status of the process
   *
   * Other than the obvious meaning of the values, it is worth to state that
   * the RECEIVED_FEEDBACK status is equivalent to "the process has received
   * a NACK feedback". An ACKed feedback will be erased immediately (after all,
   * it has been ACKed...). That is probably worth a name change.
   */
  enum Status : uint8_t
  {
    INACTIVE = 0,             //!< Inactive process
    WAITING_FEEDBACK = 1,     //!< Data transmitted, waiting the feedback
    RECEIVED_FEEDBACK = 2     //!< Received feedback (NACK)
  };

  /**
    * \brief Default constructor
    */
  HarqProcess () = default;

  /**
   * \brief HarqProcess copy constructor
   * \param other other instance
   */
  HarqProcess (const HarqProcess &other)
    : m_active (other.m_active),
    m_status (other.m_status),
    m_timer (other.m_timer),
    m_dciElement (other.m_dciElement),
    m_rlcPduInfo (other.m_rlcPduInfo)
  {
  }

  /**
   * \brief HarqProcess value-by-value constructor
   * \param active Is the process active?
   * \param status Status of the process
   * \param timer Timer of the process (in slot)
   * \param dci DCI of the process
   */
  HarqProcess (bool active, Status status, uint8_t timer, const std::shared_ptr<DciInfoElementTdma> & dci)
    : m_active (active),
    m_status (status),
    m_timer (timer),
    m_dciElement (dci)
  {
  }

  /**
   * \brief Reset the Process content.
   */
  void Erase ()
  {
    m_active = false;
    m_status = INACTIVE;
    m_timer = 0;
    m_dciElement.reset ();
    m_rlcPduInfo.clear ();
  }

  bool m_active                         {false};       //!< False indicate that the process is not active
  Status m_status                       {INACTIVE};    //!< Status of the process
  uint8_t m_timer                       {0};           //!< Timer of the process (in slot)
  std::shared_ptr<DciInfoElementTdma> m_dciElement {}; //!< DCI element
  std::vector<std::vector<RlcPduInfo> > m_rlcPduInfo {};            //!< vector of RLC PDU
  std::vector <uint8_t> nackStreamIndexes; //!< vector holding the stream indexes for which gNB received NACK
};

/**
 * \brief operator << for HARQ processes
 * \param os output stream
 * \param item process to print
 * \return a copy of the output stream
 */
static inline std::ostream & operator<< (std::ostream & os, HarqProcess const & item)
{
  if (item.m_active)
    {
      os << "is active, timer=" << static_cast<uint32_t> (item.m_timer)
         << " Status: " << static_cast<uint32_t> (item.m_status);
    }
  return os;
}

} // namespace ns3
