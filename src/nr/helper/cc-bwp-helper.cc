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

#include "cc-bwp-helper.h"
#include <ns3/log.h>
#include <memory>
#include <fstream>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/spectrum-channel.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CcBwpHelper");

bool
ComponentCarrierInfo::AddBwp (BandwidthPartInfoPtr &&bwp)
{
  NS_LOG_FUNCTION (this);

  bool ret = true;

  NS_ASSERT (bwp->m_lowerFrequency >= m_lowerFrequency);
  NS_ASSERT (bwp->m_higherFrequency <= m_higherFrequency);

  m_bwp.emplace_back (std::move (bwp));

  uint32_t i = 0;
  while (i < m_bwp.size () - 1)
    {
      auto & bwp = m_bwp.at (i);
      auto & nextBwp = m_bwp.at (i + 1);
      if (bwp->m_higherFrequency > nextBwp->m_lowerFrequency)
        {
          NS_LOG_ERROR ("BWP ID " << +bwp->m_bwpId << " has higher freq = " <<
                        bwp->m_higherFrequency / 1e6 << "MHz  while BWP ID " <<
                        +nextBwp->m_bwpId << " has lower freq = " << nextBwp->m_lowerFrequency / 1e6 <<
                        " MHz.");
          ret = false;
        }
      ++i;
    }

  for (auto & bwp : m_bwp)
  {
      NS_LOG_INFO ("Create BWP with bwpId: " << +bwp->m_bwpId << " lower: " <<
                   bwp->m_lowerFrequency / 1e6 << " with central freq: " <<
                   bwp->m_centralFrequency / 1e6 << " higher: " <<
                   bwp->m_higherFrequency/ 1e6 << " BW: " <<
                   bwp->m_channelBandwidth  / 1e6 << " MHz");
  }

  return ret;
}


bool
OperationBandInfo::AddCc (ComponentCarrierInfoPtr &&cc)
{
  NS_LOG_FUNCTION (this);
  bool ret = true;

  NS_ASSERT (cc->m_lowerFrequency >= m_lowerFrequency);
  NS_ASSERT (cc->m_higherFrequency <= m_higherFrequency);

  m_cc.emplace_back (std::move (cc));

  uint32_t i = 0;
  while (i < m_cc.size () - 1)
    {
      auto & cc = m_cc.at (i);
      auto & nextCc = m_cc.at (i + 1);
      if (cc->m_higherFrequency > nextCc->m_lowerFrequency)
        {
          NS_LOG_WARN ("Cc at " << i << " has higher freq " << cc->m_higherFrequency / 1e6 <<
                       " while Cc at " << i + 1 << " has freq at " << m_lowerFrequency / 1e6);
          ret = false;
        }
      ++i;
    }

  for (auto & cc : m_cc)
  {
      NS_LOG_INFO ("Create CC with ccId: " << +cc->m_ccId << " lower: " <<
                   cc->m_lowerFrequency / 1e6 << " with central freq: " <<
                   cc->m_centralFrequency / 1e6 << " higher: " <<
                   cc->m_higherFrequency/ 1e6 << " BW: " <<
                   cc->m_channelBandwidth / 1e6 << " MHz");
  }

  return ret;
}

BandwidthPartInfoPtr &
OperationBandInfo::GetBwpAt (uint32_t ccId, uint32_t bwpId) const
{
  return m_cc.at (ccId)->m_bwp.at (bwpId);
}

BandwidthPartInfoPtrVector
OperationBandInfo::GetBwps() const
{
  std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> ret;

  for (const auto & cc : m_cc)
    {
      for (auto & bwp : cc->m_bwp)
        {
          ret.push_back (bwp);
        }
    }

  return ret;
}


// cc is a unique_pointer... I'm allowing myself to use a reference, because
// if I would use a pointer (like I was telling you to do) then we end up
// with a pointer to a pointer, and we have to use (*cc)->m_centralFreq...
// a bit useless here.
void
CcBwpCreator::InitializeCc (std::unique_ptr<ComponentCarrierInfo> &cc,
                            double ccBandwidth, double lowerFreq, uint8_t ccPosition,
                            uint8_t ccId) const
{
  NS_LOG_FUNCTION (this);
  cc->m_centralFrequency = lowerFreq + ccPosition * ccBandwidth + ccBandwidth / 2;
  cc->m_lowerFrequency = lowerFreq + ccPosition * ccBandwidth;
  cc->m_higherFrequency = lowerFreq + (ccPosition + 1) * ccBandwidth - 1;
  cc->m_channelBandwidth = ccBandwidth;
  cc->m_ccId = ccId;
  NS_LOG_INFO ("Initialize the op band " << +ccPosition << "st (or nd) CC of BW " <<
               ccBandwidth/1e6 << " MHz " << " from " << lowerFreq/1e6 <<
               "MHz, resulting in: " << *cc);
}

