/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "vehicle-visualizer.h"

namespace ns3 {
  NS_LOG_COMPONENT_DEFINE("vehicleVisualizer");

  vehicleVisualizer::vehicleVisualizer()
  {
      // Set default ip and port
      m_ip="127.0.0.1";
      m_port=48110;

      // Set the default HTTP Node.js server port
      m_httpport=8080;

      m_is_connected=false;
      m_is_map_sent=false;
      m_is_server_active=false;
      m_serverpath=DEFAULT_NODEJS_SERVER_PATH;
  }

  vehicleVisualizer::vehicleVisualizer(int port)
  {
      // Set default ip
      m_ip="127.0.0.1";
      m_port=port;

      // Set the default HTTP Node.js server port
      m_httpport=8080;

      m_is_connected=false;
      m_is_map_sent=false;
      m_is_server_active=false;
      m_serverpath=DEFAULT_NODEJS_SERVER_PATH;
  }

  vehicleVisualizer::vehicleVisualizer(int port,std::string ipv4)
  {
      m_ip=ipv4;
      m_port=port;

      m_httpport=8080;

      m_is_connected=false;
      m_is_map_sent=false;
      m_is_server_active=false;
      m_serverpath=DEFAULT_NODEJS_SERVER_PATH;
  }

  int
  vehicleVisualizer::connectToServer()
  {
      m_sockfd=socketOpen();

      if(m_sockfd<0)
      {
          return -1;
      }

      m_is_connected=true;

      return 0;
  }

  vehicleVisualizer::~vehicleVisualizer()
  {
      // The destructor will attempt to send a termination message to the Node.js server only if a server
      // was successfully started with startServer()
      if(m_is_server_active)
      {
        terminateServer ();
        // Close the UDP socket, if it was opened before
        if(m_sockfd!=-1)
        {
          close(m_sockfd);
          m_sockfd=-1;
        }
       }
  }

  int
  vehicleVisualizer::sendMapDraw(double lat, double lon)
  {
      if(m_is_connected==false)
      {
          NS_FATAL_ERROR("Error: attempted to use a non-connected vehicle visualizer client.");
      }

      if(m_is_map_sent==true)
      {
          NS_FATAL_ERROR("Error in vehicle visualizer client: attempted to send twice a map draw message. This is not allowed.");
      }

      std::ostringstream oss;
      int send_rval=-1;

      oss.precision(7);
      oss<<"map,"<<lat<<","<<lon;

      std::string msg_string = oss.str();
      char *msg_buffer = new char[msg_string.length() + 1];
      std::copy(msg_string.c_str(), msg_string.c_str() + msg_string.length() + 1, msg_buffer);

      send_rval=send(m_sockfd,msg_buffer,msg_string.length()+1,0);

      delete[] msg_buffer;

      m_is_map_sent=true;

      return send_rval;
  }

  int
  vehicleVisualizer::sendObjectUpdate(std::string objID, double lat, double lon, double heading)
  {
      if(m_is_connected==false)
      {
          NS_FATAL_ERROR("Error: attempted to use a non-connected vehicle visualizer client.");
      }

      if(m_is_map_sent==false)
      {
          NS_FATAL_ERROR("Error in vehicle visualizer client: attempted to send an object update before sending the map draw message.");
      }

      std::ostringstream oss;
      int send_rval=-1;

      oss.precision(7);
      oss<<"object,"<<objID<<","<<lat<<","<<lon<<",";
      oss.precision(3);
      oss<<heading;

      std::string msg_string = oss.str();
      char *msg_buffer = new char[msg_string.length()+1];
      std::copy(msg_string.c_str(), msg_string.c_str() + msg_string.length() + 1, msg_buffer);

      send_rval=send(m_sockfd,msg_buffer,msg_string.length()+1,0);

      delete[] msg_buffer;

      return send_rval;
  }

