/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Marco Miozzo  <marco.miozzo@cttc.es>
 * Modified by: NIST (D2D)
 */


#include <ns3/cv2x_lte-harq-phy.h>
#include <ns3/log.h>
#include <ns3/assert.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteHarqPhy");

//NS_OBJECT_ENSURE_REGISTERED (cv2x_LteHarqPhy)
//  ;


cv2x_LteHarqPhy::cv2x_LteHarqPhy ()
{
  // Create DL Decodification HARQ buffers
  std::vector <HarqProcessInfoList_t> dlHarqLayer0;
  dlHarqLayer0.resize (8);
  std::vector <HarqProcessInfoList_t> dlHarqLayer1;
  dlHarqLayer1.resize (8);
  m_miDlHarqProcessesInfoMap.push_back (dlHarqLayer0);
  m_miDlHarqProcessesInfoMap.push_back (dlHarqLayer1);
}


cv2x_LteHarqPhy::~cv2x_LteHarqPhy ()
{
  m_miDlHarqProcessesInfoMap.clear ();
  m_miUlHarqProcessesInfoMap.clear ();
  m_miSlHarqProcessesInfoMap.clear ();
}


void
cv2x_LteHarqPhy::SubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this);

  // left shift UL HARQ buffers
  std::map <uint16_t, std::vector <HarqProcessInfoList_t> >::iterator it;
  for (it = m_miUlHarqProcessesInfoMap.begin (); it != m_miUlHarqProcessesInfoMap.end (); it++)
    {
      (*it).second.erase ((*it).second.begin ());
      HarqProcessInfoList_t h;
      (*it).second.push_back (h);      
    }

}


double
cv2x_LteHarqPhy::GetAccumulatedMiDl (uint8_t harqProcId, uint8_t layer)
{
  NS_LOG_FUNCTION (this << (uint32_t)harqProcId << (uint16_t)layer);
  HarqProcessInfoList_t list = m_miDlHarqProcessesInfoMap.at (layer).at (harqProcId);
  double mi = 0.0;
  for (uint8_t i = 0; i < list.size (); i++)
    {
      mi += list.at (i).m_mi;
    }
  return (mi);
}

HarqProcessInfoList_t
cv2x_LteHarqPhy::GetHarqProcessInfoDl (uint8_t harqProcId, uint8_t layer)
{
  NS_LOG_FUNCTION (this << (uint32_t)harqProcId << (uint16_t)layer);
  return (m_miDlHarqProcessesInfoMap.at (layer).at (harqProcId));   
}


double
cv2x_LteHarqPhy::GetAccumulatedMiUl (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);

  std::map <uint16_t, std::vector <HarqProcessInfoList_t> >::iterator it;
  it = m_miUlHarqProcessesInfoMap.find (rnti);
  NS_ASSERT_MSG (it!=m_miUlHarqProcessesInfoMap.end (), " Does not find MI for RNTI");
  HarqProcessInfoList_t list = (*it).second.at (0);
  double mi = 0.0;
  for (uint8_t i = 0; i < list.size (); i++)
    {
      mi += list.at (i).m_mi;
    }
  return (mi);
}

HarqProcessInfoList_t
cv2x_LteHarqPhy::GetHarqProcessInfoUl (uint16_t rnti, uint8_t harqProcId)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t)harqProcId);
  std::map <uint16_t, std::vector <HarqProcessInfoList_t> >::iterator it;
  it = m_miUlHarqProcessesInfoMap.find (rnti);
  if (it==m_miUlHarqProcessesInfoMap.end ())
    {
      // new entry
      std::vector <HarqProcessInfoList_t> harqList;
      harqList.resize (8);
      m_miUlHarqProcessesInfoMap.insert (std::pair <uint16_t, std::vector <HarqProcessInfoList_t> > (rnti, harqList));
      return (harqList.at (harqProcId));
    }
  else
    {
      return ((*it).second.at (harqProcId));
    }
}



