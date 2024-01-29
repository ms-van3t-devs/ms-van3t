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

#ifndef SIGNALINFOUTILS_H
#define SIGNALINFOUTILS_H
#include <string>

typedef struct {
    double timestamp;
    double rssi;
    double snr;
} signalInfo;


class SignalInfoUtils
{
public:
    SignalInfoUtils();
    void SetSignalInfo(double timestamp, double rssi, double snr);
    signalInfo GetSignalInfo();
    void WriteLastSignalInfo(std::string path, long stationID);

private:
    signalInfo m_signalInfo;
};


#endif // SIGNALINFOUTILS_H