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

#ifndef CV2X_SL_POOL_H
#define CV2X_SL_POOL_H

#include <map>
#include "cv2x_lte-rrc-sap.h"

namespace ns3 {
    
  /*
   * Class describing a resource pool for sidelink communication
   */
  class SidelinkCommResourcePool : public Object
  {
  public:
    /** types of sidelink pool */
    enum SlPoolType
    {
      UNKNOWN, 
      SCHEDULED,
      UE_SELECTED
    };

    /** Identify the location of a subframe by its frame number and subframe number */
    struct SubframeInfo {
      uint32_t frameNo; //!<The frame number
      uint32_t subframeNo; //!<The subframe number

      /**
       * Adds two subframe locations and return the new location
       * This is used for computing the absolute subframe location from a starting point
       * \param lhs One of the subframe location
       * \param rhs The other subframe location
       * \return The new subframe location
       */
      friend SubframeInfo operator+(const SubframeInfo& lhs, const SubframeInfo& rhs)
      {
        SubframeInfo res;
        uint32_t tmp1 = 10 * (lhs.frameNo % 1024) + lhs.subframeNo % 10;
        tmp1 += 10 * (rhs.frameNo % 1024) + rhs.subframeNo % 10;
        res.frameNo = tmp1 / 10;
        res.subframeNo = tmp1 % 10;
        return res;
      }
      
      /**
       * Checks if two subframe locations are identical
       * \param lhs One of the subframe location
       * \param rhs The other subframe location
       * \return true if the locations represent the same subframe
       */
      friend bool operator==(const SubframeInfo& lhs, const SubframeInfo& rhs)
      {
        return lhs.frameNo == rhs.frameNo && lhs.subframeNo == rhs.subframeNo;
      }

      /**
       * Checks if a subframe location is smaller that another subframe location
       * \param lhs One of the subframe location
       * \param rhs The other subframe location
       * \return true if the first argument represent a subframe that will happen sooner
       */
      friend bool operator<(const SubframeInfo& lhs, const SubframeInfo& rhs)
      {
        return lhs.frameNo < rhs.frameNo || (lhs.frameNo == rhs.frameNo && lhs.subframeNo < rhs.subframeNo);
      }
    };

    /** Indicates the location of a transmission on the sidelink in time and frequency **/
    struct SidelinkTransmissionInfo {
      SubframeInfo subframe; //!<The time of the transmission
      uint16_t rbStart; //!<The index of the PRB where the transmission occurs
      uint8_t nbRb; //!<The number of PRBs used by the transmission 
    };

    SidelinkCommResourcePool (void);
    virtual ~SidelinkCommResourcePool (void);
    static TypeId GetTypeId (void);

    /**
     * Checks if two sidelink pool configurations are identical
     * \param other The configuration of the other resource pool
     * \return true if this configuration is the same as the other one
     */
    bool operator==(const SidelinkCommResourcePool& other);
    
    /**
     * Configure the pool using the content of the RRC message
     * Parsing the message will indicate whether it is a scheduled or UE selected pool
     * \param pool The message containing pool information
     */
    void SetPool (cv2x_LteRrcSap::SlCommResourcePool pool);

    /**
     * Configure the pool using the content of the preconfigured pool
     * Parsing the message will indicate whether it is a scheduled or UE selected pool
     * \param pool The message containing pool information
     */
    void SetPool (cv2x_LteRrcSap::SlPreconfigCommPool pool);

    /**
     * Returns the type of scheduling 
     * \return the type of scheduling    
     */
    SlPoolType GetSchedulingType ();

    /**
     * Determines the start of the current SC period 
     * \param frameNo the frame number
     * \param subframeNo the subframe number
     * \return The start of the current sidelink period where the given subframe is located
     */
    SubframeInfo GetCurrentScPeriod (uint32_t frameNo, uint32_t subframeNo);

    /**
     * Determines the start of the next SC period 
     * \param frameNo the frame number
     * \param subframeNo the subframe number
     * \return The start of the next sidelink period where the given subframe is located
     */
    SubframeInfo GetNextScPeriod (uint32_t frameNo, uint32_t subframeNo);

    /**
     * Returns the subframe and resource block allocations of the transmissions in 
     * PSCCH for the given resource
     * \param n the selected resource within the pool
     * \return The list of transmission information
     */
    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> GetPscchTransmissions (uint32_t n);