void
cv2x_LteHarqPhy::UpdateDlHarqProcessStatus (uint8_t id, uint8_t layer, double mi, uint16_t infoBytes, uint16_t codeBytes)
{
  NS_LOG_FUNCTION (this << (uint16_t) id << mi);
  if (m_miDlHarqProcessesInfoMap.at (layer).at (id).size () == 3)  // MAX HARQ RETX
    {
      // HARQ should be disabled -> discard info
      return;
    }
  cv2x_HarqProcessInfoElement_t el;
  el.m_mi = mi;
  el.m_infoBits = infoBytes * 8;
  el.m_codeBits = codeBytes * 8;
  m_miDlHarqProcessesInfoMap.at (layer).at (id).push_back (el);
}


void
cv2x_LteHarqPhy::ResetDlHarqProcessStatus (uint8_t id)
{
  NS_LOG_FUNCTION (this << (uint16_t) id);
  for (uint8_t i = 0; i < m_miDlHarqProcessesInfoMap.size (); i++)
    {
      m_miDlHarqProcessesInfoMap.at (i).at (id).clear ();
    }
  
}


void
cv2x_LteHarqPhy::UpdateUlHarqProcessStatus (uint16_t rnti, double mi, uint16_t infoBytes, uint16_t codeBytes)
{
  NS_LOG_FUNCTION (this << rnti << mi);
  std::map <uint16_t, std::vector <HarqProcessInfoList_t> >::iterator it;
  it = m_miUlHarqProcessesInfoMap.find (rnti);
  if (it==m_miUlHarqProcessesInfoMap.end ())
    {
      // new entry
      std::vector <HarqProcessInfoList_t> harqList;
      harqList.resize (8);
      cv2x_HarqProcessInfoElement_t el;
      el.m_mi = mi;
      el.m_infoBits = infoBytes * 8;
      el.m_codeBits = codeBytes * 8;
      harqList.at (7).push_back (el);
      m_miUlHarqProcessesInfoMap.insert (std::pair <uint16_t, std::vector <HarqProcessInfoList_t> > (rnti, harqList));
    }
  else
    {
      if ((*it).second.at (0).size () == 3) // MAX HARQ RETX
        {
          // HARQ should be disabled -> discard info
          return;
        }
      
//       move current status back at the end to maintain full history
      HarqProcessInfoList_t list = (*it).second.at (0);
      for (uint8_t i = 0; i < list.size (); i++)
        {
          (*it).second.at (7).push_back (list.at (i));
        }
      
      cv2x_HarqProcessInfoElement_t el;
      el.m_mi = mi;
      el.m_infoBits = infoBytes * 8;
      el.m_codeBits = codeBytes * 8;
      (*it).second.at (7).push_back (el);
    }
}

void
cv2x_LteHarqPhy::UpdateUlHarqProcessStatus (uint16_t rnti, double sinr)
{
  NS_LOG_FUNCTION (this << rnti << sinr);
  std::map <uint16_t, std::vector <HarqProcessInfoList_t> >::iterator it;
  it = m_miUlHarqProcessesInfoMap.find (rnti);
  if (it==m_miUlHarqProcessesInfoMap.end ())
    {
      // new entry
      std::vector <HarqProcessInfoList_t> harqList;
      harqList.resize (8);
      cv2x_HarqProcessInfoElement_t el;
      el.m_mi = 0;
      el.m_infoBits = 0;
      el.m_codeBits = 0;
      el.m_sinr = sinr;
      harqList.at (7).push_back (el);
      m_miUlHarqProcessesInfoMap.insert (std::pair <uint16_t, std::vector <HarqProcessInfoList_t> > (rnti, harqList));
    }
  else
    {
      if ((*it).second.at (7).size () == 3) // MAX HARQ RETX
        {
          // HARQ should be disabled -> discard info
          return;
        }

      //Bug 731: move current status back to the end
      HarqProcessInfoList_t list = (*it).second.at (0);
      for (uint8_t i = 0; i < list.size (); i++)
        {
          (*it).second.at (7).push_back (list.at (i));
        }      

      cv2x_HarqProcessInfoElement_t el;
      el.m_mi = 0;
      el.m_infoBits = 0;
      el.m_codeBits = 0;
      el.m_sinr = sinr;
      (*it).second.at (7).push_back (el);
    }
}

