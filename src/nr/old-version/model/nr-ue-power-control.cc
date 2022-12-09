/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/uinteger.h>
#include <ns3/math.h>
#include "nr-ue-power-control.h"
#include "nr-ue-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrUePowerControl");

NS_OBJECT_ENSURE_REGISTERED (NrUePowerControl);

NrUePowerControl::NrUePowerControl ()
{
  NS_LOG_FUNCTION (this);
}

NrUePowerControl::NrUePowerControl (const Ptr<NrUePhy>& nrUePhy)
{
  m_nrUePhy = nrUePhy;
}

NrUePowerControl::~NrUePowerControl ()
{
  NS_LOG_FUNCTION (this);
}

void
NrUePowerControl::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
}

void
NrUePowerControl::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrUePowerControl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUePowerControl")
    .SetParent<Object> ()
    .SetGroupName ("NrPhy")
    .AddConstructor<NrUePowerControl> ()
    .AddAttribute ("ClosedLoop",
                   "If true Closed Loop mode will be active, otherwise Open Loop",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrUePowerControl::SetClosedLoop),
                   MakeBooleanChecker ())
    .AddAttribute ("AccumulationEnabled",
                   "If true TPC accumulation mode will be active, otherwise absolute mode will be active",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrUePowerControl::SetAccumulationEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("Alpha",
                   "Value of Alpha parameter",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&NrUePowerControl::SetAlpha),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Pcmax",
                   "Max Transmission power in dBm, Default value 23 dBm"
                   "TS36.101 section 6.2.3",
                   DoubleValue (23.0),
                   MakeDoubleAccessor (&NrUePowerControl::SetPcmax),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Pcmin",
                   "Min Transmission power in dBm, Default value -40 dBm"
                   "TS36.101 section 6.2.3",
                   DoubleValue (-40),
                   MakeDoubleAccessor (&NrUePowerControl::SetPcmin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PoNominalPusch",
                   "P_O_NOMINAL_PUSCH   INT (-126 ... 24), Default value -80",
                   IntegerValue (-80),
                   MakeIntegerAccessor (&NrUePowerControl::SetPoNominalPusch),
                   MakeIntegerChecker<int16_t> (-126, 24))
    .AddAttribute ("PoUePusch",
                   "P_O_UE_PUSCH   INT(-8...7), Default value 0",
                   IntegerValue (0),
                   MakeIntegerAccessor (&NrUePowerControl::SetPoUePusch),
                   MakeIntegerChecker<int16_t> (-8, 7))
    .AddAttribute ("PoNominalPucch",
                   "P_O_NOMINAL_PUCCH   INT (-126 ... 24), Default value -80",
                   IntegerValue (-80),
                   MakeIntegerAccessor (&NrUePowerControl::SetPoNominalPucch),
                   MakeIntegerChecker<int16_t> (-126, 24))
    .AddAttribute ("PoUePucch",
                   "P_O_UE_PUCCH   INT(-8...7), Default value 0",
                   IntegerValue (0),
                   MakeIntegerAccessor (&NrUePowerControl::SetPoUePucch),
                   MakeIntegerChecker<int16_t> (-8, 7))
    .AddAttribute ("PsrsOffset",
                   "P_SRS_OFFSET   INT(0...15), Default value 7",
                   IntegerValue (7),
                   MakeIntegerAccessor (&NrUePowerControl::m_PsrsOffset),
                   MakeIntegerChecker<int16_t> (0, 15))
    .AddAttribute ("TSpec",
                   "Technical specification TS 36.213 or TS 38.213,"
                   "By default is set TS to 36.213. To configure TS 36.213 "
                   "set the value TS36.213, while for TS 38.213 should be "
                   "configured TS38.213.",
                   EnumValue (NrUePowerControl::TS_36_213),
                   MakeEnumAccessor (&NrUePowerControl::SetTechnicalSpec),
                   MakeEnumChecker (NrUePowerControl::TS_36_213, "TS36.213",
                                    NrUePowerControl::TS_38_213, "TS38.213"))
    .AddAttribute ("KPusch",
                   "K_PUSCH parameter needed for PUSCH accumulation state "
                   "calculation. "
                   "This value must be carefully configured according to "
                   "TS 36.213 or TS 38.213 and taking into account the type "
                   "of simulation scenario. E.g. TDD, FDD, frame structure "
                   "type, etc. For, LTE FDD or FDD-TDD and frame structure "
                   "type 1, KPusch is 4.",
                   UintegerValue (4),
                   MakeUintegerAccessor(&NrUePowerControl::SetKPusch),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("K0Pucch",
                   "K0_PUCCH parameter needed for PUCCH accumulation state "
                   "calculation. Should be configured according TS 36.213 or "
                   "TS 38.213 specification depending on TSpec attribute "
                   "setting. According to TS 38.213 for FDD or FDD-TDD and "
                   "primary cell frame structure type 1, M is equal to 1 "
                   "and K0PUCCH is 4",
                   UintegerValue (4),
                   MakeUintegerAccessor (&NrUePowerControl::SetK0Pucch),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("BL_CE",
                   "When set to true means that this power control is applied to "
                   "bandwidth reduced, low complexity or coverage enhanced (BL/CE) device."
                   "By default this attribute is set to false. Default BL_CE "
                   "mode is CEModeB. This option can be used only in conjuction with "
                   "attribute TSpec being set to TS 36.213.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&NrUePowerControl::SetBlCe),
                    MakeBooleanChecker ())
    .AddTraceSource ("ReportPuschTxPower",
                     "Report PUSCH TxPower in dBm",
                     MakeTraceSourceAccessor (&NrUePowerControl::m_reportPuschTxPower),
                     "ns3::NrUePowerControl::TxPowerTracedCallback")
    .AddTraceSource ("ReportPucchTxPower",
                     "Report PUCCH TxPower in dBm",
                     MakeTraceSourceAccessor (&NrUePowerControl::m_reportPucchTxPower),
                     "ns3::NrUePowerControl::TxPowerTracedCallback")
    .AddTraceSource ("ReportSrsTxPower",
                     "Report SRS TxPower in dBm",
                     MakeTraceSourceAccessor (&NrUePowerControl::m_reportSrsTxPower),
                     "ns3::NrUePowerControl::TxPowerTracedCallback");
     return tid;
}


void
NrUePowerControl::SetClosedLoop (bool value)
{
  NS_LOG_FUNCTION (this);
  m_closedLoop = value;
}

void
NrUePowerControl::SetAccumulationEnabled (bool value)
{
  NS_LOG_FUNCTION (this);
  m_accumulationEnabled = value;
}

void
NrUePowerControl::SetAlpha (double value)
{
  NS_LOG_FUNCTION (this);
  uint32_t temp = value * 10;
  switch (temp)
    {
    case 0:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      break;
    default:
      NS_FATAL_ERROR ("Unexpected Alpha value");
    }

  m_alpha = value;
}

void
NrUePowerControl::SetRsrp (double value)
{
  NS_LOG_FUNCTION (this);

  if (!m_rsrpSet)
    {
      m_rsrp = value;
      m_rsrpSet = true;
      return;
    }

  double alphaRsrp = std::pow (0.5, m_pcRsrpFilterCoefficient / 4.0);
  m_rsrp = (1 - alphaRsrp) * m_rsrp + alphaRsrp * value;
  m_pathLoss = m_referenceSignalPower - m_rsrp;
  NS_LOG_INFO ("Pathloss updated to: " << m_pathLoss <<
               " , rsrp updated to:" << m_rsrp << 
               " for cellId/rnti: " << m_cellId << "," << m_rnti);
}

void
NrUePowerControl::SetKPusch (uint16_t value)
{
  NS_LOG_FUNCTION (this);
  m_k_PUSCH = value;
}

void
NrUePowerControl::SetK0Pucch (uint16_t value)
{
  NS_LOG_FUNCTION (this);
  m_k_PUCCH = value;
}

void
NrUePowerControl::SetTechnicalSpec (NrUePowerControl::TechnicalSpec value)
{
  NS_LOG_FUNCTION (this);
  m_technicalSpec = value;
}

void
NrUePowerControl::SetBlCe (bool blCe)
{
  NS_LOG_FUNCTION (this);
  m_blCe = blCe;
}

void
NrUePowerControl::SetP0Srs (double value)
{
  NS_LOG_FUNCTION (this);
  m_P_0_SRS = value;
}

void
NrUePowerControl::SetDeltaTF (double value)
{
  NS_LOG_FUNCTION (this);
  m_deltaTF = value;
}

void
NrUePowerControl::SetDeltaTFControl (double value)
{
  NS_LOG_FUNCTION (this);
  m_deltaTF_control = value;
}

void
NrUePowerControl::SetDeltaFPucch (double value)
{
  NS_LOG_FUNCTION (this);
  m_delta_F_Pucch = value;
}

void
NrUePowerControl::SetPoNominalPucch (int16_t value)
{
  NS_LOG_FUNCTION (this);
  m_PoNominalPucch = value;
}

void
NrUePowerControl::SetPoUePucch (int16_t value)
{
  NS_LOG_FUNCTION (this);
  m_PoUePucch = value;
}

void
NrUePowerControl::SetPcmax (double value)
{
  NS_LOG_FUNCTION (this);
  m_Pcmax = value;
}

void
NrUePowerControl::SetPcmin (double value)
{
  NS_LOG_FUNCTION (this);
  m_Pcmin = value;
}

void
NrUePowerControl::SetPoNominalPusch (int16_t value)
{
  NS_LOG_FUNCTION (this);
  m_PoNominalPusch = value;
}
void
NrUePowerControl::SetPoUePusch (int16_t value)
{
  NS_LOG_FUNCTION (this);
  m_PoUePusch = value;
}

void
NrUePowerControl::SetTxPower (double value)
{
  NS_LOG_FUNCTION (this);
  m_curPuschTxPower = value;
  m_curPucchTxPower = value;
  m_curSrsTxPower = value;
}

void
NrUePowerControl::ConfigureReferenceSignalPower (double value)
{
  NS_LOG_FUNCTION (this);
  m_referenceSignalPower = value;
}

void
NrUePowerControl::ReportTpcPusch (uint8_t tpc)
{
  NS_LOG_FUNCTION (this);

  // if closed loop is not enabled return from this function
  if (!m_closedLoop)
     {
       m_fc = 0;
       m_hc = 0;
       return;
     }

  if (m_accumulationEnabled)
    {
      m_deltaPusch.push_back (GetAccumulatedDelta (tpc));
      NS_LOG_INFO ("Reported TPC: " << (int)tpc << " delta accumulated: " << GetAccumulatedDelta (tpc) << " Fc: " << m_fc);
    }
  else
    {
      m_deltaPusch.push_back (GetAbsoluteDelta (tpc));
      NS_LOG_INFO ("Reported TPC: " << (int)tpc << " delta absolute: " << GetAbsoluteDelta (tpc) << " Fc: " << m_fc);
    }

  /**
   * If m_technicalSpec == TS_38_213 we should only save the
   * deltas, and once that transmission occasion appears then
   * apply the formula that will calculate the new value for
   * m_fc, m_hc and m_gc and reset the stored values, because
   * they are not needed to be saved any more.
   *
   * It technical specification == TS_36_213 we can update
   * immediately between it does not depend on previous
   * occasion and neither on the latest PUSCH time.
   */

  if (m_technicalSpec == TS_36_213)
    {
      // PUSCH power control accumulation or absolute value configuration
      if (m_accumulationEnabled)
        {
          if (m_deltaPusch.size () == m_k_PUSCH) // the feedback/report that should be used is from (i-m_k_PUSCH) report
            {
              if ((m_curPuschTxPower <= m_Pcmin && m_deltaPusch.at (0) < 0)
                  || (m_curPuschTxPower >= m_Pcmax && m_deltaPusch.at (0) > 0))
                {
                  //TPC commands for serving cell shall not be accumulated
                  m_deltaPusch.erase (m_deltaPusch.begin ());
                }
              else
                {
                  m_fc = m_fc + m_deltaPusch.at (0);
                  m_deltaPusch.erase (m_deltaPusch.begin ());
                }
            }
          else
            {
              m_fc = 0;
            }
        }
      else
        { // m_deltaPusch contains absolute values, assign an absolute value
          m_fc = m_deltaPusch.at (0);
          m_deltaPusch.erase (m_deltaPusch.begin ());
        }
    }
  else if (m_technicalSpec == TS_38_213)
    {
      // don't allow infinite accumulation of TPC command if they are maybe not used
      // the maximum number of command that will be saved is 100
      if (m_deltaPusch.size () == 100)
        {
          m_deltaPusch.erase (m_deltaPusch.begin ());
        }
      // update of m_fc and m_hc happens in a separated function, UpdateFc
    }
  else
    {
      NS_FATAL_ERROR ("Unknown technical specification.");
    }
}

int8_t
NrUePowerControl::GetAbsoluteDelta (uint8_t tpc) const
{
  int deltaAbsolute = 0;

  switch (tpc)
  {
    case 0:
      deltaAbsolute = -4;
      break;
    case 1:
      deltaAbsolute = -1;
      break;
    case 2:
      deltaAbsolute = 1;
      break;
    case 3:
      deltaAbsolute = 4;
      break;
    default:
      NS_FATAL_ERROR ("Unexpected TPC value");
  }
  return deltaAbsolute;
}

int8_t
NrUePowerControl::GetAccumulatedDelta (uint8_t tpc) const
{
  int deltaAccumulated = 0;

  switch (tpc)
  {
    case 0:
      deltaAccumulated = -1;
      break;
    case 1:
      deltaAccumulated = 0;
      break;
    case 2:
      deltaAccumulated = 1;
      break;
    case 3:
      deltaAccumulated = 3;
      break;
    default:
      NS_FATAL_ERROR ("Unexpected TPC value");
  }
  return deltaAccumulated;
}

void
NrUePowerControl::ReportTpcPucch (uint8_t tpc)
{
  NS_LOG_FUNCTION (this);

  // if closed loop is not enabled return from this function
  if (!m_closedLoop)
     {
       m_gc = 0;
       return;
     }

  /**
   * According to 36.213 and 38.213 there is only
   * accumulated mode for PUCCH.
   */
  m_deltaPucch.push_back (GetAccumulatedDelta (tpc));

  /**
   * If m_technicalSpec == TS_38_213 we should only save the
   * deltas, and once that transmission occasion appears then
   * apply the formula that will calculate the new value for
   * m_gc and reset the stored values, because
   * they are not needed to be saved any more.
   *
   * It technical specification == TS_36_213 we can update
   * immediately because it does not depend on previous
   * occasion and neither on the latest PUSCH time.
   */

  if (m_technicalSpec == TS_36_213)
    {
      // PUCCH power control accumulation update
      if (m_deltaPucch.size () == m_k_PUCCH) // the feedback/report that should be used is from (i-m_k_PUSCH) report
        {
          if ((m_curPucchTxPower <= m_Pcmin && m_deltaPucch.at (0) < 0) || (m_curPucchTxPower >= m_Pcmax && m_deltaPucch.at (0) > 0))
             {
               //TPC commands for should not be accumulated because the maximum or minimum is reached
               m_deltaPucch.erase (m_deltaPucch.begin ());
              }
           else
             {
                m_gc = m_gc + m_deltaPucch.at (0); // gc(i) = gc (i-1) + delta (i- KPUCCH) for TDD and FDD-TDD TS 36.213
                m_deltaPucch.erase (m_deltaPucch.begin ());
              }
         }
    }
  else if (m_technicalSpec == TS_38_213)
    {
      // don't allow infinite accumulation of TPC command if they are maybe not used
      // the maximum number of command that will be saved is 100
      if (m_deltaPucch.size () == 100)
        {
          m_deltaPucch.erase (m_deltaPucch.begin ());
        }
      // update of m_gc happens in a separated function UpdateGc
    }
  else
    {
      NS_FATAL_ERROR ("Unknown technical specification.");
    }
}

void
NrUePowerControl::SetLoggingInfo (uint16_t cellId, uint16_t rnti)
{
  m_cellId = cellId;
  m_rnti = rnti;
}

void
NrUePowerControl::UpdateFc ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (m_technicalSpec != TS_38_213, "This function is currently being used only for TS 38.213. ");

  // PUSCH power control accumulation or absolute value configuration
  if (m_accumulationEnabled)
    {
      for (const auto& i: m_deltaPusch)
        {
          m_fc += i; // fc already hold value for fc(i-i0) occasion
        }

      m_deltaPusch.clear (); // we have used these values, no need to save them any more
    }
  else
    {
      if (m_deltaPusch.size ()>0)
        {
          m_fc = m_deltaPusch.back ();
          m_deltaPusch.pop_back (); // use the last received absolute TPC command ( 7.1.1 UE behaviour)
          m_deltaPusch.clear ();
        }
    }
}

void
NrUePowerControl::UpdateGc ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (m_technicalSpec != TS_38_213, "This function is currently being used only for TS 38.213. ");

  // PUSCH power control accumulation or absolute value configuration
  for (const auto& i: m_deltaPucch)
    {
      m_gc += i; // gc already hold value for fc(i-i0) occasion
    }

  m_deltaPucch.clear (); // we have used these values, no need to save them any more
}