void
CcBwpCreator::InitializeBwp (std::unique_ptr<BandwidthPartInfo> &bwp,
                             double bwOfBwp, double lowerFreq, uint8_t bwpPosition,
                             uint8_t bwpId) const
{
  NS_LOG_FUNCTION (this);
  bwp->m_centralFrequency = lowerFreq + bwpPosition * bwOfBwp + bwOfBwp / 2;
  bwp->m_lowerFrequency = lowerFreq + bwpPosition * bwOfBwp;
  bwp->m_higherFrequency = lowerFreq + (bwpPosition + 1) * bwOfBwp - 1;
  bwp->m_channelBandwidth = bwOfBwp;
  bwp->m_bwpId = bwpId;
  NS_LOG_INFO ("Initialize the " << +bwpPosition << "st (or nd) BWP of BW " <<
               bwOfBwp/1e6 << " MHz, from " << lowerFreq/1e6 <<
               "MHz, resulting in: " << *bwp);
}

std::unique_ptr<ComponentCarrierInfo>
CcBwpCreator::CreateCc (double ccBandwidth, double lowerFreq, uint8_t ccPosition,
                        uint8_t ccId, uint8_t bwpNumber, BandwidthPartInfo::Scenario scenario)
{
  // Create a CC with a single BWP
  std::unique_ptr<ComponentCarrierInfo> cc (new ComponentCarrierInfo ());
  InitializeCc (cc, ccBandwidth, lowerFreq, ccPosition, ccId);

  double bwpBandwidth = ccBandwidth / bwpNumber;

  for (uint8_t i = 0; i < bwpNumber; ++i)
    {
      std::unique_ptr<BandwidthPartInfo> bwp (new BandwidthPartInfo ());
      InitializeBwp (bwp, bwpBandwidth, cc->m_lowerFrequency, i, m_bandwidthPartCounter++);
      bwp->m_scenario = scenario;
      bool ret = cc->AddBwp (std::move (bwp));
      NS_ASSERT (ret);
    }

  // bwp is not longer a valid pointer
  return cc;
}

OperationBandInfo
CcBwpCreator::CreateOperationBandContiguousCc (const SimpleOperationBandConf &conf)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Creating an op band formed by " << +conf.m_numCc << " contingous CC" <<
               " central freq " << conf.m_centralFrequency/1e6 << " MHz with BW " <<
               conf.m_channelBandwidth/1e6 << " MHz");

  OperationBandInfo band;
  band.m_bandId = m_operationBandCounter++;
  band.m_centralFrequency = conf.m_centralFrequency;
  band.m_channelBandwidth = conf.m_channelBandwidth;
  band.m_lowerFrequency = band.m_centralFrequency - (conf.m_channelBandwidth / 2.0);
  band.m_higherFrequency = band.m_centralFrequency + (conf.m_channelBandwidth / 2.0);

  NS_LOG_INFO ("Resulting OpBand: " << band);

  uint32_t maxCcBandwidth = 198e6;

  if (conf.m_centralFrequency > 6e9)
    {
      maxCcBandwidth = 396e6;
    }

  double ccBandwidth = std::min (static_cast<double> (maxCcBandwidth),
                                 static_cast<double> (conf.m_channelBandwidth) / conf.m_numCc);

  for (uint8_t ccPosition = 0; ccPosition < conf.m_numCc; ++ccPosition)
    {
      bool ret = band.AddCc (CreateCc (ccBandwidth, band.m_lowerFrequency,
                                       ccPosition, m_componentCarrierCounter++,
                                       conf.m_numBwp, conf.m_scenario));
      NS_ASSERT (ret);
    }

  NS_ASSERT (band.m_cc.size () == conf.m_numCc);
  return band;
}

OperationBandInfo
CcBwpCreator::CreateOperationBandNonContiguousCc (const std::vector<SimpleOperationBandConf> &configuration)
{
  OperationBandInfo band;
  band.m_bandId = m_operationBandCounter++;

  for (const auto & conf : configuration)
    {
      NS_ASSERT (conf.m_numBwp == 1);
      band.AddCc (CreateCc (conf.m_channelBandwidth, band.m_lowerFrequency,
                            0, m_componentCarrierCounter++, conf.m_numBwp, conf.m_scenario));
    }

  return band;
}

