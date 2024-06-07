#ifndef NS3_DCC_H
#define NS3_DCC_H

#include <string>
#include <vector>
#include "ns3/MetricSupervisor.h"
#include "ns3/wifi-net-device.h"
#include "ns3/nr-net-device.h"
#include "ns3/net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/nr-ue-phy.h"
#include "ns3/traci-client.h"
#include "ns3/BSMap.h"
#include "ns3/nr-module.h"

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
    Undefined = -1,
    Relaxed,
    Active1,
    Active2,
    Active3,
    Restrictive
  } ReactiveState;

  typedef struct ReactiveParametersRelaxed
  {
    double m_cbr_threshold = 0.20;
    double m_tx_power = 33.0;
    long m_tx_inter_packet_time = 100;
    // double m_tx_data_rate = 3.0;
    double m_sensitivity = -95.0;
  } ReactiveParametersRelaxed;

  typedef struct ReactiveParametersActive1
  {
    double m_cbr_threshold = 0.30;
    double m_tx_power = 25.0;
    long m_tx_inter_packet_time = 200;
    // double m_tx_data_rate = 3.0;
    double m_sensitivity = -95.0;
  } ReactiveParametersActive1;

  typedef struct ReactiveParametersActive2
  {
    double m_cbr_threshold = 0.40;
    double m_tx_power = 20.0;
    long m_tx_inter_packet_time = 400;
    // double m_tx_data_rate = 3.0;
    double m_sensitivity = -95.0;
  } ReactiveParametersActive2;

  typedef struct ReactiveParametersActive3
  {
    double m_cbr_threshold = 0.50;
    double m_tx_power = 15.0;
    long m_tx_inter_packet_time = 500;
    // double m_tx_data_rate = 3.0;
    double m_sensitivity = -95.0;
  } ReactiveParametersActive3;

  typedef struct ReactiveParametersRestricted
  {
    double m_tx_power = -10.0;
    long m_tx_inter_packet_time = 1000;
    // double m_tx_data_rate = 12.0;
    double m_sensitivity = -65.0;
  } ReactiveParametersRestricted;

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
    * \brief Set the NrHelper pointer
    *
    * \param nrHelper NRHelper object
    */
  void SetNrHelper(Ptr<NrHelper> nrHelper) {
    if(m_metric_supervisor != nullptr && m_metric_supervisor->getChannelTechnology() == "Nr")
      {
        m_nr_helper = nrHelper;
      }
    else
      {
        NS_FATAL_ERROR ("Metric Supervisor is not set or the channel technology is not Nr");
      }
  };
  /**
    * \brief Set the CAM Basic Service
    *
    * \param nodeID id of the node
    * \param caBasicService basic service for CAMs
    */
  void AddCABasicService(std::string nodeID, Ptr<CABasicService> caBasicService) {m_caService[nodeID] = caBasicService;};
  /**
    * \brief Set the CPM Basic Service
    *
    * \param nodeID id of the node
    * \param cpBasicService basic service for CPMs
    */
  void AddCPBasicService(std::string nodeID, Ptr<CPBasicService> cpBasicService) {m_cpService[nodeID] = cpBasicService;};
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
  std::unordered_map<std::string, Ptr<CPBasicService>> m_cpService; //!< Pointer to the CPBasicService object
  std::unordered_map<std::string, Ptr<VRUBasicService>> m_vruService; //!< Pointer to the VRUBasicService object
  Ptr<NrHelper> m_nr_helper = nullptr; //!< Pointer to the NRHelper object

  DCC::ReactiveParametersRelaxed m_reactive_parameters_relaxed = DCC::ReactiveParametersRelaxed(); //!< Parameters for the Relaxed state
  DCC::ReactiveParametersActive1 m_reactive_parameters_active1 = DCC::ReactiveParametersActive1(); //!< Parameters for the Active1 state
  DCC::ReactiveParametersActive2 m_reactive_parameters_active2 = DCC::ReactiveParametersActive2(); //!< Parameters for the Active2 state
  DCC::ReactiveParametersActive3 m_reactive_parameters_active3 = DCC::ReactiveParametersActive3(); //!< Parameters for the Active3 state
  DCC::ReactiveParametersRestricted m_reactive_parameters_restricted = DCC::ReactiveParametersRestricted(); //!< Parameters for the Restricted state

  std::unordered_map<uint32_t, DCC::ReactiveState> m_veh_states; //!< Map to store the state of each vehicle

  double m_CBR_its;
  double m_alpha = 0.016;
  double m_beta = 0.0012;
  double m_CBR_target = 0.68;
  double m_delta_max = 0.03;
  double m_delta_min = 0.0006;
  double m_Gmax = 0.0005;
  double m_Gmin = -0.00025;
  double m_delta = 0;
  Time m_Tonpp = Seconds(0); //Duration of last transmission

  void ToffUpdateAfterCBRupdate();

};

}



#endif //NS3_DCC_H
