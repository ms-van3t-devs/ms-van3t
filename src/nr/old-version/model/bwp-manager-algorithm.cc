/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
 *
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
 */
#include "bwp-manager-algorithm.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BwpManagerAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (BwpManagerAlgorithm);

TypeId
BwpManagerAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BwpManagerAlgorithm")
    .SetParent<ObjectBase> ()
    .SetGroupName ("nr")
  ;
  return tid;
}

NS_OBJECT_ENSURE_REGISTERED (BwpManagerAlgorithmStatic);

#define DECLARE_ATTR(NAME,DESC,GETTER,SETTER)                              \
  .AddAttribute (NAME,                                                     \
                 DESC,                                                     \
                 UintegerValue (0),                                        \
                 MakeUintegerAccessor (&BwpManagerAlgorithmStatic::GETTER, \
                                       &BwpManagerAlgorithmStatic::SETTER),\
                 MakeUintegerChecker<uint8_t> (0, 5))

TypeId
BwpManagerAlgorithmStatic::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BwpManagerAlgorithmStatic")
    .SetParent<BwpManagerAlgorithm> ()
    .SetGroupName ("nr")
    .AddConstructor<BwpManagerAlgorithmStatic> ()
    DECLARE_ATTR ("GBR_CONV_VOICE",
                  "BWP index to which flows of this Qci type should be forwarded.",
                  GetConvVoiceBwp,
                  SetConvVoiceBwp)
    DECLARE_ATTR ("GBR_CONV_VIDEO",
                  "BWP index to which flows of GBR_CONV_VIDEO Qci type should be forwarded.",
                  GetConvVideoBwp,
                  SetConvVideoBwp)
    DECLARE_ATTR ("GBR_GAMING",
                  "BWP index to which flows of GBR_GAMING Qci type should be forwarded.",
                  GetGamingBwp,
                  SetGamingBwp)
    DECLARE_ATTR ("GBR_NON_CONV_VIDEO",
                  "BWP index to which flows of GBR_NON_CONV_VIDEO Qci type should be forwarded.",
                  GetNonConvVideoBwp,
                  SetNonConvVideoBwp)
    DECLARE_ATTR ("GBR_MC_PUSH_TO_TALK",
                  "BWP index to which flows of GBR_MC_PUSH_TO_TALK Qci type should be forwarded.",
                  GetMcPttBwp,
                  SetMcPttBwp)
    DECLARE_ATTR ("GBR_NMC_PUSH_TO_TALK",
                  "BWP index to which flows of GBR_NMC_PUSH_TO_TALK Qci type should be forwarded.",
                  GetNmcPttBwp,
                  SetNmcPttBwp)
    DECLARE_ATTR ("GBR_MC_VIDEO",
                  "BWP index to which flows of GBR_MC_VIDEO Qci type should be forwarded.",
                  GetMcVideoBwp,
                  SetMcVideoBwp)
    DECLARE_ATTR ("GBR_V2X",
                  "BWP index to which flows of GBR_V2X Qci type should be forwarded.",
                  GetGbrV2xBwp,
                  SetGbrV2xBwp)
    DECLARE_ATTR ("NGBR_IMS",
                  "BWP index to which flows of NGBR_IMS Qci type should be forwarded.",
                  GetImsBwp,
                  SetImsBwp)
    DECLARE_ATTR ("NGBR_VIDEO_TCP_OPERATOR",
                  "BWP index to which flows of NGBR_VIDEO_TCP_OPERATOR Qci type should be forwarded.",
                  GetVideoTcpOpBwp,
                  SetVideoTcpOpBwp)
    DECLARE_ATTR ("NGBR_VOICE_VIDEO_GAMING",
                  "BWP index to which flows of NGBR_VOICE_VIDEO_GAMING Qci type should be forwarded.",
                  GetVideoGamingBwp,
                  SetVideoGamingBwp)
    DECLARE_ATTR ("NGBR_VIDEO_TCP_PREMIUM",
                  "BWP index to which flows of NGBR_VIDEO_TCP_PREMIUM Qci type should be forwarded.",
                  GetVideoTcpPremiumBwp,
                  SetVideoTcpPremiumBwp)
    DECLARE_ATTR ("NGBR_VIDEO_TCP_DEFAULT",
                  "BWP index to which flows of NGBR_VIDEO_TCP_DEFAULT Qci type should be forwarded.",
                  GetVideoTcpDefaultBwp,
                  SetVideoTcpDefaultBwp)
    DECLARE_ATTR ("NGBR_MC_DELAY_SIGNAL",
                  "BWP index to which flows of NGBR_MC_DELAY_SIGNAL Qci type should be forwarded.",
                  GetMcDelaySignalBwp,
                  SetMcDelaySignalBwp)
    DECLARE_ATTR ("NGBR_MC_DATA",
                  "BWP index to which flows of NGBR_MC_DATA Qci type should be forwarded.",
                  GetMcDataBwp,
                  SetMcDataBwp)
    DECLARE_ATTR ("NGBR_V2X",
                  "BWP index to which flows of NGBR_V2X Qci type should be forwarded.",
                  GetNgbrV2xBwp,
                  SetNgbrV2xBwp)
    DECLARE_ATTR ("NGBR_LOW_LAT_EMBB",
                  "BWP index to which flows of NGBR_LOW_LAT_EMBB Qci type should be forwarded.",
                  GetLowLatEmbbBwp,
                  SetLowLatEmbbBwp)
    DECLARE_ATTR ("DGBR_DISCRETE_AUT_SMALL",
                  "BWP index to which flows of DGBR_DISCRETE_AUT_SMALL Qci type should be forwarded.",
                  GetDiscreteAutSmallBwp,
                  SetDiscreteAutSmallBwp)
    DECLARE_ATTR ("DGBR_DISCRETE_AUT_LARGE",
                  "BWP index to which flows of DGBR_DISCRETE_AUT_LARGE Qci type should be forwarded.",
                  GetDiscreteAutLargeBwp,
                  SetDiscreteAutLargeBwp)
    DECLARE_ATTR ("DGBR_ITS",
                  "BWP index to which flows of DGBR_ITS Qci type should be forwarded.",
                  GetItsBwp,
                  SetItsBwp)
    DECLARE_ATTR ("DGBR_ELECTRICITY",
                  "BWP index to which flows of DGBR_ELECTRICITY Qci type should be forwarded.",
                  GetElectricityBwp,
                  SetElectricityBwp)

  ;
  return tid;
}

uint8_t
BwpManagerAlgorithmStatic::GetBwpForEpsBearer (const EpsBearer::Qci &v) const
{
  return m_qciToBwpMap.at (v);
}

} // namespace ns3
