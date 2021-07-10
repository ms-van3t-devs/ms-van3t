#ifndef NS3_UDP_ARQ_APPLICATION_H
#define NS3_UDP_ARQ_APPLICATION_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/btpHeader.h"
#include "ns3/btp.h"

using namespace ns3;

namespace ns3
{
  class SimpleBtpApplication : public Application 
  {
    public:
      SimpleBtpApplication ();
      virtual ~SimpleBtpApplication ();

      static TypeId GetTypeId ();
      virtual TypeId GetInstanceTypeId () const;

      /** \brief handles incoming packets on port 9999
       */
      void DataIndication (Ptr<Packet> packet, btpHeader header);

      /** \brief Send an outgoing packet. This creates a new socket every time (not the best solution)
      */
      void DataRequest (Ptr<Packet> packet, uint16_t servicePort, bool btpType);

    private:
      
      
      void SetupReceiveSocket (Ptr<Socket> socket, uint16_t port);
      virtual void StartApplication ();


      Ptr<Socket> m_recv_socket1; /**< A socket to receive on a specific port */
      Ptr<Socket> m_recv_socket2; /**< A socket to receive on a specific port */
      btp m_btp; // BTP object
      uint16_t m_port1; 
      uint16_t m_port2;

      Ptr<Socket> m_send_socket; /**< A socket to listen on a specific port */
  };
}

#endif
