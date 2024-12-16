/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SIONNA_HELPER_H
#define SIONNA_HELPER_H

#include "ns3/sionna_handler.h"

namespace ns3 {

class SionnaHelper
{
public:
  SionnaHelper() = default;
  ~SionnaHelper() = default;
  void SetSionna(bool sionna) {m_sionna = sionna;};
  void SetVerbose(bool verbose) {sionna_verbose = verbose;};
  void SetServerIp(std::string serverIp) {sionna_server_ip = serverIp;};
  void SetLocalMachine(bool local_machine) {sionna_local_machine = local_machine;};
  void SetMarkerFile();

private:
  bool m_sionna = false;
};

}

#endif /* SIONNA_HELPER_H */

