#ifndef APPSERVER_H
#define APPSERVER_H

#include "ns3/traci-client.h"
#include "ns3/application.h"
#include "ns3/address.h"

namespace ns3 {

struct CAMinfo
{
  libsumo::TraCIPosition position;
  double speed;
};

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

  int receiveCAM(struct CAMinfo, Address address);

  void StopApplicationNow ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  bool isInside(double, double);
  
  Ptr<TraciClient> m_client; //!< TraCI client
  std::string m_id; //!< vehicle id
  bool m_lon_lat; //!< Use LonLat instead of XY
  libsumo::TraCIPosition m_upperLimit; //!< To store the speed limit area boundaries
  libsumo::TraCIPosition m_lowerLimit; //!< To store the speed limit area boundaries

  std::map <Address, int> m_veh_position; //!< To trigger the DENM sending. If int = 0 the vehicle with Address is in the slow-speed area, 1 in the high-speed area

};

} // namespace ns3

#endif /* APPSERVER_H */

