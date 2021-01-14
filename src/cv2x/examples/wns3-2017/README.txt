WNS3 2017 - LTE D2D Model for ns-3: Validation examples (v1.0 06/13/2017)

----------------------
I. General description
----------------------
This folder contains the scripts needed to reproduce the evaluations performed
to validate the ns-3 LTE D2D Model presented in the WNS3 2017.

For further information please refer to the corresponding publication:
R. Rouil, F. Cintron, A. Ben Mosbah and S. Gamboa, "Implementation and 
Validation of an LTE D2D Model for ns-3", Proceedings of the Workshop
on ns-3 - 2017 WNS3, 2017.

-------------
II. File list  
-------------
README.txt                  This file
wns3_2017_discovery.cc      Discovery validation (scenario)
wns3_2017_discovery.sh      Discovery validation (master)
wns3_2017_communication.cc  Communication control channel validation (scenario)
wns3_2017_pscch_stats.sh    Communication control channel validation (stats)
wns3_2017_pscch.sh          Communication control channel validation (master)
wns3_2017_pssch.cc          Communication shared channel validation (scenario)
wns3_2017_pssch.sh          Communication shared channel validation (master)
wns3_2017_synch.cc          Synchronization validation (scenario)  
wns3_2017_synch.sh          Synchronization validation (master)

-------------------------
III. Running instructions
-------------------------
Copy the scenario scripts (.cc files) to the ns-3 scratch folder 
(~/ns-3.22-prose/scratch/)

Copy the master and stats scripts (.sh files) to the ns-3 root folder 
(~/ns-3.22-prose/)

From the ns-3 root folder (~/ns-3.22-prose/), run the desired master script to
execute the corresponding evaluation. 
E.g.:
  ./wns3_2017_discovery.sh 
  ./wns3_2017_pscch.sh
  ./wns3_2017_pssch.sh
  ./wns3_2017_synch.sh

---------------------------
IV. Evaluations description
---------------------------

A. Discovery 

1. Scenario
The "wns3_2017_discovery.cc" scenario is used to validate the implementation of 
the device-to-device (D2D) discovery.
We considered a resource pool consisting of 5 subframes and 10 pairs of resource 
blocks. We deployed 10 out-of-coverage UEs, distributed randomly within an area 
of 100 m x 100 m.
Discovery resource pool and other position/mobility parameters can be changed
in the scenario.

The master script is a pre-configured short version of the discovery validation 
(10 UEs). Multiple discovery parameters can be modified such as:
  * the number of users
  * the transmission probability
  * the number of runs
  * the number of trials

2. Output
The master script will compute the number of periods needed for one random 
UE to discover everyone else in the group. The average of the results 
collected from all the runs (10 runs x 500 trials) was used to generate 
the Cumulative Distribution Function (CDF) of the number of periods needed 
for one UE of the discovery group to discover all other UEs.

The simulation results (all runs and trials) are saved under the directory 
"wns3_2017_discovery_scenarios". The final plot is saved under the name 
"wns3_2017_discovery_cdf.png". Its data is saved in the following file:
"wns3_2017_discovery_scenarios/discovery__cdf_wns3_ue1_average.tr"

B. Communication control channel 

1. Scenario
The wns3_2017_communication.cc scenario is used to validate the Physical
Sidelink Control Channel (PSCCH) resource collision probability given a 
particular resource pool configuration and a fixed number of UEs contending for
transmission resources. The UEs are deployed using a hexagonal cell grid ring
topology, in a per sector manner.

The master script (wns3_2017_pscch.sh) can run the scenario with different
Sidelink pool configurations for a given range of fixed number of UEs.
Furthermore, the script will re-run each configuration a number of times 
(adjustable within the script) with different random seeds.

The master script  is pre-configured with a shorter version of the PSCCH 
validation. Users can modify the parameters values with those suggested in the
commented lines.

NOTE: gawk 4.0.0 or later is required because the script uses multi-dimensional
arrays.

2. Output
The master script will compute the mean and standard deviation of the PSCCH 
resource collision ratios, and other PSCCH error ratios corresponding to each
configuration. The results are saved to a filename starting with the prefix 
"ProsePSCCH_errors_mean" followed with the scenario input parameters.

C. Communication shared channel 

1. Scenario
This example is used to validate the maximum achievable
data rate on the Physical Sidelink Shared Channel (PSSCH) between 2
UEs given a particular resource pool configuration. The two UEs are
placed close enough so there is no loss due to the channel.

The master script will run the scenario for three different sidelink
configurations by varying the following parameters 
- period: duration of the sidelink period 
- pscchLength: length of the pysical sidelink control channel (PSCCH) 
- ktrp: repetition pattern defining how many subframe can be used for 
sidelink 
- mcs: modulation and coding scheme for transmission on the sidelink 
shared channel 
- rbSize: allocation size in resource block (RB)

2. Output
The master script will compute the achievable data rate by
looking at the packets received and print the results on the standard
output. Each line will have 25 values corresponding to the data rate
when using between 2 RBs and 50 RBs by steps of 2.

D. Synchronization 

1. Scenario
The scenario is composed of 63 UEs uniformly distributed in a urban macro 
scenario area covering 7 sites with a inter-site distance of 1732 m. All UEs
are considered out-of-coverage and unsynchronized at the beginning of the 
simulation. The UEs transmit communication data according to a given traffic
pattern. Each UE is configured to perform the D2D synchronization protocol. 
Thus, each UE transmits synchronization signals every 40 ms when transmitting
data, and each UE is configured to perform the Synchronization Reference 
selection every 1000ms. The master script is configured to run simulations
with different traffic patterns (On-Off and Always On patterns) and different
synchronization protocol configuration (values for the syncTxThreshOoC 
threshold). All this data is aggregated afterwards for comparison. 

The master script is pre-configured with a shorter/manageable version of the 
synchronization validation. It includes a shorter simulation time and a reduced 
number of simulations for each configuration. Users interested in enabling the 
complete validation should edit the master script(wns3_2017_synch.sh)
following the instructions provided in it to do so.   

2. Output
Upon completion of the master script execution, the results of the evaluation
can be found in the ns-3 root folder (~/ns-3.22-prose/):
  - wns3_2017_synchronization_syncTxThreshOoC.txt 
The output data format is:
  TP  Th  NAvg  NCI   
where:
  TP    traffic pattern
  Th    value of the syncTxThreshOoC threshold
  NAvg  average number of UEs acting as Synchronization Reference at the end of
        the simulation time  
  NCI   95% confidence interval value

The master script is also configured to plot this data using gnuplot (if 
available). The plot is written to:
  - wns3_2017_synchronization_syncTxThreshOoC.png  

----------
V. License
----------

This software was developed at the National Institute of Standards and 
Technology by employees of the Federal Government in the course of their
official duties. Pursuant to titleElement 17 Section 105 of the United States
Code this software is not subject to copyright protection and is in the public
domain. NIST assumes no responsibility whatsoever for its use by other parties,
and makes no guarantees, expressed or implied, about its quality, reliability,
or any other characteristic. 

We would appreciate acknowledgment if the software is used.  

NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND DISCLAIM ANY
LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF 
THIS SOFTWARE.                     

-----------
VI. Contact
-----------
Richard Rouil - richard.rouil@nist.gov