void
cv2x_LteHarqPhy::ResetUlHarqProcessStatus (uint16_t rnti, uint8_t id)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t)id);
  std::map <uint16_t, std::vector <HarqProcessInfoList_t> >::iterator it;
  it = m_miUlHarqProcessesInfoMap.find (rnti);
  if (it==m_miUlHarqProcessesInfoMap.end ())
    {
      // new entry
      std::vector <HarqProcessInfoList_t> harqList;
      harqList.resize (8);
      m_miUlHarqProcessesInfoMap.insert (std::pair <uint16_t, std::vector <HarqProcessInfoList_t> > (rnti, harqList));
    }
  else
    {
      (*it).second.at (id).clear ();
    }
}

double
cv2x_LteHarqPhy::GetAccumulatedMiSl (uint16_t rnti, uint8_t l1dst)
{
  NS_LOG_FUNCTION (this << rnti);

  std::map <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> >::iterator it;
  it = m_miSlHarqProcessesInfoMap.find (rnti);
  NS_ASSERT_MSG (it!=m_miSlHarqProcessesInfoMap.end (), " Does not find MI for RNTI");
  std::map <uint8_t, HarqProcessInfoList_t>::iterator it2;
  it2 = it->second.find (l1dst);
  NS_ASSERT_MSG (it2!=it->second.end (), " Does not find MI for l1dst ");
  HarqProcessInfoList_t list = (*it2).second;
  double mi = 0.0;
  for (uint8_t i = 0; i < list.size (); i++)
    {
      mi += list.at (i).m_mi;
    }
  return (mi);
}

HarqProcessInfoList_t
cv2x_LteHarqPhy::GetHarqProcessInfoSl (uint16_t rnti, uint8_t l1dst)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t)l1dst);
  std::map <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> >::iterator it;
  it = m_miSlHarqProcessesInfoMap.find (rnti);
  if (it==m_miSlHarqProcessesInfoMap.end ())
    {
      // new entry
      HarqProcessInfoList_t harqInfo;
      std::map <uint8_t, HarqProcessInfoList_t> map;
      map.insert (std::pair <uint8_t, HarqProcessInfoList_t> (l1dst, harqInfo));
      m_miSlHarqProcessesInfoMap.insert (std::pair <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> > (rnti, map));
      return (harqInfo);
    }
  else
    {
      //entry here, check ls1dst
      std::map <uint8_t, HarqProcessInfoList_t>::iterator it2;
      it2 = it->second.find (l1dst);
      if (it2==it->second.end ())
        {
          HarqProcessInfoList_t harqInfo;
          std::map <uint8_t, HarqProcessInfoList_t> map;
          it->second.insert (std::pair <uint8_t, HarqProcessInfoList_t> (l1dst, harqInfo));
          return (harqInfo);
        }
      else
        {
          return ((*it2).second);
        }
    }
}

HarqProcessInfoList_t
cv2x_LteHarqPhy::GetHarqProcessInfoSlV2X (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::map <uint16_t, HarqProcessInfoList_t>::iterator it;
  it = m_miSlV2XHarqProcessesInfoMap.find (rnti);

  // new entry
  HarqProcessInfoList_t harqInfo;
  m_miSlV2XHarqProcessesInfoMap.insert (std::pair <uint16_t, HarqProcessInfoList_t> (rnti, harqInfo));
  return (harqInfo);
}