    /**
     * Returns the list RBs that contains PSCCH opportunities in the current given subframe 
     * \param frameNo frame number
     * \param subframeNo subframe number
     * \return The list of RBs used by the control channel on the given subframe
     */
    std::list<uint8_t> GetPscchOpportunities (uint32_t frameNo, uint32_t subframeNo);

    /**
     * Returns the list of RBs to be used in the given subframe for the given opportunity
     * \param frameNo frame number
     * \param subframeNo subframe number
     * \param n resource selected in the pool
     * \return The list of RBs useed by the control channel on the given subframe and resource index
     */
    std::vector<int> GetPscchRbs (uint32_t frameNo, uint32_t subframeNo, uint32_t n);
   
    /**
     * Returns the number of PSCCH resources from the pool
     * \return the number of PSCCH resources from the pool
     */
    uint32_t GetNPscch ();
    
    /**
     * Returns the subframes and RBs associated with the transmission on PSSCH in the current period
     * \param itrp the repetition pattern from the SCI format 0 message
     * \param rbStart the index of the PRB where the transmission occurs
     * \param rbLen the length of the transmission
     * \return the subframes and RBs associated with the transmission on PSSCH
     */
    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> GetPsschTransmissions (uint8_t itrp, uint16_t rbStart, uint16_t rbLen);
    
    /**
     * Returns the subframes and RBs associated with the transmission on PSSCH
     * \param periodStart The first subframe in the sidelink period
     * \param itrp the repetition pattern from the SCI format 0 message
     * \param rbStart the index of the PRB where the transmission occurs
     * \param rbLen the length of the transmission     
     * \return the subframes and RBs associated with the transmission on PSSCH
     */
    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> GetPsschTransmissions (SubframeInfo periodStart, uint8_t itrp, uint16_t rbStart, uint16_t rbLen);
  
  protected:

    void Initialize ();

    /**
     * Translates the filtered subframes to subframe numbers relative to the start
     * of the SC period
     */
    SidelinkCommResourcePool::SidelinkTransmissionInfo TranslatePscch (SidelinkCommResourcePool::SidelinkTransmissionInfo info); 
    
    SlPoolType m_type; //!<The type of pool 
    
    //Common fields for all types of pools
    cv2x_LteRrcSap::SlCpLen m_scCpLen; //!<cyclic prefix length of control channel
    cv2x_LteRrcSap::SlPeriodComm m_scPeriod; //!<duration of the sidelink period
    cv2x_LteRrcSap::SlTfResourceConfig m_scTfResourceConfig; //!<control pool information (subframes and PRBs)
    cv2x_LteRrcSap::SlCpLen m_dataCpLen; //!<cyclic prefix for the shared channel
    cv2x_LteRrcSap::SlHoppingConfigComm m_dataHoppingConfig; //!<frequency hopping parameters
    cv2x_LteRrcSap::SlTrptSubset m_trptSubset; //!<normally only used for UE selected pools because it is assumed that all subframes are available in scheduled pools

    //Fields for UE selected pools
    cv2x_LteRrcSap::SlTfResourceConfig m_dataTfResourceConfig; //!<shared channel pool information

  private:

    /**
     * Compute the frames/RBs that are part of the PSCCH pool
     */
    void ComputeNumberOfPscchResources ();
    /**
     * Compute the frame/RBS that are part of the PSSCH pool
     */
    void ComputeNumberOfPsschResources ();

    uint32_t m_lpscch;
    std::vector <uint32_t> m_lpscchVector; //list of subframes that belong to PSCCH pool
    uint32_t m_rbpscch;
    std::vector <uint32_t> m_rbpscchVector; //list of RBs that belong to PSCCH pool
    uint32_t m_nPscchResources; //number of resources in the PSCCH pools

    uint32_t m_lpssch;
    std::vector <uint32_t> m_lpsschVector; //list of subframes that belong to PSSCH pool
    uint32_t m_rbpssch;
    std::vector <uint32_t> m_rbpsschVector; //list of RBs that belong to PSSCH pool

    bool m_preconfigured; //indicates if the pool is preconfigured
  };

  /**
   * Class describing a resource pool for receiving sidelink communication
   */
  class SidelinkRxCommResourcePool :  public SidelinkCommResourcePool
  {
  public:
    static TypeId GetTypeId (void);

