#ifndef SIONNA_CONNECTION_HANDLER_H
#define SIONNA_CONNECTION_HANDLER_H

#include <ns3/core-module.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <unistd.h>
#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include "ns3/object.h"

namespace ns3 {

typedef struct SionnaPosition
{
  std::string x;
  std::string y;
  std::string z;
  std::string angle;
} SionnaPosition;

// Connection Handling Functions
void connectToSionnaLocally();
void connectToSionnaRemotely ();
void checkConnection ();

// Basic Message Exchange Functions
int sendMessageToSionna (const std::string &str);
std::string receiveMessageFromSionna ();

// Utilities
void updateLocationInSionna(std::string obj_id, Vector Position, double Angle, Vector Velocity);
double getPathGainFromSionna (Vector a_position, Vector b_position);
double getPropagationDelayFromSionna (Vector a_position, Vector b_position);
std::string getLOSStatusFromSionna (Vector a_position, Vector b_position);

// Other
void logProgress (int piece, std::string chunk);
void shutdownSionnaServer ();

extern std::string sionna_server_ip;
extern int sionna_port;
extern int sionna_socket;
extern struct sockaddr_in sionna_addr;
extern struct in_addr sionna_destIPaddr;
extern bool is_socket_created;
extern std::unordered_map<std::string, SionnaPosition> vehiclePositions;
extern bool sionna_verbose;
extern bool sionna_local_machine;
extern std::vector<bool> sionna_los_status;
extern bool sionna_los;

}

#endif /* SIONNA-CONNECTION-HANDLER_H */