#include "ns3/log.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/btpHeader.h"
#include "ns3/btp.h"
#include "ns3/simple-btp-application.h"

#define PURPLE_CODE "\033[95m"
#define CYAN_CODE "\033[96m"
#define TEAL_CODE "\033[36m"
#define BLUE_CODE "\033[94m"
#define GREEN_CODE "\033[32m"
#define YELLOW_CODE "\033[33m"
#define LIGHT_YELLOW_CODE "\033[93m"
#define RED_CODE "\033[91m"
#define BOLD_CODE "\033[1m"
#define END_CODE "\033[0m"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("SimpleBtpApplication");
  NS_OBJECT_ENSURE_REGISTERED(SimpleBtpApplication);

  TypeId
  SimpleBtpApplication::GetTypeId()
  {
    static TypeId tid = TypeId("ns3::SimpleBtpApplication")
                            .AddConstructor<SimpleBtpApplication>()
                            .SetParent<Application>();
    return tid;
  }

  TypeId
  SimpleBtpApplication::GetInstanceTypeId() const
  {
    return SimpleBtpApplication::GetTypeId();
  }

  SimpleBtpApplication::SimpleBtpApplication()
  {
    m_port1 = 9999;
  }

  SimpleBtpApplication::~SimpleBtpApplication()
  {
  }

  void SimpleBtpApplication::SetupReceiveSocket(Ptr<Socket> socket, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
    if (socket->Bind(local) == -1)
    {
      NS_FATAL_ERROR("Failed to bind socket");
    }
  }

  void SimpleBtpApplication::StartApplication()
  {
    //Receive sockets
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
    m_recv_socket2 = Socket::CreateSocket(GetNode(), tid);

    SetupReceiveSocket(m_recv_socket1, m_port1);

    m_recv_socket1->SetRecvCallback(MakeCallback(&btp::receiveBTP, &m_btp));
    m_btp.addRxCallback (std::bind(&SimpleBtpApplication::DataIndication,this,std::placeholders::_1,std::placeholders::_2));

    //Send Socket
    m_send_socket = Socket::CreateSocket(GetNode(), tid);
    Ipv4Address destination ("10.1.1.2");
    m_send_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(destination), 9999));
    m_btp.setSocketTx(m_send_socket);
  }

  void SimpleBtpApplication::DataIndication(Ptr<Packet> packet, btpHeader header)
  {
    NS_LOG_FUNCTION(this << packet << header);

    uint8_t *buffer = new uint8_t[packet->GetSize ()];
    packet->CopyData(buffer, packet->GetSize ());
    std::string s = std::string((char*)buffer);
    NS_LOG_INFO(PURPLE_CODE << "BTPDataIndication" << END_CODE);

    NS_LOG_INFO(TEAL_CODE << " Received a Packet : " << s << " of size " << packet->GetSize() << END_CODE);

    NS_LOG_INFO(TEAL_CODE << " Destination port: " << header.GetDestinationPort ()<< "-- Source Port " << header.GetSourcePort ()<< END_CODE);
    NS_LOG_INFO(packet->ToString());

  }

  void SimpleBtpApplication::DataRequest(Ptr<Packet> packet, uint16_t servicePort, bool btpType)
  {
    NS_LOG_FUNCTION (this << packet << servicePort << btpType);
    btp::BTPDataRequest dataRequest;
    dataRequest.BTPType = btpType;
    dataRequest.destPort = servicePort;
    dataRequest.sourcePort = servicePort;
    dataRequest.data = packet;

    uint8_t *buffer = new uint8_t[packet->GetSize ()];
    packet->CopyData(buffer, packet->GetSize ());
    std::string s = std::string((char*)buffer);

    m_btp.sendBTP(dataRequest);
    NS_LOG_INFO(YELLOW_CODE << "BTPDataRequest : " << s << END_CODE);
  }

} // namespace ns3
