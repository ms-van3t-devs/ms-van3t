#include "obuEmu-helper.h"

#include "ns3/obuEmu.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

obuEmuHelper::obuEmuHelper ()
{
  m_factory.SetTypeId (obuEmu::GetTypeId ());
}


void 
obuEmuHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
obuEmuHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
obuEmuHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
obuEmuHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
obuEmuHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<obuEmu> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
