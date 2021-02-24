#ifndef SIMPLECAMSENDER_HELPER_H
#define SIMPLECAMSENDER_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/traci-client.h"
#include "ns3/gps-tc.h"
#include "ns3/inet-socket-address.h"
#include "ns3/string.h"
#include "ns3/names.h"

namespace ns3 {

class simpleCAMSenderHelper
{
public:
  simpleCAMSenderHelper ();

  void SetAttribute (std::string name, const AttributeValue &value);

  ApplicationContainer Install (Ptr<Node> node) const;

  ApplicationContainer Install (std::string nodeName) const;

  ApplicationContainer Install (NodeContainer c) const;

private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* SIMPLECAMSENDER_HELPER_H */
