#include "cv2x_lte-v2x-helper.h"
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/cv2x_epc-helper.h>
#include <ns3/angles.h>
#include <ns3/random-variable-stream.h>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteV2xHelper"); 

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteV2xHelper);

cv2x_LteV2xHelper::cv2x_LteV2xHelper (void)
{
    NS_LOG_FUNCTION(this);
}

cv2x_LteV2xHelper::~cv2x_LteV2xHelper(void)
{
    NS_LOG_FUNCTION(this);
}

TypeId 
cv2x_LteV2xHelper::GetTypeId(void)
{   
    static TypeId
        tid = 
        TypeId ("ns3::cv2x_LteV2xHelper")
        .SetParent<Object> ()
        .SetGroupName("Lte")
        .AddConstructor<cv2x_LteV2xHelper> ()
    ;
    return tid;
}

void 
cv2x_LteV2xHelper::DoDispose ()
{
    NS_LOG_FUNCTION (this);
    Object::DoDispose(); 
}

void 
cv2x_LteV2xHelper::SetLteHelper(Ptr<cv2x_LteHelper> h)
{
    NS_LOG_FUNCTION (this << h);
    m_lteHelper = h;
}

NetDeviceContainer 
cv2x_LteV2xHelper::RemoveNetDevice (NetDeviceContainer container, Ptr<NetDevice> item)
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

std::vector<NetDeviceContainer>
cv2x_LteV2xHelper::AssociateForBroadcast(double txPower, double ulEarfcn, double ulBandwidth, NetDeviceContainer ues, double rsrpThreshold, uint32_t ntransmitters, SrsrpMethod_t compMethod)
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
	            if (compMethod == cv2x_LteV2xHelper::SRSRP_STD)
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

std::vector < NetDeviceContainer > 
cv2x_LteV2xHelper::AssociateForV2xBroadcast (NetDeviceContainer ues, uint32_t ntransmitters)
{
  std::vector < NetDeviceContainer > groups; //groups created

  for (uint32_t i = 0 ; i < ntransmitters ; i++)
    {
      Ptr<NetDevice> tx = ues.Get (i);

      NetDeviceContainer remainingUes; //list of UEs not assigned to groups
      remainingUes.Add (ues);
      remainingUes = RemoveNetDevice (remainingUes, tx); 
      //prepare group for this transmitter
      NetDeviceContainer newGroup (tx);
      uint32_t nRxDevices = remainingUes.GetN ();

      for (uint32_t j = 0 ; j < nRxDevices; j++)
        {
          Ptr<NetDevice> rx = remainingUes.Get(j);
          //good receiver              
          NS_LOG_DEBUG ("\tAdding Rx to group");
          newGroup.Add (rx);
        }
      groups.push_back (newGroup);
    }
  return groups;
}

void 
cv2x_LteV2xHelper::PrintGroups (std::vector<NetDeviceContainer> groups)
{
    std::vector<NetDeviceContainer>::iterator gIt;
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
cv2x_LteV2xHelper::PrintGroups (std::vector < NetDeviceContainer > groups, Ptr< OutputStreamWrapper > stream)
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
cv2x_LteV2xHelper::ActivateSidelinkBearer (Time activationTime, NetDeviceContainer ues, Ptr<cv2x_LteSlTft> tft)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_lteHelper, "Sidelink activation requires cv2x_LteHelper to be registered with the cv2x_LteV2xHelper");
  Simulator::Schedule (activationTime, &cv2x_LteV2xHelper::DoActivateSidelinkBearer, this, ues, tft);
}

void 
cv2x_LteV2xHelper::DoActivateSidelinkBearer (NetDeviceContainer ues, Ptr<cv2x_LteSlTft> tft)
{
  NS_LOG_FUNCTION (this);
  m_lteHelper->ActivateSidelinkBearer (ues, tft);
}


}