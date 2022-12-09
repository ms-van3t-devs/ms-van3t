/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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

#ifndef SRC_NR_MODEL_NR_PHY_MAC_COMMON_H
#define SRC_NR_MODEL_NR_PHY_MAC_COMMON_H

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <deque>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/string.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/component-carrier.h>
#include <ns3/enum.h>
#include <memory>
#include <ns3/string.h>

#include "sfnsf.h"

namespace ns3 {

struct GetFirst
{
  template <class First, class Second>
  First& operator() (std::pair<First, Second>& p)
  {
    return p.first;
  }

  template <class First, class Second>
  First& operator() (typename std::map<First, Second>::iterator& p)
  {
    return p.first;
  }

  template <class First, class Second>
  const First& operator() (const std::pair<First, Second>& p)
  {
    return p.first;
  }

  template <class First, class Second>
  const First& operator() (const typename std::map<First, Second>::iterator& p)
  {
    return p.first;
  }
};

struct GetSecond
{
  template <class First, class Second>
  Second& operator() (std::pair<First, Second>& p)
  {
    return p.second;
  }

  template <class First, class Second>
  Second& operator() (typename std::map<First, Second>::iterator& p)
  {
    return p.second;
  }

  template <class First, class Second>
  Second& operator() (typename std::unordered_map<First, Second>::iterator& p)
  {
    return p.second;
  }

  template <class First, class Second>
  const Second& operator() (const std::pair<First, Second>& p)
  {
    return p.second;
  }

  template <class First, class Second>
  const Second& operator() (const typename std::map<First, Second>::iterator& p)
  {
    return p.second;
  }

  template <class First, class Second>
  const Second& operator() (const typename std::unordered_map<First, Second>::iterator& p)
  {
    return p.second;
  }
};

/**
 * \ingroup utils
 * \brief The TbInfoElement struct
 */
struct TbInfoElement
{
  TbInfoElement () :
    m_isUplink (0), m_varTtiIdx (0), m_rbBitmap (0), m_rbShift (0), m_rbStart (
      0), m_rbLen (0), m_symStart (0), m_numSym (0), m_resAlloc (0), m_mcs (
      0), m_tbSize (0), m_ndi (0), m_rv (0), m_harqProcess (0)
  {
  }

  bool m_isUplink;              // is uplink grant?
  uint8_t m_varTtiIdx;          // var tti index
  uint32_t m_rbBitmap;          // Resource Block Group bitmap
  uint8_t m_rbShift;            // shift for res alloc type 1
  uint8_t m_rbStart;            // starting RB index for uplink res alloc type 0
  uint16_t m_rbLen;
  uint8_t m_symStart;           // starting symbol index for flexible TTI scheme
  uint8_t m_numSym;             // number of symbols for flexible TTI scheme
  uint8_t m_resAlloc;           // resource allocation type
  uint8_t m_mcs;
  uint32_t m_tbSize;
  uint8_t m_ndi;
  uint8_t m_rv;
  uint8_t m_harqProcess;
};

/**
 * \ingroup utils
 * \brief Scheduling information. Despite the name, it is not TDMA.
 */
struct DciInfoElementTdma
{
  /**
   * \brief Format of the DCI
   */
  enum DciFormat
  {
    DL = 0, //!< DL DCI
    UL = 1  //!< UL DCI
  };

  /**
   * \brief The VarTtiType enum
   */
  enum VarTtiType
  {
    SRS = 0,   //!< Used for SRS (it would be like DCI format 2_3)
    DATA = 1,  //!< Used for DL/UL DATA
    CTRL = 2,  //!< Used for DL/UL CTRL
  };

  /**
   * \brief Constructor used in NrUePhy to build local DCI for DL and UL control
   * \param symStart Sym start
   * \param numSym Num sym
   * \param rbgBitmask Bitmask of RBG
   */
  DciInfoElementTdma (uint8_t symStart, uint8_t numSym, DciFormat format, VarTtiType type,
                      const std::vector<uint8_t> &rbgBitmask)
    : m_format (format),
    m_symStart (symStart),
    m_numSym (numSym),
    m_type (type),
    m_rbgBitmask (rbgBitmask)
  {
  }