//TS 38.213 Table 7.1.1-1 and Table 7.2.1-1,  Mapping of TPC Command Field in DCI to accumulated and absolute value

//Implements from from ts_138213 7.1.1
double
NrUePowerControl::CalculatePuschTxPowerNr (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);

  // if BL/CE device
  if (m_blCe && m_technicalSpec == TS_36_213)
    {
      return m_Pcmax;
    }
  int32_t PoPusch = m_PoNominalPusch + m_PoUePusch;

  NS_LOG_INFO ("RBs: " << rbNum <<
               " m_PoPusch: " << PoPusch <<
               " Alpha: " << m_alpha <<
               " PathLoss: " << m_pathLoss <<
               " deltaTF: " << m_deltaTF <<
               " fc: " << m_fc <<
               " numerology:" << m_nrUePhy->GetNumerology ());

  double puschComponent = 0;

  if (rbNum > 0)
    {
      puschComponent = 10 * log10 ( std::pow (2, m_nrUePhy->GetNumerology ()) * rbNum);
    }
  else
    {
      NS_ABORT_MSG ("Should not be called CalculatePuschTxPowerNr if no RBs are assigned.");
    }

  /**
   *  m_pathloss is a downlink path-loss estimate in dB calculated by the UE using
   *  reference signal (RS) index for a DL BWP that is linked with UL BWP b of carrier
   *  f of serving cell c
   *  m_pathloss = referenceSignalPower – higher layer filtered RSRP, where referenceSignalPower is
   *  provided by higher layers and RSRP is defined in [7, TS 38.215] for the reference serving cell and the higher
   *  layer filter configuration is defined in [12, TS 38.331] for the reference serving cell.
   *
   *  By spec. deltaTF is 0 when Ks is 0, and Ks is provided by higher layer parameter deltaMCS
   *  provided for each UL BWP b of each carrier f and serving cell c.
   *  According to 38.213 2.1.1. If the PUSCH transmission is over more than one layer [6, TS 38.214],
   *  then deltaTF is 0.
   *
   *  fc is accumulation or current absolute (calculation by using correction values received in TPC commands)
   */

  /**
   * Depends on the previous occasion timing and on the number
   * of symbols since the last PDCCH, hence it should be updated
   * at the transmission occasion time
   */
  if (m_technicalSpec == TS_38_213)
    {
      UpdateFc ();
    }

  double txPower = PoPusch + puschComponent + m_alpha * m_pathLoss + m_deltaTF + m_fc;

  NS_LOG_INFO ("Calculated PUSCH power:" << txPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  txPower = std::min (std::max (m_Pcmin, txPower), m_Pcmax);

  NS_LOG_INFO ("PUSCH TxPower after min/max constraints: " << txPower << " for cellId/rnti: " << m_cellId << "," << m_rnti);

  return txPower;
}

