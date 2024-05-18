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
   * @breif This function sets the window value in Milliseconds.
   */
  void setWindowValue(float window) {m_window=window;}
  /**
   * @breif This function sets the alpha value.
   */
  void setAlphaValue(float alpha) {m_alpha=alpha;}
  /**
   * @breif This function sets the simulation time in Seconds.
   */
  void setSimulationTimeValue(float simTime) {m_simulation_time=simTime;}
  /**
   * @breif This function sets the channel technology.
   */
  void setChannelTechnology(std::string channelTechnology)
    {
      // Define the set of valid channel technologies
      std::set<std::string> validChannelTechnologies = {"80211p", "Nr", "Lte", "CV2X"};

      // Check if the provided channelTechnology is valid
      if (validChannelTechnologies.find(channelTechnology) == validChannelTechnologies.end()) {
          // If the channelTechnology is not valid, throw an error
          NS_FATAL_ERROR("Invalid channel technology. Must be one of '80211p', 'Nr', 'Lte', or 'CV2X'.");
        }

      // If the channelTechnology is valid, set it
      m_channel_technology = channelTechnology;
    }
  /**
   * @breif This function gets the CBR for a specific node.
   */
  std::tuple<std::string, float> getCBRForNode(std::string node);

private:
  /**
   * @breif This function computes the CBR for each node..
   */
  void checkCBR();
  /**
   * @breif This function logs the last CBR values for each node and write the results into a file.
   */
  void logLastCBRs();
  float m_window; //!< The window for the CBR computation
  float m_alpha; //!< The alpha parameter for the exponential moving average
  bool m_verbose_stdout = false; //!< True if the verbose mode is enabled, false otherwise
  bool m_write_to_file = false; //!< True if the CBR values are written to a file, false otherwise
  std::string m_channel_technology; //!< The channel technology used
  float m_simulation_time; //!< The simulation time
  std::mutex m_mutex; //!< Mutex to protect the access to the CBR values
};
} // namespace ns3

#endif //NS3_CBRSUPERVISOR_H
