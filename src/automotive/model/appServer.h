#ifndef APPSERVER_H
#define APPSERVER_H

#include "ns3/traci-client.h"
#include "ns3/application.h"
#include "ns3/address.h"
#include "v2i-DENM-sender.h"
#include "utils.h"

namespace ns3 {

enum ret {
	DO_NOT_SEND,	/* Dont send DENM */
	SEND_0,		/* Send DENM with seq=0 */
	SEND_1		/* Send DENM with seq=1 */
};

enum pos {
	INSIDE,
	OUTSIDE
};

class appServer : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  appServer ();

  virtual ~appServer ();

  void receiveCAM(ca_data_t cam, Address address);

  void StopApplicationNow ();

  bool m_lon_lat; //!< Use LonLat instead of XY

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  bool isInside(double, double);

  void TriggerDenm(long detectionTime, int speedmode, Address address);
  /**
   * @brief This function compute the timestamps
  */
  struct timespec compute_timestamp();
  /**
   * @brief This function compute the milliseconds elapsed from 2004-01-01
  */
  long compute_timestampIts ();
  /**
   * @brief Used to print a report on number of msg received each second
  */
  void aggregateOutput(void);
  
  Ptr<TraciClient> m_client; //!< TraCI client
  std::string m_id; //!< vehicle id
  libsumo::TraCIPosition m_upperLimit; //!< To store the speed limit area boundaries
  libsumo::TraCIPosition m_lowerLimit; //!< To store the speed limit area boundaries
  bool m_aggregate_output; //!< To decide wheter to print the report each second or not
  bool m_real_time; //!< To decide wheter to use realtime scheduler

  /* Counters */
  u_int m_cam_received;
  u_int m_denm_sent;

  EventId m_aggegateOutputEvent; //!< Event to create aggregate output

  std::map <Address, int> m_veh_position; //!< To trigger the DENM sending. If int = 0 the vehicle with Address is in the slow-speed area, 1 in the high-speed area

};

} // namespace ns3

#endif /* APPSERVER_H */

