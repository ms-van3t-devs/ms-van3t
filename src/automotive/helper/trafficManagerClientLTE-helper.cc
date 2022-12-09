#include "trafficManagerClientLTE-helper.h"
#include "ns3/trafficManagerClientLTE.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {
trafficManagerClientLTEHelper::trafficManagerClientLTEHelper()
{
  m_factory.SetTypeId (trafficManagerClientLTE::GetTypeId ());
}

void
trafficManagerClientLTEHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
trafficManagerClientLTEHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
trafficManagerClientLTEHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
trafficManagerClientLTEHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
trafficManagerClientLTEHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<trafficManagerClientLTE> ();
  node->AddApplication (app);

  return app;
}
}
