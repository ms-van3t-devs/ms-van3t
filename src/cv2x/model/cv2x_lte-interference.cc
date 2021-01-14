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
 */


#include "cv2x_lte-interference.h"
#include "cv2x_lte-chunk-processor.h"

#include <ns3/simulator.h>
#include <ns3/log.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteInterference");

cv2x_LteInterference::cv2x_LteInterference ()
  : m_receiving (false),
    m_lastSignalId (0),
    m_lastSignalIdBeforeReset (0)
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteInterference::~cv2x_LteInterference ()
{
  NS_LOG_FUNCTION (this);
}

void 
cv2x_LteInterference::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_rsPowerChunkProcessorList.clear ();
  m_sinrChunkProcessorList.clear ();
  m_interfChunkProcessorList.clear ();
  m_rxSignal = 0;
  m_allSignals = 0;
  m_noise = 0;
  Object::DoDispose ();
} 


TypeId
cv2x_LteInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteInterference")
    .SetParent<Object> ()
    .SetGroupName("Lte")
  ;
  return tid;
}


void
cv2x_LteInterference::StartRx (Ptr<const SpectrumValue> rxPsd)
{ 
  NS_LOG_FUNCTION (this << *rxPsd);
  if (m_receiving == false)
    {
      NS_LOG_LOGIC ("first signal");
      m_rxSignal = rxPsd->Copy ();
      m_lastChangeTime = Now ();
      m_receiving = true;
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
        {
          (*it)->Start ();
        }
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
        {
          (*it)->Start ();
        }
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->Start (); 
        }
    }
  else
    {
      NS_LOG_LOGIC ("additional signal" << *m_rxSignal);
      // receiving multiple simultaneous signals, make sure they are synchronized
      NS_ASSERT (m_lastChangeTime == Now ());
      // make sure they use orthogonal resource blocks
      NS_ASSERT (Sum ((*rxPsd) * (*m_rxSignal)) == 0.0);
      (*m_rxSignal) += (*rxPsd);
    }
}


void
cv2x_LteInterference::EndRx ()
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
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
        {
          (*it)->End ();
        }
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
        {
          (*it)->End ();
        }
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->End (); 
        }
    }
}


void
cv2x_LteInterference::AddSignal (Ptr<const SpectrumValue> spd, const Time duration)
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
  Simulator::Schedule (duration, &cv2x_LteInterference::DoSubtractSignal, this, spd, signalId);
}


void
cv2x_LteInterference::DoAddSignal  (Ptr<const SpectrumValue> spd)
{ 
  NS_LOG_FUNCTION (this << *spd);
  ConditionallyEvaluateChunk ();
  (*m_allSignals) += (*spd);
}

void
cv2x_LteInterference::DoSubtractSignal  (Ptr<const SpectrumValue> spd, uint32_t signalId)
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
cv2x_LteInterference::ConditionallyEvaluateChunk ()
{
  NS_LOG_FUNCTION (this);
  if (m_receiving)
    {
      NS_LOG_DEBUG (this << " Receiving");
    }
  NS_LOG_DEBUG (this << " now "  << Now () << " last " << m_lastChangeTime);
  if (m_receiving && (Now () > m_lastChangeTime))
    {
      NS_LOG_LOGIC (this << " signal = " << *m_rxSignal << " allSignals = " << *m_allSignals << " noise = " << *m_noise);

      SpectrumValue interf =  (*m_allSignals) - (*m_rxSignal) + (*m_noise);

      SpectrumValue sinr = (*m_rxSignal) / interf;
      Time duration = Now () - m_lastChangeTime;
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->EvaluateChunk (sinr, duration);
        }
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
        {
          (*it)->EvaluateChunk (interf, duration);
        }
      for (std::list<Ptr<cv2x_LteChunkProcessor> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
        {
          (*it)->EvaluateChunk (*m_rxSignal, duration);
        }
      m_lastChangeTime = Now ();
    }
}

void
cv2x_LteInterference::SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd)
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
cv2x_LteInterference::AddRsPowerChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  NS_LOG_FUNCTION (this << p);
  m_rsPowerChunkProcessorList.push_back (p);
}

void
cv2x_LteInterference::AddSinrChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  NS_LOG_FUNCTION (this << p);
  m_sinrChunkProcessorList.push_back (p);
}

void
cv2x_LteInterference::AddInterferenceChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  NS_LOG_FUNCTION (this << p);
  m_interfChunkProcessorList.push_back (p);
}




} // namespace ns3


