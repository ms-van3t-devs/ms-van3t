/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * 
 * We would appreciate acknowledgement if the software is used.
 * 
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * 
 * Modified by: NIST (It was tested under ns-3.22)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#include "cv2x_sl-pool.h"

namespace ns3 {
  /**
   * Mapping betwen the index of the transmission repetition pattern and the number
   * of subframes available for D2D transmission (out of 8)
   */
  static const uint8_t ItrpToKtrpMap[107] =
    {
      1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 8
    };

  /**
   * Mapping between the ITRP and the actual bitmap identifying which
   * subframes can be used for D2D transmissions
   */
  static const std::bitset<8> ItrpToBitmap[107] =
    {
      std::bitset<8> (std::string("10000000")),
      std::bitset<8> (std::string("01000000")),
      std::bitset<8> (std::string("00100000")),
      std::bitset<8> (std::string("00010000")),
      std::bitset<8> (std::string("00001000")),
      std::bitset<8> (std::string("00000100")),
      std::bitset<8> (std::string("00000010")),
      std::bitset<8> (std::string("00000001")),
      std::bitset<8> (std::string("11000000")), //k=2
      std::bitset<8> (std::string("10100000")),
      std::bitset<8> (std::string("01100000")), //10
      std::bitset<8> (std::string("10010000")),
      std::bitset<8> (std::string("01010000")),
      std::bitset<8> (std::string("00110000")),
      std::bitset<8> (std::string("10001000")),
      std::bitset<8> (std::string("01001000")),
      std::bitset<8> (std::string("00101000")),
      std::bitset<8> (std::string("00011000")),
      std::bitset<8> (std::string("10000100")),
      std::bitset<8> (std::string("01000100")),
      std::bitset<8> (std::string("00100100")), //20
      std::bitset<8> (std::string("00010100")),
      std::bitset<8> (std::string("00001100")),
      std::bitset<8> (std::string("10000010")),
      std::bitset<8> (std::string("01000010")),
      std::bitset<8> (std::string("00100010")),
      std::bitset<8> (std::string("00010010")),
      std::bitset<8> (std::string("00001010")),
      std::bitset<8> (std::string("00000110")),
      std::bitset<8> (std::string("10000001")),
      std::bitset<8> (std::string("01000001")), //30
      std::bitset<8> (std::string("00100001")),
      std::bitset<8> (std::string("00010001")),
      std::bitset<8> (std::string("00001001")),
      std::bitset<8> (std::string("00000101")),
      std::bitset<8> (std::string("00000011")),
      std::bitset<8> (std::string("11110000")),
      std::bitset<8> (std::string("11101000")),
      std::bitset<8> (std::string("11011000")),
      std::bitset<8> (std::string("10111000")),
      std::bitset<8> (std::string("01111000")), //40
      std::bitset<8> (std::string("11100100")),
      std::bitset<8> (std::string("11010100")),
      std::bitset<8> (std::string("10110100")),
      std::bitset<8> (std::string("01110100")),
      std::bitset<8> (std::string("11001100")),
      std::bitset<8> (std::string("10101100")),
      std::bitset<8> (std::string("01101100")),
      std::bitset<8> (std::string("10011100")),
      std::bitset<8> (std::string("01011100")),
      std::bitset<8> (std::string("00111100")), //50
      std::bitset<8> (std::string("11100010")),
      std::bitset<8> (std::string("11010010")),
      std::bitset<8> (std::string("10110010")),
      std::bitset<8> (std::string("01110010")),
      std::bitset<8> (std::string("11001010")),
      std::bitset<8> (std::string("10101010")),
      std::bitset<8> (std::string("01101010")),
      std::bitset<8> (std::string("10011010")),
      std::bitset<8> (std::string("01011010")),
      std::bitset<8> (std::string("00111010")), //60
      std::bitset<8> (std::string("11000110")),
      std::bitset<8> (std::string("11000110")),
      std::bitset<8> (std::string("01100110")),
      std::bitset<8> (std::string("10010110")),
      std::bitset<8> (std::string("01010110")),
      std::bitset<8> (std::string("00110110")),
      std::bitset<8> (std::string("10001110")),
      std::bitset<8> (std::string("01001110")),
      std::bitset<8> (std::string("00101110")),
      std::bitset<8> (std::string("00011110")), //70
      std::bitset<8> (std::string("11100001")),
      std::bitset<8> (std::string("11010001")),
      std::bitset<8> (std::string("10110001")),
      std::bitset<8> (std::string("01110001")),
      std::bitset<8> (std::string("11001001")),
      std::bitset<8> (std::string("10101001")),
      std::bitset<8> (std::string("01101001")),
      std::bitset<8> (std::string("10011001")),
      std::bitset<8> (std::string("01011001")),
      std::bitset<8> (std::string("00111001")), //80
      std::bitset<8> (std::string("11000101")),
      std::bitset<8> (std::string("10100101")),
      std::bitset<8> (std::string("01100101")),
      std::bitset<8> (std::string("10010101")),
      std::bitset<8> (std::string("01010101")),
      std::bitset<8> (std::string("00110101")),
      std::bitset<8> (std::string("10001101")),
      std::bitset<8> (std::string("01001101")),
      std::bitset<8> (std::string("00101101")),
      std::bitset<8> (std::string("00011101")), //90
      std::bitset<8> (std::string("11000011")),
      std::bitset<8> (std::string("10100011")),
      std::bitset<8> (std::string("01100011")),
      std::bitset<8> (std::string("10010011")),
      std::bitset<8> (std::string("01010011")),
      std::bitset<8> (std::string("00110011")),
      std::bitset<8> (std::string("10001011")),
      std::bitset<8> (std::string("01001011")),
      std::bitset<8> (std::string("00101011")),
      std::bitset<8> (std::string("00011011")), //100
      std::bitset<8> (std::string("10000111")),
      std::bitset<8> (std::string("01000111")),
      std::bitset<8> (std::string("00100111")),
      std::bitset<8> (std::string("00010111")),
      std::bitset<8> (std::string("00001111")),
      std::bitset<8> (std::string("11111111"))      
    };

  
  NS_OBJECT_ENSURE_REGISTERED (SidelinkCommResourcePool);
  
  ///// SidelinkCommResourcePool //////
  SidelinkCommResourcePool::SidelinkCommResourcePool (void) : m_type (SidelinkCommResourcePool::UNKNOWN)
  {
    m_preconfigured = false;
  }
  
  SidelinkCommResourcePool::~SidelinkCommResourcePool (void) 
  {
    
  }


  TypeId 
  SidelinkCommResourcePool::GetTypeId (void)
  {
    static TypeId
      tid =
      TypeId ("ns3::SidelinkCommResourcePool")
      .SetParent<Object> ()
      .AddConstructor<SidelinkCommResourcePool> ()
      ;
    return tid;
  }

  
  bool SidelinkCommResourcePool::operator==(const SidelinkCommResourcePool& other)
  {
    bool equal = m_type == other.m_type
      && m_scCpLen.cplen == other.m_scCpLen.cplen
      && m_scPeriod.period == other.m_scPeriod.period
      && m_scTfResourceConfig == other.m_scTfResourceConfig
      && m_dataCpLen.cplen == other.m_dataCpLen.cplen
      && m_dataHoppingConfig == other.m_dataHoppingConfig;
    if (equal && m_type == SidelinkCommResourcePool::UE_SELECTED) {
      equal = equal && m_dataTfResourceConfig == other.m_dataTfResourceConfig
        && m_trptSubset.subset == other.m_trptSubset.subset;
    }
    return equal;
  }

  void
  SidelinkCommResourcePool::SetPool (cv2x_LteRrcSap::SlCommResourcePool pool)
  {
    //parse information
    m_scCpLen = pool.scCpLen;
    m_scPeriod = pool.scPeriod;
    m_scTfResourceConfig = pool.scTfResourceConfig;
    m_dataCpLen = pool.dataCpLen;
    m_dataHoppingConfig = pool.dataHoppingConfig;

    if (pool.haveUeSelectedResourceConfig) {
      //this is a UE selected pool
      m_type = SidelinkCommResourcePool::UE_SELECTED;
      m_dataTfResourceConfig = pool.ueSelectedResourceConfig.dataTfResourceConfig;
      if (pool.ueSelectedResourceConfig.haveTrptSubset) {
	m_trptSubset = pool.ueSelectedResourceConfig.trptSubset;
      } else {
	//assumes all T-RPT set available
	m_trptSubset.subset.set ();
      }
      
    } else {
      //this is a scheduled pool
      m_type = SidelinkCommResourcePool::SCHEDULED;
    }

    Initialize ();
  }

  void
  SidelinkCommResourcePool::SetPool (cv2x_LteRrcSap::SlPreconfigCommPool pool)
  {
    //preconfigured pools are always UE selected
    m_type = SidelinkCommResourcePool::UE_SELECTED;
    m_preconfigured = false;
    //parse information
    m_scCpLen = pool.scCpLen;
    m_scPeriod = pool.scPeriod;
    m_scTfResourceConfig = pool.scTfResourceConfig;
    m_dataCpLen = pool.dataCpLen;
    m_dataHoppingConfig = pool.dataHoppingConfig;
    m_dataTfResourceConfig = pool.dataTfResourceConfig;
    m_trptSubset = pool.trptSubset;

    Initialize ();
  }

