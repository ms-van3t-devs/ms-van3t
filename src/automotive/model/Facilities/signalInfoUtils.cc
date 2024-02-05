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

#include "signalInfoUtils.h"

SignalInfoUtils::SignalInfoUtils()
{
    m_signalInfo.timestamp = NULL;
    m_signalInfo.rssi = NULL;
    m_signalInfo.snr = NULL;
    m_signalInfo.rsrp = NULL;
}

void SignalInfoUtils::SetSignalInfo(double timestamp, double rssi, double snr, double rsrp)
{
    m_signalInfo.timestamp = timestamp;

    if(std::isinf(rssi)) {
        m_signalInfo.rssi = DEFAULT_VALUE;
      }
    else {
      m_signalInfo.rssi = rssi;
      }

    if(std::isinf(snr)) {
        m_signalInfo.snr = DEFAULT_VALUE;
      }
    else {
      m_signalInfo.snr = snr;
      }

    if(std::isinf(rsrp)) {
        m_signalInfo.rsrp = DEFAULT_VALUE;
      }
    else {
      m_signalInfo.rsrp = rsrp;
      }
};

SignalInfo SignalInfoUtils::GetSignalInfo() 
{
    return m_signalInfo;
};

// Function to write the last signal information to a file
void SignalInfoUtils::WriteLastSignalInfo(std::string path, long stationID) 
{
    // Check if the file needs to be signed
    std::ifstream file(path, std::ios::app);
    bool needsSignature = !file.good();
    file.close();

    // Open the file in append mode
    std::ofstream outFile(path, std::ios::app);

    // If the file needs a signature, write the header
    if (needsSignature) {
        outFile << "#Timestamp, StationID, RSSI, SNR, RSRP(CV2X only)\n";
    }

    // Write the signal information with StationID
    outFile << m_signalInfo.timestamp << "," << stationID << ",";

    if(std::isnan(m_signalInfo.rssi)){
        outFile << "Inf" << ",";
      }
    else {
        outFile << m_signalInfo.rssi << ",";
      }

    if(std::isnan(m_signalInfo.snr)){
        outFile << "Inf" << ",";
      }
    else {
        outFile << m_signalInfo.snr << ",";
      }

    if(std::isnan(m_signalInfo.rsrp)){
        outFile << "Inf" << "\n";
      }
    else {
        outFile << m_signalInfo.rsrp << "\n";
      }

    // Close the file
    outFile.close();
}