  protected:
    

  };

  /**
   * Class describing a resource pool for transmitting sidelink communication
   */
  class SidelinkTxCommResourcePool :  public SidelinkCommResourcePool
  {
  public:

    static TypeId GetTypeId (void);
   
    
    /**
     * Configure the pool using the content of the RRC message
     * \param pool The message containing the pool information
     */
    void SetPool (cv2x_LteRrcSap::SlCommResourcePool pool);

    /**
     * Configure the pool using the content of the preconfigured pool
     * \param pool The message containing the pool information
     */
    void SetPool (cv2x_LteRrcSap::SlPreconfigCommPool pool);
   
    /**
     * Set parameters for a scheduled pool
     * \param slrnti The sidelink RNTI assigned by the eNodeB
     * \param macMainConfig MAC layer configuration
     * \param commTxConfig The pool configuration
     * \param index associated with the resource pool 
     */
    void SetScheduledTxParameters (uint16_t slrnti, cv2x_LteRrcSap::SlMacMainConfigSl macMainConfig, cv2x_LteRrcSap::SlCommResourcePool commTxConfig, uint8_t index);

    /**
     * Set parameters for a scheduled pool
     * \param slrnti The sidelink RNTI assigned by the eNodeB
     * \param macMainConfig MAC layer configuration
     * \param commTxConfig The pool configuration
     * \param index associated with the resource pool 
     * \param mcs The MCS to be used for transmitting data
     */
    void SetScheduledTxParameters (uint16_t slrnti, cv2x_LteRrcSap::SlMacMainConfigSl macMainConfig, cv2x_LteRrcSap::SlCommResourcePool commTxConfig, uint8_t index, uint8_t mcs);

    /**
     * Returns the index of the resource pool
     * \return the index of the resource pool
     */
    uint8_t GetIndex ();

    /**
     * Returns the MCS to use
     * \return The MCS to use
     */
    uint8_t GetMcs ();


    
  protected:
    
    //Fields for UE selected pools
    cv2x_LteRrcSap::SlTxParameters m_scTxParameters; //!<configuration of the control channel
    cv2x_LteRrcSap::SlTxParameters m_dataTxParameters; //!<configuration of the shared channel

    //Fields for scheduled pools
    uint16_t m_slrnti; //!<sidelink RNTI 
    cv2x_LteRrcSap::SlMacMainConfigSl m_macMainConfig; //!<MAC layer configuration
    cv2x_LteRrcSap::SlCommResourcePool m_commTxConfig; //!<resource pool configuration
    bool m_haveMcs; //!<indicates if MCS is being set
    uint8_t m_mcs; //!<the MCS value to use if set (0..28)
    uint8_t m_index; //!<index to be used in BSR
    
  };
  
  
  /**
   * Class describing a resource pool for sidelink V2X communication 
   */
  class SidelinkCommResourcePoolV2x : public Object 
  {
    public:

  public:
    /** types of sidelink pool */
    enum SlPoolType
    {
      UNKNOWN, 
      SCHEDULED,
      UE_SELECTED
    };

    /** Identify the location of a subframe by its frame number and subframe number */
    struct SubframeInfo {
      uint32_t frameNo; //!<The frame number
      uint32_t subframeNo; //!<The subframe number

    /**
     * Adds two subframe locations and return the new location
     * This is used for computing the absolute subframe location from a starting point
     * \param lhs One of the subframe location
     * \param rhs The other subframe location
     * \return The new subframe location
     */
    friend SubframeInfo operator+(const SubframeInfo& lhs, const SubframeInfo& rhs)
    {
      SubframeInfo res;
      uint32_t tmp1 = 10 * (lhs.frameNo % 1024) + lhs.subframeNo % 10;
      tmp1 += 10 * (rhs.frameNo % 1024) + rhs.subframeNo % 10;
      res.frameNo = tmp1 / 10;
      res.subframeNo = tmp1 % 10;
      return res;
    }
    
    /**
     * Checks if two subframe locations are identical
     * \param lhs One of the subframe location
     * \param rhs The other subframe location
     * \return true if the locations represent the same subframe
     */
    friend bool operator==(const SubframeInfo& lhs, const SubframeInfo& rhs)
    {
      return lhs.frameNo == rhs.frameNo && lhs.subframeNo == rhs.subframeNo;
    }

