#ifndef APPCLIENT_H
#define APPCLIENT_H

#include "ns3/traci-client.h"
#include "ns3/application.h"

#include "v2i-CAM-sender.h"

namespace ns3 {

class appClient : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  appClient ();

  virtual ~appClient ();

  void receiveDENM(den_data_t denm);

  void StopApplicationNow ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void TriggerCam(void);
  /**
   * @brief This function compute the timestamps
  */
  struct timespec compute_timestamp();

  /**
   * @brief This function compute the milliseconds elapsed from 2004-01-01
  */
  long compute_timestampIts ();

  Ptr<TraciClient> m_client; //!< TraCI client
  std::string m_id; //!< vehicle id
  bool m_lon_lat; //!< Use LonLat instead of XY
  double m_cam_intertime; //!< Time between two consecutives CAMs
  bool m_send_cam; //!< To decide if CAM dissemination is active or not
  bool m_real_time; //!< To decide wheter to use realtime scheduler
  std::string m_csv_name; //!< CSV log file name
  std::ofstream m_csv_ofstream;
  bool m_print_summary; //!< To print a small summary when vehicle leaves the simulation
  bool m_already_print; //!< To avoid printing two summary

  EventId m_sendCamEvent; //!< Event to send the CAM

  /* Counters */
  int m_cam_sent;
  int m_denm_received;
};

} // namespace ns3

#endif /* APPCLIENT_H */

