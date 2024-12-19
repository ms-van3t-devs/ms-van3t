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
 *
 * Created by:
 *   Diego Gasco, Politecnico di Torino (diego.gasco@polito.it, diego.gasco99@gmail.com)
 */

#include "txTracker.h" // Include the header file defining the TxTracker class and related methods

namespace ns3 {

// Define the logging component for this module
NS_LOG_COMPONENT_DEFINE ("TxTracker");

// Helper function to convert dBm to Watts
double DbmToW (double dbm)
{
  return std::pow(10, (dbm - 30) / 10);
}

// Helper function to convert Watts to dBm
double WToDbm (double w)
{
  return 10 * std::log10(w) + 30;
}

// Insert 802.11p nodes into the tracker
void
TxTracker::Insert11pNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<WifiNetDevice>>> nodes)
{
  for (auto n : nodes)
    {
      // Extract vehicle ID, node ID, and network device from the tuple
      std::string vehID = std::get<0>(n);
      uint8_t nodeID = std::get<1>(n);
      Ptr<WifiNetDevice> netDevice = std::get<2>(n);

      // Ensure the network device is valid
      NS_ASSERT_MSG (netDevice != nullptr, "Node is nullptr");

      // Get the physical layer (PHY) of the Wi-Fi device
      Ptr<WifiPhy> wifiPhy = netDevice->GetPhy();

      // Store the node's transmission parameters in the tracker
      m_txMap11p[vehID] = txParameters11p {
          nodeID,
          netDevice,
          std::pow(10, (wifiPhy->GetTxPowerStart() - 30) / 10), // Convert transmission power from dBm to Watts
      };
    }
}

// Insert NR (New Radio) nodes into the tracker
void
TxTracker::InsertNrNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<NrUeNetDevice>>> nodes)
{
  for (auto n : nodes)
    {
      // Extract vehicle ID, node ID, and network device from the tuple
      std::string vehID = std::get<0>(n);
      uint8_t nodeID = std::get<1>(n);
      Ptr<NrUeNetDevice> netDevice = std::get<2>(n);

      // Ensure the network device is valid
      NS_ASSERT_MSG (netDevice != nullptr, "Node is nullptr");

      // Get the physical layer (PHY) of the NR device and calculate RB (Resource Block) bandwidth
      Ptr<NrUePhy> uePhy = netDevice->GetPhy (0);
      uint32_t bandwidth = uePhy->GetChannelBandwidth();
      double rbBand = bandwidth / uePhy->GetRbNum();

      // Store the node's transmission parameters in the tracker
      m_txMapNr[vehID] = txParametersNR {
          nodeID,
          netDevice,
          rbBand,
      };
    }
}

// Add interference from 80211p signals to NR signals
std::unordered_map<std::string, std::pair<Ptr<SpectrumValue>, Time>>
TxTracker::AddInterferenceToNr (Ptr<SpectrumValue> nrSignal, Ptr<SpectrumValue> wifiSignal, Ptr<MobilityModel> receiverMobility, Time delay, Ptr<PropagationLossModel> propagationLoss)
{
  std::unordered_map<std::string, std::pair<Ptr<SpectrumValue>, Time>> interferenceNodes;

  for (auto it = m_txMap11p.begin(); it != m_txMap11p.end(); ++it)
    {
      Ptr<WifiPhy> wifiPhy = it->second.netDevice->GetPhy();
      WifiPhyState current_state = wifiPhy->GetState()->GetState();

      // Process only if the Wi-Fi PHY is transmitting
      if (current_state == WifiPhyState::TX)
        {
          // Check for frequency overlap between NR and Wi-Fi signals
          double interferenceCentralFreq = centralFrequency11p;
          double interferenceBandwidth = bandWidth11p;
          bool overlap = (interferenceCentralFreq - interferenceBandwidth / 2 <= centralFrequencyNr + bandWidthNr / 2 &&
                          interferenceCentralFreq + interferenceBandwidth / 2 >= centralFrequencyNr - bandWidthNr / 2);
          if (!overlap)
            {
              continue;
            }

          double rbBandwidth = m_txMapNr.begin()->second.rbBandwidth;
          Ptr<MobilityModel> interferenceMobility = it->second.netDevice->GetNode()->GetObject<ConstantPositionMobilityModel>();

          // Calculate interference delay and received power
          Time interferenceDelay = delay;
          double finalInterferencePowerDbm = propagationLoss->CalcRxPower(10 * log10(it->second.txPower_W) + 30, interferenceMobility, receiverMobility);
          double finalInterferencePowerW = std::pow(10, (finalInterferencePowerDbm - 30) / 10);

          // Iterate over the NR signal's resource blocks to distribute interference
          uint8_t j = 1;
          std::vector<uint8_t> indexesRB;
          double wifiStartFreq = centralFrequency11p - bandWidth11p / 2;
          double wifiEndFreq = centralFrequency11p + bandWidth11p / 2;
          double nrStartFreq = centralFrequencyNr - bandWidthNr / 2;
          for (auto it2 = nrSignal->ValuesBegin(); it2 != nrSignal->ValuesEnd(); ++it2)
            {
              double subBandFreq = nrStartFreq + j * rbBandwidth;
              if (subBandFreq >= wifiStartFreq && subBandFreq <= wifiEndFreq)
                {
                  (*wifiSignal)[j-1] = finalInterferencePowerW / rbBandwidth;
                }
              j += 1;
            }

          // Add the interference node to the result map
          interferenceNodes.insert({ it->first, std::make_pair (wifiSignal, interferenceDelay)});
        }
    }
  return interferenceNodes;
}

