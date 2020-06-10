#include "sumo_xml_parser.h"

namespace ns3
{
  int XML_rou_count_vehicles(xmlDocPtr doc)
  {
      xmlXPathContextPtr xpathCtx;
      xmlXPathObjectPtr xpathObj;

      int num_vehicles=-1;

      // Create xPath to select all the 'vehicle' nodes in the rou.xml file
      xpathCtx = xmlXPathNewContext(doc);
      if(xpathCtx == NULL) {
          return -1;
      }

      // Evaluate the xPath expression "//vehicle" to look for all the "<vehicle>" elements
      xpathObj = xmlXPathEvalExpression((xmlChar *)"//vehicle",xpathCtx);
      if(xpathObj == NULL || xpathObj->nodesetval==NULL) {
          xmlXPathFreeContext(xpathCtx);
          return -1;
      }

      num_vehicles = xpathObj->nodesetval->nodeNr;

      xmlXPathFreeObject(xpathObj);
      xmlXPathFreeContext(xpathCtx);

      return num_vehicles;
  }
}
