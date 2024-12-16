/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "sionna-helper.h"

namespace ns3 {

void
SionnaHelper::SetMarkerFile ()
{
  std::ofstream outFile("src/sionna/setup.txt");
  if (!outFile.is_open())
    {
      std::cerr << "Unable to open file";
    }

  if (m_sionna)
    {
      if (sionna_server_ip.empty() && !sionna_local_machine)
        {
          std::cerr << "SIONNA server IP address is empty. Please provide a valid IP address." << std::endl;
          return;
        }
      std::cout << "SIONNA mode enabled" << std::endl;
      outFile << "1";
      outFile.close();
    }
  else
    {
      outFile << "0";
      outFile.close();
    }
}


}

