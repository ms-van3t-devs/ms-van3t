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

// TODO check for the overlap conditions and do the method for 11p

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
      double rbBand = m_bandWidthNr / uePhy->GetRbNum();

      // Store the node's transmission parameters in the tracker
      m_txMapNr[vehID] = txParametersNR {
          nodeID,
          netDevice,
          rbBand
      };
    }
}

// Insert LTE nodes into the tracker
void
TxTracker::InsertLteNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<cv2x_LteUeNetDevice>>> nodes)
{
  for (auto n : nodes)
    {
      // Extract vehicle ID, node ID, and network device from the tuple
      std::string vehID = std::get<0>(n);
      uint8_t nodeID = std::get<1>(n);
      Ptr<cv2x_LteUeNetDevice> netDevice = std::get<2>(n);

      // Ensure the network device is valid
      NS_ASSERT_MSG (netDevice != nullptr, "Node is nullptr");

      // Get the physical layer (PHY) of the NR device and calculate RB (Resource Block) bandwidth
      uint8_t size = netDevice->GetPhy()->GetUlSpectrumPhy()->GetTxPowerSpectralDensity()->GetSpectrumModel()->GetNumBands();
      double rbBand = m_bandWidthLte / size;

      // Store the node's transmission parameters in the tracker
      m_txMapLte[vehID] = txParametersLTE {
          nodeID,
          netDevice,
          rbBand,
      };
    }
}