    /**
     * Checks if a subframe location is smaller that another subframe location
     * \param lhs One of the subframe location
     * \param rhs The other subframe location
     * \return true if the first argument represent a subframe that will happen sooner
     */
    friend bool operator<(const SubframeInfo& lhs, const SubframeInfo& rhs)
    {
      return lhs.frameNo < rhs.frameNo || (lhs.frameNo == rhs.frameNo && lhs.subframeNo < rhs.subframeNo);
    }
  };

    /** Indicates the location of a transmission on the sidelink in time and frequency **/
    struct SidelinkTransmissionInfo{
      SubframeInfo subframe;  //!< The index of the subframe where the transmission occur 
      uint16_t rbStart; //!<The index of the RBs where the transmission occurs
      uint16_t rbLen; //!<The number of RBs used by the transmission 
    };

    SidelinkCommResourcePoolV2x (void);
    virtual ~SidelinkCommResourcePoolV2x (void);
    static TypeId GetTypeId (void);

    /**
     * Checks if two sidelink pool configurations are identical
     * \param other The configuration of the other resource pool
     * \return true if this configuration is the same as the other one
     */
    bool operator==(const SidelinkCommResourcePoolV2x& other);

    /**
     * Configure the pool using the content of the RRC message
     * \param pool The message containing the pool information
     */
    void SetPool (cv2x_LteRrcSap::SlCommResourcePoolV2x pool);

    /**
     * Configure the pool using the content of the preconfigured pool
     * \param pool The message containing the pool information
     */
    void SetPool (cv2x_LteRrcSap::SlV2xPreconfigCommPool pool);    

    /**
     * Returns the type of scheduling 
     * \return the type of scheduling    
     */
    SlPoolType GetSchedulingType ();

    /**
     * Returns the candidate resources for SPS
     * \param startSelectionWindow The actual subframe
     * \param t1 T1 value for defining the selection window
     * \param t2 T2 value for defining the selection window
     * \param subchLen The length of allocated subchannels for transmission
     * \return the candidate resources for SPS 
     */
    std::list<SidelinkTransmissionInfo> GetCandidateResources (SubframeInfo startSelectionWindow, uint16_t t1, uint16_t t2, uint16_t subchLen);

     /**
     * Returns the subframes and RBs associated with the transmission on PSSCH
     * \param subframe The actual subframe
     * \param riv The resource indication value 
     * \param pRsvp The resource reservation interval 
     * \param sfGap The gap in subframe domain between first and optional second transmission
     * \param reTxIdx The retransmission Index
     * \param pscchResource The resource of the control data 
     * \param reselectionctr The reselection counter
     * \return the subframes and RBs associated with the transmission on PSSCH
     */
    std::list<SidelinkTransmissionInfo> GetPsschTransmissions (SubframeInfo subframe, uint8_t riv, uint16_t pRsvp, uint8_t sfGap, uint8_t reTxIdx, uint8_t pscchResource, uint8_t reselCtr);
    
     /**
     * Returns the subframes and RBs associated with the transmission on PSSCH
     * \param riv The resource indication value 
     * \param pscchResource The resource of the control data 
     * \return the subframes and RBs associated with the transmission on PSSCH
     */    
    std::list<SidelinkTransmissionInfo> GetPsschTransmissions (uint8_t riv, uint8_t pscchResource);

    /**
     * Returns the subframes and RBs associated with the transmission on PSCCH
     * \param subframe The actual subframe
     * \param riv The resource indication value 
     * \param pRsvp The resource reservation interval 
     * \param sfGap The gap in subframe domain between first and optional second transmission
     * \param reTxIdx The retransmission Index
     * \param pscchResource The resource of the control data 
     * \param reselectionctr The reselection counter 
     * \return the subframes and RBs associated with the transmission on PSCCH
     */
    std::list<SidelinkTransmissionInfo> GetPscchTransmissions (SubframeInfo subframe, uint8_t riv, uint16_t pRsvp, uint8_t sfGap, uint8_t reTxIdx, uint8_t pscchResource, uint8_t reselCtr);

  protected: 

    void Initialize (); 

    SlPoolType m_type; //!<The type of pool 

