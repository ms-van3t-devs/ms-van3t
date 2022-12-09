#include "trafficManagerServer80211p-helper.h"

#include "ns3/trafficManagerServer80211p.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

trafficManagerServer80211pHelper::trafficManagerServer80211pHelper ()
{
  m_factory.SetTypeId (trafficManagerServer80211p::GetTypeId ());
}


void
trafficManagerServer80211pHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
trafficManagerServer80211pHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
trafficManagerServer80211pHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
trafficManagerServer80211pHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
trafficManagerServer80211pHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<trafficManagerServer80211p> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
