/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.

 * We would appreciate acknowledgement if the software is used.

 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 *
 * Modified by: NIST
 */

#include "cv2x_lte-prose-helper.h"
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/cv2x_epc-helper.h>
#include <ns3/angles.h>
#include <ns3/random-variable-stream.h>
#include <iostream>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteProseHelper");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteProseHelper);

cv2x_LteProseHelper::cv2x_LteProseHelper (void)
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteProseHelper::~cv2x_LteProseHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId cv2x_LteProseHelper::GetTypeId (void)
{
  static TypeId
    tid = 
    TypeId ("ns3::cv2x_LteProseHelper")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteProseHelper> ()
  ;
  return tid;
}

void
cv2x_LteProseHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}


void 
cv2x_LteProseHelper::SetLteHelper (Ptr<cv2x_LteHelper> h)
{
  NS_LOG_FUNCTION (this << h);
  m_lteHelper = h;
}

NetDeviceContainer 
cv2x_LteProseHelper::RemoveNetDevice (NetDeviceContainer container, Ptr<NetDevice> item)
{
  NetDeviceContainer newContainer;
  uint32_t nDevices = container.GetN ();
  for (uint32_t i = 0 ; i < nDevices; ++i)
    {
      Ptr<NetDevice> p = container.Get(i);
      if (item != p)
        {
          newContainer.Add (p);
        }
    }
  return newContainer;
}


std::vector < NetDeviceContainer > 
cv2x_LteProseHelper::AssociateForGroupcast (double txPower, double ulEarfcn, double ulBandwidth, NetDeviceContainer ues, double rsrpThreshold, int ngroups, int nreceivers, SrsrpMethod_t compMethod)
{
  std::vector < NetDeviceContainer > groups; //groups created
  Ptr<UniformRandomVariable> ueSelector = CreateObject<UniformRandomVariable> (); //random variable to select Tx/Rx nodes

  NetDeviceContainer remainingUes; //list of UEs not assigned to groups
  remainingUes.Add (ues);

  // Start association of groupcast links, set NUM_GROUPS_ASSOCIATED = 0.
  int32_t numGroupsAssociated = 0;
  
  NetDeviceContainer candidateTx; //list of UEs not assigned to groups that can be selected for transmission
  candidateTx.Add (ues);

  while (numGroupsAssociated < ngroups && candidateTx.GetN() > 0)
    {
      //Transmitter UE is randomly selected from all UEs within the entire 19 or 7 macro sites that are already not selected as transmitter or receiver UEs.
      uint32_t iTx = ueSelector->GetValue (0, candidateTx.GetN());
      Ptr<NetDevice> tx = candidateTx.Get (iTx);
      NS_LOG_DEBUG (" Candidate Tx= " << tx->GetNode()->GetId());
      candidateTx = RemoveNetDevice (candidateTx, tx);
      //build list of candidate receivers      
      NetDeviceContainer candidateRx = RemoveNetDevice (remainingUes, tx);
      NetDeviceContainer selectedRx;

      //Start selecting the receiver for the transmitter, set NUM_RECEIVERS_ASSOCIATED = 0.
      int32_t numReceiversAssociated = 0;
      //Receiver UE is randomly selected from the remaining UEs (i.e., not already part of a group) within the entire 19 or 7 macro sites.
      while (numReceiversAssociated < nreceivers && candidateRx.GetN() > 0)
        {
          uint32_t iRx = ueSelector->GetValue (0, candidateRx.GetN());
          Ptr<NetDevice> rx = candidateRx.Get (iRx);
          candidateRx = RemoveNetDevice (candidateRx, rx);
          double rsrpRx = 0;
          if (compMethod == cv2x_LteProseHelper::SRSRP_STD)
            {
              rsrpRx = m_lteHelper->CalcSidelinkRsrp (txPower, ulEarfcn, ulBandwidth, tx, rx);
            } else 
            {
              rsrpRx = m_lteHelper->CalcSidelinkRsrpEval (txPower, ulBandwidth, tx, rx);
            }
          //If receiver UE is not within RSRP* of X dBm of the transmitter UE then randomly reselect the receiver UE among the UEs that are within the RSRP of X dBm of the transmitter UE and are not part of a group already.
          NS_LOG_DEBUG ("\tCandidate Rx= " << rx->GetNode()->GetId() << " Rsrp=" << rsrpRx << " required=" << rsrpThreshold);
          if (rsrpRx >= rsrpThreshold)
            {
              //good receiver
              selectedRx.Add (rx);
              numReceiversAssociated++;
              NS_LOG_DEBUG ("\tAdding Rx to group");
            }
        }
      if (numReceiversAssociated == nreceivers)
        {
          NS_LOG_DEBUG(" Group successfully created");
          //found all the receivers, update lists
          //remove receivers from candidate Tx and remaining nodes
          remainingUes = RemoveNetDevice (remainingUes, tx); //remove selected Tx
          uint32_t nDevices = selectedRx.GetN ();
          for (uint32_t i = 0 ; i < nDevices; ++i)
            {
              Ptr<NetDevice> p = selectedRx.Get(i);
              candidateTx = RemoveNetDevice (candidateTx, p); 
              remainingUes = RemoveNetDevice (remainingUes, p);
            }
          NetDeviceContainer newGroup (tx);
          newGroup.Add (selectedRx);
          groups.push_back (newGroup);
          numGroupsAssociated++;
        } else 
        {
          NS_LOG_DEBUG (" Group failed. Found only " << numReceiversAssociated << " receivers");
        }
    } 

  NS_LOG_INFO ("Groups created " << groups.size() << " expected " << ngroups);

  return groups;
}