//Implements from from ts_138213 7.2.1
double
NrUePowerControl::CalculatePucchTxPowerNr (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);

  // if BL/CE device
  if (m_blCe && m_technicalSpec == TS_36_213)
    {
      return m_Pcmax;
    }

  int32_t PoPucch = m_PoNominalPucch + m_PoUePucch;
  double pucchComponent = 0;
  if (rbNum > 0)
    {
      pucchComponent = 10 * log10 ( std::pow (2, m_nrUePhy->GetNumerology ()) * rbNum);
    }
  else
    {
      NS_ABORT_MSG ("Should not be called CalculatePuschTxPowerNr if no RBs are assigned.");
    }

  /**
   *  - m_pathloss is a downlink path-loss estimate in dB calculated by the UE using
   *  reference signal (RS) index for a DL BWP that is linked with UL BWP b of carrier
   *  f of serving cell c
   *  m_pathloss = referenceSignalPower – higher layer filtered RSRP, where referenceSignalPower is
   *  provided by higher layers and RSRP is defined in [7, TS 38.215] for the reference serving cell and the higher
   *  layer filter configuration is defined in [12, TS 38.331] for the reference serving cell.
   *
   *  m_delta_F_Pucch value corresponds to a PUCCH format. It is provided by higher layers
   *  through deltaF-PUCCH-f0 for PUCCH format 0, deltaF-PUCCH-f1 for PUCCH format 1,
   *  deltaF-PUCCH-f2 for PUCCH format 2, deltaF-PUCCH-f3 for PUCCH format 3, and
   *  deltaF-PUCCH-f4 for PUCCH format 4.
   *
   *  m_deltaTF_control is a PUCCH transmission power adjustment component
   *
   *  m_gc is equal to 0 if PO_PUCCH value is provided by higher layers. Currently is
   *  calculated in the same way as m_fc for PUSCH
   */

  /**
   * Depends on the previous occasion timing and on the number
   * of symbols since the last PDCCH, hence it should be updated
   * at the transmission occasion time.
   */
  if (m_technicalSpec == TS_38_213)
    {
      UpdateGc ();
    }

  NS_LOG_INFO ("RBs: " << rbNum <<
               " m_PoPucch: " << PoPucch <<
               " Alpha: " << m_alpha <<
               " PathLoss: " << m_pathLoss <<
               " deltaTF: " << m_deltaTF_control <<
               " gc: " << m_gc <<
               " numerology: " << m_nrUePhy->GetNumerology ());

  double txPower = PoPucch + pucchComponent + m_alpha * m_pathLoss + m_delta_F_Pucch + m_deltaTF_control +  m_gc;

  NS_LOG_INFO ("Calculated PUCCH power: " << txPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  txPower = std::min (std::max (m_Pcmin, txPower), m_Pcmax);

  NS_LOG_INFO ("PUCCH TxPower after min/max constraints: " << txPower << " for cellId/rnti: " << m_cellId << "," << m_rnti);

  return txPower;
}