  /**
   * \brief Construct to build brand new DCI. Please remember to update manually
   * the HARQ process ID and the RBG bitmask
   *
   * \param rnti
   * \param format
   * \param symStart
   * \param numSym
   * \param mcs
   * \param tbs
   * \param ndi
   * \param rv
   */
  DciInfoElementTdma (uint16_t rnti, DciFormat format, uint8_t symStart,
                      uint8_t numSym, uint8_t mcs, uint32_t tbs, uint8_t ndi,
                      uint8_t rv, VarTtiType type, uint8_t bwpIndex, uint8_t tpc)
    : m_rnti (rnti), m_format (format), m_symStart (symStart),
    m_numSym (numSym), m_mcs (mcs), m_tbSize (tbs), m_ndi (ndi), m_rv (rv), m_type (type),
    m_bwpIndex (bwpIndex), m_tpc (tpc)
  {
  }


  /**
   * \brief Copy constructor except for some values that have to be overwritten
   * \param symStart Sym start
   * \param numSym Num sym
   * \param ndi New Data Indicator: 0 for Retx, 1 for New Data
   * \param rv Retransmission value
   * \param o Other object from which copy all that is not specified as parameter
   */
  DciInfoElementTdma (uint8_t symStart, uint8_t numSym, uint8_t ndi, uint8_t rv,
                      const DciInfoElementTdma &o)
    : m_rnti (o.m_rnti),
      m_format (o.m_format),
      m_symStart (symStart),
      m_numSym (numSym),
      m_mcs (o.m_mcs),
      m_tbSize (o.m_tbSize),
      m_ndi (ndi),
      m_rv (rv),
      m_type (o.m_type),
      m_bwpIndex (o.m_bwpIndex),
      m_harqProcess (o.m_harqProcess),
      m_rbgBitmask (o.m_rbgBitmask),
      m_tpc (o.m_tpc)
  {
  }

  const uint16_t m_rnti       {0};
  const DciFormat m_format    {DL};
  const uint8_t m_symStart    {0};   // starting symbol index for flexible TTI scheme
  const uint8_t m_numSym      {0};   // number of symbols for flexible TTI scheme
  const uint8_t m_mcs         {0};
  const uint32_t m_tbSize     {0};
  const uint8_t m_ndi         {0};   // By default is retransmission
  const uint8_t m_rv          {0};   // not used for UL DCI
  const VarTtiType m_type     {SRS};
  const uint8_t m_bwpIndex    {0};  //!< BWP Index to identify to which BWP this DCI applies to.
  uint8_t m_harqProcess       {0};
  std::vector<uint8_t> m_rbgBitmask  {};   //!< RBG mask: 0 if the RBG is not used, 1 otherwise
  const uint8_t m_tpc         {0};  //!< Tx power control command
};

/**
 * \ingroup utils
 * \brief The TbAllocInfo struct
 */
struct TbAllocInfo
{
  TbAllocInfo () :
    m_rnti (0)
  {

  }
  //struct
  SfnSf m_sfnSf;
  uint16_t m_rnti;
  std::vector<unsigned> m_rbMap;
  TbInfoElement m_tbInfo;
};

/**
 * \ingroup utils
 * \brief The RlcPduInfo struct
 */
struct RlcPduInfo
{
  RlcPduInfo () = default;
  RlcPduInfo (const RlcPduInfo &o) = default;
  ~RlcPduInfo () = default;

  RlcPduInfo (uint8_t lcid, uint32_t size) :
    m_lcid (lcid), m_size (size)
  {
  }
  uint8_t m_lcid  {0};
  uint32_t m_size {0};
};

struct VarTtiAllocInfo
{
  VarTtiAllocInfo (const VarTtiAllocInfo &o) = default;

  VarTtiAllocInfo (const std::shared_ptr<DciInfoElementTdma> &dci)
    : m_dci (dci)
  {
  }

