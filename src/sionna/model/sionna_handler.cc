#include "sionna_handler.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("");

std::string sionna_server_ip = "";
int sionna_port = 8103;
int sionna_socket;
struct sockaddr_in sionna_addr = {};
struct in_addr sionna_destIPaddr;
bool is_socket_created = false;
std::unordered_map<std::string, SionnaPosition> vehiclePositions;
bool sionna_verbose = false;
bool sionna_local_machine = false;

std::vector<bool> sionna_los_status = {false, false, false};

// THIS ONE WORKS LOCALLY!
void connect_now_local_machine() {
    printf("Starting connect_now\n");
    sionna_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sionna_addr.sin_family = AF_INET;
    sionna_addr.sin_port = 0;
    sionna_addr.sin_addr.s_addr = INADDR_ANY;
    bind(sionna_socket, (struct sockaddr *)&(sionna_addr), sizeof(sionna_addr));

    sionna_addr.sin_port = htons(sionna_port);
    sionna_addr.sin_addr = sionna_destIPaddr;
    connect(sionna_socket, (struct sockaddr *)&(sionna_addr), sizeof(sionna_addr));
    is_socket_created = true;
}

void
connect_now() {
  printf("Starting connect_now\n");

  // Create UDP socket
  sionna_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sionna_socket < 0) {
      perror("Socket creation failed");
      NS_FATAL_ERROR("Error! Unable to create UDP socket.");
    }

  // Set up local address for binding on ns-3 machine (specific IP if needed or INADDR_ANY)
  sionna_addr.sin_family = AF_INET;
  sionna_addr.sin_port = htons(sionna_port);  // Bind port on ns-3 side
  sionna_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind the socket to the local address and port
  if (bind(sionna_socket, (struct sockaddr *)&sionna_addr, sizeof(sionna_addr)) < 0) {
      perror("Error binding socket on ns-3");
      NS_FATAL_ERROR("Binding failed on ns-3 side.");
    }

  // Set the destination IP and port for the Python machine
  inet_aton(sionna_server_ip.c_str(), &sionna_destIPaddr);
  sionna_addr.sin_family = AF_INET;
  sionna_addr.sin_port = htons(sionna_port);
  sionna_addr.sin_addr = sionna_destIPaddr;

  // Connect the socket to the Python machine
  if (connect(sionna_socket, (struct sockaddr *)&sionna_addr, sizeof(sionna_addr)) < 0) {
      perror("Error connecting socket on ns-3");
      NS_FATAL_ERROR("Connection failed on ns-3 side.");
    }

  // Set a timeout on the socket for receiving responses
  struct timeval tv;
  tv.tv_sec = 60;  // 60-second timeout
  tv.tv_usec = 0;
  setsockopt(sionna_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

  is_socket_created = true;
  printf("Socket successfully created and connected to Python machine\n");
}

void
checkConnection ()
{
  if (!is_socket_created) {
      printf("No socket, creating new one \n");
      if (sionna_local_machine)
        {
          connect_now_local_machine();
        }
      else
        {
          connect_now();
        }
    }
}

int
sendString2sionna_UDP(const std::string& str) {
  checkConnection();
  int send_statusCode = send(sionna_socket, str.c_str(), str.length(), 0);
  if (send_statusCode == -1) {
      perror("Error while sending string to Sionna");
      NS_FATAL_ERROR("Error! Impossible to send string to Sionna via the UDP socket.");
    }
  return send_statusCode;
}

std::string
getFromSionna_UDP() {
  checkConnection();
  char msg_buffer[100];
  int received_payload = recv(sionna_socket, msg_buffer, sizeof(msg_buffer), 0);
  if (received_payload == -1) {
      perror("Error while receiving data from Sionna");
      NS_FATAL_ERROR("Error! Impossible to receive data from Sionna via the UDP socket.");
    } else {
      msg_buffer[received_payload] = '\0'; // Null-terminate the string
      return std::string(msg_buffer);
    }
}

void
updateLocationSionna(std::string veh, std::string x, std::string y, std::string z, std::string angle) {
  bool updated = false;
  std::string confirmation = "UPDATED" + veh;

  std::string message_for_Sionna = "map_update:" + veh + "," + x + "," + y + "," + z + "," + angle;
  sendString2sionna_UDP(message_for_Sionna);
  //std::cout << "LU: " << message_for_Sionna << std::endl;

  while (!updated) {
      std::string server_response = getFromSionna_UDP();

      if (server_response == confirmation) {
          vehiclePositions[veh] = {x, y, z, angle};
          updated = true;
        }
    }
}

