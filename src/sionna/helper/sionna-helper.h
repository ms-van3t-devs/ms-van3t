/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SIONNA_HELPER_H
#define SIONNA_HELPER_H

#include "ns3/sionna-connection-handler.h"

namespace ns3 {

class SionnaHelper
{
public:
  static SionnaHelper& GetInstance()
  {
    static SionnaHelper instance;
    return instance;
  }
  void SetSionna(bool sionna) {m_sionna = sionna;};
  bool GetSionna() {return m_sionna;};
  void SetVerbose(bool verbose) {sionna_verbose = verbose;};
  void SetServerIp(std::string serverIp) {sionna_server_ip = serverIp;};
  void SetLocalMachine(bool local_machine) {sionna_local_machine = local_machine;};

private:
  SionnaHelper() = default;
  ~SionnaHelper() = default;
  SionnaHelper(const SionnaHelper&) = delete;
  SionnaHelper& operator = (const SionnaHelper&) = delete;
  bool m_sionna = false;
};

}

#endif /* SIONNA_HELPER_H */

