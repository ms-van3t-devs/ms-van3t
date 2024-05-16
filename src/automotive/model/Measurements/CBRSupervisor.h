#ifndef NS3_CBRSUPERVISOR_H
#define NS3_CBRSUPERVISOR_H

#include <list>
#include <unordered_map>
#include <string>
#include <utility>
#include "ns3/traci-client.h"
#include "ns3/event-id.h"
#include "ns3/net-device-container.h"
#include "ns3/wifi-phy-state.h"

namespace ns3 {

/**
 * \ingroup automotive
 *
 * \brief This class is used to provide a mechanism to monitor the Channel Busy Ratio (CBR).
 *
 */

class CBRSupervisor : public Object
{
public:
  /**
   * \brief Default constructor
   *
   */
  CBRSupervisor () {}
  virtual ~CBRSupervisor ();
  /**
   * @breif This function starts the reactive approach for the CBRSupervisor mechanism.
   */
  void startCheckCBR();
  /**
   * @breif This function enables the verbose mode on stdout.
   */
  void enableVerboseOnStdout() {m_verbose_stdout=true;}
  /**
   * @breif This function disables the verbose mode on stdout.
   */
  void disableVerboseOnStdout() {m_verbose_stdout=false;}
  /**
   * @breif This function enables the writing of the CBR values to a file.
   */
  void enableWriteToFile() {m_write_to_file=true;}
  /**
   * @breif This function disables the writing of the CBR values to a file.
   */
  void disableWriteToFile() {m_write_to_file=false;}
  /**
   * @breif This function sets the window value.
   */
  void setWindowValue(float window) {m_window=window;}
  /**
   * @breif This function sets the alpha value.
   */
  void setAlphaValue(float alpha) {m_alpha=alpha;}
  /**
   * @breif This function sets the simulation time.
   */
  void setSimulationTimeValue(float simTime) {m_simulation_time=simTime;}

private:
  /**
   * @breif This function computes the CBR for each node.
   */
  void checkCBR();
  /**
   * @breif This function logs the last CBR values to a file.
   */
  void logLastCBRs();
  float m_window; //!< The window for the CBR computation
  float m_alpha; //!< The alpha parameter for the exponential moving average
  bool m_verbose_stdout = false; //!< True if the verbose mode is enabled, false otherwise
  bool m_write_to_file = false; //!< True if the CBR values are written to a file, false otherwise
  float m_simulation_time; //!< The simulation time
};
} // namespace ns3

#endif //NS3_CBRSUPERVISOR_H
