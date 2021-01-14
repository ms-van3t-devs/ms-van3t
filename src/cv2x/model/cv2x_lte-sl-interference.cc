/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
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
 * Modified by: NIST
 */


#include "cv2x_lte-sl-interference.h"
#include "cv2x_lte-sl-chunk-processor.h"

#include <ns3/simulator.h>
#include <ns3/log.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteSlInterference");

cv2x_LteSlInterference::cv2x_LteSlInterference ()
  : m_receiving (false),
    m_lastSignalId (0),
    m_lastSignalIdBeforeReset (0)
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteSlInterference::~cv2x_LteSlInterference ()
{
  NS_LOG_FUNCTION (this);
}

void 
cv2x_LteSlInterference::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_rsPowerChunkProcessorList.clear ();
  m_sinrChunkProcessorList.clear ();
  m_interfChunkProcessorList.clear ();
  m_rxSignal.clear();
  m_allSignals = 0;
  m_noise = 0;
  Object::DoDispose ();
} 


TypeId
cv2x_LteSlInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteSlInterference")
    .SetParent<Object> ()
  ;
  return tid;
}


void
cv2x_LteSlInterference::StartRx (Ptr<const SpectrumValue> rxPsd)
{ 
  NS_LOG_FUNCTION (this << *rxPsd);
  bool init = !m_receiving;

  if (m_receiving == false) {
    NS_LOG_LOGIC ("first signal");//Still check that receiving multiple simultaneous signals, make sure they are synchronized
    m_rxSignal.clear ();
    m_receiving = true;
  } else {
    NS_LOG_LOGIC ("additional signal (Nb simulateanous Rx = " << m_rxSignal.size() << ")");
    NS_ASSERT (m_lastChangeTime == Now ());
  }

  //In sidelink, each packet must be monitor seperatly
  m_rxSignal.push_back (rxPsd->Copy ());
  m_lastChangeTime = Now ();
  
  //trigger the initialization of each chunk processor 
  for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
    {
      (*it)->Start (init);
    }
  for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
    {
      (*it)->Start (init);
    }
  for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
    {
      (*it)->Start (init); 
    }
}

void
cv2x_LteSlInterference::EndRx ()
{
  NS_LOG_FUNCTION (this);
  if (m_receiving != true)
    {
      NS_LOG_INFO ("EndRx was already evaluated or RX was aborted");
    }
  else
    {
      ConditionallyEvaluateChunk ();
      m_receiving = false;
      for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
        {
          (*it)->End ();
        }
      for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
        {
          (*it)->End ();
        }
      for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->End (); 
        }
    }
}


void
cv2x_LteSlInterference::AddSignal (Ptr<const SpectrumValue> spd, const Time duration)
{
  NS_LOG_FUNCTION (this << *spd << duration);
  DoAddSignal (spd);
  uint32_t signalId = ++m_lastSignalId;
  if (signalId == m_lastSignalIdBeforeReset)
    {
      // This happens when m_lastSignalId eventually wraps around. Given that so
      // many signals have elapsed since the last reset, we hope that by now there is
      // no stale pending signal (i.e., a signal that was scheduled
      // for subtraction before the reset). So we just move the
      // boundary further.
      m_lastSignalIdBeforeReset += 0x10000000;
    }
  Simulator::Schedule (duration, &cv2x_LteSlInterference::DoSubtractSignal, this, spd, signalId);
}


void
cv2x_LteSlInterference::DoAddSignal  (Ptr<const SpectrumValue> spd)
{ 
  NS_LOG_FUNCTION (this << *spd);
  ConditionallyEvaluateChunk ();
  (*m_allSignals) += (*spd);
}

void
cv2x_LteSlInterference::DoSubtractSignal  (Ptr<const SpectrumValue> spd, uint32_t signalId)
{ 
  NS_LOG_FUNCTION (this << *spd);
  ConditionallyEvaluateChunk ();   
  int32_t deltaSignalId = signalId - m_lastSignalIdBeforeReset;
  if (deltaSignalId > 0)
    {   
      (*m_allSignals) -= (*spd);
    }
  else
    {
      NS_LOG_INFO ("ignoring signal scheduled for subtraction before last reset");
    }
}


void
cv2x_LteSlInterference::ConditionallyEvaluateChunk ()
{
  NS_LOG_FUNCTION (this);
  if (m_receiving)
    {
      NS_LOG_DEBUG (this << " Receiving");
    }
  NS_LOG_DEBUG (this << " now "  << Now () << " last " << m_lastChangeTime);
  if (m_receiving && (Now () > m_lastChangeTime))
    {
      //compute values for each signal being received
      for (uint32_t index = 0 ; index < m_rxSignal.size() ; index++)
        {
          NS_LOG_LOGIC (this << " signal = " << *(m_rxSignal[index]) << " allSignals = " << *m_allSignals << " noise = " << *m_noise);
          
          SpectrumValue interf =  (*m_allSignals) - (*(m_rxSignal[index])) + (*m_noise);
          
          SpectrumValue sinr = (*(m_rxSignal[index])) / interf;
          Time duration = Now () - m_lastChangeTime;
          for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
            {
              (*it)->EvaluateChunk (index, sinr, duration);
            }
          for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
            {
              (*it)->EvaluateChunk (index, interf, duration);
            }
          for (std::list<Ptr<cv2x_LteSlChunkProcessor> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
            {
              (*it)->EvaluateChunk (index, *(m_rxSignal[index]), duration);
            }
        }
      m_lastChangeTime = Now ();
    }
}

void
cv2x_LteSlInterference::SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd)
{
  NS_LOG_FUNCTION (this << *noisePsd);
  ConditionallyEvaluateChunk ();
  m_noise = noisePsd;
  // reset m_allSignals (will reset if already set previously)
  // this is needed since this method can potentially change the SpectrumModel
  m_allSignals = Create<SpectrumValue> (noisePsd->GetSpectrumModel ());
  if (m_receiving == true)
    {
      // abort rx
      m_receiving = false;
    }
  // record the last SignalId so that we can ignore all signals that
  // were scheduled for subtraction before m_allSignal 
  m_lastSignalIdBeforeReset = m_lastSignalId;
}

void
cv2x_LteSlInterference::AddRsPowerChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p)
{
  NS_LOG_FUNCTION (this << p);
  m_rsPowerChunkProcessorList.push_back (p);
}

void
cv2x_LteSlInterference::AddSinrChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p)
{
  NS_LOG_FUNCTION (this << p);
  m_sinrChunkProcessorList.push_back (p);
}

void
cv2x_LteSlInterference::AddInterferenceChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p)
{
  NS_LOG_FUNCTION (this << p);
  m_interfChunkProcessorList.push_back (p);
}




} // namespace ns3


