#include "trafficManagerClient80211p-helper.h"
#include "ns3/trafficManagerClient80211p.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {
trafficManagerClient80211pHelper::trafficManagerClient80211pHelper()
{
  m_factory.SetTypeId (trafficManagerClient80211p::GetTypeId ());
}

void
trafficManagerClient80211pHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
trafficManagerClient80211pHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
trafficManagerClient80211pHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
trafficManagerClient80211pHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
trafficManagerClient80211pHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<trafficManagerClient80211p> ();
  node->AddApplication (app);

  return app;
}
}