    // V2X Pool Parameter 
    cv2x_LteRrcSap::SlOffsetIndicator m_slOffsetIndicator; 
    cv2x_LteRrcSap::SubframeBitmapSlV2x m_slSubframe; 
    cv2x_LteRrcSap::AdjacencyPscchPssch m_adjacencyPscchPssch; 
    cv2x_LteRrcSap::SizeSubchannel m_sizeSubchannel;
    cv2x_LteRrcSap::cv2x_NumSubchannel m_numSubchannel; 
    cv2x_LteRrcSap::cv2x_StartRbSubchannel m_startRbSubchannel; 
    cv2x_LteRrcSap::SlTxParameters m_dataTxParameters; 
    cv2x_LteRrcSap::StartRbPscchPool m_startRbPscchPool; 
    cv2x_LteRrcSap::ZoneId m_zoneId; 
    cv2x_LteRrcSap::ThreshSrssiCbr m_thresSrssiCbr; 
    cv2x_LteRrcSap::SlCbrPpppTxConfigList m_cbrPsschTxConfigList; 
    cv2x_LteRrcSap::SlP2xResourceSelectionConfig m_resourceSelectionConfigP2x; 
    cv2x_LteRrcSap::SlSyncAllowed m_syncAllowed; 
    cv2x_LteRrcSap::SlRestrictResourceReservationPeriodList m_restrictResourceReservationPeriod;

  private: 

    /**
     * Compute the frames/RBs that are part of the PSCCH pool
     */
    void ComputeNumberOfPscchResources ();
    /**
     * Compute the frame/RBS that are part of the PSSCH pool
     */
    void ComputeNumberOfPsschResources ();

    /**
     * \brief See 36.213 section 14.2.1 V15.0.0  
     */
    uint16_t ConvertResourceReservationFromBitToInt(std::bitset<4> pBits);
   
    /**
     * \brief See 36.213 section 14.1.1.4C V15.0.0  
     * \return frequency location of initial transmission and retransmission; return [startSubchannelIdx,lSubchannel]
     * \param riv(resource indication value)
     */
    uint8_t* GetValsFromRiv(uint8_t riv); 

    uint32_t m_rbpscch;
    std::vector <uint32_t> m_rbpscchVector; // list of RBs that belong to PSCCH pool
    uint32_t m_rbpssch;
    std::vector <uint32_t> m_rbpsschVector; // list of Rbs that belong to PSSCH pool

    bool m_preconfigured; // indicates if the pool is preconfigured

  }; 

  /**
   * Class describing a resource pool for receiving sidelink V2X communication
   */
  class SidelinkRxCommResourcePoolV2x : public SidelinkCommResourcePoolV2x
  {
  public:
    static TypeId GetTypeId (void);

  protected: 

  };

  /**
   * Class describing a resource pool for transmitting sidelink V2X communication
   */
  class SidelinkTxCommResourcePoolV2x : public SidelinkCommResourcePoolV2x 
  {
  public:

    static TypeId GetTypeId (void);

  /**
     * Configure the pool using the content of the RRC message
     * \param pool The message containing the pool information
     */
    void SetPool (cv2x_LteRrcSap::SlCommResourcePoolV2x pool);

    /**
     * Configure the pool using the content of the preconfigured pool
     * \param pool The message containing the pool information
     */
    void SetPool (cv2x_LteRrcSap::SlV2xPreconfigCommPool pool);
   
    /**
     * Returns the index of the resource pool
     * \return the index of the resource pool
     */
    uint8_t GetIndex ();

    /**
     * Returns the MCS to use
     * \return The MCS to use
     */
    uint8_t GetMcs ();


    
  protected:
    
    //Fields for UE selected pools
    cv2x_LteRrcSap::SlTxParameters m_dataTxParameters; //!<configuration of the shared channel

    //Fields for scheduled pools
    uint16_t m_slrnti; //!<sidelink RNTI 
    bool m_haveMcs; //!<indicates if MCS is being set
    uint8_t m_mcs; //!<the MCS value to use if set (0..28)
    uint8_t m_index; //!<index to be used in BSR

  };

  /**
   * Class describing a resource pool for sidelink discovery
   */
  class SidelinkDiscResourcePool : public Object
  {
  public:
    
    /** types of sidelink pool */
    enum SlPoolType
    {
      UNKNOWN, 
      SCHEDULED,
      UE_SELECTED
    };
    
    /** Identify the location of a subframe by its frame number and subframe number */
    struct SubframeInfo {
      uint32_t frameNo; //!<The frame number
      uint32_t subframeNo; //!<The subframe number