std::vector < NetDeviceContainer > 
cv2x_LteProseHelper::AssociateForBroadcast (double txPower, double ulEarfcn, double ulBandwidth, NetDeviceContainer ues, double rsrpThreshold, uint32_t ntransmitters, SrsrpMethod_t compMethod)
{
  std::vector < NetDeviceContainer > groups; //groups created
  Ptr<UniformRandomVariable> ueSelector = CreateObject<UniformRandomVariable> (); //random variable to select Tx/Rx nodes

  NetDeviceContainer remainingUes; //list of UEs not assigned to groups
  remainingUes.Add (ues);

  // Start the selection of the transmitters
  NetDeviceContainer candidateTx; //list of UEs that can be selected for transmission
  candidateTx.Add (ues);
  NetDeviceContainer selectedTx;
  uint32_t numTransmittersSelected = 0;
  
  while (numTransmittersSelected < ntransmitters)
    {
      //Transmitter UE is randomly selected from all UEs within the entire 19 or 7 macro sites that are already not selected as transmitter or receiver UEs.
      uint32_t iTx = ueSelector->GetValue (0, candidateTx.GetN());
      Ptr<NetDevice> tx = candidateTx.Get (iTx);
      NS_LOG_DEBUG (" Candidate Tx= " << tx->GetNode()->GetId());
      selectedTx.Add (tx);
      candidateTx = RemoveNetDevice (candidateTx, tx);
      remainingUes = RemoveNetDevice (remainingUes, tx);
      numTransmittersSelected++;
    }

  //For each remaining UE, associate to all transmitters where RSRP is greater than X dBm
  for (uint32_t i = 0 ; i < numTransmittersSelected ; i++)
    {
      Ptr<NetDevice> tx = selectedTx.Get (i);
      //prepare group for this transmitter
      NetDeviceContainer newGroup (tx);
      uint32_t nRxDevices = remainingUes.GetN ();
      double rsrpRx = 0;
      for (uint32_t j = 0 ; j < nRxDevices; ++j)
        {
          Ptr<NetDevice> rx = remainingUes.Get(j);
          if (compMethod == cv2x_LteProseHelper::SRSRP_STD)
            {
              rsrpRx = m_lteHelper->CalcSidelinkRsrp (txPower, ulEarfcn, ulBandwidth, tx, rx);
            } else 
            {
              rsrpRx = m_lteHelper->CalcSidelinkRsrpEval (txPower, ulBandwidth, tx, rx);
            }          
          //If receiver UE is not within RSRP* of X dBm of the transmitter UE then randomly reselect the receiver UE among the UEs that are within the RSRP of X dBm of the transmitter UE and are not part of a group already.
          NS_LOG_DEBUG ("\tCandidate Rx= " << rx->GetNode()->GetId() << " Rsrp=" << rsrpRx << " required=" << rsrpThreshold);
          if (rsrpRx >= rsrpThreshold)
            {
              //good receiver              
              NS_LOG_DEBUG ("\tAdding Rx to group");
              newGroup.Add (rx);
            }
        }

      //Initializing link to other transmitters to be able to receive SLSSs from other transmitters
      for (uint32_t k = 0 ; k < numTransmittersSelected ; k++)
        {
          if (k != i)
            {
              Ptr<NetDevice> othertx = selectedTx.Get (k);
              if (compMethod == cv2x_LteProseHelper::SRSRP_STD)
                {
                  rsrpRx = m_lteHelper->CalcSidelinkRsrp (txPower, ulEarfcn, ulBandwidth, tx, othertx);
                } else
                  {
                    rsrpRx = m_lteHelper->CalcSidelinkRsrpEval (txPower, ulBandwidth, tx, othertx);
                  }
              NS_LOG_DEBUG ("\tOther Tx= " << othertx->GetNode()->GetId() << " Rsrp=" << rsrpRx);
            }
        }

      groups.push_back (newGroup);
    }

  return groups;
}