  void
  SidelinkCommResourcePool::Initialize ()
  {
    ComputeNumberOfPscchResources ();
    ComputeNumberOfPsschResources ();
  }

  SidelinkCommResourcePool::SlPoolType
  SidelinkCommResourcePool::GetSchedulingType ()
  {
    return m_type;
  }


  SidelinkCommResourcePool::SubframeInfo
  SidelinkCommResourcePool::GetCurrentScPeriod (uint32_t frameNo, uint32_t subframeNo)
  {
    SubframeInfo currentScPeriod;
    int32_t subframe = 10 * (frameNo % 1024) + subframeNo % 10;
    int32_t period = cv2x_LteRrcSap::PeriodAsInt (m_scPeriod);
    // std::cout << subframe << ", " << m_scTfResourceConfig.offsetIndicator.offset << ", " << (int32_t) (subframe - m_scTfResourceConfig.offsetIndicator.offset) << ", " << (subframe - m_scTfResourceConfig.offsetIndicator.offset) / period << std::endl;
    int32_t currentPeriod = std::floor ((subframe - m_scTfResourceConfig.offsetIndicator.offset) / (double) period);
    uint32_t currentStart = m_scTfResourceConfig.offsetIndicator.offset + currentPeriod * period;
    currentScPeriod.frameNo = (currentStart/10) % 1024;
    currentScPeriod.subframeNo = currentStart % 10;

    // std::cout << "subframe=" << subframe << ", period (ms)=" << period << ", currentPeriod=" << currentPeriod << ", nextStart=" << nextStart << std::endl;
    
    return currentScPeriod;
  }
  
  SidelinkCommResourcePool::SubframeInfo
  SidelinkCommResourcePool::GetNextScPeriod (uint32_t frameNo, uint32_t subframeNo)
  {
    SubframeInfo nextScPeriod;
    int32_t subframe = 10 * (frameNo % 1024) + subframeNo % 10;
    int32_t period = cv2x_LteRrcSap::PeriodAsInt (m_scPeriod);
    // std::cout << subframe << ", " << m_scTfResourceConfig.offsetIndicator.offset << ", " << (int32_t) (subframe - m_scTfResourceConfig.offsetIndicator.offset) << ", " << (subframe - m_scTfResourceConfig.offsetIndicator.offset) / period << std::endl;
    int32_t currentPeriod = std::floor ((subframe - m_scTfResourceConfig.offsetIndicator.offset) / (double) period);

    //if the frame is the last period, we need to start over
    if (currentPeriod == 10240 / period) {
      currentPeriod = -1;
    }
    
    uint32_t nextStart = (m_scTfResourceConfig.offsetIndicator.offset + (currentPeriod + 1) * period);
    nextScPeriod.frameNo = (nextStart/10) % 1024;
    nextScPeriod.subframeNo = nextStart % 10;

    // std::cout << "subframe=" << subframe << ", period (ms)=" << period << ", currentPeriod=" << currentPeriod << ", nextStart=" << nextStart << std::endl;
    
    return nextScPeriod;
  }

  std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>
  SidelinkCommResourcePool::GetPscchTransmissions (uint32_t n)
  {
    NS_ASSERT_MSG (n < m_nPscchResources, "Requesting resource " << n << " but max is " << m_nPscchResources);

    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> trans;
    //36.213 rel 12.5 - 14.2.1.1
    SidelinkCommResourcePool::SidelinkTransmissionInfo first;
    uint32_t subframe = n % m_lpscch;
    first.subframe.frameNo = subframe / 10;
    first.subframe.subframeNo = subframe % 10;
    first.rbStart = std::floor (n / m_lpscch);
    first.nbRb = 1;

    SidelinkCommResourcePool::SidelinkTransmissionInfo second;
    subframe = (n + 1 + (uint32_t) (std::floor (n / m_lpscch)) % (m_lpscch -1)) % m_lpscch;
    second.subframe.frameNo = subframe / 10;
    second.subframe.subframeNo = subframe % 10;
    second.rbStart = std::floor (n / m_lpscch) + std::floor (m_rbpscch / 2);
    second.nbRb = 1;
    
    if (first.subframe < second.subframe)
      {
        trans.push_back (TranslatePscch(first));
        trans.push_back (TranslatePscch(second));
      }
    else
      {    
        trans.push_back (TranslatePscch(second));
        trans.push_back (TranslatePscch(first));
      }
    
    return trans;
  }

  std::list<uint8_t>
  SidelinkCommResourcePool::GetPscchOpportunities (uint32_t frameNo, uint32_t subframeNo)
  {
    std::list<uint8_t> opportunities;
    //first check if the subframe is part of the SC period
    int32_t subframe = 10 * (frameNo % 1024) + subframeNo % 10;
    int32_t period = cv2x_LteRrcSap::PeriodAsInt (m_scPeriod);
    // std::cout << subframe << ", " << m_scTfResourceConfig.offsetIndicator.offset << ", " << (int32_t) (subframe - m_scTfResourceConfig.offsetIndicator.offset) << ", " << (subframe - m_scTfResourceConfig.offsetIndicator.offset) / period << std::endl;
    int32_t currentPeriod = std::floor ((subframe - m_scTfResourceConfig.offsetIndicator.offset) / (double) period);
    uint32_t currentStart = m_scTfResourceConfig.offsetIndicator.offset + currentPeriod * period;

    //check if we are still within the frames that could contain PSCCH opportunities
    if (subframe - currentStart < m_scTfResourceConfig.subframeBitmap.bitmap.size())
      {
        //check if the bitmap is set for this subframe
        if (m_scTfResourceConfig.subframeBitmap.bitmap.test (m_scTfResourceConfig.subframeBitmap.bitmap.size() - ((subframe - currentStart) +1)))
          {
            //subframe is set, report the RBs that could be used
            for (std::vector<uint32_t>::iterator it = m_rbpscchVector.begin(); it != m_rbpscchVector.end() ; it++)
              {
                opportunities.push_back ((*it));
              }
          }
      }
    return opportunities;
  }

  std::vector<int>
  SidelinkCommResourcePool::GetPscchRbs (uint32_t frameNo, uint32_t subframeNo, uint32_t n)
  {
    std::vector<int> rbMask;
    
    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> pscchN = GetPscchTransmissions (n);

    SubframeInfo current;
    current.frameNo = frameNo;
    current.subframeNo = subframeNo;
    
    SubframeInfo currentStart = GetCurrentScPeriod (frameNo, subframeNo);

    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt;
    for (txIt = pscchN.begin (); txIt != pscchN.end() ; txIt++) {
      if (currentStart + txIt->subframe == current) {
        //provide list of RBs to use
        for (int i = txIt->rbStart ; i < txIt->rbStart + txIt->nbRb ; i++) {
          rbMask.push_back (i);
        }
      }
    }
    
    return rbMask;
  }
  
  uint32_t
  SidelinkCommResourcePool::GetNPscch ()
  {
    return m_nPscchResources;
  }

  std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>
  SidelinkCommResourcePool::GetPsschTransmissions (uint8_t itrp, uint16_t rbStart, uint16_t rbLen)
  {
    if (m_type == SidelinkCommResourcePool::UE_SELECTED)
      {
        //check that the itrp is allowed. If preconfigured pool, all values are allowed
        uint8_t ktrp = ItrpToKtrpMap[itrp];
        NS_ASSERT ((ktrp == 1 && m_trptSubset.subset[0]) || (ktrp == 2 && m_trptSubset.subset[1]) || (ktrp == 4 && m_trptSubset.subset[2]) || ktrp==8 /*m_preconfigured*/);

        //All RBs must be within the allowable RBs defined in the pool
        for (uint8_t i = rbStart ; i < rbStart+rbLen ; i++)
          {
            NS_ASSERT (i < m_dataTfResourceConfig.prbStart + m_dataTfResourceConfig.prbNum
                       || i > m_dataTfResourceConfig.prbEnd - m_dataTfResourceConfig.prbNum);
          }
      }
    //N_TRP and the bitmap b' as defined in 14.1.1.1.1
    uint32_t ntrp = 8;
    std::bitset<8> bitmap = ItrpToBitmap[itrp];
    // std::cout << "itrp=" << (uint32_t) itrp << ", bitmap = " << bitmap << std::endl;
    
    std::vector<uint32_t> psschsubframes;
    for (uint32_t i=0; i < m_lpssch; i++)
      {
        //compute b_i = b'_(i mod N_TRP)
        // std::cout << "i=" << i << ", i%ntrp=" << (i % ntrp) << ", b'=" << bitmap[i % ntrp] << std::endl;
        if (bitmap[7- (i % ntrp)]) //reverse order because b0 is the msb in the bitmap
          {
            psschsubframes.push_back (m_lpsschVector[i]);
          }
      }
    // std::cout << " found " << psschsubframes.size() << " subframes" << std::endl;
    //make sure the number of subframes is a multiple of 4
    int tmp = psschsubframes.size () % 4;
    for (int i = 0 ; i < tmp ; i++) {
      psschsubframes.pop_back ();
    }
    // std::cout << " reduced to " << psschsubframes.size() << " subframes" << std::endl;
    
    //for now we do not consider hoping, so all the transmissions occur on the same RBs
    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> txInfo;
    for (std::vector<uint32_t>::iterator it = psschsubframes.begin (); it != psschsubframes.end(); it++) {
      SidelinkCommResourcePool::SidelinkTransmissionInfo info;
      info.subframe.frameNo = (*it) / 10;
      info.subframe.subframeNo = (*it) % 10;
      info.rbStart = rbStart;
      info.nbRb = rbLen;
      txInfo.push_back(info);
    }
    return txInfo;
  }

