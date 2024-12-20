#ifndef NS3_TXTRACKER_H
#define NS3_TXTRACKER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "ns3/epc-helper.h"
#include "ns3/nstime.h"
#include "ns3/wifi-phy-state.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-net-device.h"
#include "ns3/callback.h"
#include "ns3/simulator.h"
#include "ns3/nr-spectrum-phy.h"
#include "ns3/net-device.h"
#include "ns3/ptr.h"
#include "ns3/nr-ue-phy.h"
#include "ns3/nr-ue-net-device.h"
#include "ns3/cv2x_lte-ue-net-device.h"
#include "ns3/cv2x_lte-ue-phy.h"
#include "ns3/cv2x_lte-spectrum-phy.h"
#include "ns3/node.h"
#include "ns3/config.h"
#include "ns3/sionna_handler.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/yans-wifi-phy.h"

namespace ns3 {

  /**
 * \ingroup automotive
 *
 * \brief This module implements the Tracker for nodes that use the channel at a certain moment
 *
 * This module provides capabilities for tracking the nodes that are using the channel at a certain moment
 */
  class TxTracker
  {
  public:
    // Singleton pattern to get the instance of TxTracker
    static TxTracker &
    GetInstance ()
    {
      static TxTracker instance;
      return instance;
    }

    // Enumeration for transmission types
    enum TxType { WIFI, NR };

    // Structure to hold 11p transmission parameters
    typedef struct txParameters11p
    {
      uint8_t nodeID; // Node ID
      Ptr<WifiNetDevice> netDevice; // Pointer to the WifiNetDevice
      double txPower_W; // Transmission power in watts
    } txParameters11p;

    // Structure to hold NR transmission parameters
    typedef struct txParametersNR
    {
      uint8_t nodeID; // Node ID
      Ptr<NrUeNetDevice> netDevice; // Pointer to the NrUeNetDevice
      double rbBandwidth; // Resource block bandwidth
    } txParametersNR;

    // Structure to hold LTE transmission parameters
    typedef struct txParametersLTE
    {
      uint8_t nodeID; // Node ID
      Ptr<cv2x_LteUeNetDevice> netDevice; // Pointer to the NrUeNetDevice
      double rbBandwidth; // Resource block bandwidth
    } txParametersLTE;

    // Method to insert 11p nodes into the tracker
    void Insert11pNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<WifiNetDevice>>> nodes);
  
    // Method to insert NR nodes into the tracker
    void InsertNrNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<NrUeNetDevice>>> nodes);

    // Method to insert LTE nodes into the tracker
    void InsertLteNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<cv2x_LteUeNetDevice>>> nodes);
  
    // Method to set the central frequencies for 11p, NR, and LTE
    void
    SetCentralFrequencies (double frequency11p_Hz = 0.0, double frequencyNr_Hz = 0.0, double frequencyLte_Hz = 0.0)
    {
      centralFrequency11p = frequency11p_Hz;
      centralFrequencyNr = frequencyNr_Hz;
      centralFrequencyLte = frequencyLte_Hz;
    };
  
    // Method to set the bandwidths for 11p, NR, and LTE
    void
    SetBandwidths (double band11p_Hz = 0.0, double bandNr_Hz = 0.0, double bandLte_Hz = 0.0)
    {
      bandWidth11p = band11p_Hz;
      bandWidthNr = bandNr_Hz;
      bandWidthLte = bandLte_Hz;
    };
  
    // Method to add interference for NR signals
    std::unordered_map<std::string, std::pair<Ptr<SpectrumValue>, Time>>
    AddInterferenceToCV2X (Ptr<SpectrumValue> nrSignal, Ptr<SpectrumValue> wifiSignal, Ptr<MobilityModel> receiverMobility, Time delay,
                       Ptr<PropagationLossModel> propagationLoss);
  
    // Method to add interference for 11p signals
    std::unordered_map<std::string, std::pair<RxPowerWattPerChannelBand, Time>>
    AddInterferenceTo11p (Ptr<YansWifiPhy> sender, Ptr<MobilityModel> receiverMobility,
                        Ptr<PropagationLossModel> propagationLoss,
                        Ptr<PropagationDelayModel> propagationDelay);

  private:
    // Private constructor for singleton pattern
    TxTracker() = default;
  
    // Private destructor
    ~TxTracker() = default;
  
    // Delete copy constructor
    TxTracker(const TxTracker&) = delete;
  
    // Delete assignment operator
    TxTracker& operator=(const TxTracker&) = delete;

    // Map to store 11p transmission parameters
    std::unordered_map<std::string, txParameters11p> m_txMap11p;
  
    // Map to store NR transmission parameters
    std::unordered_map<std::string, txParametersNR> m_txMapNr;

    // Map to store LTE transmission parameters
    std::unordered_map<std::string, txParametersLTE> m_txMapLte;
  
    // Central frequency for 11p
    double centralFrequency11p;
  
    // Central frequency for NR
    double centralFrequencyNr;

    // Central frequency for Lte
    double centralFrequencyLte;
  
    // Bandwidth for 11p
    double bandWidth11p;
  
    // Bandwidth for NR
    double bandWidthNr;

    // Bandwidth for LTE
    double bandWidthLte;
  };
}

#endif //NS3_TXTRACKER_H