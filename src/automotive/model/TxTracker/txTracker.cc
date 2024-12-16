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

* Created by:
    *  Diego Gasco, Politecnico di Torino (diego.gasco@polito.it, diego.gasco99@gmail.com)
*/

#include "txTracker.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("TxTracker");

double DbmToW (double dbm)
{
  return std::pow(10, (dbm - 30) / 10);
}

double WToDbm (double w)
{
  return 10 * std::log10(w) + 30;
}

void
Insert11pNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<WifiNetDevice>>> nodes)
{
  for (auto n : nodes)
    {
      std::string vehID = std::get<0>(n);
      uint8_t nodeID = std::get<1>(n);
      Ptr<WifiNetDevice> netDevice = std::get<2>(n);
      NS_ASSERT_MSG (netDevice != nullptr, "Node is nullptr");
      Ptr<WifiPhy> wifiPhy = netDevice->GetPhy();
      double bandwidth = wifiPhy->GetChannelWidth();
      m_txMap11p[vehID] = txParameters11p {
          nodeID,
          netDevice,
          bandwidth,
          std::pow(10, (wifiPhy->GetTxPowerStart() - 30) / 10),
      };
    }
}

void
InsertNrNodes (std::vector<std::tuple<std::string, uint8_t, Ptr<NrUeNetDevice>>> nodes)
{
  for (auto n : nodes)
    {
      std::string vehID = std::get<0>(n);
      uint8_t nodeID = std::get<1>(n);
      Ptr<NrUeNetDevice> netDevice = std::get<2>(n);
      NS_ASSERT_MSG (netDevice != nullptr, "Node is nullptr");
      Ptr<NrUePhy> uePhy = netDevice->GetPhy (0);
      uint32_t bandwidth = uePhy->GetChannelBandwidth();
      double rbBand = bandwidth / uePhy->GetRbNum();
      m_txMapNr[vehID] = txParametersNR {
          nodeID,
          netDevice,
          rbBand,
      };
    }
}

std::unordered_map<std::string, std::pair<Ptr<SpectrumValue>, Time>>
AddInterferenceNr (Ptr<SpectrumValue> wifiSignal, Ptr<MobilityModel> receiverMobility, Time delay, Ptr<PropagationLossModel> propagationLoss)
{
  std::unordered_map<std::string, std::pair<Ptr<SpectrumValue>, Time>> interferenceNodes;
  for (auto it = m_txMap11p.begin(); it != m_txMap11p.end(); ++it)
  {
    Ptr<WifiPhy> wifiPhy = it->second.netDevice->GetPhy();
    WifiPhyState current_state = wifiPhy->GetState()->GetState();
    if (current_state == WifiPhyState::TX)
      {
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
        // Time interferenceDelay = m_propagationDelay->GetDelay (interferenceMobility, receiverMobility);
        Time interferenceDelay = delay;
        double finalInterferencePowerDbm = propagationLoss->CalcRxPower(10 * log10(it->second.txPower_W) + 30, interferenceMobility, receiverMobility);
        double finalInterferencePowerW = std::pow(10, (finalInterferencePowerDbm - 30) / 10);

        uint8_t j = 1;
        uint8_t counterRB = 0;
        double nrStartFreq = centralFrequencyNr - bandWidthNr / 2;
        double nrEndFreq = centralFrequencyNr + bandWidthNr / 2;
        double wifiStartFreq = centralFrequency11p - bandWidth11p / 2;
        double wifiEndFreq = centralFrequency11p + bandWidth11p / 2;
        for (auto it2 = wifiSignal->ValuesBegin(); it2 != wifiSignal->ValuesEnd(); ++it2)
          {
            double subBandFreq = wifiStartFreq + j * rbBandwidth;
            if (subBandFreq >= nrStartFreq && subBandFreq <= nrEndFreq)
              {
                counterRB++;
              }
            else
              {
                break;
              }
          }

        for(uint8_t i = 0; i < counterRB; i++)
          {
            (*wifiSignal)[i] = finalInterferencePowerW / rbBandwidth;
          }

        interferenceNodes.insert({ it->first, std::make_pair (wifiSignal, interferenceDelay)});
      }
  }
  return interferenceNodes;
}

std::unordered_map<std::string, std::pair<RxPowerWattPerChannelBand, Time>>
AddInterference11p (Ptr<YansWifiPhy> sender, Ptr<MobilityModel> receiverMobility, Ptr<PropagationLossModel> propagationLoss, Ptr<PropagationDelayModel> propagationDelay)
{
  std::unordered_map<std::string, std::pair<RxPowerWattPerChannelBand, Time>> noisePowerPerNode;
  double interferencePower = 0.0;
  for (auto it = m_txMapNr.begin(); it != m_txMapNr.end(); ++it)
    {
      Ptr<NrUePhy> nrPhy = it->second.netDevice->GetPhy(0);
      NrSpectrumPhy::State current_state = nrPhy->GetSpectrumPhy ()->GetState();
      if (current_state == NrSpectrumPhy::State::TX)
        {
          Ptr<SpectrumValue> spectrum = nrPhy->GetSpectrumPhy ()->GetTxPowerSpectralDensity();
          double rbBandwidth = m_txMapNr.begin()->second.rbBandwidth;
          uint8_t j = 1;
          double wifiStartFreq = centralFrequency11p - bandWidth11p / 2;
          double wifiEndFreq = centralFrequency11p + bandWidth11p / 2;
          double nrStartFreq = centralFrequencyNr - bandWidthNr / 2;
          double nrEndFreq = centralFrequencyNr + bandWidthNr / 2;
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
              double interferenceRxPowerDbm = propagationLoss->CalcRxPower(WToDbm (interferencePower), interferenceMobility, receiverMobility);
              RxPowerWattPerChannelBand rxInterference = RxPowerWattPerChannelBand ();
              rxInterference.insert({std::make_pair (0, 0), DbmToW (interferenceRxPowerDbm)});
              noisePowerPerNode[it->first] = std::make_pair (rxInterference, interferenceDelay);
            }
        }
    }
  return noisePowerPerNode;
}

}