  std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>
  SidelinkCommResourcePool::GetPsschTransmissions (SidelinkCommResourcePool::SubframeInfo periodStart, uint8_t itrp, uint16_t rbStart, uint16_t rbLen)
  {
    if (m_type == SidelinkCommResourcePool::UE_SELECTED)
      {
        //check that the itrp is allowed. If preconfigured, all values are allowed
        uint8_t ktrp = ItrpToKtrpMap[itrp];
        NS_ASSERT ((ktrp == 1 && m_trptSubset.subset[0]) || (ktrp == 2 && m_trptSubset.subset[1]) || (ktrp == 4 && m_trptSubset.subset[2]) || ktrp==8 /*m_preconfigured*/);

        //All RBs must be within the allowable RBs defined in the pool
        for (uint8_t i = rbStart ; i < rbStart+rbLen ; i++)
          {
            NS_ASSERT_MSG (i < m_dataTfResourceConfig.prbStart + m_dataTfResourceConfig.prbNum
                           || i > m_dataTfResourceConfig.prbEnd - m_dataTfResourceConfig.prbNum,
                           "Rb="<< (uint16_t) i << " not within range: prbStart=" << (uint16_t) m_dataTfResourceConfig.prbStart << ", prbStop="
                           << (uint16_t) m_dataTfResourceConfig.prbEnd << ", prbNum=" << (uint16_t) m_dataTfResourceConfig.prbNum);
          }
      }
    int32_t periodSubframe = 10 * (periodStart.frameNo % 1024) + periodStart.subframeNo % 10;
    
    //N_TRP and the bitmap b' as defined in 14.1.1.1.1
    uint32_t ntrp = 8;
    std::bitset<8> bitmap = ItrpToBitmap[itrp];
    // std::cout << "itrp=" << (uint32_t) itrp << ", bitmap = " << bitmap << std::endl;
    
    std::vector<uint32_t> psschsubframes;
    for (uint32_t i=0; i < m_lpssch; i++)
      {
        //compute b_i = b'_(i mod N_TRP)
        // std::cout << "i=" << i << ", i%ntrp=" << (i % ntrp) << ", b'=" << bitmap[i % ntrp] << std::endl;
        if (bitmap[7- (i % ntrp)] && periodSubframe+m_lpsschVector[i] < 10240) //reverse order because b0 is the msb in the bitmap
          {
            psschsubframes.push_back (periodSubframe+m_lpsschVector[i]);
          }
      }
    // std::cout << " found " << psschsubframes.size() << " subframes" << std::endl;
    //make sure the number of subframes is a multiple of 4
    int tmp = psschsubframes.size () % 4;
    for (int i = 0 ; i < tmp ; i++) {
      psschsubframes.pop_back ();
    }
    // std::cout << " reduced to " << psschsubframes.size() << " subframes" << std::endl;
    
    //for now we do not consider hoping, so all the transmissions occur on the same RBs
    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> txInfo;
    for (std::vector<uint32_t>::iterator it = psschsubframes.begin (); it != psschsubframes.end(); it++) {
      SidelinkCommResourcePool::SidelinkTransmissionInfo info;
      info.subframe.frameNo = (*it) / 10;
      info.subframe.subframeNo = (*it) % 10;
      info.rbStart = rbStart;
      info.nbRb = rbLen;
      txInfo.push_back(info);
    }
    return txInfo;
  }


  SidelinkCommResourcePool::SidelinkTransmissionInfo
  SidelinkCommResourcePool::TranslatePscch (SidelinkCommResourcePool::SidelinkTransmissionInfo info)
  {
    SidelinkCommResourcePool::SidelinkTransmissionInfo translatedInfo;

    //For FDD the bitmap is 40 bits so assuming that all the subframe are set
    //then the maximum is 39
    uint32_t frameNo = 10 * info.subframe.frameNo + info.subframe.subframeNo;
    NS_ASSERT (frameNo < m_lpscchVector.size()) ;

    uint32_t mappedFrame = m_lpscchVector.at (frameNo);
    translatedInfo.subframe.frameNo = mappedFrame / 10;
    translatedInfo.subframe.subframeNo = mappedFrame % 10;
    // std::cout << "Mapped subframe " << info.subframe.frameNo << "/" << info.subframe.subframeNo << " to " << translatedInfo.subframe.frameNo << "/" << translatedInfo.subframe.subframeNo << std::endl;

    //translate the RB
    NS_ASSERT (info.rbStart != m_rbpscchVector.size());
    translatedInfo.rbStart = m_rbpscchVector.at(info.rbStart);
    // std::cout << "Mapped Rb " << (uint32_t) info.rbStart << " to " << (uint32_t) translatedInfo.rbStart << std::endl; 
    translatedInfo.nbRb = info.nbRb;
    
    return translatedInfo;
  }
    
  
  void
  SidelinkCommResourcePool::ComputeNumberOfPscchResources ()
  {
    //number of usable subframes
    for (uint32_t i = 0 ; i < m_scTfResourceConfig.subframeBitmap.bitmap.size() ; i++)
      {
        if (m_scTfResourceConfig.subframeBitmap.bitmap.test (m_scTfResourceConfig.subframeBitmap.bitmap.size() - (i+1)))
          {
            //found a subframe that is part of the PSCCH pool
            m_lpscchVector.push_back (i);
          }
      }
    m_lpscch = m_lpscchVector.size();
    //number of usable RBs
    for (int i = m_scTfResourceConfig.prbStart ; i <= m_scTfResourceConfig.prbEnd ; i++)
      {
        if (i < m_scTfResourceConfig.prbStart + m_scTfResourceConfig.prbNum
            || i > m_scTfResourceConfig.prbEnd - m_scTfResourceConfig.prbNum)
          {
            //found an RB that is part of the PSCCH pool
            m_rbpscchVector.push_back ((uint32_t)i);
          }
      }
    m_rbpscch = m_rbpscchVector.size();
    m_nPscchResources = std::floor (m_lpscch * m_rbpscch / 2);
    // std::cout << "L_pscch=" << m_lpscch << ", m_rb=" << m_rbpscch << ", m_nPscchResources=" << m_nPscchResources << std::endl;
  }

  void
  SidelinkCommResourcePool::ComputeNumberOfPsschResources ()
  {
    //number of usable subframes
    //m_lpssch = 0;
    if (m_type == SidelinkCommResourcePool::SCHEDULED)
      {
        //section 14.1.4 UE procedure for determining subframe pool for sidelink transmission mode 1
        //all subframes with index greater than lpscch + 1
        for (uint32_t i = m_lpscchVector.at (m_lpscch - 1) + 1; i < cv2x_LteRrcSap::PeriodAsInt (m_scPeriod); i++)
          {
            m_lpsschVector.push_back (i);
          }
        m_lpssch = m_lpsschVector.size();
        m_rbpssch = 0;
      }
    else if (m_type == SidelinkCommResourcePool::UE_SELECTED)
      {
        //section 14.1.3
        uint32_t b_j;
        for (uint32_t i = m_dataTfResourceConfig.offsetIndicator.offset ; i < cv2x_LteRrcSap::PeriodAsInt (m_scPeriod) ; i++)
          {
            b_j=(i-m_dataTfResourceConfig.offsetIndicator.offset) % m_dataTfResourceConfig.subframeBitmap.bitmap.size();
            if (m_dataTfResourceConfig.subframeBitmap.bitmap.test (m_dataTfResourceConfig.subframeBitmap.bitmap.size() - (b_j+1)))
              {
                m_lpsschVector.push_back (i);
              }
          }
        m_lpssch = m_lpsschVector.size();

        //number of usable RBs
        for (int i = m_dataTfResourceConfig.prbStart ; i <= m_dataTfResourceConfig.prbEnd ; i++)
          {
            if (i < m_dataTfResourceConfig.prbStart + m_dataTfResourceConfig.prbNum
                || i > m_dataTfResourceConfig.prbEnd - m_dataTfResourceConfig.prbNum)
              {
                m_rbpsschVector.push_back ((uint32_t)i);
              }
          }
        m_rbpssch = m_rbpsschVector.size();
      }
    // std::cout << "PSSCH subframes = " << m_lpssch << ", RBs=" << m_rbpssch << std::endl;     
  }

  ///// SidelinkRxCommResourcePool //////
  TypeId 
  SidelinkRxCommResourcePool::GetTypeId (void)
  {
    static TypeId
      tid =
      TypeId ("ns3::SidelinkRxCommResourcePool")
      .SetParent<SidelinkCommResourcePool> ()
      .AddConstructor<SidelinkRxCommResourcePool> ()
      ;
    return tid;
  }