//Modified for vehicular d2d
std::vector<NetDeviceContainer>
cv2x_LteProseHelper::AssociateForVehicularBroadcast(double txPower, double ulEarfcn, double ulBandwidth, NetDeviceContainer ues, double rsrpThreshold, uint32_t ntransmitters, SrsrpMethod_t compMethod)
{
	std::vector < NetDeviceContainer > groups; //groups created
	NetDeviceContainer remainingUes; //list of UEs not assigned to groups
	remainingUes.Add (ues);
	// Start the selection of the transmitters
	NetDeviceContainer selectedTx;
	uint32_t numTransmittersSelected = 0;


	while (numTransmittersSelected < ntransmitters)
	  {
      //Transmitter UE is selected from all UEs within the entire 19 or 7 macro sites that are already not selected as transmitter or receiver UEs.

      Ptr<NetDevice> tx = ues.Get (numTransmittersSelected);
      NS_LOG_DEBUG (" Candidate Tx= " << tx->GetNode()->GetId());
      selectedTx.Add (tx);
      numTransmittersSelected++;
	  }

	  //For each remaining UE, associate to all transmitters where RSRP is greater than X dBm
	  for (uint32_t i = 0 ; i < numTransmittersSelected ; i++)
	    {
	      Ptr<NetDevice> tx = selectedTx.Get (i);
	      //prepare group for this transmitter
	      NetDeviceContainer newGroup (tx);
	      uint32_t nRxDevices = remainingUes.GetN ();
	      double rsrpRx = 0;
	      for (uint32_t j = 0 ; j < nRxDevices; ++j)
	        {
	          Ptr<NetDevice> rx = remainingUes.Get(j);
	          if(rx->GetNode()->GetId() != tx->GetNode()->GetId())//No loopback link possible due to half-duplex
	          {
	            if (compMethod == cv2x_LteProseHelper::SRSRP_STD)
	              {
	                rsrpRx = m_lteHelper->CalcSidelinkRsrp (txPower, ulEarfcn, ulBandwidth, tx, rx);
	              }
              else
	              {
	                rsrpRx = m_lteHelper->CalcSidelinkRsrpEval (txPower, ulBandwidth, tx, rx);
	              }
	              //If receiver UE is not within RSRP* of X dBm of the transmitter UE then randomly reselect the receiver UE among the UEs that are within the RSRP of X dBm of the transmitter UE and are not part of a group already.
	            NS_LOG_DEBUG ("\tCandidate Rx= " << rx->GetNode()->GetId() << " Rsrp=" << rsrpRx << " required=" << rsrpThreshold);
	            if (rsrpRx >= rsrpThreshold)
	              {
	                //good receiver
	                NS_LOG_DEBUG ("\tAdding Rx to group");
	                newGroup.Add (rx);
	              }
	          }
	        }
	      groups.push_back (newGroup);
	    }
	return groups;
}

