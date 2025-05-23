#ifndef NS3_DCC_H
#define NS3_DCC_H

#include <string>
#include <vector>
#include "ns3/MetricSupervisor.h"
#include "ns3/wifi-net-device.h"
//#include "ns3/nr-net-device.h"
#include "ns3/net-device.h"
#include "ns3/wifi-phy.h"
//#include "ns3/nr-ue-phy.h"
#include "ns3/traci-client.h"
#include "ns3/BSMap.h"
//#include "ns3/nr-module.h"

namespace ns3 {

/**
 * \ingroup automotive
 *
 * \brief This class implements the Decentralized Congestion Control (DCC) algorithm.
 *
 * This class provides capabilities for computing both the Reactive DCC and the Proactive DCC.
 */

class DCC : public Object
{
public:

  typedef enum ReactiveState
  {
    Relaxed,
    Active1,
    Active2,
    Active3,
    Restrictive
  } ReactiveState;

  typedef struct ReactiveParameters {
    double cbr_threshold;
    double tx_power;
    long tx_inter_packet_time;
    double sensitivity;
  } ReactiveParameters;

  static TypeId GetTypeId(void);
  /**
   * \brief Default constructor
   *
   */
  DCC ();
  ~DCC();

  /**
   * \brief Set the metric supervisor
   *
   * \param metric_supervisor Pointer to the MetricSupervisor object
   */
  void SetMetricSupervisor(Ptr<MetricSupervisor> metric_supervisor) {m_metric_supervisor = metric_supervisor;};
  /**
   * \brief Set the TraciClient
   *
   * \param traci_client Pointer to the TraciClient object
   */
  void SetTraciClient(Ptr<TraciClient> traci_client) {m_traci_client = traci_client;};
   /**
    * \brief Set the reactive interval
    *
    * \param reactive_interval Time interval for DCC
    */
  void SetDCCInterval(Time dcc_interval) {m_dcc_interval = dcc_interval;};
 /**
    * \brief Set the CAM Basic Service
    *
    * \param nodeID id of the node
    * \param caBasicService basic service for CAMs
    */
  void AddCABasicService(std::string nodeID, Ptr<CABasicService> caBasicService) {m_caService[nodeID] = caBasicService;};
  /**
    * \brief Set the CAM Basic Service (Version 1)
    *
    * \param nodeID id of the node
    * \param caBasicService basic service for CAMs
    */
  void AddCABasicServiceV1(std::string nodeID, Ptr<CABasicServiceV1> caBasicService) {m_caServiceV1[nodeID] = caBasicService;};
  /**
    * \brief Set the CPM Basic Service
    *
    * \param nodeID id of the node
    * \param cpBasicService basic service for CPMs
    */
  void AddCPBasicService(std::string nodeID, Ptr<CPBasicService> cpBasicService) {m_cpService[nodeID] = cpBasicService;};
  /**
    * \brief Set the CPM Basic Service (Version 1)
    *
    * \param nodeID id of the node
    * \param cpBasicService basic service for CPMs
    */
  void AddCPBasicService(std::string nodeID, Ptr<CPBasicServiceV1> cpBasicService) {m_cpServiceV1[nodeID] = cpBasicService;};
  /**
    * \brief Set the VRU Basic Service
    *
    * \param nodeID id of the node
    * \param vruBasicService basic service for VRUs
    */
  void AddVRUBasicService(std::string nodeID, Ptr<VRUBasicService> vruBasicService) {m_vruService[nodeID] = vruBasicService;};
    /**
     * \brief Set the DCC modality (reactive or proactive)
     *
     * \param reactive Boolean to indicate if the DCC is reactive or proactive
     */
  void SetReactive(bool reactive) {m_reactive = reactive;};
  /**
   * \brief Start the reactive DCC mechanism
   *
   */
  void reactiveDCC();
  /**
   * \brief Start the adaptive DCC mechanism
   *
   */
  void adaptiveDCC();


private:

  bool m_reactive = true; //!< Boolean to indicate if the DCC is reactive or proactive
  Time m_dcc_interval = Time(-1.0); //!< Time interval for DCC
  Ptr<MetricSupervisor> m_metric_supervisor = NULL; //!< Pointer to the MetricSupervisor object
  Ptr<TraciClient> m_traci_client = NULL; //!< Pointer to the TraciClient object
  std::unordered_map<std::string, Ptr<CABasicService>> m_caService; //!< Pointer to the CABasicService object
  std::unordered_map<std::string, Ptr<CABasicServiceV1>> m_caServiceV1; //!< Pointer to the CABasicService object
  std::unordered_map<std::string, Ptr<CPBasicService>> m_cpService; //!< Pointer to the CPBasicService object
  std::unordered_map<std::string, Ptr<CPBasicServiceV1>> m_cpServiceV1; //!< Pointer to the CPBasicService object
  std::unordered_map<std::string, Ptr<VRUBasicService>> m_vruService; //!< Pointer to the VRUBasicService object
  //Ptr<NrHelper> m_nr_helper = nullptr; //!< Pointer to the NRHelper object

  std::unordered_map<std::string, DCC::ReactiveState> m_vehicle_state; //!< Map to store the state of each vehicle

  std::unordered_map<std::string, double> m_CBR_its;
  double m_alpha = 0.016;
  double m_beta = 0.0012;
  double m_CBR_target = 0.68;
  double m_delta_max = 0.03;
  double m_delta_min = 0.0006;
  double m_Gmax = 0.0005;
  double m_Gmin = -0.00025;
  double m_delta = 0;

  std::unordered_map<ReactiveState, ReactiveParameters> m_reactive_parameters;

};

}



#endif //NS3_DCC_H
