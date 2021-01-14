/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by : Marco Miozzo <mmiozzo@cttc.es>
 *        (move from CQI to Ctrl and Data SINR Chunk processors
 * Modified by: NIST
 */


#include <ns3/log.h>
#include <ns3/spectrum-value.h>
#include "cv2x_lte-sl-chunk-processor.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteSlChunkProcessor");

cv2x_LteSlChunkProcessor::cv2x_LteSlChunkProcessor ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteSlChunkProcessor::~cv2x_LteSlChunkProcessor ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteSlChunkProcessor::AddCallback (cv2x_LteSlChunkProcessorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_lteChunkProcessorCallbacks.push_back (c);
}

void
cv2x_LteSlChunkProcessor::Start (bool init)
{
  NS_LOG_FUNCTION (this);

  if (init)
    {
      m_chunkValues.clear ();
    }
  //creates a new storage 
  cv2x_LteSlChunkValue newValue;
  newValue.m_sumValues = 0;
  newValue.m_totDuration = MicroSeconds (0);

  m_chunkValues.push_back (newValue);
}


void
cv2x_LteSlChunkProcessor::EvaluateChunk (uint32_t index, const SpectrumValue& sinr, Time duration)
{
  NS_LOG_FUNCTION (this << index << sinr << duration);
  if (m_chunkValues[index].m_sumValues == 0)
    {
      m_chunkValues[index].m_sumValues = Create<SpectrumValue> (sinr.GetSpectrumModel ());
    }
  *(m_chunkValues[index].m_sumValues) += sinr * duration.GetSeconds ();
  m_chunkValues[index].m_totDuration += duration;
}

void
cv2x_LteSlChunkProcessor::End ()
{
  NS_LOG_FUNCTION (this);

  if (m_chunkValues[0].m_totDuration.GetSeconds () > 0)
    {
      std::vector<SpectrumValue> values;
      std::vector<cv2x_LteSlChunkValue>::iterator itValues;
      for (itValues = m_chunkValues.begin() ; itValues != m_chunkValues.end () ; itValues++)
        {
          values.push_back (*((*itValues).m_sumValues) / (*itValues).m_totDuration.GetSeconds ());
        }

      std::vector<cv2x_LteSlChunkProcessorCallback>::iterator it;
      for (it = m_lteChunkProcessorCallbacks.begin (); it != m_lteChunkProcessorCallbacks.end (); it++)
        {
          (*it)(values);
        }
    }
  else
    {
      NS_LOG_WARN ("m_numSinr == 0");
    }
}


} // namespace ns3
