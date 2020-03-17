#ifndef APPCLIENT_H
#define APPCLIENT_H

#include "ns3/traci-client.h"
#include "ns3/application.h"

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

  void receiveDENM(int speedmode);

  void StopApplicationNow ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  
  Ptr<TraciClient> m_client; //!< TraCI client
  std::string m_id; //!< vehicle id

};

} // namespace ns3

#endif /* APPCLIENT_H */