BandwidthPartInfoPtrVector
CcBwpCreator::GetAllBwps(const std::vector<std::reference_wrapper<OperationBandInfo>> &operationBands)
{
  BandwidthPartInfoPtrVector ret;

  for (const auto & operationBand : operationBands)
    {
      auto v = operationBand.get().GetBwps ();
      ret.insert (ret.end (),
                  std::make_move_iterator(v.begin ()),
                  std::make_move_iterator(v.end ()));
    }

  return ret;
}

void
CcBwpCreator::PlotNrCaBwpConfiguration (const std::vector<OperationBandInfo *> &bands,
                                        const std::string &filename)
{

  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  // FIXME: I think I can do this with calling the gnuclass in ns3 by calling
  //        plot.AppendExtra (whatever gnu line sting) (see gnuplot documantation
  //        in ns3

  // Set the range for the x axis.
  double minFreq = 100e9;
  double maxFreq = 0;
  for (const auto & band : bands)
    {
      if (band->m_lowerFrequency < minFreq)
        {
          minFreq = band->m_lowerFrequency;
        }
      if (band->m_higherFrequency > maxFreq)
        {
          maxFreq = band->m_higherFrequency;
        }
    }

  outFile << "set term eps" << std::endl;
  outFile << "set output \"" << filename << ".eps\"" << std::endl;
  outFile << "set grid" << std::endl;

  outFile << "set xrange [";
  outFile << minFreq * 1e-6 - 1;
  outFile << ":";
  outFile << maxFreq * 1e-6 + 1;
  outFile << "]";
  outFile << std::endl;

  outFile << "set yrange [1:100]" << std::endl;
  outFile << "set xlabel \"f [MHz]\"" << std::endl;

  uint16_t index = 1;   //<! Index must be larger than zero for gnuplot
  for (const auto & band : bands)
    {
      std::string label = "n";
      uint16_t bandId = static_cast<uint16_t> (band->m_bandId);
      label += std::to_string (bandId);
      PlotFrequencyBand (outFile, index, band->m_lowerFrequency * 1e-6, band->m_higherFrequency * 1e-6,
                         70, 90, label);
      index++;
      for (uint32_t i = 0; i < band->m_cc.size (); ++i)
        {
          const auto & cc = band->m_cc.at (i);
          uint16_t ccId = static_cast<uint16_t> (cc->m_ccId);
          label = "CC" + std::to_string (ccId);
          PlotFrequencyBand (outFile, index, cc->m_lowerFrequency * 1e-6, cc->m_higherFrequency * 1e-6,
                             40, 60, label);
          index++;
          for (uint32_t j = 0; j < cc->m_bwp.size (); ++j)
            {
              const auto & bwp = cc->m_bwp.at (j);
              uint16_t bwpId = static_cast<uint16_t> (bwp->m_bwpId);
              label = "BWP" + std::to_string (bwpId);
              PlotFrequencyBand (outFile, index, bwp->m_lowerFrequency * 1e-6, bwp->m_higherFrequency * 1e-6,
                                 10, 30, label);
              index++;
            }
        }
    }

  outFile << "unset key" << std::endl;
  outFile << "plot -x" << std::endl;

}

void
CcBwpCreator::PlotLteCaConfiguration (const std::vector<OperationBandInfo *> &bands,
                                      const std::string &filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  // FIXME: I think I can do this with calling the gnuclass in ns3 and use
  //        plot.AppendExtra (whatever sting);

  double minFreq = 100e9;
  double maxFreq = 0;
  for (const auto & band : bands)
    {
      if (band->m_lowerFrequency < minFreq)
        {
          minFreq = band->m_lowerFrequency;
        }
      if (band->m_higherFrequency > maxFreq)
        {
          maxFreq = band->m_higherFrequency;
        }
    }

  outFile << "set term eps" << std::endl;
  outFile << "set output \"" << filename << ".eps\"" << std::endl;
  outFile << "set grid" << std::endl;

  outFile << "set xrange [";
  outFile << minFreq * 1e-6 - 1;
  outFile << ":";
  outFile << maxFreq * 1e-6 + 1;
  outFile << "]";
  outFile << std::endl;

  outFile << "set yrange [1:100]" << std::endl;
  outFile << "set xlabel \"f [MHz]\"" << std::endl;

  uint16_t index = 1;   //<! Index must be larger than zero for gnuplot
  for (const auto & band : bands)
    {
      std::string label = "n";
      uint16_t bandId = static_cast<uint16_t> (band->m_bandId);
      label += std::to_string (bandId);
      PlotFrequencyBand (outFile, index, band->m_lowerFrequency * 1e-6, band->m_higherFrequency * 1e-6,
                         70, 90, label);
      index++;
      for (uint32_t i = 0; i < band->m_cc.size (); ++i)
        {
          const auto & cc = band->m_cc.at (i);
          uint16_t ccId = static_cast<uint16_t> (cc->m_ccId);
          label = "CC" + std::to_string (ccId);
          PlotFrequencyBand (outFile, index, cc->m_lowerFrequency * 1e-6, cc->m_higherFrequency * 1e-6,
                             40, 60, label);
          index++;
        }
    }

  outFile << "unset key" << std::endl;
  outFile << "plot -x" << std::endl;

}

