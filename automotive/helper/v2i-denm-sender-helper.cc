#include "v2i-denm-sender-helper.h"

#include "ns3/v2i-DENM-sender.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

DENMSenderHelper::DENMSenderHelper (uint16_t port)
{
  m_factory.SetTypeId (DENMSender::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

DENMSenderHelper::DENMSenderHelper (std::string protocol, Address address)
{
  m_factory.SetTypeId (DENMSender::GetTypeId ());
  m_factory.Set ("Protocol", StringValue (protocol));
  m_factory.Set ("Local", AddressValue (address));
}

void 
DENMSenderHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
DENMSenderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DENMSenderHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DENMSenderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
DENMSenderHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<DENMSender> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
