#include "appSample-helper.h"

#include "ns3/appSample.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

appSampleHelper::appSampleHelper ()
{
  m_factory.SetTypeId (appSample::GetTypeId ());
}


void 
appSampleHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
appSampleHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
appSampleHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
appSampleHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
appSampleHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<appSample> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