  ///// SidelinkTxCommResourcePool //////
  TypeId 
  SidelinkTxCommResourcePool::GetTypeId (void)
  {
    static TypeId
      tid =
      TypeId ("ns3::SidelinkTxCommResourcePool")
      .SetParent<SidelinkCommResourcePool> ()
      .AddConstructor<SidelinkTxCommResourcePool> ()
      ;
    return tid;
  }

  void
  SidelinkTxCommResourcePool::SetPool (cv2x_LteRrcSap::SlCommResourcePool pool)
  {
    SidelinkCommResourcePool::SetPool (pool);
    m_index = 0;
    m_mcs = 0;
    //check Tx parameters
    if (m_type == SidelinkCommResourcePool::UE_SELECTED)
    {
      NS_ASSERT (pool.haveTxParameters);
      m_scTxParameters = pool.txParameters.scTxParameters;
      m_dataTxParameters = pool.txParameters.dataTxParameters;
    }
  }

  void
  SidelinkTxCommResourcePool::SetPool (cv2x_LteRrcSap::SlPreconfigCommPool pool)
  {
    SidelinkCommResourcePool::SetPool (pool);
    m_index = 0;
    m_mcs = 0;
    m_scTxParameters = pool.scTxParameters;
    m_dataTxParameters = pool.dataTxParameters;
  }

  void
  SidelinkTxCommResourcePool::SetScheduledTxParameters (uint16_t slrnti, cv2x_LteRrcSap::SlMacMainConfigSl macMainConfig, cv2x_LteRrcSap::SlCommResourcePool commTxConfig, uint8_t index)
  {
    m_slrnti = slrnti;
    m_macMainConfig = macMainConfig;
    m_commTxConfig = commTxConfig;
    m_index = index;
  }
  
  void
  SidelinkTxCommResourcePool::SetScheduledTxParameters (uint16_t slrnti, cv2x_LteRrcSap::SlMacMainConfigSl macMainConfig, cv2x_LteRrcSap::SlCommResourcePool commTxConfig, uint8_t index, uint8_t mcs)
  {
    SetScheduledTxParameters (slrnti, macMainConfig, commTxConfig, index);
    m_mcs = mcs;
  }

  uint8_t
  SidelinkTxCommResourcePool::GetIndex ()
  {
    return m_index;
  }

  uint8_t
  SidelinkTxCommResourcePool::GetMcs ()
  {
    return m_mcs;
  }


    ///////////////////////////////////
    ///// SidelinkV2XResourcePool ///// 
    ///////////////////////////////////

  SidelinkCommResourcePoolV2x::SidelinkCommResourcePoolV2x (void) : m_type (SidelinkCommResourcePoolV2x::UNKNOWN)
  {
    m_preconfigured = false; 
  }

  SidelinkCommResourcePoolV2x::~SidelinkCommResourcePoolV2x (void)
  {

  }

  TypeId
  SidelinkCommResourcePoolV2x::GetTypeId (void)
  {
      static TypeId
      tid =
      TypeId ("ns3::SidelinkCommResourcePoolV2x")
      .SetParent<Object> ()
      .AddConstructor<SidelinkCommResourcePoolV2x> ()
      ;
    return tid;
  }

  bool SidelinkCommResourcePoolV2x::operator==(const SidelinkCommResourcePoolV2x& other)
  {
    bool equal = m_type == other.m_type
      && m_slSubframe.bitmap == other.m_slSubframe.bitmap
      && m_adjacencyPscchPssch.adjacency == other.m_adjacencyPscchPssch.adjacency
      && m_sizeSubchannel.size == other.m_sizeSubchannel.size
      && m_numSubchannel.num == other.m_numSubchannel.num
      && m_startRbSubchannel.startRb == other.m_startRbSubchannel.startRb
      && m_startRbPscchPool.startRb == other.m_startRbPscchPool.startRb; 
    return equal; 
  }

  void 
  SidelinkCommResourcePoolV2x::SetPool (cv2x_LteRrcSap::SlCommResourcePoolV2x pool)
  {
    // not implemented yet 
  } 

  void 
  SidelinkCommResourcePoolV2x::SetPool (cv2x_LteRrcSap::SlV2xPreconfigCommPool pool)
  {
    //preconfigured pools are always UE selected
    m_type = SidelinkCommResourcePoolV2x::UE_SELECTED;
    m_preconfigured = false; 
    // parse information
    m_slSubframe = pool.slSubframeV2x;
    m_adjacencyPscchPssch = pool.adjacencyPscchPssch; 
    m_sizeSubchannel = pool.sizeSubchannel; 
    m_numSubchannel = pool.numSubchannel; 
    m_startRbSubchannel = pool.startRbSubchannel; 
    m_startRbPscchPool = pool.startRbPscchPool; 
    m_dataTxParameters = pool.dataTxParameters; 
    m_slOffsetIndicator = pool.slOffsetIndicator; 
    m_zoneId = pool.zoneId; 
    m_thresSrssiCbr = pool.threshSrssiCbr;
    m_cbrPsschTxConfigList = pool.cbrPsschTxConfigList;
    m_resourceSelectionConfigP2x = pool.resourceSelectionConfigP2x; 
    m_syncAllowed = pool.syncAllowed; 
    m_restrictResourceReservationPeriod = pool.restrictResourceReservationPeriod; 
    Initialize();    
  }

  void 
  SidelinkCommResourcePoolV2x::Initialize ()
  {
    ComputeNumberOfPscchResources ();
    ComputeNumberOfPsschResources ();
  }

  SidelinkCommResourcePoolV2x::SlPoolType
  SidelinkCommResourcePoolV2x::GetSchedulingType ()
  {
    return m_type; 
  }

  void
  SidelinkCommResourcePoolV2x::ComputeNumberOfPscchResources()
  {
    uint16_t sizeSubchhanel = cv2x_LteRrcSap::sizeSubchannelAsInt(m_sizeSubchannel);  
    uint16_t numSubchannel = cv2x_LteRrcSap::numSubchannelAsInt(m_numSubchannel);
    bool adjacency = cv2x_LteRrcSap::adjacencyAsBool(m_adjacencyPscchPssch);
    uint16_t startRbSubchannel = cv2x_LteRrcSap::startRbSubchannelAsInt(m_startRbSubchannel);
    uint16_t prb; 

    // 14.2.4
    if(adjacency == true) 
    { 
      // n_PRB = n_startRB_Subchannel + m*n_sizeSubchannel+j for j=0 and 1
      for(uint16_t m=0; m<numSubchannel; m++){  
        for (uint16_t j=0; j <= 1; j++){
            prb = startRbSubchannel + m*sizeSubchhanel + j; 
            m_rbpscchVector.push_back(prb); 
        }
      }
    }
    else 
    {
      // n_PRB = n_PSCCHstart + 2*m + j for j=0 and 1
      uint16_t pscchStart = cv2x_LteRrcSap::startRbPscchPoolAsInt(m_startRbPscchPool); // only available when adjacency is not enabled
      for(uint16_t m=0; m<numSubchannel; m++){
        for(uint16_t j=0; j<=1; j++){
          prb = pscchStart + 2*m+j; 
          m_rbpscchVector.push_back(prb);
        }
      }
    }
    m_rbpscch = m_rbpscchVector.size(); 
  }

  void 
  SidelinkCommResourcePoolV2x::ComputeNumberOfPsschResources()
  {
    uint16_t sizeSubchannel = cv2x_LteRrcSap::sizeSubchannelAsInt(m_sizeSubchannel);
    uint16_t numSubchannel = cv2x_LteRrcSap::numSubchannelAsInt(m_numSubchannel);
    uint16_t startRbSubchannel = cv2x_LteRrcSap::startRbSubchannelAsInt(m_startRbSubchannel);
    uint16_t prb; 
    
    // 14.1.1
    // n_PRB = n_subCHRBstart + m*n_subCHsize + j
    for(uint16_t m=0; m<numSubchannel; m++){
      for(uint16_t j=0; j<sizeSubchannel; j++){
        prb = startRbSubchannel + m*sizeSubchannel+j; 
        m_rbpsschVector.push_back(prb); 
      }
    }
    m_rbpssch = m_rbpsschVector.size();
  }

