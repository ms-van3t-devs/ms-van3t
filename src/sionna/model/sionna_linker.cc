/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "sionna_linker.h"

namespace ns3 {

Vector GetPositionFromMobilityModel(Ptr<MobilityModel> mobilityModel) {
    Ptr<ConstantPositionMobilityModel> constantPositionModel = mobilityModel->GetObject<ConstantPositionMobilityModel>();
    return constantPositionModel->GetPosition();
}

} // namespace ns3