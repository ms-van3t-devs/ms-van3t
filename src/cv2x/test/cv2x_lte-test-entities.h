/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
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
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#ifndef CV2X_LTE_TEST_ENTITIES_H
#define CV2X_LTE_TEST_ENTITIES_H

#include "ns3/simulator.h"
#include "ns3/test.h"

#include "ns3/cv2x_lte-mac-sap.h"
#include "ns3/cv2x_lte-rlc-sap.h"
#include "ns3/cv2x_lte-pdcp-sap.h"

#include "ns3/net-device.h"
#include <ns3/cv2x_epc-enb-s1-sap.h>

namespace ns3 {

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief This class implements a testing RRC entity
 */
class cv2x_LteTestRrc : public Object
{
    /// allow cv2x_LtePdcpSpecificLtePdcpSapUser<cv2x_LteTestRrc> class friend access
    friend class cv2x_LtePdcpSpecificLtePdcpSapUser<cv2x_LteTestRrc>;
//   friend class cv2x_EnbMacMemberLteEnbCmacSapProvider;
//   friend class cv2x_EnbMacMemberLteMacSapProvider<cv2x_LteTestMac>;
//   friend class cv2x_EnbMacMemberFfMacSchedSapUser;
//   friend class cv2x_EnbMacMemberFfMacCschedSapUser;
//   friend class cv2x_EnbMacMemberLteEnbPhySapUser;

  public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
    static TypeId GetTypeId (void);

    cv2x_LteTestRrc (void);
    virtual ~cv2x_LteTestRrc (void);
    virtual void DoDispose (void);


    /**
    * \brief Set the PDCP SAP provider
    * \param s a pointer to the PDCP SAP provider
    */
    void SetLtePdcpSapProvider (cv2x_LtePdcpSapProvider* s);
    /**
    * \brief Get the PDCP SAP user
    * \return a pointer to the SAP user of the RLC
    */
    cv2x_LtePdcpSapUser* GetLtePdcpSapUser (void);

    /// Start function
    void Start ();
    /// Stop function
    void Stop ();

    /**
    * \brief Send data function
    * \param at the time to send
    * \param dataToSend the data to send
    */
    void SendData (Time at, std::string dataToSend);
    /**
    * \brief Get data received function
    * \returns the received data string
    */
    std::string GetDataReceived (void);

    // Stats
    /**
    * \brief Get the transmit PDUs
    * \return the number of transmit PDUS
    */
    uint32_t GetTxPdus (void);
    /**
    * \brief Get the transmit bytes
    * \return the number of bytes transmitted
    */
    uint32_t GetTxBytes (void);
    /**
    * \brief Get the receive PDUs
    * \return the number of receive PDUS
    */
    uint32_t GetRxPdus (void);
    /**
    * \brief Get the receive bytes
    * \return the number of bytes received
    */
    uint32_t GetRxBytes (void);

    /**
    * \brief Get the last transmit time
    * \return the time of the last transmit
    */
    Time GetTxLastTime (void);
    /**
    * \brief Get the last receive time
    * \return the time of the last receive
    */
    Time GetRxLastTime (void);

    /**
    * \brief Set the arrival time
    * \param arrivalTime the arrival time
    */
    void SetArrivalTime (Time arrivalTime);
    /**
    * \brief Set the PDU size
    * \param pduSize the PDU size
    */
    void SetPduSize (uint32_t pduSize);

    /**
    * \brief Set the device
    * \param device the device
    */
    void SetDevice (Ptr<NetDevice> device);

  private:
    /**
     * Interface forwarded by cv2x_LtePdcpSapUser
     * \param params the cv2x_LtePdcpSapUser::ReceivePdcpSduParameters
     */
    virtual void DoReceivePdcpSdu (cv2x_LtePdcpSapUser::ReceivePdcpSduParameters params);

    cv2x_LtePdcpSapUser* m_pdcpSapUser; ///< PDCP SAP user
    cv2x_LtePdcpSapProvider* m_pdcpSapProvider; ///< PDCP SAP provider

    std::string m_receivedData; ///< the received data

    uint32_t m_txPdus; ///< number of transmit PDUs
    uint32_t m_txBytes; ///< number of transmit bytes
    uint32_t m_rxPdus; ///< number of receive PDUs
    uint32_t m_rxBytes; ///< number of receive bytes
    Time     m_txLastTime; ///< last transmit time
    Time     m_rxLastTime; ///< last reeive time

    EventId m_nextPdu; ///< next PDU event
    Time m_arrivalTime; ///< next arrival time
    uint32_t m_pduSize; ///< PDU size