HarqProcessInfoList_t
cv2x_LteHarqPhy::GetHarqProcessInfoDisc (uint16_t rnti, uint8_t resPsdch)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t)resPsdch);
  std::map <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> >::iterator it;
  it = m_miDiscHarqProcessesInfoMap.find (rnti);
  if (it==m_miDiscHarqProcessesInfoMap.end ())
    {
      // new entry
      HarqProcessInfoList_t harqInfo;
      std::map <uint8_t, HarqProcessInfoList_t> map;
      map.insert (std::pair <uint8_t, HarqProcessInfoList_t> (resPsdch, harqInfo));
      m_miDiscHarqProcessesInfoMap.insert (std::pair <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> > (rnti, map));
      return (harqInfo);
    }
  else
    {
      //entry here, check ls1dst
      std::map <uint8_t, HarqProcessInfoList_t>::iterator it2;
      it2 = it->second.find (resPsdch);
      if (it2==it->second.end ())
        {
          HarqProcessInfoList_t harqInfo;
          std::map <uint8_t, HarqProcessInfoList_t> map;
          it->second.insert (std::pair <uint8_t, HarqProcessInfoList_t> (resPsdch, harqInfo));
          return (harqInfo);
        }
      else
        {
          return ((*it2).second);
        }
    }
}

void
cv2x_LteHarqPhy::UpdateSlHarqProcessStatus (uint16_t rnti, uint8_t l1dst, double mi, uint16_t infoBytes, uint16_t codeBytes)
{
  NS_LOG_FUNCTION (this << rnti << mi);
  std::map <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> >::iterator it;
  std::map <uint8_t, HarqProcessInfoList_t>::iterator it2;
  
  it = m_miSlHarqProcessesInfoMap.find (rnti);
  if (it==m_miSlHarqProcessesInfoMap.end ())
    {
      // new entry
      HarqProcessInfoList_t harqInfo;
      cv2x_HarqProcessInfoElement_t el;
      el.m_mi = mi;
      el.m_infoBits = infoBytes * 8;
      el.m_codeBits = codeBytes * 8;
      harqInfo.push_back (el);
      std::map <uint8_t, HarqProcessInfoList_t> map;
      map.insert (std::pair <uint8_t, HarqProcessInfoList_t> (l1dst, harqInfo));
      m_miSlHarqProcessesInfoMap.insert (std::pair <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> > (rnti, map));
    }
  else
    {
      it2 = it->second.find (l1dst);
      if (it2 == it->second.end())
        {
          // new entry
          HarqProcessInfoList_t harqInfo;
          cv2x_HarqProcessInfoElement_t el;
          el.m_mi = mi;
          el.m_infoBits = infoBytes * 8;
          el.m_codeBits = codeBytes * 8;
          harqInfo.push_back (el);
          (*it).second.insert (std::pair <uint8_t, HarqProcessInfoList_t> (l1dst, harqInfo));
        }
      else
        {
          if ((*it2).second.size () == 3) // MAX HARQ RETX
            {
              // HARQ should be disabled -> discard info
              return;
            }
          cv2x_HarqProcessInfoElement_t el;
          el.m_mi = mi;
          el.m_infoBits = infoBytes * 8;
          el.m_codeBits = codeBytes * 8;
          (*it2).second.push_back (el);
        }
    }
}

