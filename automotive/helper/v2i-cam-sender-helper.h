#ifndef V2I_CAM_SENDER_HELPER_H
#define V2I_CAM_SENDER_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/traci-client.h"
#include "ns3/inet-socket-address.h"
#include "ns3/string.h"
#include "ns3/names.h"

namespace ns3 {

/**
 * \ingroup TrafficInfo
 * \brief Create an application which sends a UDP packet and waits for an echo of this packet
 */
class CAMSenderHelper
{
public:
  /**
   * Create TrafficInfoClientHelper which will make life easier for people trying
   * to set up simulations with echos.
   *
   * \param ip The IP address of the remote Traffic Info server
   * \param port The port number of the remote Traffic Info server
   */
  CAMSenderHelper (uint16_t port);
  /**
   * Create TrafficInfoClientHelper which will make life easier for people trying
   * to set up simulations with echos.
   *
   * \param ip The IPv4 address of the remote Traffic Info server
   * \param port The port number of the remote Traffic Info server
   */
  CAMSenderHelper (std::string protocol, Address address);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Create a TrafficInfoServerApplication on the specified Node.
   *
   * \param node The node on which to create the Application.  The node is
   *             specified by a Ptr<Node>.
   *
   * \returns An ApplicationContainer holding the Application created,
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a Traffic Info client application on the specified node.  The Node
   * is provided as a string name of a Node that has been previously 
   * associated using the Object Name Service.
   *
   * \param nodeName The name of the node on which to create the TrafficInfoClientApplication
   *
   * \returns An ApplicationContainer that holds a Ptr<Application> to the 
   *          application created
   */
  ApplicationContainer Install (std::string nodeName) const;

  /**
   * \param c the nodes
   *
   * Create one Traffic Info client application on each of the input nodes
   *
   * \returns the applications created, one application per input node.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::TrafficInfoClient on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an TrafficInfoClient will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* TRAFFIC_INFO_HELPER_H */
