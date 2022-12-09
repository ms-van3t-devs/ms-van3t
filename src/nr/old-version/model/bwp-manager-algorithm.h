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
#ifndef BWPMANAGERALGORITHM_H
#define BWPMANAGERALGORITHM_H

#include <ns3/object.h>
#include <ns3/eps-bearer.h>

namespace ns3 {

/**
 * \ingroup bwp
 * \brief Interface for a Bwp selection algorithm based on the bearer
 *
 *
 * At the moment, we provide only a static algorithm that has to be configured
 * before the simulation starts (BwpManagerAlgorithmStatic).
 *
 *
 * \section bwp_manager_conf Configuration
 *
 * The algorithm can be set, before the scenario creation, through the
 * helper method NrHelper::SetGnbBwpManagerAlgorithmTypeId(). It is
 * also possible to set attributes, through NrHelper::SetGnbBwpManagerAlgorithmAttribute().
 *
 * For the UE, the methods to use are, respectively, NrHelper::SetUeBwpManagerAlgorithmTypeId()
 * and NrHelper::SetUeBwpManagerAlgorithmAttribute().
 *
 *
 * \see GetBwpForEpsBearer
 * \see BwpManagerAlgorithmStatic
 */
class BwpManagerAlgorithm : public Object
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the object
   */
  static TypeId GetTypeId ();

  /**
   * \brief constructor
   */
  BwpManagerAlgorithm () = default;
  /**
    * ~BwpManagerAlgorithm
    */
  virtual ~BwpManagerAlgorithm () override = default;
  /**
   * \brief Get the bandwidth part id for the Qci specified
   * \param v the qci
   * \return the bwp id that the algorithm selects for the qci specified
   */
  virtual uint8_t GetBwpForEpsBearer (const EpsBearer::Qci &v) const = 0;
};

/**
 * \ingroup bwp
 * \brief The BwpManagerAlgorithmStatic class
 *
 * A static manager: it gets the association through a series of Attributes.
 */
class BwpManagerAlgorithmStatic : public BwpManagerAlgorithm
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the object
   */
  static TypeId GetTypeId ();

  /**
   * \brief constructor
   */
  BwpManagerAlgorithmStatic () = default;
  /**
    * \brief deconstructor
    */
  virtual ~BwpManagerAlgorithmStatic () override = default;

  // inherited
  virtual uint8_t GetBwpForEpsBearer (const EpsBearer::Qci &v) const override;

  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetConvVoiceBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_CONV_VOICE] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetConvVoiceBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::GBR_CONV_VOICE);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetConvVideoBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_CONV_VIDEO] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetConvVideoBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::GBR_CONV_VIDEO);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetGamingBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_GAMING] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetGamingBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::GBR_GAMING);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNonConvVideoBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_NON_CONV_VIDEO] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetNonConvVideoBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::GBR_NON_CONV_VIDEO);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcPttBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_MC_PUSH_TO_TALK] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetMcPttBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::GBR_MC_PUSH_TO_TALK);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNmcPttBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_NMC_PUSH_TO_TALK] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetNmcPttBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::GBR_NMC_PUSH_TO_TALK);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcVideoBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_MC_VIDEO] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetMcVideoBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::GBR_MC_VIDEO);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetGbrV2xBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_V2X] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetGbrV2xBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::GBR_V2X);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetImsBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_IMS] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetImsBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_IMS);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpOpBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_OPERATOR] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetVideoTcpOpBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_VIDEO_TCP_OPERATOR);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoGamingBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_VOICE_VIDEO_GAMING] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetVideoGamingBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_VOICE_VIDEO_GAMING);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpPremiumBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_PREMIUM] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetVideoTcpPremiumBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_VIDEO_TCP_PREMIUM);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpDefaultBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_DEFAULT] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetVideoTcpDefaultBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcDelaySignalBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_MC_DELAY_SIGNAL] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetMcDelaySignalBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_MC_DELAY_SIGNAL);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcDataBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_MC_DATA] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetMcDataBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_MC_DATA);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNgbrV2xBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_V2X] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetNgbrV2xBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_V2X);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetLowLatEmbbBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_LOW_LAT_EMBB] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetLowLatEmbbBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::NGBR_LOW_LAT_EMBB);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetDiscreteAutSmallBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::DGBR_DISCRETE_AUT_SMALL] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetDiscreteAutSmallBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::DGBR_DISCRETE_AUT_SMALL);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetDiscreteAutLargeBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::DGBR_DISCRETE_AUT_LARGE] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetDiscreteAutLargeBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::DGBR_DISCRETE_AUT_LARGE);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetItsBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::DGBR_ITS] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetItsBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::DGBR_ITS);
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetElectricityBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::DGBR_ELECTRICITY] = bwpIndex;
  }
  /**
   * \brief Get the BWP index of the QCI in the function name
   * \return the BWP index of the selected QCI
   */
  uint8_t GetElectricityBwp () const
  {
    return m_qciToBwpMap.at (EpsBearer::DGBR_ELECTRICITY);
  }

private:
  /**
   * \brief Map between QCI and BWP
   */
  std::unordered_map <uint8_t, uint8_t> m_qciToBwpMap;
};

} // namespace ns3
#endif // BWPMANAGERALGORITHM_H