  std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>
  SidelinkCommResourcePoolV2x::GetCandidateResources (SidelinkCommResourcePoolV2x::SubframeInfo subframe, uint16_t t1, uint16_t t2, uint16_t subchLen)
  { 
    NS_ASSERT (subframe.frameNo > 0 && subframe.frameNo <= 1024 && subframe.subframeNo > 0 && subframe.subframeNo <= 10);
    NS_ASSERT (t1 >= 0 && t1 <= 4 && t2 >= 20 && t2 <= 100);

    bool adjacency = cv2x_LteRrcSap::adjacencyAsBool(m_adjacencyPscchPssch);
    uint16_t sizeSubch = cv2x_LteRrcSap::sizeSubchannelAsInt(m_sizeSubchannel); 
    uint16_t numSubch = cv2x_LteRrcSap::numSubchannelAsInt(m_numSubchannel);
    uint16_t startRbSubch = cv2x_LteRrcSap::startRbSubchannelAsInt(m_startRbSubchannel);
    uint16_t lenSelectionWindow = t2-t1; 

    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> txInfo;
    SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo info;   

    info.subframe.subframeNo = subframe.subframeNo + t1 - 1;
    info.subframe.frameNo = subframe.frameNo; 
 
    if(info.subframe.subframeNo > 10)
    {
      ++info.subframe.frameNo;
      if(info.subframe.frameNo > 1024) 
      {
        info.subframe.frameNo = 1; 
      }
      info.subframe.subframeNo -= 10; 
    }
 
    if(adjacency) 
    {
      info.rbLen = subchLen*sizeSubch-2;
    }
    else 
    {
      info.rbLen = subchLen*sizeSubch;
    }

    for(uint16_t sfCtr = 0; sfCtr <= lenSelectionWindow; sfCtr++) 
      {
        // due to half duplex the UE doesn't receive SCIs in the subframes in which it transmits itself
        // so it should not choose the last transmission subframe as candidate resource
        if(t2 == 100 && sfCtr == lenSelectionWindow-1) 
        {
          continue; 
        }

        ++info.subframe.subframeNo;  
        if (info.subframe.subframeNo > 10) 
        {
          ++info.subframe.frameNo; 
          if (info.subframe.frameNo > 1024) 
          {
            info.subframe.frameNo = 1; 
          }
          info.subframe.subframeNo -= 10;
        }

        for(uint16_t subchCtr = 0; subchCtr < numSubch; subchCtr++)
        {
          if((subchCtr+subchLen) <= numSubch)
          {
            if(adjacency)
            {
              info.rbStart = startRbSubch + subchCtr*sizeSubch + 2;
            }
            else
            {
              info.rbStart = startRbSubch + subchCtr*sizeSubch;
            }
            txInfo.push_back(info); 
          }
        }
      }
    return txInfo;
  }

  std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>
  SidelinkCommResourcePoolV2x::GetPscchTransmissions (SidelinkCommResourcePoolV2x::SubframeInfo subframe, uint8_t riv, uint16_t pRsvp, uint8_t sfGap, uint8_t reTxIdx, uint8_t pscchResource, uint8_t reselCtr)
  { 
    NS_ASSERT (subframe.frameNo > 0 && subframe.frameNo <= 1024 && subframe.subframeNo > 0 && subframe.subframeNo <= 10);
    
    bool adjacency = cv2x_LteRrcSap::adjacencyAsBool(m_adjacencyPscchPssch);
    uint16_t sizeSubch = cv2x_LteRrcSap::sizeSubchannelAsInt(m_sizeSubchannel);
    uint16_t startRbSubch = cv2x_LteRrcSap::startRbSubchannelAsInt(m_startRbSubchannel); 
    uint16_t startRbPscch = cv2x_LteRrcSap::startRbPscchPoolAsInt(m_startRbPscchPool); 

    uint8_t* tmp = GetValsFromRiv(riv);
    uint8_t subchReTxIdx = tmp[1]; // index for the subchannel of the retransmission

    // 36.213 14.1.1.4C
    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> txInfo;
    SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo first;

    if (adjacency)
    {
      first.subframe.subframeNo = subframe.subframeNo; 
      first.rbStart = startRbSubch + pscchResource*sizeSubch;
      first.rbLen = 2;

      if (sfGap != 0)
      {
        SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo second; 
        second.rbStart = startRbSubch + subchReTxIdx*sizeSubch;
        second.rbLen = 2;

        if (reTxIdx == 0)
        {
          // adjacent, retransmission occurs after initial transmission
          for (uint8_t ctr = 0; ctr < reselCtr; ctr++)
          {
            first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10;
   
            if (first.subframe.subframeNo + sfGap > 20)
            {
              second.subframe.frameNo = first.subframe.frameNo + 2; 
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap - 20; 
            }
            else if (first.subframe.subframeNo + sfGap > 10)
            {
              second.subframe.frameNo = first.subframe.frameNo + 1;
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap - 10; 
            }
            else
            {
              second.subframe.frameNo = first.subframe.frameNo; 
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap; 
            }

            if (first.subframe.frameNo > 2048) 
            {
              first.subframe.frameNo -= 2048;
              second.subframe.frameNo -= 2048;
            }
            else if (second.subframe.frameNo > 2048)
            {
              second.subframe.frameNo -= 2048; 
            }
            else if (first.subframe.frameNo > 1024)
            {
              first.subframe.frameNo -= 1024;
              second.subframe.frameNo -= 1024; 
            }
            else if (second.subframe.frameNo > 1024)
            {
              second.subframe.frameNo -= 1024; 
            }
            txInfo.push_back(first);
            txInfo.push_back(second);
          }
        } // endif reTxIdx == 0
        else 
        {          
          // adjacent, retransmission occurs before initial transmission
          for (uint8_t ctr = 0; ctr < reselCtr; ctr++)
          { 
            first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10;

            if (sfGap >= first.subframe.subframeNo + 10)
            {
              if(first.subframe.frameNo == 2)
              {
                second.subframe.frameNo = 1024;
              }
              else if (first.subframe.frameNo == 1)
              {
                second.subframe.frameNo = 1023;
              }
              else
              {
                second.subframe.frameNo = first.subframe.frameNo - 2; 
              }
              second.subframe.subframeNo = first.subframe.subframeNo + 20 - sfGap;               
            }
            else if (sfGap >= first.subframe.subframeNo)
            {
              if(first.subframe.frameNo == 1)
              {
                second.subframe.frameNo = 1024; 
              }
              else
              {
                second.subframe.frameNo = first.subframe.frameNo - 1; 
              }
              second.subframe.subframeNo = first.subframe.subframeNo + 10 - sfGap; 
            }
            else 
            {
              second.subframe.frameNo = first.subframe.frameNo;
              second.subframe.subframeNo = first.subframe.subframeNo - sfGap; 
            }

            txInfo.push_back(second);
            txInfo.push_back(first);
          }
        }
      } // endif sfGap != 0
      else
      { 
        // adjacent, no retransmission 
        for (uint8_t ctr = 0; ctr < reselCtr; ctr++)
        {
          first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10; 
          if (first.subframe.frameNo > 2048) 
          {
            first.subframe.frameNo -= 2048; 
          }
          else if (first.subframe.frameNo > 1024) 
          {
            first.subframe.frameNo -= 1024; 
          }
          txInfo.push_back(first); 
        }
      }
    } // endif adjacency
    else // non-adjacent
    {
      first.subframe.subframeNo = subframe.subframeNo; 
      first.rbStart = startRbPscch + 2*pscchResource;
      first.rbLen = 2;

      if (sfGap != 0) 
      {
        SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo second;
        second.rbStart = startRbSubch + 2*subchReTxIdx;
        second.rbLen = 2;

        if (reTxIdx == 0)
        {
          // non-adjacent, retransmission occurs after initial transmission
          for (uint8_t ctr = 0; ctr < reselCtr; ctr++)
          { 
            first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10; 

            if (first.subframe.subframeNo + sfGap > 20)
            {
              second.subframe.frameNo = first.subframe.frameNo + 2; 
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap - 20; 
            }
            else if (first.subframe.subframeNo + sfGap > 10)
            {
              second.subframe.frameNo = first.subframe.frameNo + 1;
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap - 10; 
            }
            else
            {
              second.subframe.frameNo = first.subframe.frameNo; 
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap; 
            }
            if (first.subframe.frameNo > 2048) {
              first.subframe.frameNo -= 2048; 
            }
            else if (first.subframe.frameNo > 1024) {
              first.subframe.frameNo -= 1024;
              second.subframe.frameNo -= 1024; 
            }

            txInfo.push_back(first);
            txInfo.push_back(second);
          }
        }
        else 
        {
          // non-adjacent, retransmission occurs before initial transmission
          for (uint8_t ctr = 0; ctr < reselCtr; ctr++)
          {
            first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10; 

            if (sfGap >= first.subframe.subframeNo + 10)
            {
              if(first.subframe.frameNo == 2)
              {
                second.subframe.frameNo = 1024;
              }
              else if (first.subframe.frameNo == 1)
              {
                second.subframe.frameNo = 1023;
              }
              else
              {
                second.subframe.frameNo = first.subframe.frameNo - 2; 
              }
              second.subframe.subframeNo = first.subframe.subframeNo + 20 - sfGap;               
            }
            else if (sfGap >= first.subframe.subframeNo)
            {
              if(first.subframe.frameNo == 1)
              {
                second.subframe.frameNo = 1024; 
              }
              else
              {
                second.subframe.frameNo = first.subframe.frameNo - 1; 
              }
              second.subframe.subframeNo = first.subframe.subframeNo + 10 - sfGap; 
            }
            else 
            {
              second.subframe.frameNo = first.subframe.frameNo;
              second.subframe.subframeNo = first.subframe.subframeNo - sfGap; 
            }
            txInfo.push_back(second);
            txInfo.push_back(first); 
          }
        }
      } // endif sfGap != 0
      else 
      {
        // non-adjacent, no retransmission
        for(uint8_t ctr = 0; ctr < reselCtr; ctr++)
        {
          first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10; 
          if (first.subframe.frameNo > 2048) 
          {
            first.subframe.frameNo -= 2048; 
          }
          else if (first.subframe.frameNo > 1024) 
          {
            first.subframe.frameNo -= 1024; 
          }
          txInfo.push_back(first); 
        }
      }
    }
    return txInfo;
  }

