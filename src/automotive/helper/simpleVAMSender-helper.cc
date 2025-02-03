#include "simpleVAMSender-helper.h"

#include "ns3/simpleVAMSender-gps-tc.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

simpleVAMSenderHelper::simpleVAMSenderHelper ()
{
  m_factory.SetTypeId (simpleVAMSender::GetTypeId ());
}


void 
simpleVAMSenderHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
simpleVAMSenderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
simpleVAMSenderHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
simpleVAMSenderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
simpleVAMSenderHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<simpleVAMSender> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