std::vector<NetDeviceContainer>
cv2x_LteProseHelper::AssociateForPlatoon(double txPower, double ulEarfcn, double ulBandwidth, NetDeviceContainer ues, double rsrpThreshold, uint32_t ntransmitters, SrsrpMethod_t compMethod)
{
	std::vector < NetDeviceContainer > groups; //groups created
	NetDeviceContainer remainingUes; //list of UEs not assigned to groups
	remainingUes.Add (ues);
	// Start the selection of the transmitters
	NetDeviceContainer selectedTx;
	uint32_t numTransmittersSelected = 0;


	while (numTransmittersSelected < ntransmitters)
	    {
	      //Transmitter UE is selected from all UEs within the entire 19 or 7 macro sites that are already not selected as transmitter or receiver UEs.

	      Ptr<NetDevice> tx = ues.Get (numTransmittersSelected);
	      NS_LOG_DEBUG (" Candidate Tx= " << tx->GetNode()->GetId());
	      selectedTx.Add (tx);
	      numTransmittersSelected++;

	    }

	//for platoonhead: all are receivers:
	Ptr<NetDevice> tx = selectedTx.Get (0);
	//prepare group for this transmitter
  NetDeviceContainer newGroup (tx);
  uint32_t nRxDevices = remainingUes.GetN ();
  double rsrpRx = 0;
  for (uint32_t j = 0 ; j < nRxDevices; ++j)
    {
      Ptr<NetDevice> rx = remainingUes.Get(j);
      if(rx->GetNode()->GetId() != tx->GetNode()->GetId())//No loopback link possible due to half-duplex
      {
        if (compMethod == cv2x_LteProseHelper::SRSRP_STD)
          {
            rsrpRx = m_lteHelper->CalcSidelinkRsrp (txPower, ulEarfcn, ulBandwidth, tx, rx);
          }
        else
          {
            rsrpRx = m_lteHelper->CalcSidelinkRsrpEval (txPower, ulBandwidth, tx, rx);
          }
          //If receiver UE is not within RSRP* of X dBm of the transmitter UE then randomly reselect the receiver UE among the UEs that are within the RSRP of X dBm of the transmitter UE and are not part of a group already.
        NS_LOG_DEBUG ("\tCandidate Rx= " << rx->GetNode()->GetId() << " Rsrp=" << rsrpRx << " required=" << rsrpThreshold);
        if (rsrpRx >= rsrpThreshold)
          {
            //good receiver
            NS_LOG_DEBUG ("\tAdding Rx to group");
            newGroup.Add (rx);
          }
      }
    }
  groups.push_back (newGroup);

  //For each remaining UE, associate just to head transmitters if RSRP is greater than X dBm
  for (uint32_t i = 1 ; i < numTransmittersSelected ; i++)
    {
      Ptr<NetDevice> tx = selectedTx.Get (i);
      //prepare group for this transmitter
      NetDeviceContainer newGroup (tx);
      double rsrpRx = 0;

      Ptr<NetDevice> rx = remainingUes.Get(0);
      if(rx->GetNode()->GetId() != tx->GetNode()->GetId())//No loopback link possible due to half-duplex
        {
          if (compMethod == cv2x_LteProseHelper::SRSRP_STD)
            {
              rsrpRx = m_lteHelper->CalcSidelinkRsrp (txPower, ulEarfcn, ulBandwidth, tx, rx);
            }
          else
            {
              rsrpRx = m_lteHelper->CalcSidelinkRsrpEval (txPower, ulBandwidth, tx, rx);
            }
            //If receiver UE is not within RSRP* of X dBm of the transmitter UE then randomly reselect the receiver UE among the UEs that are within the RSRP of X dBm of the transmitter UE and are not part of a group already.
          NS_LOG_DEBUG ("\tCandidate Rx= " << rx->GetNode()->GetId() << " Rsrp=" << rsrpRx << " required=" << rsrpThreshold);
          if (rsrpRx >= rsrpThreshold)
            {
              //good receiver
              NS_LOG_DEBUG ("\tAdding Rx to group");
              std::cout<<"RSRP ist in Ordnung "<<rsrpRx<<" ist größer als "<<rsrpThreshold<<std::endl;
              newGroup.Add (rx);
            }
        }
      groups.push_back (newGroup);
	  }
	return groups;
}
//