      /**
       * Adds two subframe locations and return the new location
       * This is used for computing the absolute subframe location from a starting point
       * \param lhs One of the subframe location
       * \param rhs The other subframe location
       * \return The new subframe location
       */
      friend SubframeInfo operator+(const SubframeInfo& lhs, const SubframeInfo& rhs)
      {
        SubframeInfo res;
        uint32_t tmp1 = 10 * (lhs.frameNo % 1024) + lhs.subframeNo % 10;
        tmp1 += 10 * (rhs.frameNo % 1024) + rhs.subframeNo % 10;
        res.frameNo = tmp1 / 10;
        res.subframeNo = tmp1 % 10;
        return res;
      }

      /**
       * Checks if two subframe locations are identical
       * \param lhs One of the subframe location
       * \param rhs The other subframe location
       * \return true if the locations represent the same subframe
       */
      friend bool operator==(const SubframeInfo& lhs, const SubframeInfo& rhs)
      {
        return lhs.frameNo == rhs.frameNo && lhs.subframeNo == rhs.subframeNo;
      }

      /**
       * Checks if a subframe location is smaller that another subframe location
       * \param lhs One of the subframe location
       * \param rhs The other subframe location
       * \return true if the first argument represent a subframe that will happen sooner
       */
      friend bool operator<(const SubframeInfo& lhs, const SubframeInfo& rhs)
      {
        return lhs.frameNo < rhs.frameNo || (lhs.frameNo == rhs.frameNo && lhs.subframeNo < rhs.subframeNo);
      }
    };

    /** indicates the location of a discovery transmission on the sidelink */
    struct SidelinkTransmissionInfo {
      SubframeInfo subframe; //!<The time of the transmission
      uint16_t rbStart; //!<The index of the PRB where the transmission occurs
      uint16_t nbRb; //!<The number of PRBs used by the transmission 
    };

    SidelinkDiscResourcePool (void);
    virtual ~SidelinkDiscResourcePool (void);
    static TypeId GetTypeId (void);
    
    /**
     * Configure the pool using the content of the RRC message
     * Parsing the message will indicate whether it is a scheduled or UE selected pool
     * \param pool discovery pool
     */
    void SetPool (cv2x_LteRrcSap::SlDiscResourcePool pool);

    /**
     * Configure the pool using the content of the preconfigured pool
     * Parsing the message will indicate 
     * \param pool preconfigured discovery pool
     */
    void SetPool (cv2x_LteRrcSap::SlPreconfigDiscPool pool);


    /**
     * Returns the type of scheduling 
     * \return the type of scheduling    
     */
    SlPoolType GetSchedulingType ();

    /**
     * Determines the start of the current discovery period 
     * \param frameNo the frame number
     * \param subframeNo the subframe number
     */
    SubframeInfo GetCurrentDiscPeriod (uint32_t frameNo, uint32_t subframeNo);

    /**
     * Determines the start of the next discovery period 
     * \param frameNo the frame number
     * \param subframeNo the subframe number
     */
    SubframeInfo GetNextDiscPeriod (uint32_t frameNo, uint32_t subframeNo);
    
    /**
     * Returns the number of PSDCH resources from the pool
     * \return the number of PSDCH resources from the pool
     */
    uint32_t GetNPsdch ();

    /**
     * Returns the number of subframes reserved to discovery
     * \return the number of subframes reserved to discovery
     */
    uint32_t GetNSubframes ();

    /**
     * Returns the number of resource block pairs
     * \return the number of resource block pairs
     */
    uint32_t GetNRbPairs ();

    /**
     * Returns the maximum number of retransmission
     * \return the maximum number of retransmission
     */
    uint8_t GetNumRetx ();

    /**
     * Returns discovery period
     * \return discovery period
     */
    int32_t GetDiscPeriod ();

    
    /**
     * Returns the subframes and RBs associated with the transmission on PSDCH
     * \param npsdch The index of the resource within the pool
     *
     */
    std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo> GetPsdchTransmissions (uint32_t npsdch);
   
    /**
     * Returns the list RBs that contains PSDCH opportunities in the current given subframe 
     * \param frameNo frame number
     * \param subframeNo subframe number
     * \return the list RBs that contains PSDCH opportunities in the current given subframe
     */
    std::list<uint8_t> GetPsdchOpportunities (uint32_t frameNo, uint32_t subframeNo);

