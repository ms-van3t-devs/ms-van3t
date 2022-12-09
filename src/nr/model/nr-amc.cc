/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "nr-amc.h"
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/math.h>
#include <ns3/enum.h>
#include <ns3/uinteger.h>
#include "nr-error-model.h"
#include "nr-lte-mi-error-model.h"
#include "lena-error-model.h"
#include <ns3/nr-spectrum-value-helper.h>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrAmc");
NS_OBJECT_ENSURE_REGISTERED (NrAmc);

NrAmc::NrAmc ()
{
  NS_LOG_INFO ("Initialze AMC module");
}

NrAmc::~NrAmc ()
{
}

void
NrAmc::SetDlMode ()
{
  NS_LOG_FUNCTION (this);
  m_emMode = NrErrorModel::DL;
}

void
NrAmc::SetUlMode ()
{
  NS_LOG_FUNCTION (this);
  m_emMode = NrErrorModel::UL;
}

TypeId
NrAmc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrAmc")
    .SetParent<Object> ()
    .AddAttribute ("NumRefScPerRb",
                   "Number of Subcarriers carrying Reference Signals per RB",
                   UintegerValue (1),
                   MakeUintegerAccessor (&NrAmc::SetNumRefScPerRb,
                                         &NrAmc::GetNumRefScPerRb),
                   MakeUintegerChecker<uint8_t> (0, 12))
    .AddAttribute ("AmcModel",
                   "AMC model used to assign CQI",
                   EnumValue (NrAmc::ErrorModel),
                   MakeEnumAccessor (&NrAmc::SetAmcModel,
                                     &NrAmc::GetAmcModel),
                   MakeEnumChecker (NrAmc::ErrorModel, "ErrorModel",
                                    NrAmc::ShannonModel, "ShannonModel"))
    .AddAttribute ("ErrorModelType",
                   "Type of the Error Model to use when AmcModel is set to ErrorModel. "
                   "This parameter has to match the ErrorModelType in nr-spectrum-model,"
                   "because they need to refer to same MCS tables and indexes",
                   TypeIdValue (NrLteMiErrorModel::GetTypeId ()),
                   MakeTypeIdAccessor (&NrAmc::SetErrorModelType,
                                       &NrAmc::GetErrorModelType),
                   MakeTypeIdChecker ())
    .AddConstructor <NrAmc> ()
  ;
  return tid;
}

TypeId
NrAmc::GetInstanceTypeId() const
{
  return NrAmc::GetTypeId ();
}

uint8_t
NrAmc::GetMcsFromCqi (uint8_t cqi) const
{
  NS_LOG_FUNCTION (cqi);
  NS_ASSERT_MSG (cqi >= 0 && cqi <= 15, "CQI must be in [0..15] = " << cqi);

  double spectralEfficiency = m_errorModel->GetSpectralEfficiencyForCqi (cqi);
  uint8_t mcs = 0;

  while ((mcs < m_errorModel->GetMaxMcs ()) && (m_errorModel->GetSpectralEfficiencyForMcs (mcs + 1) <= spectralEfficiency))
    {
      ++mcs;
    }

  NS_LOG_LOGIC ("mcs = " << mcs);

  return mcs;
}

uint8_t
NrAmc::GetNumRefScPerRb () const
{
  NS_LOG_FUNCTION (this);
  return m_numRefScPerRb;
}

void NrAmc::SetNumRefScPerRb(uint8_t nref)
{
  NS_LOG_FUNCTION (this);
  m_numRefScPerRb = nref;
}

uint32_t
NrAmc::CalculateTbSize (uint8_t mcs, uint32_t nprb) const
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (mcs));

  NS_ASSERT_MSG (mcs <= m_errorModel->GetMaxMcs (), "MCS=" << static_cast<uint32_t> (mcs) <<
                 " while maximum MCS is " << static_cast<uint32_t> (m_errorModel->GetMaxMcs ()));

  uint32_t payloadSize = GetPayloadSize (mcs, nprb);
  uint32_t tbSize = payloadSize;

  if (m_errorModelType != LenaErrorModel::GetTypeId ())
    {
      if (payloadSize >= m_crcLen)
        {
          tbSize = payloadSize - m_crcLen;  //subtract parity bits of m_crcLen used in transport block
        }
      uint32_t cbSize = m_errorModel->GetMaxCbSize (payloadSize, mcs); // max size of a code block (including m_crcLen)
      if (tbSize > cbSize)  // segmentation of the transport block occurs
        {
          double C = ceil (tbSize / cbSize);
          tbSize = payloadSize - static_cast<uint32_t> (C * m_crcLen);   //subtract bits of m_crcLen used in code blocks, in case of code block segmentation
        }
    }

  NS_LOG_INFO (" mcs:" << (unsigned) mcs << " TB size:" << tbSize);

  return tbSize;
}

uint32_t
NrAmc::GetPayloadSize (uint8_t mcs, uint32_t nprb) const
{
  return m_errorModel->GetPayloadSize (NrSpectrumValueHelper::SUBCARRIERS_PER_RB - GetNumRefScPerRb (),
                                       mcs, nprb, m_emMode);
}

