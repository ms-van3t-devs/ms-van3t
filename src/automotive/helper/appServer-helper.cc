#include "appServer-helper.h"

#include "ns3/appServer.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

appServerHelper::appServerHelper ()
{
  m_factory.SetTypeId (appServer::GetTypeId ());
}


void 
appServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
appServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
appServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
appServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
appServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<appServer> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