std::vector < NetDeviceContainer > 
cv2x_LteProseHelper::AssociateForBroadcastWithTxEnabledToReceive (double txPower, double ulEarfcn, double ulBandwidth, NetDeviceContainer ues, double rsrpThreshold, uint32_t ntransmitters, SrsrpMethod_t compMethod)
{
  std::vector < NetDeviceContainer > groups; //groups created
  Ptr<UniformRandomVariable> ueSelector = CreateObject<UniformRandomVariable> (); //random variable to select Tx/Rx nodes

  NetDeviceContainer remainingUes; //list of UEs not assigned to groups
  remainingUes.Add (ues);

  // Start the selection of the transmitters
  NetDeviceContainer candidateTx; //list of UEs that can be selected for transmission
  candidateTx.Add (ues);
  NetDeviceContainer selectedTx;
  uint32_t numTransmittersSelected = 0;
  
  while (numTransmittersSelected < ntransmitters)
    {
      //Transmitter UE is randomly selected from all UEs within the entire 19 or 7 macro sites that are already not selected as transmitter or receiver UEs.
      uint32_t iTx = ueSelector->GetValue (0, candidateTx.GetN());
      Ptr<NetDevice> tx = candidateTx.Get (iTx);
      NS_LOG_DEBUG (" Candidate Tx= " << tx->GetNode()->GetId());
      selectedTx.Add (tx);
      candidateTx = RemoveNetDevice (candidateTx, tx);
      //remainingUes = RemoveNetDevice (remainingUes, tx);
      numTransmittersSelected++;
    }

  //For each remaining UE, associate to all transmitters where RSRP is greater than X dBm
  for (uint32_t i = 0 ; i < numTransmittersSelected ; i++)
    {
      Ptr<NetDevice> tx = selectedTx.Get (i);
      //prepare group for this transmitter
      NetDeviceContainer newGroup (tx);
      uint32_t nRxDevices = remainingUes.GetN ();
      double rsrpRx = 0;
      for (uint32_t j = 0 ; j < nRxDevices; ++j)
        {
          Ptr<NetDevice> rx = remainingUes.Get(j);
          if(rx->GetNode()->GetId() != tx->GetNode()->GetId())//No loopback link possible due to half-duplex
          {
            if (compMethod == cv2x_LteProseHelper::SRSRP_STD)
              {
                rsrpRx = m_lteHelper->CalcSidelinkRsrp (txPower, ulEarfcn, ulBandwidth, tx, rx);
              } else 
              {
                rsrpRx = m_lteHelper->CalcSidelinkRsrpEval (txPower, ulBandwidth, tx, rx);
              }          
              //If receiver UE is not within RSRP* of X dBm of the transmitter UE then randomly reselect the receiver UE among the UEs that are within the RSRP of X dBm of the transmitter UE and are not part of a group already.
            NS_LOG_DEBUG ("\tCandidate Rx= " << rx->GetNode()->GetId() << " Rsrp=" << rsrpRx << " required=" << rsrpThreshold);
            if (rsrpRx >= rsrpThreshold)
              {
                //good receiver              
                NS_LOG_DEBUG ("\tAdding Rx to group");
                newGroup.Add (rx);
              }
          }
        }
      groups.push_back (newGroup);
    }

  return groups;
}

std::vector < NetDeviceContainer > 
cv2x_LteProseHelper::AssociateForBroadcastWithWrapAround (double txPower, double ulEarfcn, double ulBandwidth, NetDeviceContainer ues, double rsrpThreshold, uint32_t ntransmitters, Ptr<cv2x_LteHexGridEnbTopologyHelper> topologyHelper, SrsrpMethod_t compMethod)
{
  std::vector < NetDeviceContainer > groups; //groups created
  Ptr<UniformRandomVariable> ueSelector = CreateObject<UniformRandomVariable> (); //random variable to select Tx/Rx nodes

  NetDeviceContainer remainingUes; //list of UEs not assigned to groups
  remainingUes.Add (ues);

  // Start the selection of the transmitters
  NetDeviceContainer candidateTx; //list of UEs that can be selected for transmission
  candidateTx.Add (ues);
  NetDeviceContainer selectedTx;
  uint32_t numTransmittersSelected = 0;
  
  while (numTransmittersSelected < ntransmitters)
    {
      //Transmitter UE is randomly selected from all UEs within the entire 19 or 7 macro sites that are already not selected as transmitter or receiver UEs.
      uint32_t iTx = ueSelector->GetValue (0, candidateTx.GetN());
      Ptr<NetDevice> tx = candidateTx.Get (iTx);
      NS_LOG_DEBUG (" Candidate Tx= " << tx->GetNode()->GetId());
      selectedTx.Add (tx);
      candidateTx = RemoveNetDevice (candidateTx, tx);
      remainingUes = RemoveNetDevice (remainingUes, tx);
      numTransmittersSelected++;
    }

  //For each remaining UE, associate to all transmitters where RSRP is greater than X dBm
  for (uint32_t i = 0 ; i < numTransmittersSelected ; i++)
    {
      Ptr<NetDevice> tx = selectedTx.Get (i);
      //prepare group for this transmitter
      NetDeviceContainer newGroup (tx);
      uint32_t nRxDevices = remainingUes.GetN ();
      double rsrpRx = 0;
      for (uint32_t j = 0 ; j < nRxDevices; ++j)
        {
          Ptr<NetDevice> rx = remainingUes.Get(j);
          //With wrap around, the closest location may be in one of the extended hexagon
          //store position
          Vector rxPos = rx->GetNode()->GetObject<MobilityModel> ()->GetPosition ();
          Vector closestPos = topologyHelper->GetClosestPositionInWrapAround (tx->GetNode()->GetObject<MobilityModel> ()->GetPosition (), rxPos);
          //assign temporary position to computer RSRP
          rx->GetNode()->GetObject<MobilityModel> ()->SetPosition (closestPos);

          if (compMethod == cv2x_LteProseHelper::SRSRP_STD)
            {
              rsrpRx = m_lteHelper->CalcSidelinkRsrp (txPower, ulEarfcn, ulBandwidth, tx, rx);
            } else 
            {
              rsrpRx = m_lteHelper->CalcSidelinkRsrpEval (txPower, ulBandwidth, tx, rx);
            }          
          //If receiver UE is not within RSRP* of X dBm of the transmitter UE then randomly reselect the receiver UE among the UEs that are within the RSRP of X dBm of the transmitter UE and are not part of a group already.
          NS_LOG_DEBUG ("\tCandidate Rx= " << rx->GetNode()->GetId() << " Rsrp=" << rsrpRx << " required=" << rsrpThreshold);
          if (rsrpRx >= rsrpThreshold)
            {
              //good receiver              
              NS_LOG_DEBUG ("\tAdding Rx to group");
              newGroup.Add (rx);
            }

          //restore position
          rx->GetNode()->GetObject<MobilityModel> ()->SetPosition (rxPos);
        }
      groups.push_back (newGroup);
    }

  return groups;
}