    Ptr<NetDevice> m_device; ///< the device
};

/////////////////////////////////////////////////////////////////////

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief This class implements a testing PDCP entity
 */
class cv2x_LteTestPdcp : public Object
{
  /// allow cv2x_LteRlcSpecificLteRlcSapUser<cv2x_LteTestPdcp> class friend access
  friend class cv2x_LteRlcSpecificLteRlcSapUser<cv2x_LteTestPdcp>;
  
  public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
    static TypeId GetTypeId (void);

    cv2x_LteTestPdcp (void);
    virtual ~cv2x_LteTestPdcp (void);
    virtual void DoDispose (void);


    /**
    * \brief Set the RLC SAP provider
    * \param s a pointer to the RLC SAP provider
    */
    void SetLteRlcSapProvider (cv2x_LteRlcSapProvider* s);
    /**
    * \brief Get the RLC SAP user
    * \return a pointer to the SAP user of the RLC
    */
    cv2x_LteRlcSapUser* GetLteRlcSapUser (void);

    /// Start function
    void Start ();

    /**
    * \brief Send data function
    * \param time the time to send
    * \param dataToSend the data to send
    */
    void SendData (Time time, std::string dataToSend);
    /**
    * \brief Get data received function
    * \returns the received data string
    */
    std::string GetDataReceived (void);

  private:
    /**
     * Interface forwarded by cv2x_LteRlcSapUser
     * \param p the PDCP PDU packet received
     */
    virtual void DoReceivePdcpPdu (Ptr<Packet> p);

    cv2x_LteRlcSapUser* m_rlcSapUser; ///< RLC SAP user
    cv2x_LteRlcSapProvider* m_rlcSapProvider; ///< RLC SAP provider

    std::string m_receivedData; ///< the received data 
};

/////////////////////////////////////////////////////////////////////

/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief This class implements a testing loopback MAC layer
 */
class cv2x_LteTestMac : public Object
{
//   friend class cv2x_EnbMacMemberLteEnbCmacSapProvider;
    /// allow cv2x_EnbMacMemberLteMacSapProvider<cv2x_LteTestMac> class friend access
    friend class cv2x_EnbMacMemberLteMacSapProvider<cv2x_LteTestMac>;
//   friend class cv2x_EnbMacMemberFfMacSchedSapUser;
//   friend class cv2x_EnbMacMemberFfMacCschedSapUser;
//   friend class cv2x_EnbMacMemberLteEnbPhySapUser;

  public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
    static TypeId GetTypeId (void);

    cv2x_LteTestMac (void);
    virtual ~cv2x_LteTestMac (void);
    virtual void DoDispose (void);

    /**
    * \brief Set the device function
    * \param device the device
    */
    void SetDevice (Ptr<NetDevice> device);

    /**
    * \brief Send transmit opportunity function
    * \param time the time
    * \param bytes the number of bytes
    */
    void SendTxOpportunity (Time time, uint32_t bytes);
    /**
    * \brief Get data received function
    * \returns the received data string
    */
    std::string GetDataReceived (void);

    /**
    * \brief the Receive function
    * \param nd the device
    * \param p the packet
    * \param protocol the protocol
    * \param addr the address
    * \returns true if successful
    */
    bool Receive (Ptr<NetDevice> nd, Ptr<const Packet> p, uint16_t protocol, const Address& addr);

    /**
     * \brief Set the MAC SAP user
     * \param s a pointer to the MAC SAP user
     */
    void SetLteMacSapUser (cv2x_LteMacSapUser* s);
    /**
     * \brief Get the MAC SAP provider
     * \return a pointer to the SAP provider of the MAC
     */
    cv2x_LteMacSapProvider* GetLteMacSapProvider (void);

    /**
     * \brief Set the other side of the MAC Loopback
     * \param s a pointer to the other side of the MAC loopback
     */
    void SetLteMacLoopback (Ptr<cv2x_LteTestMac> s);

    /**
     * \brief Set PDCP header present function
     * \param present true iif PDCP header present
     */
    void SetPdcpHeaderPresent (bool present);

    /**
     * \brief Set RLC header type
     * \param rlcHeaderType the RLC header type
     */
    void SetRlcHeaderType (uint8_t rlcHeaderType);

    /// RCL Header Type enumeration
    typedef enum {
      UM_RLC_HEADER = 0,
      AM_RLC_HEADER = 1,
    } RlcHeaderType_t; ///< the RLC header type