  int
  vehicleVisualizer::sendObjectUpdate(std::string objID, double lat, double lon)
  {
      return sendObjectUpdate (objID,lat,lon,VIS_HEADING_INVALID);
  }

  int
  vehicleVisualizer::startServer()
  {
      std::string servercmd;

      int nodeCheckRval = std::system("command -v node > /dev/null");

      if(nodeCheckRval != 0)
      {
        NS_FATAL_ERROR("Error. Node.js does not seem to be installed. Please install it before using the ms-van3t vehicle visualizer.");
      }

      servercmd = "node " + m_serverpath + " " + std::to_string(m_httpport) + " &";
      int startCmdRval = std::system(servercmd.c_str());

      // If the result is 0, system() was able to successfully launch the command (which may fail afterwards, though)
      if (startCmdRval == 0)
      {
        NS_LOG_INFO("Used the following command to start up the vehicle visualizer Node.js server: " << servercmd);
        m_is_server_active = true;
      }
      else
      {
        NS_FATAL_ERROR("Cannot send the command for starting the Node.js server.");
      }

      // Wait 1 second for the server to come up
      sleep(1);

      return startCmdRval;
  }

  int
  vehicleVisualizer::terminateServer()
  {
      if(m_is_connected==false)
      {
          NS_FATAL_ERROR("Error: attempted to use a non-connected vehicle visualizer client.");
      }

      // Just send a message with the "terminate" string
      char terminatebuf[10]="terminate";

      int retval = send(m_sockfd,terminatebuf,9,0);
      if (retval==9)
      {
        m_is_connected=false;
        m_is_server_active=false;
      }

      return retval;
  }

  int
  vehicleVisualizer::socketOpen(void)
  {
      struct sockaddr_in saddr={};
      struct in_addr destIPaddr;
      int sockfd;

      sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

      if(sockfd<0)
      {
        perror("socket() error:");
        NS_FATAL_ERROR("Error! Cannot open the UDP socket for the vehicle visualizer.");
      }

      // Bind structure
      saddr.sin_family=AF_INET;
      saddr.sin_port=0;
      saddr.sin_addr.s_addr=INADDR_ANY;

      if(bind(sockfd,(struct sockaddr *) &(saddr),sizeof(saddr))<0) {
          perror("Cannot bind socket: bind() error");
          NS_FATAL_ERROR("Error! Cannot bind the UDP socket for the vehicle visualizer.");
      }

      // Get struct in_addr corresponding to the destination IP address
      if(inet_pton(AF_INET,m_ip.c_str(),&(destIPaddr))!=1) {
          fprintf(stderr,"Error in parsing the destination IP address.\n");
          NS_FATAL_ERROR("Error! Cannot parse the destination IP for the UDP socket for the vehicle visualizer.");
      }

      // Connect structure (re-using the same structure - sin_family should remain the same)
      saddr.sin_port=htons(m_port);
      saddr.sin_addr=destIPaddr;

      if(connect(sockfd,(struct sockaddr *) &(saddr),sizeof(saddr))<0) {
          perror("Cannot connect socket: connect() error");
          NS_FATAL_ERROR("Error! Cannot connect the UDP socket for the vehicle visualizer.");
      }

      return sockfd;
  }

  void
  vehicleVisualizer::setPort(int port)
  {
      // Set the default port when an invalid port is specified
      if(port>=1 && port<=65535)
      {
          m_port=port;
      }
      else
      {
          NS_LOG_ERROR("Error: called setPort for vehicleVisualizer with an invalid port number. Using the default port.");
          m_port=48110;
      }
  }

  void
  vehicleVisualizer::setHTTPPort(int port)
  {
      // Set the default port when an invalid port is specified
      if(port>=1 && port<=65535)
      {
          m_httpport=port;
      }
      else
      {
          NS_LOG_ERROR("Error: called setPort for vehicleVisualizer with an invalid port number. Using the default port.");
          m_httpport=8080;
      }
  }
}

