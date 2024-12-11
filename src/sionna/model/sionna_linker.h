/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SIONNA_LINKER_H
#define SIONNA_LINKER_H

#include "ns3/mobility-model.h"
#include "ns3/constant-position-mobility-model.h"

namespace ns3 {

Vector GetPositionFromMobilityModel(Ptr<MobilityModel> mobilityModel);

} // namespace ns3

#endif /* SIONNA_LINKER_H */