#ifndef NS3_DCC_H
#define NS3_DCC_H

#include <string>
#include <vector>
#include "ns3/MetricSupervisor.h"
#include "ns3/wifi-net-device.h"
#include "ns3/net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-mac.h"
#include "ns3/traci-client.h"
#include "ns3/BSMap.h"

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

  typedef struct ReactiveParametersRelaxed
  {
    double m_tx_power = 33.0;
    long m_tx_inter_packet_time = 100;
    // double m_tx_data_rate = 3.0;
    double m_sensitivity = -95.0;
  } ReactiveParametersRelaxed;

  typedef struct ReactiveParametersActive1
  {
    double m_tx_power = 25.0;
    long m_tx_inter_packet_time = 200;
    // double m_tx_data_rate = 3.0;
    double m_sensitivity = -95.0;
  } ReactiveParametersActive1;

  typedef struct ReactiveParametersActive2
  {
    double m_tx_power = 20.0;
    long m_tx_inter_packet_time = 400;
    // double m_tx_data_rate = 3.0;
    double m_sensitivity = -95.0;
  } ReactiveParametersActive2;

  typedef struct ReactiveParametersActive3
  {
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

  typedef struct CBRThresholds
  {
    double m_cbr_threshold_relaxed = 0.20;
    double m_cbr_threshold_active1 = 0.30;
    double m_cbr_threshold_active2 = 0.40;
    double m_cbr_threshold_active3 = 0.50;
  } CBRThresholds;

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
     * \brief Set the DCC modality (reactive or proactive)
     *
     * \param reactive Boolean to indicate if the DCC is reactive or proactive
     */
  /** \brief Set the BSMap
   *
   * \param bs_map Pointer to the BSMap object
   */
  void SetBSMap(Ptr<BSMap> bs_map) {m_bs_map = bs_map;};
  void SetReactive(bool reactive) {m_reactive = reactive;};
  /**
   * \brief Start the DCC mechanism
   *
   */
  void reactiveDCC();


private:

  bool m_reactive = true; //!< Boolean to indicate if the DCC is reactive or proactive
  Time m_dcc_interval = Time(-1.0); //!< Time interval for DCC
  Ptr<MetricSupervisor> m_metric_supervisor = NULL; //!< Pointer to the MetricSupervisor object
  Ptr<TraciClient> m_traci_client = NULL; //!< Pointer to the TraciClient object
  Ptr<BSMap> m_bs_map = NULL; //!< Pointer to the BSMap object

  DCC::CBRThresholds m_cbr_thresholds = DCC::CBRThresholds(); //!< CBR thresholds for the different states
  DCC::ReactiveParametersRelaxed m_reactive_parameters_relaxed = DCC::ReactiveParametersRelaxed(); //!< Parameters for the Relaxed state
  DCC::ReactiveParametersActive1 m_reactive_parameters_active1 = DCC::ReactiveParametersActive1(); //!< Parameters for the Active1 state
  DCC::ReactiveParametersActive2 m_reactive_parameters_active2 = DCC::ReactiveParametersActive2(); //!< Parameters for the Active2 state
  DCC::ReactiveParametersActive3 m_reactive_parameters_active3 = DCC::ReactiveParametersActive3(); //!< Parameters for the Active3 state
  DCC::ReactiveParametersRestricted m_reactive_parameters_restricted = DCC::ReactiveParametersRestricted(); //!< Parameters for the Restricted state

  std::unordered_map<uint32_t, DCC::ReactiveState> m_veh_states; //!< Map to store the state of each vehicle

};

}



#endif //NS3_DCC_H
