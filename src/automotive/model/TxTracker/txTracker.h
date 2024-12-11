//
// Created by diego on 20/11/24.
//

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
#include "ns3/node.h"
#include "ns3/config.h"
#include "ns3/sionna_handler.h"

namespace ns3 {

/**
 * \ingroup automotive
 *
 * \brief This module implements the Tracker for nodes that use the channel at a certain moment
 *
 * This module provides capabilities for tracking the nodes that are using the channel at a certain moment
 */

enum TxType {
  WIFI,
  NR
};

typedef struct txParameters11p{
  uint8_t nodeID;
  Ptr<WifiNetDevice> netDevice;
  double bandwidth;
  double txPower_W;
} txParameters11p;

typedef struct txParametersNR {
  uint8_t nodeID;
  Ptr<NrUeNetDevice> netDevice;
  double rbBandwidth;
} txParametersNR;

extern std::unordered_map<std::string, txParameters11p> m_txMap11p;
extern std::unordered_map<std::string, txParametersNR> m_txMapNr;

void Insert11pNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<WifiNetDevice>>> nodes);
void InsertNrNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<NrUeNetDevice>>> nodes);

}

#endif //NS3_TXTRACKER_H