void
cv2x_LteHarqPhy::UpdateSlHarqProcessStatus (uint16_t rnti, uint8_t l1dst, double sinr)
{
  NS_LOG_FUNCTION (this << rnti << sinr);
  std::map <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> >::iterator it;
  std::map <uint8_t, HarqProcessInfoList_t>::iterator it2;
  
  it = m_miSlHarqProcessesInfoMap.find (rnti);
  if (it==m_miSlHarqProcessesInfoMap.end ())
    {
      // new entry
      HarqProcessInfoList_t harqInfo;
      cv2x_HarqProcessInfoElement_t el;
      el.m_mi = 0;
      el.m_infoBits = 0;
      el.m_codeBits = 0;
      el.m_sinr = sinr;
      harqInfo.push_back (el);
      std::map <uint8_t, HarqProcessInfoList_t> map;
      map.insert (std::pair <uint8_t, HarqProcessInfoList_t> (l1dst, harqInfo));
      m_miSlHarqProcessesInfoMap.insert (std::pair <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> > (rnti, map));
    }
  else
    {
      it2 = it->second.find (l1dst);
      if (it2 == it->second.end())
        {
          // new entry
          HarqProcessInfoList_t harqInfo;
          cv2x_HarqProcessInfoElement_t el;
          el.m_mi = 0;
          el.m_infoBits = 0;
          el.m_codeBits = 0;
          el.m_sinr = sinr;
          harqInfo.push_back (el);
          (*it).second.insert (std::pair <uint8_t, HarqProcessInfoList_t> (l1dst, harqInfo));
        }
      else
        {
          if ((*it2).second.size () == 3) // MAX HARQ RETX
            {
              // HARQ should be disabled -> discard info
              return;
            }
          cv2x_HarqProcessInfoElement_t el;
          el.m_mi = 0;
          el.m_infoBits = 0;
          el.m_codeBits = 0;
          el.m_sinr = sinr;
          (*it2).second.push_back (el);
        }
    }
}

void
cv2x_LteHarqPhy::UpdateSlV2XHarqProcessStatus (uint16_t rnti, double mi, uint16_t infoBytes, uint16_t codeBytes)
{
  NS_LOG_FUNCTION (this << rnti << mi);
  std::map <uint16_t, HarqProcessInfoList_t>::iterator it;

  it = m_miSlV2XHarqProcessesInfoMap.find (rnti);
  // new entry
  HarqProcessInfoList_t harqInfo;
  cv2x_HarqProcessInfoElement_t el;
  el.m_mi = mi;
  el.m_infoBits = infoBytes * 8;
  el.m_codeBits = codeBytes * 8;
  harqInfo.push_back (el);
  m_miSlV2XHarqProcessesInfoMap.insert (std::pair <uint16_t, HarqProcessInfoList_t> (rnti, harqInfo));
}

void
cv2x_LteHarqPhy::UpdateSlV2XHarqProcessStatus (uint16_t rnti, double sinr)
{
  NS_LOG_FUNCTION (this << rnti << sinr);
  std::map <uint16_t, HarqProcessInfoList_t>::iterator it;


  it = m_miSlV2XHarqProcessesInfoMap.find (rnti);
  // new entry
  HarqProcessInfoList_t harqInfo;
  cv2x_HarqProcessInfoElement_t el;
  el.m_mi = 0;
  el.m_infoBits = 0;
  el.m_codeBits = 0;
  el.m_sinr = sinr;
  harqInfo.push_back (el);
  m_miSlV2XHarqProcessesInfoMap.insert (std::pair <uint16_t, HarqProcessInfoList_t> (rnti, harqInfo));
}