void
cv2x_LteProseHelper::PrintGroups (std::vector < NetDeviceContainer > groups)
{
  std::vector < NetDeviceContainer >::iterator gIt;
    for (gIt = groups.begin() ; gIt != groups.end() ; gIt++)
      {        
        std::cout << "Tx=" << (*gIt).Get (0)->GetNode()->GetId() << " Rx=";
        for (uint32_t i = 1; i < (*gIt).GetN(); i++)
          {
            std::cout << (*gIt).Get (i)->GetNode()->GetId() << " ";
          } 
        std::cout << std::endl;
      }
}

void
cv2x_LteProseHelper::PrintGroups (std::vector < NetDeviceContainer > groups, Ptr< OutputStreamWrapper > stream)
{
  //*stream->GetStream () << "TxNID\tRxNID" << std::endl;
  *stream->GetStream () << "TxNID\tRxNID\tTxIMSI\tRxIMSI" << std::endl;
  std::vector < NetDeviceContainer >::iterator gIt;
    for (gIt = groups.begin() ; gIt != groups.end() ; gIt++)
      {        
        if ((*gIt).GetN() < 2)
		{//No receiveres in group!
			*stream->GetStream () << (*gIt).Get (0)->GetNode()->GetId() << "\t0" << std::endl;
		}
		else
		{
		  for (uint32_t i = 1; i < (*gIt).GetN(); i++)
			{
			  //*stream->GetStream () << (*gIt).Get (0)->GetNode()->GetId() << "\t" << (*gIt).Get (i)->GetNode()->GetId() << std::endl;
			  *stream->GetStream () << (*gIt).Get (0)->GetNode()->GetId() << "\t" << (*gIt).Get (i)->GetNode()->GetId() << "\t" << (*gIt).Get(0)->GetObject<cv2x_LteUeNetDevice> ()->GetImsi () << "\t" << (*gIt).Get(i)->GetObject<cv2x_LteUeNetDevice> ()->GetImsi () <<std::endl;
			} 
        }
      }
}

void 
cv2x_LteProseHelper::ActivateSidelinkBearer (Time activationTime, NetDeviceContainer ues, Ptr<cv2x_LteSlTft> tft)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_lteHelper, "Sidelink activation requires cv2x_LteHelper to be registered with the cv2x_LteProseHelper");
  Simulator::Schedule (activationTime, &cv2x_LteProseHelper::DoActivateSidelinkBearer, this, ues, tft);
}

void 
cv2x_LteProseHelper::DoActivateSidelinkBearer (NetDeviceContainer ues, Ptr<cv2x_LteSlTft> tft)
{
  NS_LOG_FUNCTION (this);
  m_lteHelper->ActivateSidelinkBearer (ues, tft);
}

} // namespace ns3
