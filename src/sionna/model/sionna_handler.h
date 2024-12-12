#ifndef SIONNA_HANDLER_H
#define SIONNA_HANDLER_H

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

void connect_now ();
void connect_now_local_machine();
void checkConnection ();
int sendString2sionna_UDP (const std::string &str);
std::string getFromSionna_UDP ();
void updateLocationSionna (std::string veh, std::string x, std::string y, std::string z,
                           std::string angle);
double getRxPowerFromSionna (Vector a_position, Vector b_position);
double getPropagationDelayFromSionna (Vector a_position, Vector b_position);
std::string getLOSStatusFromSionna (Vector a_position, Vector b_position);
void LogProgress (int piece, std::string chunk);

extern std::string sionna_server_ip;
extern int sionna_port;
extern int sionna_socket;
extern struct sockaddr_in sionna_addr;
extern struct in_addr sionna_destIPaddr;
extern bool is_socket_created;
extern std::unordered_map<std::string, SionnaPosition> vehiclePositions;
extern bool sionna_verbose;
extern bool sionna_local_machine;

}

#endif /* SIONNA_HANDLER_H */