// Add interference from NR signals to 80211p signals
std::unordered_map<std::string, std::pair<RxPowerWattPerChannelBand, Time>>
TxTracker::AddInterferenceTo11p (Ptr<YansWifiPhy> sender, Ptr<MobilityModel> receiverMobility, Ptr<PropagationLossModel> propagationLoss, Ptr<PropagationDelayModel> propagationDelay)
{
  std::unordered_map<std::string, std::pair<RxPowerWattPerChannelBand, Time>> noisePowerPerNode;
  double interferencePower = 0.0;

  for (auto it = m_txMapNr.begin(); it != m_txMapNr.end(); ++it)
    {
      Ptr<NrUePhy> nrPhy = it->second.netDevice->GetPhy(0);
      NrSpectrumPhy::State current_state = nrPhy->GetSpectrumPhy ()->GetState();

      // Process only if the NR PHY is transmitting
      if (current_state == NrSpectrumPhy::State::TX)
        {
          Ptr<SpectrumValue> spectrum = nrPhy->GetSpectrumPhy ()->GetTxPowerSpectralDensity();
          double rbBandwidth = m_txMapNr.begin()->second.rbBandwidth;
          uint8_t j = 1;
          double wifiStartFreq = centralFrequency11p - bandWidth11p / 2;
          double wifiEndFreq = centralFrequency11p + bandWidth11p / 2;
          double nrStartFreq = centralFrequencyNr - bandWidthNr / 2;
          double interferenceCentralFreq = centralFrequencyNr;
          double interferenceBandwidth = bandWidthNr;

          bool overlap = (interferenceCentralFreq - interferenceBandwidth / 2 <= centralFrequency11p + bandWidth11p / 2 &&
                          interferenceCentralFreq + interferenceBandwidth / 2 >= centralFrequency11p - bandWidth11p / 2);
          if (!overlap)
            {
              continue;
            }

          // Calculate interference for overlapping frequency bands
          for (auto it2 = spectrum->ValuesBegin(); it2 != spectrum->ValuesEnd(); ++it2)
            {
              double subBandfreq = nrStartFreq + j * rbBandwidth;
              if ((*it2) > 0 && subBandfreq >= wifiStartFreq && subBandfreq <= wifiEndFreq)
                {
                  interferencePower += (*it2) * rbBandwidth;
                }
              j += 1;
            }

          if (interferencePower > 0.0)
            {
              Ptr<MobilityModel> interferenceMobility = it->second.netDevice->GetNode()->GetObject<ConstantPositionMobilityModel>();
              Time interferenceDelay = propagationDelay->GetDelay (interferenceMobility, receiverMobility);

              // Calculate received interference power and store it in the map
              double interferenceRxPowerDbm = propagationLoss->CalcRxPower(WToDbm (interferencePower), interferenceMobility, receiverMobility);
              RxPowerWattPerChannelBand rxInterference = RxPowerWattPerChannelBand ();
              rxInterference.insert({std::make_pair (0, 0), DbmToW (interferenceRxPowerDbm)});
              noisePowerPerNode[it->first] = std::make_pair (rxInterference, interferenceDelay);
            }
        }
    }
  return noisePowerPerNode;
}

} // namespace ns3