  bool m_isOmni           {false};
  std::shared_ptr<DciInfoElementTdma> m_dci;
  std::vector<RlcPduInfo> m_rlcPduInfo;

  bool operator < (const VarTtiAllocInfo& o) const
  {
    NS_ASSERT (m_dci != nullptr);
    return (m_dci->m_symStart < o.m_dci->m_symStart);
  }
};

/**
 * \ingroup utils
 * \brief The SlotAllocInfo struct
 */
struct SlotAllocInfo
{
  SlotAllocInfo (SfnSf sfn)
    : m_sfnSf (sfn)
  {
  }

  /**
   * \brief Enum which indicates the allocations that are inside the allocation info
   */
  enum AllocationType
  {
    NONE = 0, //!< No allocations
    DL   = 1,  //!< DL Allocations
    UL   = 2,  //!< UL Allocations
    BOTH = 3 //!< DL and UL allocations
  };

  /**
   * \brief Merge the input parameter to this SlotAllocInfo
   * \param other SlotAllocInfo to merge in this allocation
   *
   * After the merge, order the allocation by symStart in DCI
   */
  void Merge (const SlotAllocInfo & other);

  /**
   * \brief Check if we have data allocations
   * \return true if m_varTtiAllocInfo contains data allocations
   */
  bool ContainsDataAllocation () const;

  /**
   * \return true if m_varTtiAllocInfo contains a DL ctrl allocation
   */
  bool ContainsDlCtrlAllocation () const;

  /**
   * \return true if m_varTtiAllocInfo contains a scheduled UL ctrl allocation (e.g., SRS)
   */
  bool ContainsUlCtrlAllocation () const;

  SfnSf m_sfnSf          {};     //!< SfnSf of this allocation
  uint32_t m_numSymAlloc {0};    //!< Number of allocated symbols
  std::deque<VarTtiAllocInfo> m_varTtiAllocInfo; //!< queue of allocations
  AllocationType m_type {NONE}; //!< Allocations type

  /**
   * \brief operator < (less than)
   * \param rhs other SlotAllocInfo to compare
   * \return true if this SlotAllocInfo is less than rhs
   *
   * The comparison is done on sfnSf
   */
  bool operator < (const SlotAllocInfo& rhs) const;
};

/**
 * \ingroup utils
 * \brief The DlCqiInfo struct
 */
struct DlCqiInfo
{
  uint16_t m_rnti {0};
  uint8_t m_ri    {0};
  enum DlCqiType
  {
    WB, SB
  } m_cqiType {WB};
  std::vector<uint8_t> m_rbCqi;   // CQI for each Rsc Block, set to -1 if SINR < Threshold
  uint8_t m_wbCqi {0};   // Wide band CQI
  uint8_t m_wbPmi {0};
};

/**
 * \ingroup utils
 * \brief The UlCqiInfo struct
 */
struct UlCqiInfo
{
  //std::vector <uint16_t> m_sinr;
  std::vector<double> m_sinr;
  enum UlCqiType
  {
    SRS, PUSCH, PUCCH_1, PUCCH_2, PRACH
  } m_type;
};

/**
 * \ingroup utils
 * \brief The MacCeValue struct
 */
struct MacCeValue
{
  MacCeValue () :
    m_phr (0), m_crnti (0)
  {
  }
  uint8_t m_phr;
  uint8_t m_crnti;
  std::vector<uint8_t> m_bufferStatus;
};

/**
 * \ingroup utils
 * \brief See section 4.3.14 macCEListElement
 */
struct MacCeElement
{
  MacCeElement () :
    m_rnti (0)
  {
  }
  uint16_t m_rnti;
  enum MacCeType
  {
    BSR, PHR, CRNTI
  } m_macCeType;
  struct MacCeValue m_macCeValue;
};

/**
 * \ingroup utils
 * \brief The RlcListElement struct
 */
struct RlcListElement
{
  std::vector<struct RlcPduInfo> m_rlcPduElements;
};

/**
 * \ingroup utils
 * \brief The UePhyPacketCountParameter struct
 */
struct UePhyPacketCountParameter
{
  uint64_t m_imsi;
  uint32_t m_noBytes;
  bool m_isTx;   //Set to false if Rx and true if tx
  uint32_t m_subframeno;
};

/**
 * \ingroup utils
 * \brief The GnbPhyPacketCountParameter struct
 */
struct GnbPhyPacketCountParameter
{
  uint64_t m_cellId;
  uint32_t m_noBytes;
  bool m_isTx;   //Set to false if Rx and true if tx
  uint32_t m_subframeno;
};

/**
 * \ingroup utils
 * \brief The RxPacketTraceParams struct
 */
struct RxPacketTraceParams
{
  uint64_t m_cellId {0};
  uint16_t m_rnti {0};
  uint32_t m_frameNum {std::numeric_limits<uint32_t>::max ()};
  uint8_t m_subframeNum {std::numeric_limits<uint8_t>::max ()};
  uint16_t m_slotNum {std::numeric_limits<uint16_t>::max ()};
  uint8_t m_symStart {std::numeric_limits<uint8_t>::max ()};
  uint8_t m_numSym {std::numeric_limits<uint8_t>::max ()};
  uint32_t m_tbSize {0};
  uint8_t m_mcs {std::numeric_limits<uint8_t>::max ()};
  uint8_t m_rv {std::numeric_limits<uint8_t>::max ()};
  double m_sinr {-1.0};
  double m_sinrMin {-1.0};
  double m_tbler {-1.0};
  bool m_corrupt {false};
  uint16_t m_bwpId {std::numeric_limits<uint16_t>::max ()};
  uint32_t m_rbAssignedNum {std::numeric_limits<uint32_t>::max ()};
};

/**
 * \ingroup utils
 * \brief Store information about HARQ
 *
 * \see DlHarqInfo
 * \see UlHarqInfo
 */
struct HarqInfo
{
  virtual ~HarqInfo ()
  {
  }
  uint16_t m_rnti          {55};   //!< RNTI
  uint8_t m_harqProcessId  {15};   //!< ProcessId
  uint8_t m_numRetx        {5};    //!< Num of Retx
  uint8_t m_bwpIndex       {8};    //!< BWP identifier, uniquely identifies BWP within the UE