// Add interference from CV2X signals
void
TxTracker::AddInterferenceFromCV2X (Ptr<NetDevice> netDevice, Ptr<SpectrumValue> signal, Ptr<PropagationLossModel> propagationLoss, Time duration)
{
  // We need to determine the technology type of the transmitting node
  bool found = false;
  std::string technologyType;
  for (auto it = m_txMapNr.begin(); it != m_txMapNr.end(); ++it)
    {
      if (it->second.netDevice == DynamicCast<NrUeNetDevice>(netDevice))
        {
          found = true;
          technologyType = "Nr";
          break;
        }
    }
  if (!found)
    {
      for (auto it = m_txMapLte.begin(); it != m_txMapLte.end(); ++it)
        {
          if (it->second.netDevice == DynamicCast<cv2x_LteUeNetDevice>(netDevice))
            {
              found = true;
              technologyType = "Lte";
              break;
            }
        }
    }

  NS_ASSERT_MSG (found, "NetDevice not found in TxTracker.");

  std::unordered_map<std::string, std::pair<Ptr<SpectrumValue>, Time>> interferenceNodes;

  if (!m_txMap11p.empty())
    {
      // Iterate over the 80211p signals to collect the interference
      for (auto it = m_txMap11p.begin(); it != m_txMap11p.end(); ++it)
        {
          Ptr<YansWifiPhy> wifiPhy = DynamicCast<YansWifiPhy>(it->second.netDevice->GetPhy());
          // Check for frequency overlap between NR and Wi-Fi signals
          double wifiCentralFreq = m_centralFrequency11p;
          double wifiBandwidth = m_bandWidth11p;
          double c_frequency = technologyType == "Nr" ? m_centralFrequencyNr : m_centralFrequencyLte;
          double c_bandwidth = technologyType == "Nr" ? m_bandWidthNr : m_bandWidthLte;
          bool overlap = (wifiCentralFreq - wifiBandwidth / 2 <= c_frequency + c_bandwidth / 2 &&
                          wifiCentralFreq + wifiBandwidth / 2 >= c_frequency - c_bandwidth / 2);
          if (!overlap)
            {
              continue;
            }

          double rbBandwidth = technologyType == "Nr" ? m_txMapNr.begin()->second.rbBandwidth : m_txMapLte.begin()->second.rbBandwidth;
          Ptr<MobilityModel> wifiMobility = it->second.netDevice->GetNode()->GetObject<ConstantPositionMobilityModel>();
          Ptr<MobilityModel> cMobility = netDevice->GetNode()->GetObject<ConstantPositionMobilityModel>();
          uint8_t j = 1;
          std::vector<uint8_t> indexesRB;
          double powerW = 0.0;
          double wifiStartFreq = m_centralFrequency11p - m_bandWidth11p / 2;
          double wifiEndFreq = m_centralFrequency11p + m_bandWidth11p / 2;
          double cStartFreq = c_frequency - c_bandwidth / 2;
          for (auto it2 = signal->ValuesBegin(); it2 != signal->ValuesEnd(); ++it2)
             {
               if ((*it2) > 0)
                 {
                    double subBandFreq = cStartFreq + j * rbBandwidth;
                    if (subBandFreq >= wifiStartFreq && subBandFreq <= wifiEndFreq)
                      {
                        powerW += (*it2) * rbBandwidth;
                      }
                 }
             }

          // Calculate interference delay and received power
          if (powerW == 0.0)
            {
               continue;
            }
          double powerDbm = WToDbm(powerW);
          double finalInterferencePowerDbm = propagationLoss->CalcRxPower(powerDbm, wifiMobility, cMobility);
          double finalInterferencePowerW = DbmToW(finalInterferencePowerDbm);

          if ((finalInterferencePowerDbm + wifiPhy->GetRxGain ()) < wifiPhy->GetRxSensitivity ())
            {
              continue;
            }
          else
            {
              RxPowerWattPerChannelBand rxInterference = RxPowerWattPerChannelBand ();
              rxInterference.insert ({std::make_pair (0, 0), finalInterferencePowerW});
              wifiPhy->GetInterferenceHelper()->AddForeignSignal(duration, rxInterference);
            }
        }
    }

  if (technologyType == "Nr" && !m_txMapLte.empty())
    {
      // The transmission is from NR, so we need to add interference from LTE signals
      for (auto it = m_txMapLte.begin(); it != m_txMapLte.end(); ++it)
        {
          Ptr<cv2x_LteSpectrumPhy> ltePhy = it->second.netDevice->GetPhy ()->GetSlSpectrumPhy ();
          Ptr<cv2x_LteUeNetDevice> nd = DynamicCast<cv2x_LteUeNetDevice>(netDevice);
          Ptr<SpectrumValue> spectrum = ltePhy->GetTxPowerSpectralDensity ();
          Ptr<SpectrumValue> interferenceSignal = Create<SpectrumValue> (spectrum->GetSpectrumModel());
          double rbBandwidth = m_txMapLte.begin ()->second.rbBandwidth;
          uint8_t j = 1;
          double nrStartFreq = m_centralFrequencyNr - m_bandWidthNr / 2;
          double nrEndFreq = m_centralFrequencyNr + m_bandWidthNr / 2;
          double lteStartFreq = m_centralFrequencyLte - m_bandWidthLte / 2;
          double lteCentralFreq = m_centralFrequencyLte;
          double lteBandwidth = m_bandWidthLte;

          bool overlap = (lteCentralFreq - lteBandwidth / 2 <= nrEndFreq &&
                          lteCentralFreq + lteBandwidth / 2 >= nrStartFreq);
          if (!overlap)
            {
              continue;
            }
          Ptr<MobilityModel> c1Mobility = it->second.netDevice->GetNode()->GetObject<ConstantPositionMobilityModel>();
          Ptr<MobilityModel> c2Mobility = netDevice->GetNode()->GetObject<ConstantPositionMobilityModel>();
          double pathGain = propagationLoss->CalcRxPower (0, c1Mobility, c2Mobility);
          if (pathGain > m_noisePowerThreshold)
            {
              continue;
            }
          for (auto it2 = signal->ValuesBegin (); it2 != signal->ValuesEnd (); ++it2)
            {
              double subBandfreq = lteStartFreq + j * rbBandwidth;
              if ((*it2) > 0)
                {
                  if (subBandfreq >= nrStartFreq && subBandfreq <= nrEndFreq)
                    {
                      (*interferenceSignal)[j - 1] = (*it2);
                    }
                }
              j += 1;
            }

          double pathGainLinear = std::pow (10.0, (-pathGain) / 10.0);
          *(interferenceSignal) *= pathGainLinear;

          std::vector<SpectrumValue> interferenceSignals = { *interferenceSignal };
          ltePhy->UpdateSlIntPerceived (interferenceSignals);
        }
    }
  else if (technologyType == "Lte" && !m_txMapNr.empty())
    {
      // The transmission is from LTE, so we need to add interference from NR signals
      for (auto it = m_txMapNr.begin(); it != m_txMapNr.end(); ++it)
        {
          Ptr<NrSpectrumPhy> nrPhy = it->second.netDevice->GetPhy (0)->GetSpectrumPhy ();
          Ptr<SpectrumValue> spectrum = nrPhy->GetTxPowerSpectralDensity ();
          Ptr<SpectrumValue> interferenceSignal = Create<SpectrumValue> (spectrum->GetSpectrumModel());
          double rbBandwidth = m_txMapNr.begin ()->second.rbBandwidth;
          uint8_t j = 1;
          double lteStartFreq = m_centralFrequencyLte - m_bandWidthLte / 2;
          double lteEndFreq = m_centralFrequencyLte + m_bandWidthLte / 2;
          double nrStartFreq = m_centralFrequencyNr - m_bandWidthNr / 2;
          double nrCentralFreq = m_centralFrequencyNr;
          double nrBandwidth = m_bandWidthNr;

          bool overlap = (nrCentralFreq - nrBandwidth / 2 <= lteEndFreq &&
                          nrCentralFreq + nrBandwidth / 2 >= lteStartFreq);
          if (!overlap)
            {
              continue;
            }

          Ptr<MobilityModel> c1Mobility = it->second.netDevice->GetNode()->GetObject<ConstantPositionMobilityModel>();
          Ptr<MobilityModel> c2Mobility = netDevice->GetNode()->GetObject<ConstantPositionMobilityModel>();
          double pathGain = propagationLoss->CalcRxPower (0, c1Mobility, c2Mobility);
          if (pathGain > m_noisePowerThreshold)
            {
              continue;
            }
          for (auto it2 = signal->ValuesBegin (); it2 != signal->ValuesEnd (); ++it2)
            {
              double subBandfreq = lteStartFreq + j * rbBandwidth;
              if ((*it2) > 0)
                {
                  if (subBandfreq >= nrStartFreq && subBandfreq <= lteEndFreq)
                    {
                      (*interferenceSignal)[j - 1] = (*it2);
                    }
                }
              j += 1;
            }

          double pathGainLinear = std::pow (10.0, (-pathGain) / 10.0);
          *(interferenceSignal) *= pathGainLinear;

          nrPhy->GetNrInterference()->AddSignal (interferenceSignal, duration);
        }
    }
}