void
cv2x_LteHarqPhy::UpdateDiscHarqProcessStatus (uint16_t rnti, uint8_t resPsdch, double sinr )
{
  NS_LOG_FUNCTION (this << rnti << sinr);
  std::map <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> >::iterator it;
  std::map <uint8_t, HarqProcessInfoList_t>::iterator it2;
  
  it = m_miDiscHarqProcessesInfoMap.find (rnti);
  if (it==m_miDiscHarqProcessesInfoMap.end ())
    {
      // new entry
      HarqProcessInfoList_t harqInfo;
      cv2x_HarqProcessInfoElement_t el;
      el.m_mi = 0;
      el.m_infoBits = 0;
      el.m_codeBits = 0;
      el.m_sinr = sinr;
      harqInfo.push_back (el);
      std::map <uint8_t, HarqProcessInfoList_t> map;
      map.insert (std::pair <uint8_t, HarqProcessInfoList_t> (resPsdch, harqInfo));
      m_miDiscHarqProcessesInfoMap.insert (std::pair <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> > (rnti, map));
    }
  else
    {
      it2 = it->second.find (resPsdch);
      if (it2 == it->second.end())
        {
          // new entry
          HarqProcessInfoList_t harqInfo;
          cv2x_HarqProcessInfoElement_t el;
          el.m_mi = 0;
          el.m_infoBits = 0;
          el.m_codeBits = 0;
          el.m_sinr = sinr;
          harqInfo.push_back (el);
          (*it).second.insert (std::pair <uint8_t, HarqProcessInfoList_t> (resPsdch, harqInfo));
        }
      else
        {
          if ((*it2).second.size () == m_discNumRetx) // MAX HARQ RETX
            {
              //std::cout << "numRetx=" << (uint16_t)m_discNumRetx << std::endl;
              // HARQ should be disabled -> discard info
              return;
            }
          cv2x_HarqProcessInfoElement_t el;
          el.m_mi = 0;
          el.m_infoBits = 0;
          el.m_codeBits = 0;
          el.m_sinr = sinr;
          (*it2).second.push_back (el);
        }
    }
}


void
cv2x_LteHarqPhy::ResetSlHarqProcessStatus (uint16_t rnti, uint8_t l1dst)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t)l1dst);
  std::map <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> >::iterator it;
  std::map <uint8_t, HarqProcessInfoList_t>::iterator it2;
  it = m_miSlHarqProcessesInfoMap.find (rnti);
  if (it==m_miSlHarqProcessesInfoMap.end ())
    {
      // new entry
      HarqProcessInfoList_t harqInfo;
      std::map <uint8_t, HarqProcessInfoList_t> map;
      map.insert (std::pair <uint8_t, HarqProcessInfoList_t> (l1dst, harqInfo));
      m_miSlHarqProcessesInfoMap.insert (std::pair <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> > (rnti, map));
    }
  else
    {
      it2 = (*it).second.find (l1dst);
      if (it2 == (*it).second.end())
        {
          HarqProcessInfoList_t harqInfo;
          std::map <uint8_t, HarqProcessInfoList_t> map;
          it->second.insert (std::pair <uint8_t, HarqProcessInfoList_t> (l1dst, harqInfo));
        }
      else
        {
          (*it2).second.clear ();
        }
    }
}

void
cv2x_LteHarqPhy::ResetDiscHarqProcessStatus (uint16_t rnti, uint8_t resPsdch)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t)resPsdch);
  std::map <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> >::iterator it;
  std::map <uint8_t, HarqProcessInfoList_t>::iterator it2;
  it = m_miDiscHarqProcessesInfoMap.find (rnti);
  if (it==m_miDiscHarqProcessesInfoMap.end ())
    {
      // new entry
      HarqProcessInfoList_t harqInfo;
      std::map <uint8_t, HarqProcessInfoList_t> map;
      map.insert (std::pair <uint8_t, HarqProcessInfoList_t> (resPsdch, harqInfo));
      m_miDiscHarqProcessesInfoMap.insert (std::pair <uint16_t, std::map <uint8_t, HarqProcessInfoList_t> > (rnti, map));
    }
  else
    {
      it2 = (*it).second.find (resPsdch);
      if (it2 == (*it).second.end())
        {
          HarqProcessInfoList_t harqInfo;
          std::map <uint8_t, HarqProcessInfoList_t> map;
          it->second.insert (std::pair <uint8_t, HarqProcessInfoList_t> (resPsdch, harqInfo));
        }
      else
        {
          (*it2).second.clear ();
        }
    }
}

void
cv2x_LteHarqPhy::SetDiscNumRetx (uint8_t retx)
{
  NS_LOG_FUNCTION (this << retx);
  m_discNumRetx = retx;
}




} // end namespace
