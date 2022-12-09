#include "trafficManagerServerLTE-helper.h"

#include "ns3/trafficManagerServerLTE.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

trafficManagerServerLTEHelper::trafficManagerServerLTEHelper ()
{
  m_factory.SetTypeId (trafficManagerServerLTE::GetTypeId ());
}


void
trafficManagerServerLTEHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
trafficManagerServerLTEHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
trafficManagerServerLTEHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
trafficManagerServerLTEHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
trafficManagerServerLTEHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<trafficManagerServerLTE> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