void
CcBwpCreator::PlotFrequencyBand (std::ofstream &outFile,
                                 uint16_t index,
                                 double xmin,
                                 double xmax,
                                 double ymin,
                                 double ymax,
                                 const std::string &label)
{
  outFile << "set object " << index << " rect from " << xmin  << "," << ymin <<
    " to "   << xmax  << "," << ymax << " front fs empty " << std::endl;

  outFile << "LABEL" << index << " = \"" << label << "\"" << std::endl;

  outFile << "set label " << index << " at " << xmin << "," <<
    (ymin + ymax) / 2 << " LABEL" << index << std::endl;

}

std::string
BandwidthPartInfo::GetScenario () const
{
  NS_LOG_FUNCTION (this);
  static std::unordered_map <Scenario, std::string, std::hash<int>> lookupTable
  {
    { RMa, "RMa"},
    { RMa_LoS, "RMa"},
    { RMa_nLoS, "RMa"},
    { UMa, "UMa" },
    { UMa_LoS, "UMa" },
    { UMa_nLoS, "UMa" },
    { UMi_StreetCanyon, "UMi-StreetCanyon" },
    { UMi_StreetCanyon_LoS, "UMi-StreetCanyon" },
    { UMi_StreetCanyon_nLoS, "UMi-StreetCanyon" },
    { InH_OfficeOpen, "InH-OfficeOpen" },
    { InH_OfficeOpen_LoS, "InH-OfficeOpen" },
    { InH_OfficeOpen_nLoS, "InH-OfficeOpen" },
    { InH_OfficeMixed, "InH-OfficeMixed" },
    { InH_OfficeMixed_LoS, "InH-OfficeMixed" },
    { InH_OfficeMixed_nLoS, "InH-OfficeMixed" },
    { UMa_Buildings, "UMa" },
    { UMi_Buildings, "UMi-StreetCanyon" },
    { V2V_Highway, "V2V-Highway" },
    { V2V_Urban, "V2V-Urban" },
  };

  return lookupTable[m_scenario];
}

std::ostream &
operator<< (std::ostream & os, ComponentCarrierInfo const & item)
{
  os << "id: " << +item.m_ccId << " lower freq " << item.m_lowerFrequency/1e6 <<
        " MHz central freq " << item.m_centralFrequency/1e6 << " MHz higher freq " <<
        item.m_higherFrequency/1e6 << " MHz bw " << item.m_channelBandwidth/1e6 << " MHz." << std::endl;
  for (const auto & bwp : item.m_bwp)
    {
      os << "\t\t" << *bwp << std::endl;
    }
  return os;
}

std::ostream &
operator<< (std::ostream & os, OperationBandInfo const & item)
{
  os << "id: " << +item.m_bandId << " lower freq " << item.m_lowerFrequency/1e6 <<
        " MHz central freq " << item.m_centralFrequency/1e6 << " MHz higher freq " <<
        item.m_higherFrequency/1e6 << " MHz bw " << item.m_channelBandwidth/1e6 << " MHz." << std::endl;
  for (const auto & cc : item.m_cc)
    {
      os << "\t" << *cc << std::endl;
    }
  return os;
}

std::ostream &
operator<< (std::ostream & os, BandwidthPartInfo const & item)
{
  os << "id: " << +item.m_bwpId << " lower freq " << item.m_lowerFrequency/1e6 <<
        " MHz central freq " << item.m_centralFrequency/1e6 << " MHz higher freq " <<
        item.m_higherFrequency/1e6 << " MHz bw " << item.m_channelBandwidth/1e6 << " MHz.";
  return os;
}

}