  /**
   * \return true if the HARQ should be eliminated, since the info has been
   * correctly received
   */
  virtual bool IsReceivedOk () const = 0;
};

/**
 * \ingroup utils
 * \brief A struct that contains info for the DL HARQ
 *
 * http://www.eurecom.fr/~kaltenbe/fapi-2.0/structDlInfoListElement__s.html
 * Note: This should really be called DlInfoListElement ...
 */
struct DlHarqInfo : public HarqInfo
{
  /**
   * \brief Status of the DL Harq: ACKed or NACKed
   */
  enum HarqStatus
  {
    ACK, NACK
  } m_harqStatus {NACK};   //!< HARQ status

  virtual bool IsReceivedOk () const override
  {
    return m_harqStatus == ACK;
  }
};

/**
 * \ingroup utils
 * \brief A struct that contains info for the UL HARQ
 */
struct UlHarqInfo : public HarqInfo
{
  std::vector<uint16_t> m_ulReception;
  enum ReceptionStatus
  {
    Ok, NotOk, NotValid
  } m_receptionStatus;
  uint8_t m_tpc;

  virtual bool IsReceivedOk () const override
  {
    return m_receptionStatus == Ok;
  }
};

std::ostream & operator<< (std::ostream & os, DciInfoElementTdma const & item);
std::ostream & operator<< (std::ostream & os, DciInfoElementTdma::DciFormat const & item);
std::ostream & operator<< (std::ostream & os, DlHarqInfo const & item);
std::ostream & operator<< (std::ostream & os, UlHarqInfo const & item);
std::ostream & operator<< (std::ostream & os, SfnSf const & item);
std::ostream & operator<< (std::ostream & os, SlotAllocInfo const & item);
std::ostream & operator<< (std::ostream & os, SlotAllocInfo::AllocationType const & item);
}

#endif /* SRC_NR_MODEL_NR_PHY_MAC_COMMON_H_ */