    /**
     * Set transmit opportunity mode
     * \param mode the transmit opportunity mode
     */
    void SetTxOpportunityMode (uint8_t mode);

    /// Transmit opportunity mode enumeration
    typedef enum {
      MANUAL_MODE     = 0,
      AUTOMATIC_MODE  = 1,
      RANDOM_MODE     = 2
    } TxOpportunityMode_t; ///< transmit opportunity mode

    /**
     * Set transmit opportunity time
     * \param txOppTime the transmit opportunity time
     */
    void SetTxOppTime (Time txOppTime);
    /**
     * Set transmit opportunity time
     * \param txOppSize the transmit opportunity size
     */
    void SetTxOppSize (uint32_t txOppSize);

    // Stats
    /**
    * \brief Get the transmit PDUs
    * \return the number of transmit PDUS
    */
    uint32_t GetTxPdus (void);
    /**
    * \brief Get the transmit bytes
    * \return the number of bytes transmitted
    */
    uint32_t GetTxBytes (void);
    /**
    * \brief Get the receive PDUs
    * \return the number of receive PDUS
    */
    uint32_t GetRxPdus (void);
    /**
    * \brief Get the receive bytes
    * \return the number of bytes received
    */
    uint32_t GetRxBytes (void);

  private:
    // forwarded from cv2x_LteMacSapProvider
    /**
     * Transmit PDU
     * \param params cv2x_LteMacSapProvider::TransmitPduParameters
     */
    void DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params);
    /**
     * Report buffer status function
     * \param params cv2x_LteMacSapProvider::ReportBufferStatusParameters
     */
    void DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params);

    cv2x_LteMacSapProvider* m_macSapProvider; ///< MAC SAP provider
    cv2x_LteMacSapUser* m_macSapUser; ///< MAC SAP user
    Ptr<cv2x_LteTestMac> m_macLoopback; ///< MAC loopback

    std::string m_receivedData; ///< the received data string

    uint8_t m_rlcHeaderType; ///< RLC header type
    bool m_pdcpHeaderPresent; ///< PDCP header present?
    uint8_t m_txOpportunityMode; ///< transmit opportunity mode

    Ptr<NetDevice> m_device; ///< the device

    // TxOpportunity configuration
    EventId m_nextTxOpp; ///< next transmit opportunity event
    Time m_txOppTime; ///< transmit opportunity time
    uint32_t m_txOppSize; ///< transmit opportunity size
    std::list<EventId> m_nextTxOppList; ///< next transmit opportunity list

    // Stats
    uint32_t m_txPdus; ///< the number of transmit PDUs
    uint32_t m_txBytes; ///< the number of transmit bytes
    uint32_t m_rxPdus; ///< the number of receive PDUs
    uint32_t m_rxBytes; ///< the number of receive bytes

};



/**
 * \ingroup lte-test
 * \ingroup tests
 *
 * \brief RRC stub providing a testing S1 SAP user to be used with the cv2x_EpcEnbApplication
 */
class cv2x_EpcTestRrc : public Object
{
  /// allow cv2x_MemberEpcEnbS1SapUser<cv2x_EpcTestRrc> class friend access
  friend class cv2x_MemberEpcEnbS1SapUser<cv2x_EpcTestRrc>;

public:
  cv2x_EpcTestRrc ();
  virtual ~cv2x_EpcTestRrc ();

  // inherited from Object
  virtual void DoDispose (void);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /** 
   * Set the S1 SAP Provider
   * 
   * \param s the S1 SAP Provider
   */
  void SetS1SapProvider (cv2x_EpcEnbS1SapProvider* s);

  /** 
   * 
   * \return the S1 SAP user
   */
  cv2x_EpcEnbS1SapUser* GetS1SapUser ();

private:

  // S1 SAP methods
  /**
   * Data radio bearer setup request
   * \param params cv2x_EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters
   */
  void DoDataRadioBearerSetupRequest (cv2x_EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params);
  /**
   * Path switch request acknowledge function
   * \param params cv2x_EpcEnbS1SapUser::PathSwitchRequestAcknowledgeParameters
   */
  void DoPathSwitchRequestAcknowledge (cv2x_EpcEnbS1SapUser::PathSwitchRequestAcknowledgeParameters params);  
  
  cv2x_EpcEnbS1SapProvider* m_s1SapProvider; ///< S1 SAP provider
  cv2x_EpcEnbS1SapUser* m_s1SapUser; ///< S1 SAP user
  

};


} // namespace ns3

#endif /* LTE_TEST_MAC_H */