  std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>
  SidelinkCommResourcePoolV2x::GetPsschTransmissions (SidelinkCommResourcePoolV2x::SubframeInfo subframe, uint8_t riv, uint16_t pRsvp, uint8_t sfGap, uint8_t reTxIdx, uint8_t pscchResource, uint8_t reselCtr)
  {
    NS_ASSERT (subframe.frameNo > 0 && subframe.frameNo <= 1024 && subframe.subframeNo > 0 && subframe.subframeNo <= 10);

    bool adjacency = cv2x_LteRrcSap::adjacencyAsBool(m_adjacencyPscchPssch);
    uint8_t* tmp = GetValsFromRiv(riv);
    uint16_t subchLen = tmp[0]; // number of contigious subchannels 
    uint8_t subchReTxIdx = tmp[1]; // index for the subchannel of the retransmission
    uint16_t sizeSubch = cv2x_LteRrcSap::sizeSubchannelAsInt(m_sizeSubchannel);
    uint16_t startRbSubch = cv2x_LteRrcSap::startRbSubchannelAsInt(m_startRbSubchannel); // start of the resource pool for transmission

    // 36.213 14.1.1.4C
    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> txInfo;
    SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo first;

    if(adjacency)
    { 
      first.subframe.subframeNo = subframe.subframeNo; 
      first.rbStart = startRbSubch + pscchResource*sizeSubch+2;
      first.rbLen = subchLen*sizeSubch-2;

      if(sfGap != 0)
      {
        SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo second;
        second.rbStart = startRbSubch + subchReTxIdx*sizeSubch+2;
        second.rbLen = subchLen*sizeSubch-2;

        if (reTxIdx == 0)
        {
          // adjacent, retransmission occurs after initial transmission
          for (uint8_t ctr = 0; ctr < reselCtr; ctr++)
          {
            first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10;

            if (first.subframe.subframeNo + sfGap > 20)
            {
              second.subframe.frameNo = subframe.frameNo + (ctr*pRsvp/10) + 2; 
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap - 20; 
            }
            else if (first.subframe.subframeNo + sfGap > 10)
            {
              second.subframe.frameNo = subframe.frameNo + (ctr*pRsvp/10) + 1;
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap - 10; 
            }
            else 
            {
              second.subframe.frameNo = subframe.frameNo + (ctr*pRsvp/10); 
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap; 
            }

            if (first.subframe.frameNo > 1024) {
              first.subframe.frameNo -= 1024;
              second.subframe.frameNo -= 1024; 
            }
            else if (first.subframe.frameNo > 2048) {
              first.subframe.frameNo -= 2048;
              second.subframe.frameNo -= 2048; 
            }
            else if (second.subframe.frameNo > 2048) {
              second.subframe.frameNo -= 2048; 
            }
            else if (second.subframe.frameNo > 1024) {
              second.subframe.frameNo -= 1024; 
            }
            txInfo.push_back(first);
            txInfo.push_back(second);
          }   
        } // endif reTxIdx == 0
        else
        {
          // adjacent, retransmission occurs before initial transmission+
          for (uint8_t ctr = 0; ctr < reselCtr; ctr++)
          {
            first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10; 

            if (sfGap >= first.subframe.subframeNo + 10)
            {
              if(first.subframe.frameNo == 2)
              {
                second.subframe.frameNo = 1024;
              }
              else if (first.subframe.frameNo == 1)
              {
                second.subframe.frameNo = 1023;
              }
              else
              {
                second.subframe.frameNo = first.subframe.frameNo - 2; 
              }
              second.subframe.subframeNo = first.subframe.subframeNo + 20 - sfGap;               
            }
            else if (sfGap >= first.subframe.subframeNo)
            {
              if(first.subframe.frameNo == 1)
              {
                second.subframe.frameNo = 1024; 
              }
              else
              {
                second.subframe.frameNo = first.subframe.frameNo - 1; 
              }
              second.subframe.subframeNo = first.subframe.subframeNo + 10 - sfGap; 
            }
            else 
            {
              second.subframe.frameNo = first.subframe.frameNo;
              second.subframe.subframeNo = first.subframe.subframeNo - sfGap; 
            }

            txInfo.push_back(second);
            txInfo.push_back(first); 
          }
        }
      } // endif sfGap != 0
      else
      {
        // adjacent, no retransmission
        for(uint8_t ctr = 0; ctr < reselCtr; ctr++)
        {
          first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10; 
          if (first.subframe.frameNo > 2048) 
          {
            first.subframe.frameNo -= 2048; 
          }
          else if (first.subframe.frameNo > 1024) 
          {
            first.subframe.frameNo -= 1024;
          }

          txInfo.push_back(first); 
        }
      }
    } // endif adjacenct
    else 
    {
      first.subframe.subframeNo = subframe.subframeNo; 
      first.rbStart = startRbSubch + pscchResource*sizeSubch;
      first.rbLen = subchLen*sizeSubch;

      if (sfGap != 0)
      {
        SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo second;
        second.rbStart = startRbSubch + subchReTxIdx*sizeSubch+2;
        second.rbLen = subchLen*sizeSubch-2;        

        if (reTxIdx == 0)
        {
          // non-adjacent, retransmission occurs after initial transmission
          for(uint8_t ctr = 0; ctr < reselCtr; ctr++)
          {
            first.subframe.frameNo = subframe.frameNo + ctr*pRsvp; 

            if (first.subframe.subframeNo + sfGap > 20)
            {
              second.subframe.frameNo = first.subframe.frameNo + 2; 
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap - 20; 
            }
            else if (first.subframe.subframeNo + sfGap > 10)
            {
              second.subframe.frameNo = first.subframe.frameNo + 1;
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap - 10; 
            }
            else
            {
              second.subframe.frameNo = first.subframe.frameNo; 
              second.subframe.subframeNo = first.subframe.subframeNo + sfGap; 
            }
            if (first.subframe.frameNo > 2048) {
              first.subframe.frameNo -= 2048;
              second.subframe.frameNo -= 2048;  
            }
            else if (first.subframe.frameNo > 1024)
            {
              first.subframe.frameNo -= 1024;
              second.subframe.frameNo -= 1024; 
            }

            txInfo.push_back(first);
            txInfo.push_back(second);
          }
        }
        else
        {
          // non-adjacent, retransmission occurs before initial transmission
          for(uint8_t ctr = 0; ctr < reselCtr; ctr++)
          {
            first.subframe.frameNo = subframe.frameNo + ctr*pRsvp; 

            if (sfGap >= first.subframe.subframeNo + 10)
            {
              if(first.subframe.frameNo == 2)
              {
                second.subframe.frameNo = 1024;
              }
              else if (first.subframe.frameNo == 1)
              {
                second.subframe.frameNo = 1023;
              }
              else
              {
                second.subframe.frameNo = first.subframe.frameNo - 2; 
              }
              second.subframe.subframeNo = first.subframe.subframeNo + 20 - sfGap;               
            }
            else if (sfGap >= first.subframe.subframeNo)
            {
              if(first.subframe.frameNo == 1)
              {
                second.subframe.frameNo = 1024; 
              }
              else
              {
                second.subframe.frameNo = first.subframe.frameNo - 1; 
              }
              second.subframe.subframeNo = first.subframe.subframeNo + 10 - sfGap; 
            }
            else 
            {
              second.subframe.frameNo = first.subframe.frameNo;
              second.subframe.subframeNo = first.subframe.subframeNo - sfGap; 
            }
            txInfo.push_back(first);
            txInfo.push_back(second);
          }
        }
      } // endif sfGap != 0
      else
      {
        // non-adjacent, no retransmission 
        for(uint8_t ctr = 0; ctr < reselCtr; ctr++)
        {
          first.subframe.frameNo = subframe.frameNo + ctr*pRsvp/10; 

          if (first.subframe.frameNo > 2048)
          {
            first.subframe.frameNo -= 2048; 
          }
          else if (first.subframe.frameNo > 1024)
          {
            first.subframe.frameNo -= 1024; 
          }
          txInfo.push_back(first); 
        }
      }
    }
    return txInfo;  
  }
  
  std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>
  SidelinkCommResourcePoolV2x::GetPsschTransmissions (uint8_t riv, uint8_t pscchResource)
  {
    bool adjacency = cv2x_LteRrcSap::adjacencyAsBool(m_adjacencyPscchPssch);
    uint16_t sizeSubch = cv2x_LteRrcSap::sizeSubchannelAsInt(m_sizeSubchannel);
    uint16_t startRbSubch = cv2x_LteRrcSap::startRbSubchannelAsInt(m_startRbSubchannel); // start of the resource pool for transmission
    uint8_t* tmp = GetValsFromRiv(riv);
    uint8_t subchLen = tmp[0]; // number of contigious subchannels 
    delete[] tmp;

    // 36.213 14.1.1.4C
    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> txInfo;
    SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo first;

    if(adjacency)
    {
      first.rbStart = pscchResource*sizeSubch + 2 + startRbSubch;
    }
    else 
    {
      first.rbStart = pscchResource*sizeSubch + startRbSubch; 
    }
    first.rbLen = subchLen*sizeSubch-2; 
    txInfo.push_back(first); 
    return txInfo;  
  }