  protected:

    void Initialize ();
   
    /**
     * Translates the filtered subframes to subframe numbers relative to the start discovery period
     */
    SidelinkDiscResourcePool::SidelinkTransmissionInfo TranslatePsdch (SidelinkDiscResourcePool::SidelinkTransmissionInfo info); 
    

    SlPoolType m_type; //!<The type of pool 
    
    //Common fields for all types of pools
    cv2x_LteRrcSap::SlCpLen m_discCpLen; //!<cyclic prefix length
    cv2x_LteRrcSap::SlPeriodDisc m_discPeriod; //!<duration of the discovery period
    uint8_t m_numRetx;  //!<number of retransmission
    uint32_t m_numRepetition; //!<number of repetition
    cv2x_LteRrcSap::SlTfResourceConfig m_discTfResourceConfig; //!<resource pool information
    bool m_haveTxParameters; //!<indicates if the Tx parameters are present or not
    cv2x_LteRrcSap::SlTxParameters m_txParametersGeneral; //!<Tx parameters to use if defined
    
  private:
    /**
     * Compute the frames/RBs that are part of the PSDCH pool
     * for ue-selected and scheduled cases
     */
    void ComputeNumberOfPsdchResources ();

    uint32_t m_lpsdch;
    std::vector <uint32_t> m_lpsdchVector; //list of subframes that belong to PSDCH pool
    uint32_t m_rbpsdch;
    std::vector <uint32_t> m_rbpsdchVector; //list of RBs that belong to PSDCH pool
    uint32_t m_nPsdchResources; //number of resources in the PSDCH pools
    bool m_preconfigured; //indicates if the pool is preconfigured
  };

  /**
   * Class describing a resource pool for receiving sidelink discovery (Monitor)
   */
  class SidelinkRxDiscResourcePool :  public SidelinkDiscResourcePool
  {
  public:
    static TypeId GetTypeId (void);

  protected:
    

  };

  /**
   * Class describing a resource pool for transmitting sidelink discovery (Announce)
   */
  class SidelinkTxDiscResourcePool :  public SidelinkDiscResourcePool
  {
  public:

    static TypeId GetTypeId (void);
   
    
    /**
     * Configure the pool 
     * \param pool discovery pool to be configured
     */
    void SetPool (cv2x_LteRrcSap::SlDiscResourcePool pool);

    /**
     * Configure the pool using the content of the preconfigured pool
     * \param pool The preconfigured pool
     */
    void SetPool (cv2x_LteRrcSap::SlPreconfigDiscPool pool);

    /**
     * Set parameters for scheduled config
     * \param discPool discovery resource pool
     * \param discResources resources in terms of resource blocks and subframes
     * \param discHopping frequency hopping configuration
     */
    void SetScheduledTxParameters (cv2x_LteRrcSap::SlDiscResourcePool discPool, cv2x_LteRrcSap::SlTfIndexPairList discResources, cv2x_LteRrcSap::SlHoppingConfigDisc discHopping);

    /**
     * Returns the transmission probability
     * \return The transmission probability
     */
    uint32_t GetTxProbability ();

    /**
     * Sets the transmission probability
     * \param theta The transmission prbability (0.25, 0.5, 0.75, or 1)
     */
    void SetTxProbability ( uint32_t theta);

  protected:
    
    //Fields for UE selected pools
    cv2x_LteRrcSap::PoolSelection m_poolSelection; //!<method for selecting the pool
    bool m_havePoolSelectionRsrpBased; //!<indicates if the pool selection is RSRP based
    cv2x_LteRrcSap::PoolSelectionRsrpBased m_poolSelectionRsrpBased; //!<parameters for the RSRP based selection
    cv2x_LteRrcSap::TxProbability m_txProbability;  //!<transmission probability
    bool m_txProbChanged; //!<indicates if the transmission probability has changed

    //Fields for scheduled pools
    cv2x_LteRrcSap::SlDiscResourcePool m_discTxConfig; //!<resource configuration
    cv2x_LteRrcSap::SlTfIndexPairList m_discTfIndexList; //!<index of the resource pool
    cv2x_LteRrcSap::SlHoppingConfigDisc m_discHoppingConfigDisc; //!<frequency hopping configuration

  };

 

} // namespace ns3

#endif //CV2X_SL_POOL_H
