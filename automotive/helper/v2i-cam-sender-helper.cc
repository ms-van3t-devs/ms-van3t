#include "v2i-cam-sender-helper.h"

#include "ns3/v2i-CAM-sender.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

CAMSenderHelper::CAMSenderHelper (uint16_t port)
{
  m_factory.SetTypeId (CAMSender::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

CAMSenderHelper::CAMSenderHelper (std::string protocol, Address address)
{
  m_factory.SetTypeId (CAMSender::GetTypeId ());
  m_factory.Set ("Protocol", StringValue (protocol));
  m_factory.Set ("Local", AddressValue (address));
}

void 
CAMSenderHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
CAMSenderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
CAMSenderHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
CAMSenderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
CAMSenderHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<CAMSender> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