  uint8_t*
  SidelinkCommResourcePoolV2x::GetValsFromRiv(uint8_t riv)
  {
    uint16_t numSubchannel = cv2x_LteRrcSap::numSubchannelAsInt(m_numSubchannel); // Number of subchannels per subframe
    //uint8_t *vals = (uint8_t*)malloc(2*sizeof(uint8_t));
    uint8_t *vals = new uint8_t[2];
    //uint8_t vals[2]; // vals[0] := L_subCH, vals[1] := startSubchannelIdx

    for(uint16_t n=1; n<=numSubchannel;n++)
    {
      for(uint16_t m=0; m<=(numSubchannel-n);m++)
      {
          if((n-1) <= std::floor(numSubchannel/2))
          {
              if(riv == (numSubchannel*(n-1)+m))
              {
                  vals[0] = n;
                  vals[1] = m;
              }
          }
          else
          {
              if(riv == (numSubchannel*(numSubchannel-n+1)+(numSubchannel-1-m)))
              {
                  vals[0] = n;
                  vals[1] = m;
              } 
          } 
      } 
    }
    return vals; 
  }

  uint16_t 
  SidelinkCommResourcePoolV2x::ConvertResourceReservationFromBitToInt(std::bitset<4> pBits)
  {
    uint16_t pRsvp = pBits.to_ulong(); 

    NS_ASSERT (pRsvp != 0 && pRsvp != 13 && pRsvp != 14 && pRsvp != 15);

    if(pRsvp == 11)
    {
      pRsvp = 50; // := 50ms
    }
    else if (pRsvp == 12)
    {
      pRsvp = 20; // := 20ms
    }
    else 
    {
      pRsvp = pRsvp * 100; // := 100 .. 1000ms
    }
    return pRsvp; 
  }

  TypeId 
  SidelinkRxCommResourcePoolV2x::GetTypeId (void)
  {
    static TypeId
      tid =
      TypeId ("ns3::SidelinkRxCommResourcePoolV2x")
      .SetParent<SidelinkCommResourcePoolV2x> ()
      .AddConstructor<SidelinkRxCommResourcePoolV2x> ()
      ;
    return tid;
  }

TypeId 
  SidelinkTxCommResourcePoolV2x::GetTypeId (void)
  {
    static TypeId
      tid =
      TypeId ("ns3::SidelinkTxCommResourcePoolV2x")
      .SetParent<SidelinkCommResourcePoolV2x> ()
      .AddConstructor<SidelinkTxCommResourcePoolV2x> ()
      ;
    return tid;
  }

  void
  SidelinkTxCommResourcePoolV2x::SetPool (cv2x_LteRrcSap::SlCommResourcePoolV2x pool)
  {
    SidelinkCommResourcePoolV2x::SetPool (pool);
    m_index = 0;
    m_mcs = 0;
    //check Tx parameters
    if (m_type == SidelinkCommResourcePoolV2x::UE_SELECTED)
    {
      m_dataTxParameters = pool.dataTxParameters;
    }
  }

  void
  SidelinkTxCommResourcePoolV2x::SetPool (cv2x_LteRrcSap::SlV2xPreconfigCommPool pool)
  {
    SidelinkCommResourcePoolV2x::SetPool (pool);
    m_index = 0;
    m_mcs = 0;
    m_dataTxParameters = pool.dataTxParameters;
  }

  uint8_t
  SidelinkTxCommResourcePoolV2x::GetIndex ()
  {
    return m_index;
  }

  uint8_t
  SidelinkTxCommResourcePoolV2x::GetMcs ()
  {
    return m_mcs;
  }



    ///// SidelinkDiscResourcePool //////
  SidelinkDiscResourcePool::SidelinkDiscResourcePool (void) : m_type (SidelinkDiscResourcePool::UNKNOWN)
  {
    m_preconfigured = false;
  }
  
  SidelinkDiscResourcePool::~SidelinkDiscResourcePool (void) 
  {
    
  }

  TypeId 
  SidelinkDiscResourcePool::GetTypeId (void)
  {
    static TypeId
      tid =
      TypeId ("ns3::SidelinkDiscResourcePool")
      .SetParent<Object> ()
      .AddConstructor<SidelinkDiscResourcePool> ()
      ;
    return tid;
  }

  void
  SidelinkDiscResourcePool::SetPool (cv2x_LteRrcSap::SlDiscResourcePool pool)
  {
    //parse information
    m_discCpLen = pool.cpLen;
    m_discPeriod = pool.discPeriod;
    m_numRetx = pool.numRetx;
    m_numRepetition = pool.numRepetition;
    //NS_ASSERT_MSG (pool.tfResourceConfig.prbNum % 2 == 0, "code currently supports only multiple of 2 RB"); //not true
    m_discTfResourceConfig = pool.tfResourceConfig;
    m_haveTxParameters = pool.haveTxParameters;
    m_txParametersGeneral = pool.txParameters.txParametersGeneral;
    if (pool.haveTxParameters) {
      m_type = SidelinkDiscResourcePool::UE_SELECTED;
    }
    else {
      m_type = SidelinkDiscResourcePool::SCHEDULED;
    }
    Initialize ();
  }

  void
  SidelinkDiscResourcePool::SetPool (cv2x_LteRrcSap::SlPreconfigDiscPool pool)
  {
    //preconfigured pools are always UE selected
    m_type = SidelinkDiscResourcePool::UE_SELECTED;
    m_preconfigured = true;
    m_discCpLen = pool.cpLen;
    m_discPeriod = pool.discPeriod;
    m_numRetx = pool.numRetx;
    m_numRepetition = pool.numRepetition;
    //NS_ASSERT_MSG (pool.tfResourceConfig.prbNum % 2 == 0, "code currently supports only multiple of 2 RB"); //not true
    m_discTfResourceConfig = pool.tfResourceConfig;
    Initialize ();
  }


  void
  SidelinkDiscResourcePool::Initialize ()
  {
    ComputeNumberOfPsdchResources (); 
  }

  SidelinkDiscResourcePool::SlPoolType
  SidelinkDiscResourcePool::GetSchedulingType ()
  {
    return m_type;
  }

  SidelinkDiscResourcePool::SubframeInfo
  SidelinkDiscResourcePool::GetCurrentDiscPeriod (uint32_t frameNo, uint32_t subframeNo)
  {
    SubframeInfo currentDiscPeriod;
    int32_t subframe = 10 * (frameNo % 1024) + subframeNo % 10;
    int32_t period = cv2x_LteRrcSap::DiscPeriodAsInt (m_discPeriod);
    int32_t currentPeriod = std::floor ((subframe - m_discTfResourceConfig.offsetIndicator.offset) / (double) period);

    uint32_t currentStart = m_discTfResourceConfig.offsetIndicator.offset + currentPeriod * period;
    currentDiscPeriod.frameNo = (currentStart/10) % 1024;
    currentDiscPeriod.subframeNo = currentStart % 10;    
    return currentDiscPeriod;
  }

  SidelinkDiscResourcePool::SubframeInfo
  SidelinkDiscResourcePool::GetNextDiscPeriod (uint32_t frameNo, uint32_t subframeNo)
  {
    SubframeInfo nextDiscPeriod;
    int32_t subframe = 10 * (frameNo % 1024) + subframeNo % 10;
    int32_t period = cv2x_LteRrcSap::DiscPeriodAsInt (m_discPeriod);
    int32_t currentPeriod = std::floor ((subframe - m_discTfResourceConfig.offsetIndicator.offset) / (double) period);

    //if the frame is the last period, we need to start over
    if (currentPeriod == 10240 / period) {
      currentPeriod = -1;
    }
    uint32_t nextStart = (m_discTfResourceConfig.offsetIndicator.offset + (currentPeriod + 1) * period);
    nextDiscPeriod.frameNo = (nextStart/10) % 1024;
    nextDiscPeriod.subframeNo = nextStart % 10;
    return nextDiscPeriod;
  }

  uint32_t
  SidelinkDiscResourcePool::GetNPsdch ()
  {
    return m_nPsdchResources;
  }

  uint32_t
  SidelinkDiscResourcePool::GetNSubframes ()
  {
    return m_lpsdch;
  }

  uint32_t
  SidelinkDiscResourcePool::GetNRbPairs ()
  {
    return m_rbpsdch/2;
  }

  uint8_t
  SidelinkDiscResourcePool::GetNumRetx ()
  {
    return m_numRetx;
  }
 
  int32_t 
  SidelinkDiscResourcePool::GetDiscPeriod ()
  {
    return cv2x_LteRrcSap::DiscPeriodAsInt (m_discPeriod);
  }


  std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo>
  SidelinkDiscResourcePool::GetPsdchTransmissions (uint32_t npsdch)
  {
    std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo> txInfo;

    // 36.213 14.3.1
    uint32_t n = m_numRetx + 1;
    uint32_t nf = std::floor (m_rbpsdch / 2);
    uint32_t nt = std::floor (m_lpsdch / n);
    uint32_t aj;
    uint32_t b1;

    if (m_type == SidelinkDiscResourcePool::UE_SELECTED)
    {
      for (uint32_t j = 1; j <=n; j++)
      {
        uint32_t inter = ((j -1) * std::floor (nf / n)) + std::floor (npsdch / nt);
        aj = inter % nf;
        b1 = npsdch % nt;
        SidelinkDiscResourcePool::SidelinkTransmissionInfo info;
        uint32_t subframe = (n * b1) + j - 1;
        info.subframe.frameNo = subframe / 10;
        info.subframe.subframeNo = subframe % 10;
        info.rbStart = std::floor (2 * aj);
        info.nbRb = 2; 
        txInfo.push_back (TranslatePsdch (info));
      }
    }
    else if (m_type == SidelinkDiscResourcePool::SCHEDULED)
    {
      // not implemented yet //
      // TODO
    }


    return txInfo;
  }
  