double
NrUePowerControl::CalculateSrsTxPowerNr (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);
  int32_t PoPusch = m_PoNominalPusch + m_PoUePusch;

  NS_LOG_INFO ("RBs: " << rbNum <<
               " m_PoPusch: " << PoPusch <<
               " Alpha: " << m_alpha <<
               " PathLoss: " << m_pathLoss <<
               " deltaTF: " << m_deltaTF <<
               " fc: " << m_fc);

  /*
   * According to TS 36.213, 5.1.3.1, alpha can be the same alpha as for PUSCH,
   * P0_SRS can be used P0_PUSCH, and m_hc (accumulation state) is equal to m_fc.
   *
   * Also, as per TS 38.213. 7.3.1, the latest m_fc value ( PUSCH power
   * control adjustment state) as described in Subclause 7.1.1, if higher layer parameter
   * srs-PowerControlAdjustmentStates indicates a same power control adjustment state for
   * SRS transmissions and PUSCH transmissions
   */
  m_hc = m_fc;
  double txPower = 0;
  double component = 0;

  if (rbNum > 0)
    {
      component = 10 * log10 (std::pow (2, m_nrUePhy->GetNumerology ()) * rbNum);
    }
  else
    {
      NS_ABORT_MSG ("Should not be called CalculateSrsTxPowerNr if no RBs are assigned.");
    }

  if (m_technicalSpec == TS_36_213)
    {
      double pSrsOffsetValue = -10.5 + m_PsrsOffset * 1.5;
      txPower = pSrsOffsetValue + component + PoPusch + m_alpha * m_pathLoss + m_hc;
    }
  else if (m_technicalSpec == TS_38_213)
    {
      txPower = m_P_0_SRS + component +  m_alpha * m_pathLoss + m_hc;  // this formula also can apply for TS_36_213,
                                                                       //See 5.1.3 Sounding Reference Symbol (SRS) 5.1.3.1 UE behavior
    }

  NS_LOG_INFO ("CalcPower: " << txPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  txPower = std::min (std::max (m_Pcmin, txPower), m_Pcmax);

  NS_LOG_INFO ("SrsTxPower after min/max constraints: " << m_curSrsTxPower << " for cellId/rnti: "<< m_cellId << "," << m_rnti);

  return txPower;
}

double
NrUePowerControl::GetPuschTxPower (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);
  m_curPuschTxPower = CalculatePuschTxPowerNr (rbNum);
  m_reportPuschTxPower (m_nrUePhy->GetCellId (), m_nrUePhy->GetRnti (), m_curPuschTxPower);
  return m_curPuschTxPower;
}

double
NrUePowerControl::GetPucchTxPower (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);
  m_curPucchTxPower = CalculatePucchTxPowerNr (rbNum);
  m_reportPucchTxPower (m_nrUePhy->GetCellId (), m_nrUePhy->GetRnti (), m_curPucchTxPower);
  return m_curPucchTxPower;
}

double
NrUePowerControl::GetSrsTxPower (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);
  m_curSrsTxPower = CalculateSrsTxPowerNr (rbNum);
  m_reportSrsTxPower (m_nrUePhy->GetCellId (), m_nrUePhy->GetRnti (), m_curSrsTxPower);
  return m_curSrsTxPower;
}


} // namespace ns3
