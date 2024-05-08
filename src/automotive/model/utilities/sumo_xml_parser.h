#ifndef SUMO_XML_PARSER_H
#define SUMO_XML_PARSER_H

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>
#include <fstream>
#include <string>
#include <vector>

namespace ns3 {

  int XML_rou_count_vehicles(xmlDocPtr doc);
  int XML_rou_count_pedestrians(xmlDocPtr doc);
  std::vector<std::tuple<std::string, float, float>> XML_poli_count_stations(std::ifstream &file);
}

#endif