// Add interference from NR signals to 80211p signals
void
TxTracker::AddInterferenceFrom11p (Ptr<YansWifiPhy> sender, Ptr<MobilityModel> receiverMobility, Ptr<PropagationLossModel> propagationLoss, Ptr<PropagationDelayModel> propagationDelay)
{
  uint8_t nodeId = 0;
  if(!m_txMapNr.empty())
    {
      for (auto it = m_txMapNr.begin (); it != m_txMapNr.end (); ++it)
        {
          double interferencePower = 0.0;
          Ptr<NrUePhy> nrPhy = it->second.netDevice->GetPhy (0);
          NrSpectrumPhy::State current_state = nrPhy->GetSpectrumPhy ()->GetState ();

          // Process only if the NR PHY is transmitting
          if (current_state == NrSpectrumPhy::State::TX)
            {
              Ptr<SpectrumValue> spectrum = nrPhy->GetSpectrumPhy ()->GetTxPowerSpectralDensity ();
              double rbBandwidth = m_txMapNr.begin ()->second.rbBandwidth;
              uint8_t j = 1;
              double wifiStartFreq = centralFrequency11p - bandWidth11p / 2;
              double wifiEndFreq = centralFrequency11p + bandWidth11p / 2;
              double nrStartFreq = centralFrequencyNr - bandWidthNr / 2;
              double interferenceCentralFreq = centralFrequencyNr;
              double interferenceBandwidth = bandWidthNr;

              bool overlap = (interferenceCentralFreq - interferenceBandwidth / 2 <= wifiEndFreq &&
                              interferenceCentralFreq + interferenceBandwidth / 2 >= wifiStartFreq);
              if (!overlap)
                {
                  continue;
                }

              // Calculate interference for overlapping frequency bands
              for (auto it2 = spectrum->ValuesBegin (); it2 != spectrum->ValuesEnd (); ++it2)
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
                  Ptr<MobilityModel> interferenceMobility =
                      it->second.netDevice->GetNode ()->GetObject<ConstantPositionMobilityModel> ();
                  Time interferenceDelay =
                      propagationDelay->GetDelay (interferenceMobility, receiverMobility);

                  // Calculate received interference power and store it in the map
                  double interferenceRxPowerDbm = propagationLoss->CalcRxPower (
                      WToDbm (interferencePower), interferenceMobility, receiverMobility);
                  RxPowerWattPerChannelBand rxInterference = RxPowerWattPerChannelBand ();
                  rxInterference.insert ({std::make_pair (0, 0), DbmToW (interferenceRxPowerDbm)});
                  noisePowerPerNode[std::to_string (nodeId)] = std::make_pair (rxInterference, interferenceDelay);
                  nodeId++;
                }
            }
          else if (current_state == NrSpectrumPhy::State::RX_DATA)
            {
              nrPhy->GetSpectrumPhy ()->GetNrInterference()->AddSignal (it->second.first, it->second.second);
            }
        }
    }

  if (!m_txMapLte.empty())
    {
      for (auto it = m_txMapLte.begin(); it != m_txMapLte.end(); ++it)
        {
          double interferencePower = 0.0;
          Ptr<cv2x_LteSpectrumPhy> ltePhy = it->second.netDevice->GetPhy ()->GetSlSpectrumPhy ();
          cv2x_LteSpectrumPhy::State current_state = ltePhy->GetState ();

          // Process only if the LTE PHY is transmitting
          if (current_state == cv2x_LteSpectrumPhy::State::TX_DL_CTRL ||
              current_state == cv2x_LteSpectrumPhy::State::TX_DATA ||
              current_state == cv2x_LteSpectrumPhy::State::TX_UL_V2X_SCI ||
              current_state == cv2x_LteSpectrumPhy::State::TX_UL_SRS)
            {
              std::cout << current_state << std::endl;
              Ptr<SpectrumValue> spectrum = ltePhy->GetTxPowerSpectralDensity ();
              double rbBandwidth = m_txMapLte.begin ()->second.rbBandwidth;
              uint8_t j = 1;
              double wifiStartFreq = centralFrequency11p - bandWidth11p / 2;
              double wifiEndFreq = centralFrequency11p + bandWidth11p / 2;
              double lteStartFreq = centralFrequencyLte - bandWidthLte / 2;
              double interferenceCentralFreq = centralFrequencyLte;
              double interferenceBandwidth = bandWidthLte;

              bool overlap = (interferenceCentralFreq - interferenceBandwidth / 2 <= wifiEndFreq &&
                              interferenceCentralFreq + interferenceBandwidth / 2 >= wifiStartFreq);
              if (!overlap)
                {
                  continue;
                }

              // Calculate interference for overlapping frequency bands
              for (auto it2 = spectrum->ValuesBegin (); it2 != spectrum->ValuesEnd (); ++it2)
                {
                  double subBandfreq = lteStartFreq + j * rbBandwidth;
                  if ((*it2) > 0 && subBandfreq >= wifiStartFreq && subBandfreq <= wifiEndFreq)
                    {
                      interferencePower += (*it2) * rbBandwidth;
                    }
                  j += 1;
                }

              if (interferencePower > 0.0)
                {
                  Ptr<MobilityModel> interferenceMobility =
                      it->second.netDevice->GetNode () ->GetObject<ConstantPositionMobilityModel> ();
                  Time interferenceDelay = propagationDelay->GetDelay (interferenceMobility, receiverMobility);

                  // Calculate received interference power and store it in the map
                  double interferenceRxPowerDbm = propagationLoss->CalcRxPower (WToDbm (interferencePower), interferenceMobility, receiverMobility);
                  RxPowerWattPerChannelBand rxInterference = RxPowerWattPerChannelBand ();
                  rxInterference.insert ({std::make_pair (0, 0), DbmToW (interferenceRxPowerDbm)});
                  noisePowerPerNode[std::to_string (nodeId)] = std::make_pair (rxInterference, interferenceDelay);
                  nodeId++;
                }
            }
          else if (current_state == cv2x_LteSpectrumPhy::State::RX_DATA)
            {
              std::cout << current_state << std::endl;
            }
        }
    }
}

} // namespace ns3