uint8_t
NrAmc::CreateCqiFeedbackWbTdma (const SpectrumValue& sinr, uint8_t &mcs) const
{
  NS_LOG_FUNCTION (this);

  // produces a single CQI/MCS value

  //std::vector<int> cqi;
  uint8_t cqi = 0;
  double seAvg = 0;
  double cqiAvg = 0;

  Values::const_iterator it;
  if (m_amcModel == ShannonModel)
    {
      //use shannon model
      double m_ber = GetBer();   // Shannon based model reference BER
      uint32_t rbNum = 0;
      for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
        {
          double sinr_ = (*it);
          if (sinr_ == 0.0)
            {
              //cqi.push_back (-1); // SINR == 0 (linear units) means no signal in this RB
            }
          else
            {
              /*
               * Compute the spectral efficiency from the SINR
               *                                        SINR
               * spectralEfficiency = log2 (1 + -------------------- )
               *                                    -ln(5*BER)/1.5
               * NB: SINR must be expressed in linear units
               */

              double s = log2 ( 1 + ( sinr_ / ( (-std::log (5.0 * m_ber )) / 1.5) ));
              seAvg += s;

              int cqi_ = GetCqiFromSpectralEfficiency (s);
              cqiAvg += cqi_;
              rbNum++;

              NS_LOG_LOGIC (" PRB =" << sinr.GetSpectrumModel ()->GetNumBands ()
                                     << ", sinr = " << sinr_
                                     << " (=" << 10 * std::log10 (sinr_) << " dB)"
                                     << ", spectral efficiency =" << s
                                     << ", CQI = " << cqi_ << ", BER = " << m_ber);
              //cqi.push_back (cqi_);
            }
        }
      if (rbNum != 0)
        {
          seAvg /= rbNum;
          cqiAvg /= rbNum;
        }
      cqi = GetCqiFromSpectralEfficiency (seAvg);   //ceil (cqiAvg);
      mcs = GetMcsFromSpectralEfficiency (seAvg);
    }
  else if (m_amcModel == ErrorModel)
    {
      std::vector <int> rbMap;
      int rbId = 0;
      for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
        {
          if (*it != 0.0)
            {
              rbMap.push_back (rbId);
            }
          rbId += 1;
        }

      mcs = 0;
      Ptr<NrErrorModelOutput> output;
      while (mcs <= m_errorModel->GetMaxMcs ())
        {
          output = m_errorModel->GetTbDecodificationStats (sinr, rbMap,
                                                           CalculateTbSize (mcs, rbMap.size ()),
                                                           mcs,
                                                           NrErrorModel::NrErrorModelHistory ());
          if (output->m_tbler > 0.1)
            {
              break;
            }
          mcs++;
        }

      if (mcs > 0)
        {
          mcs--;
        }

      if ((output->m_tbler > 0.1) && (mcs == 0))
        {
          cqi = 0;
        }
      else if (mcs == m_errorModel->GetMaxMcs ())
        {
          cqi = 15;   // all MCSs can guarantee the 10 % of BER
        }
      else
        {
          double s = m_errorModel->GetSpectralEfficiencyForMcs (mcs);
          cqi = 0;
          while ((cqi < 15) && (m_errorModel->GetSpectralEfficiencyForCqi (cqi + 1) <= s))
            {
              ++cqi;
            }
        }
      NS_LOG_DEBUG (this << "\t MCS " << (uint16_t)mcs << "-> CQI " << cqi);
    }
  return cqi;
}

uint8_t
NrAmc::GetCqiFromSpectralEfficiency (double s) const
{
  NS_LOG_FUNCTION (s);
  NS_ASSERT_MSG (s >= 0.0, "negative spectral efficiency = " << s);
  uint8_t cqi = 0;
  while ((cqi < 15) && (m_errorModel->GetSpectralEfficiencyForCqi (cqi + 1) < s))
    {
      ++cqi;
    }
  NS_LOG_LOGIC ("cqi = " << cqi);
  return cqi;
}

uint8_t
NrAmc::GetMcsFromSpectralEfficiency (double s) const
{
  NS_LOG_FUNCTION (s);
  NS_ASSERT_MSG (s >= 0.0, "negative spectral efficiency = " << s);
  uint8_t mcs = 0;
  while ((mcs < m_errorModel->GetMaxMcs ()) && (m_errorModel->GetSpectralEfficiencyForMcs (mcs + 1) < s))
    {
      ++mcs;
    }
  NS_LOG_LOGIC ("cqi = " << mcs);
  return mcs;
}

uint32_t
NrAmc::GetMaxMcs() const
{
  NS_LOG_FUNCTION (this);
  return m_errorModel->GetMaxMcs ();
}

void
NrAmc::SetAmcModel (NrAmc::AmcModel m)
{
  NS_LOG_FUNCTION (this);
  m_amcModel = m;
}

NrAmc::AmcModel
NrAmc::GetAmcModel () const
{
  NS_LOG_FUNCTION (this);
  return m_amcModel;
}

void
NrAmc::SetErrorModelType (const TypeId &type)
{
  NS_LOG_FUNCTION (this);
  ObjectFactory factory;
  m_errorModelType = type;

  factory.SetTypeId (m_errorModelType);
  m_errorModel = DynamicCast<NrErrorModel> (factory.Create ());
  NS_ASSERT (m_errorModel != nullptr);
}

TypeId
NrAmc::GetErrorModelType () const
{
  NS_LOG_FUNCTION (this);
  return m_errorModelType;
}

double
NrAmc::GetBer () const
{
  NS_LOG_FUNCTION (this);
  if (GetErrorModelType() == NrLteMiErrorModel::GetTypeId () ||
      GetErrorModelType() == LenaErrorModel::GetTypeId ())
    {
      return 0.00005;  // Value for LTE error model
    }
  else
    {
      return 0.00001;  // Value for NR error model
    }
}


} // namespace ns3

