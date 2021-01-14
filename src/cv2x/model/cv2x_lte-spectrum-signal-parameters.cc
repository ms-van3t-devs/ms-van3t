/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 CTTC
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
 * Modified by:
 *          Marco Miozzo <mmiozzo@cttc.es> (add data and ctrl diversity)
 *          NIST (D2D)
 */

#include <ns3/log.h>
#include <ns3/packet-burst.h>
#include <ns3/ptr.h>
#include <ns3/cv2x_lte-spectrum-signal-parameters.h>
#include <ns3/cv2x_lte-control-messages.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteSpectrumSignalParameters");

cv2x_LteSpectrumSignalParameters::cv2x_LteSpectrumSignalParameters ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteSpectrumSignalParameters::cv2x_LteSpectrumSignalParameters (const cv2x_LteSpectrumSignalParameters& p)
  : SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  packetBurst = p.packetBurst->Copy ();
}

Ptr<SpectrumSignalParameters>
cv2x_LteSpectrumSignalParameters::Copy ()
{
  NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<cv2x_LteSpectrumSignalParameters> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<cv2x_LteSpectrumSignalParameters> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<cv2x_LteSpectrumSignalParameters> lssp (new cv2x_LteSpectrumSignalParameters (*this), false);  
  return lssp;
}



cv2x_LteSpectrumSignalParametersDataFrame::cv2x_LteSpectrumSignalParametersDataFrame ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteSpectrumSignalParametersDataFrame::cv2x_LteSpectrumSignalParametersDataFrame (const cv2x_LteSpectrumSignalParametersDataFrame& p)
: SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  cellId = p.cellId;
  if (p.packetBurst)
    {
      packetBurst = p.packetBurst->Copy ();
    }
  ctrlMsgList = p.ctrlMsgList;
}

Ptr<SpectrumSignalParameters>
cv2x_LteSpectrumSignalParametersDataFrame::Copy ()
{
  NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<cv2x_LteSpectrumSignalParametersDataFrame> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<cv2x_LteSpectrumSignalParametersDataFrame> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<cv2x_LteSpectrumSignalParametersDataFrame> lssp (new cv2x_LteSpectrumSignalParametersDataFrame (*this), false);  
  return lssp;
}



cv2x_LteSpectrumSignalParametersDlCtrlFrame::cv2x_LteSpectrumSignalParametersDlCtrlFrame ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteSpectrumSignalParametersDlCtrlFrame::cv2x_LteSpectrumSignalParametersDlCtrlFrame (const cv2x_LteSpectrumSignalParametersDlCtrlFrame& p)
: SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  cellId = p.cellId;
  pss = p.pss;
  ctrlMsgList = p.ctrlMsgList;
}

Ptr<SpectrumSignalParameters>
cv2x_LteSpectrumSignalParametersDlCtrlFrame::Copy ()
{
  NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<cv2x_LteSpectrumSignalParametersDlCtrlFrame> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<cv2x_LteSpectrumSignalParametersDlCtrlFrame> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<cv2x_LteSpectrumSignalParametersDlCtrlFrame> lssp (new cv2x_LteSpectrumSignalParametersDlCtrlFrame (*this), false);  
  return lssp;
}


cv2x_LteSpectrumSignalParametersUlSrsFrame::cv2x_LteSpectrumSignalParametersUlSrsFrame ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteSpectrumSignalParametersUlSrsFrame::cv2x_LteSpectrumSignalParametersUlSrsFrame (const cv2x_LteSpectrumSignalParametersUlSrsFrame& p)
: SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  cellId = p.cellId;
}

Ptr<SpectrumSignalParameters>
cv2x_LteSpectrumSignalParametersUlSrsFrame::Copy ()
{
  NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<cv2x_LteSpectrumSignalParametersUlSrsFrame> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<cv2x_LteSpectrumSignalParametersUlSrsFrame> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<cv2x_LteSpectrumSignalParametersUlSrsFrame> lssp (new cv2x_LteSpectrumSignalParametersUlSrsFrame (*this), false);  
  return lssp;
}

cv2x_LteSpectrumSignalParametersSlFrame::cv2x_LteSpectrumSignalParametersSlFrame ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteSpectrumSignalParametersSlFrame::cv2x_LteSpectrumSignalParametersSlFrame (const cv2x_LteSpectrumSignalParametersSlFrame& p)
: SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  nodeId = p.nodeId;
  groupId = p.groupId;
  slssId = p.slssId;
  ctrlMsgList = p.ctrlMsgList;
  if (p.packetBurst)
    {
      packetBurst = p.packetBurst->Copy ();
    }
}

Ptr<SpectrumSignalParameters>
cv2x_LteSpectrumSignalParametersSlFrame::Copy ()
{
    NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<cv2x_LteSpectrumSignalParametersSlCtrlFrame> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<cv2x_LteSpectrumSignalParametersSlCtrlFrame> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<cv2x_LteSpectrumSignalParametersSlFrame> lssp (new cv2x_LteSpectrumSignalParametersSlFrame (*this), false);  
  return lssp;
}







} // namespace ns3