  void
  SidelinkDiscResourcePool::ComputeNumberOfPsdchResources ()
  {
    //36.213 14.3.3
    if (m_type == SidelinkDiscResourcePool::UE_SELECTED)
    {
      //subframes pool
      uint32_t bj;
      for (uint32_t j = 0; j < (m_numRepetition * m_discTfResourceConfig.subframeBitmap.bitmap.size()); j++)
      {
        bj = j % m_discTfResourceConfig.subframeBitmap.bitmap.size();
        if (m_discTfResourceConfig.subframeBitmap.bitmap.test (m_discTfResourceConfig.subframeBitmap.bitmap.size() - (bj + 1)))
        {
          m_lpsdchVector.push_back (j);
        }
      }

      m_lpsdch = m_lpsdchVector.size();

      //resource blocs pool
      for (int i = m_discTfResourceConfig.prbStart ; i <= m_discTfResourceConfig.prbEnd ; i++)
      {
        if ((i < m_discTfResourceConfig.prbStart + m_discTfResourceConfig.prbNum) || (i > m_discTfResourceConfig.prbEnd - m_discTfResourceConfig.prbNum))
        {
          m_rbpsdchVector.push_back ((uint32_t)i);
        }
      }
      m_rbpsdch = m_rbpsdchVector.size();
    }

    else if (m_type == SidelinkDiscResourcePool::SCHEDULED) 
    {
      // Not implemented yet //
      //TODO
      /*
      for (uint32_t i = 0; i < m_discTfIndexList.nbPair; i++)
      {
        m_lpsdchVector.push_back (m_discTfIndexList.pair [i].discSfIndex);
        m_rbpsdchVector.push_back (m_discTfIndexList.pair [i].discPrbIndex);
      }
      m_lpsdch = m_lpsdchVector.size();
      m_rbpsdch = m_rbpsdchVector.size();
      */
    }

    m_nPsdchResources = std::floor (m_lpsdch /(m_numRetx + 1)) * std::floor (m_rbpsdch / 2);
    // std::cout << "l_psdch=" << m_lpsdch << ", m_rb=" << m_rbpsdch << ", m_nPsdchResources=" << m_nPsdchResources << std::endl;
  }

  SidelinkDiscResourcePool::SidelinkTransmissionInfo
  SidelinkDiscResourcePool::TranslatePsdch (SidelinkDiscResourcePool::SidelinkTransmissionInfo info)
  {
    SidelinkDiscResourcePool::SidelinkTransmissionInfo translatedInfo;

    //For FDD the bitmap is 40 bits so assuming that all the subframe are set
    //then the maximum is 39
    uint32_t frameNo = 10 * info.subframe.frameNo + info.subframe.subframeNo;
    NS_ASSERT (frameNo < m_lpsdchVector.size()) ;

    uint32_t mappedFrame = m_lpsdchVector.at (frameNo);
    translatedInfo.subframe.frameNo = mappedFrame / 10;
    translatedInfo.subframe.subframeNo = mappedFrame % 10;
    // std::cout << "Mapped subframe " << info.subframe.frameNo << "/" << info.subframe.subframeNo << " to " << translatedInfo.subframe.frameNo << "/" << translatedInfo.subframe.subframeNo << std::endl;

    //translate the RB
    NS_ASSERT (info.rbStart != m_rbpsdchVector.size());
    translatedInfo.rbStart = m_rbpsdchVector.at(info.rbStart);
    // std::cout << "Mapped Rb " << (uint32_t) info.rbStart << " to " << (uint32_t) translatedInfo.rbStart << std::endl; 
    translatedInfo.nbRb = info.nbRb;
    
    return translatedInfo;
  }
     
  std::list<uint8_t>
  SidelinkDiscResourcePool::GetPsdchOpportunities (uint32_t frameNo, uint32_t subframeNo)
  {
    std::list<uint8_t> opportunities;
    //first check if the subframe is part of the discovery period
    int32_t subframe = 10 * (frameNo % 1024) + subframeNo % 10;
    int32_t period = cv2x_LteRrcSap::DiscPeriodAsInt (m_discPeriod);
    int32_t currentPeriod = std::floor ((subframe - m_discTfResourceConfig.offsetIndicator.offset) / (double) period);
    uint32_t currentStart = m_discTfResourceConfig.offsetIndicator.offset + currentPeriod * period;

    //check if we are still within the frames that could contain PSCCH opportunities
    if (subframe - currentStart < m_discTfResourceConfig.subframeBitmap.bitmap.size())
      {
        //check if the bitmap is set for this subframe
        if (m_discTfResourceConfig.subframeBitmap.bitmap.test (m_discTfResourceConfig.subframeBitmap.bitmap.size() - ((subframe - currentStart) +1)))
          {
            //subframe is set, report the RBs that could be used
            for (std::vector<uint32_t>::iterator it = m_rbpsdchVector.begin(); it != m_rbpsdchVector.end() ; it++)
              {
                opportunities.push_back ((*it));
              }
          }
      }
    return opportunities;
  }
 
  ///// SidelinkRxDiscResourcePool //////
  TypeId 
  SidelinkRxDiscResourcePool::GetTypeId (void)
  {
    static TypeId
      tid =
      TypeId ("ns3::SidelinkRxDiscResourcePool")
      .SetParent<SidelinkDiscResourcePool> ()
      .AddConstructor<SidelinkRxDiscResourcePool> ()
      ;
    return tid;
  }

  ///// SidelinkTxDiscResourcePool //////
  TypeId 
  SidelinkTxDiscResourcePool::GetTypeId (void)
  {
    static TypeId
      tid =
      TypeId ("ns3::SidelinkTxDiscResourcePool")
      .SetParent<SidelinkDiscResourcePool> ()
      .AddConstructor<SidelinkTxDiscResourcePool> ()
      ;
    return tid;
  }

  void
  SidelinkTxDiscResourcePool::SetPool (cv2x_LteRrcSap::SlDiscResourcePool pool)
  {
    SidelinkDiscResourcePool::SetPool (pool);

    //check Tx parameters
    if (m_type == SidelinkDiscResourcePool::UE_SELECTED)
    {
	    NS_ASSERT (pool.haveTxParameters);
      m_haveTxParameters = pool.haveTxParameters;
      m_txProbability = pool.txParameters.ueSelectedResourceConfig.txProbability;
      m_poolSelection = pool.txParameters.ueSelectedResourceConfig.poolSelection;
      m_havePoolSelectionRsrpBased = pool.txParameters.ueSelectedResourceConfig.havePoolSelectionRsrpBased;
      if (pool.txParameters.ueSelectedResourceConfig.havePoolSelectionRsrpBased) 
      {
        m_poolSelectionRsrpBased.threshLow = pool.txParameters.ueSelectedResourceConfig.poolSelectionRsrpBased.threshLow;
        m_poolSelectionRsrpBased.threshHigh = pool.txParameters.ueSelectedResourceConfig.poolSelectionRsrpBased.threshHigh;
      }
      else 
      { 
        /*random */ 
      }
      m_txProbChanged = false;
    }
  }

  void
  SidelinkTxDiscResourcePool::SetPool (cv2x_LteRrcSap::SlPreconfigDiscPool pool)
  {
    SidelinkDiscResourcePool::SetPool (pool);
    m_txParametersGeneral = pool.txParameters.txParametersGeneral;
    m_txProbability = pool.txParameters.txProbability;
    m_txProbChanged = false;
  }


  void
  SidelinkTxDiscResourcePool::SetScheduledTxParameters (cv2x_LteRrcSap::SlDiscResourcePool discPool, cv2x_LteRrcSap::SlTfIndexPairList discResources, cv2x_LteRrcSap::SlHoppingConfigDisc discHopping)
  {
    m_discTxConfig = discPool;
    m_discTfIndexList = discResources;
    m_discHoppingConfigDisc = discHopping;
  }

  uint32_t 
  SidelinkTxDiscResourcePool::GetTxProbability ()
  {
    return cv2x_LteRrcSap::TxProbabilityAsInt (m_txProbability);
  }

  void SidelinkTxDiscResourcePool::SetTxProbability (uint32_t theta)
  {
    cv2x_LteRrcSap::TxProbability txProb;
    if (theta == 25)
    {
      txProb.probability = cv2x_LteRrcSap::TxProbability::p25;
    }
    else if  (theta == 50)
    {
      txProb.probability = cv2x_LteRrcSap::TxProbability::p50;
    }
    else if (theta == 75)
    {
      txProb.probability = cv2x_LteRrcSap::TxProbability::p75;
    }
    else // if(theta == 100)
    {
      txProb.probability = cv2x_LteRrcSap::TxProbability::p100;
    }
    m_txProbability = txProb;
  }

}
