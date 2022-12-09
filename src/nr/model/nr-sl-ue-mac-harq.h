/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef NR_SL_UE_MAC_HARQ_H
#define NR_SL_UE_MAC_HARQ_H


#include <ns3/object.h>

#include <map>
#include <unordered_set>
#include <deque>

namespace ns3 {

class PacketBurst;
class Packet;

/**
 * \ingroup MAC
 * \brief NR Sidelink MAC HARQ entity
 *
 * This is HARQ entity to NR SL MAC PDUs under retransmission. The
 * total number of HARQ or Sidelink processes can be configured
 * only once by calling \ref InitHarqBuffer (uint8_t), which is
 * the responsibility of a UE MAC.
 */
class NrSlUeMacHarq : public Object
{
public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrSlUeMacHarq constructor
   */
  NrSlUeMacHarq ();

  /**
   * \brief NrSlUeMacHarq destructor
   */
  virtual ~NrSlUeMacHarq ();

  /**
   * \brief Add destination to this HARQ entity
   *
   * This method is responsible to initialize NR SL HARQ process id buffer
   * (see NrSlUeMacHarq#m_nrSlHarqIdBuffer) and NR SL HARQ packet buffer
   * (see NrSlUeMacHarq#m_nrSlHarqPktBuffer). The size of these buffers
   * will be equivalent to the number of max sidelink process number passed
   * through this method.
   *
   * \param maxSlProcesses The maximum number of sidelink processes for
   *        this HARQ entity.
   */
  void InitHarqBuffer (uint8_t maxSlProcesses);

  /**
   * \brief Assign NR Sidelink HARQ process id to a destination
   *
   * This method is used to assign a HARQ process id to a destination
   * if there is an available Sidelink process in NrSlUeMacHarq#m_nrSlHarqIdBuffer.
   * In this implementation, an idle Sidelink process id is basically a number
   * stored in NrSlUeMacHarq#m_nrSlHarqIdBuffer vector, which starts from zero,
   * and ends at <b>maxSidelinkProcess - 1</b>.
   * Moreover, HARQ process id is the same as Sidelink process id.
   *
   * \param dstL2Id The destination Layer 2 id
   * \return The NR Sidelink HARQ id
   */
  uint8_t AssignNrSlHarqProcessId (uint32_t dstL2Id);

  /**
   * \brief Get the total number of available HARQ process ids
   * \return The total number of available HARQ process ids
   */
  uint8_t GetNumAvaiableHarqIds () const;

  /**
   * \brief Is the given HARQ id available
   * \param harqId The HARQ process id
   * \return returns true if the HARQ id is available; otherwise false
   */
  bool IsHarqIdAvaiable (uint8_t harqId) const;

  /**
   * \brief Add the packet to the Sidelink process buffer, which is identified
   *        using destination L2 id, LC id, and the HARQ id.
   * \param dstL2Id The destination Layer 2 id
   * \param lcId The logical channel id
   * \param harqId The HARQ id
   * \param pkt Packet
   */
  void AddPacket (uint32_t dstL2Id, uint8_t lcId, uint8_t harqId, Ptr<Packet> pkt);

  /**
   * \brief Get the packet burst from the Sidelink process buffer, which is identified
   *        using destination L2 id and the HARQ id.
   * \param dstL2Id The destination Layer 2 id
   * \param harqId The HARQ id
   * \return The packet burst
   */
  Ptr<PacketBurst> GetPacketBurst (uint32_t dstL2Id, uint8_t harqId) const;

  /**
   * \brief Receive NR Sidelink Harq feedback
   * \param dstL2Id Destination Layer 2 id
   * \param harqId The harq process id
   */
  void RecvNrSlHarqFeedback (uint32_t dstL2Id, uint8_t harqId);


protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;

private:
  /**
   * \brief struct to store the NR SL HARQ information
   */
  struct NrSlProcessInfo
  {
    Ptr<PacketBurst> pktBurst; //!< TB under HARQ
    // maintain list of LCs contained in this TB
    // used to signal HARQ failure to RLC handlers
    std::unordered_set<uint8_t> lcidList; //!< LC id container
    uint32_t dstL2Id {std::numeric_limits <uint32_t>::max ()};//!< Destination L2 id
  };

  std::vector <NrSlProcessInfo> m_nrSlHarqPktBuffer; //!< NR SL HARQ packet buffer
  std::deque <uint8_t> m_nrSlHarqIdBuffer; //!< A container to store available HARQ/SL process ids
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_HARQ_H */
