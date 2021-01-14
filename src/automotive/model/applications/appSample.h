#ifndef APPSAMPLE_H
#define APPSAMPLE_H

#include "ns3/traci-client.h"
#include "ns3/application.h"
#include "ns3/asn_utils.h"

#include <unordered_map>

#include "ns3/denBasicService.h"
#include "ns3/caBasicService.h"
#include "ns3/vdpTraci.h"
#include "ns3/socket.h"

namespace ns3 {

class appSample : public Application
{
public:

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  appSample ();

  virtual ~appSample ();

  void StopApplicationNow ();

  /**
   * \brief Callback to handle a CAM reception.
   *
   * This function is called everytime a packet is received by the CABasicService.
   *
   * \param the ASN.1 CAM structure containing the info of the packet that was received.
   */
  void receiveCAM (CAM_t *cam, Address from);

  /**
   * \brief Callback to handle a DENM reception.
   *
   * This function is called everytime a packet is received by the DENBasicService.
   *
   * \param the denData structure containing the info of the packet that was received.
   */
  void receiveDENM (denData denm, Address from);

protected:
  virtual void DoDispose (void);

private:

  DENBasicService m_denService; //!< DEN Basic Service object
  CABasicService m_caService; //!< CA Basic Service object
  Ipv4Address m_ipAddress; //!< C-V2X self IP address (set by 'v2v-cv2x.cc')
  Ptr<Socket> m_socket_tx_denm; //!< Socket TX (DENM)
  Ptr<Socket> m_socket_tx_cam; //!< Socket TX (CAM)
  Ptr<Socket> m_socket_rx_denm; //!< Socket RX (DENM)
  Ptr<Socket> m_socket_rx_cam; //!< Socket RX (CAM)
  std::string m_model; //!< Communication Model (possible values: 80211p and cv2x)

  /**
   * \brief Send a new updated DENM (i.e. call appDENM_update as foreseen by ETSI EN 302 637-3 V1.3.1)
   *
   * This function can be called to send a DENM containing updated information, with the
   * same ActionID, after sending the first DENM with TriggerDenm()
   *
   */
  void UpdateDenm(ActionID_t actionid);

  /**
   * \brief Trigger a new DENM (i.e. call appDENM_trigger as foreseen by ETSI EN 302 637-3 V1.3.1)
   *
   * This function can be called to send a new DENM.
   *
   */
  void TriggerDenm(void);


  /**
   * \brief Set the maximum speed of the current vehicle
   *
   * This function rolls back the speed of the vehicle, turning it to its original value.
   * It also change the color of the vehicle to yellow (i.e. the default vehicle color)
   *
   */
  void SetMaxSpeed ();

  /**
   * @brief This function return the current time with respect to an arbitrary point in the past
   *
   * The returned timestamp is an ns-3 simulated time if m_real_time is false, or a
   * Linux CLOCK_REALTIME time if m_real_time is true.
   * The time is returned in terms of nanoseconds.
   * If m_real_time is false, only the tv_nsec field is filled, and it contains also the seconds.
   * If m_real_time is true, a standard struct timespec is returned.
  */
  struct timespec compute_timestamp();

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  Ptr<TraciClient> m_client; //!< TraCI client
  std::string m_id; //!< vehicle id
  std::string m_type; //!< vehicle type
  double m_max_speed; //!< To save initial veh max speed
  bool m_send_denm;  //!< To decide if DENM dissemination is active or not
  bool m_send_cam;  //!< To decide if CAM dissemination is active or not
  double m_denm_intertime; //!< Time between two consecutives DENMs
  bool m_print_summary; //!< To print a small summary when vehicle leaves the simulation
  bool m_already_print; //!< To avoid printing two summaries
  bool m_real_time; //!< To decide wheter to use realtime scheduler
  std::string m_csv_name; //!< CSV log file name
  std::ofstream m_csv_ofstream_cam; //!< CSV log stream (CAM), created using m_csv_name
  std::ofstream m_csv_ofstream_denm; //!< CSV log stream (DENM), created using m_csv_name

  /* Counters */
  int m_cam_received;
  int m_denm_sent;
  int m_denm_received;

  EventId m_speed_ev; //!< Event to change the vehicle speed
  EventId m_send_denm_ev; //!< Event to send the DENM
  EventId m_send_cam_ev; //!< Event to send the CAM
  EventId m_update_denm_ev; //!< Event to update the DENM

};

} // namespace ns3

#endif /* APP_H */

