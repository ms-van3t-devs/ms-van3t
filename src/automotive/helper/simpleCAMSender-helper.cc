#include "simpleCAMSender-helper.h"

#include "ns3/simpleCAMSender-gps-tc.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

simpleCAMSenderHelper::simpleCAMSenderHelper ()
{
  m_factory.SetTypeId (simpleCAMSender::GetTypeId ());
}


void 
simpleCAMSenderHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
simpleCAMSenderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
simpleCAMSenderHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
simpleCAMSenderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
simpleCAMSenderHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<simpleCAMSender> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