double
getPathGainFromSionna(Vector a_position, Vector b_position) {
  bool got_response = false;
  std::string found_veh_a_id, found_veh_b_id;

  for (const auto& [veh, position] : vehiclePositions) {
      if (a_position.x == 0 && a_position.y == 0 && a_position.z == 0) {
          found_veh_a_id = std::to_string(0);
        }

      if (b_position.x == 0 && b_position.y == 0 && b_position.z == 0) {
          found_veh_b_id = std::to_string(0);
        }

      if (position.x == std::to_string(a_position.x) && position.y == std::to_string(a_position.y)) {
          found_veh_a_id = veh;
        }

      if (position.x == std::to_string(b_position.x) && position.y == std::to_string(b_position.y)) {
          found_veh_b_id = veh;
        }
    }

  std::string message_for_Sionna = "calc_request:" + found_veh_a_id + "," + found_veh_b_id;
  sendString2sionna_UDP(message_for_Sionna);
  // std::cout << message_for_Sionna << std::endl;

  while (!got_response) {
      std::string server_response = getFromSionna_UDP();

      if (server_response.rfind("CALC_DONE:", 0) == 0) {
          std::string value_str = server_response.substr(10);
          try {
              double value = std::stof(value_str);
              if (found_veh_b_id != "0") {
                  if (sionna_verbose)
                    {
                      printf("tx_id: %s, rx_id: %s, ", found_veh_a_id.c_str(), found_veh_b_id.c_str());
                    }

                  // To fix missing delays in nr-v2x
                  //std::string log_delays = std::to_string(0) + "," + std::to_string(0);
                  //LogProgress(0, log_delays);

                  std::string log_names = found_veh_a_id + "," + found_veh_b_id;
                  LogProgress(1, log_names);
                }
              return value;
            } catch (const std::invalid_argument& e) {
              std::cerr << "Invalid response format: " << server_response << std::endl;
            } catch (const std::out_of_range& e) {
              std::cerr << "Value out of range: " << server_response << std::endl;
            }
        }
      got_response = true;
    }
  return 0.0;  // default return if response not processed
}

double
getPropagationDelayFromSionna(Vector a_position, Vector b_position) {
  bool got_response = false;
  std::string found_veh_a_id, found_veh_b_id;

  for (const auto& [veh, position] : vehiclePositions) {
      if (a_position.x == 0 && a_position.y == 0 && a_position.z == 0) {
          found_veh_a_id = std::to_string(0);
        }

      if (b_position.x == 0 && b_position.y == 0 && b_position.z == 0) {
          found_veh_b_id = std::to_string(0);
        }

      if (position.x == std::to_string(a_position.x) && position.y == std::to_string(a_position.y)) {
          found_veh_a_id = veh;
        }

      if (position.x == std::to_string(b_position.x) && position.y == std::to_string(b_position.y)) {
          found_veh_b_id = veh;
        }
    }

  std::string message_for_Sionna = "get_delay:" + found_veh_a_id + "," + found_veh_b_id;
  sendString2sionna_UDP(message_for_Sionna);
  //std::cout << message_for_Sionna << std::endl;

  while (!got_response) {
      std::string server_response = getFromSionna_UDP();

      if (server_response.rfind("DELAY:", 0) == 0) {
          std::string value_str = server_response.substr(6);
          try {
              double value = std::stof(value_str);
              return value;
            } catch (const std::invalid_argument& e) {
              std::cerr << "Invalid response format: " << server_response << std::endl;
            } catch (const std::out_of_range& e) {
              std::cerr << "Value out of range: " << server_response << std::endl;
            }
        }
      got_response = true;
    }
  return 0.0;  // default return if response not processed
}

std::string
getLOSStatusFromSionna(Vector a_position, Vector b_position) {
  bool got_response = false;
  std::string found_veh_a_id, found_veh_b_id;

  for (const auto& [veh, position] : vehiclePositions) {
      if (a_position.x == 0 && a_position.y == 0 && a_position.z == 0) {
          found_veh_a_id = std::to_string(0);
        }

      if (b_position.x == 0 && b_position.y == 0 && b_position.z == 0) {
          found_veh_b_id = std::to_string(0);
        }

      if (position.x == std::to_string(a_position.x) && position.y == std::to_string(a_position.y)) {
          found_veh_a_id = veh;
        }

      if (position.x == std::to_string(b_position.x) && position.y == std::to_string(b_position.y)) {
          found_veh_b_id = veh;
        }
    }

  std::string message_for_Sionna = "are_they_LOS:" + found_veh_a_id + "," + found_veh_b_id;
  sendString2sionna_UDP(message_for_Sionna);
  //std::cout << message_for_Sionna << std::endl;

  while (!got_response) {
      std::string server_response = getFromSionna_UDP();

      if (server_response.rfind("LOS:", 0) == 0) {
          std::string value_str = server_response.substr(4);
          try {
              return value_str;
            } catch (const std::invalid_argument& e) {
              std::cerr << "Invalid response format: " << server_response << std::endl;
            } catch (const std::out_of_range& e) {
              std::cerr << "Value out of range: " << server_response << std::endl;
            }
        }
      got_response = true;
    }
  return "Null";  // default return if response not processed
}

void
LogProgress(int piece, std::string chunk) {

  bool first_row_of_log = true;
  std::ofstream csv_file("src/sionna/sionna_log.csv", std::ios::out | std::ios::app);

  csv_file.seekp (0, std::ios::end);
  if (csv_file.tellp() == 0)
    {
      csv_file << "delay_ns3_ms,sionna_delay_ms,tx_id,rx_id,pathloss_ns3,pathloss_sionna,LOS" << std::endl;
    }

  // Piece 0 = delays
  // Piece 1 = tx_id & rx_id
  // Piece 2 = pathloss & LOS

  static std::string row = "";

  // Process the current piece
  if (piece == 0)
    {
      // Start a new row with the initial data
      row = chunk + ",";
      sionna_los_status[0] = true;
    }
  else if (piece == 1)
    {
      // Append the second piece of data to the row
      row += chunk + ",";
      sionna_los_status[1] = true;
    }
  else if (piece == 2)
    {
      // Append the final piece of data and write the row to the CSV file
      row += chunk;
      sionna_los_status[2] = true;
      if (sionna_los_status[1] && sionna_los_status[2])
        {
          csv_file << row << std::endl;
        }
      sionna_los_status = {false, false, false};
      row = "";  // Clear the row for the next set of data
    }
}
}
