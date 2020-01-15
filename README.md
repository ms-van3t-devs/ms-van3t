# v2i-sandbox

ns3 modules to build a simple V2I application using SUMO (v-1.2.0) and ns-3 (v-3.29).

It has been tested with SUMO v1.2.0 and ns3 v3.29 on Ubuntu 18.04.
Back compatibility **is not** ensured with new version of TraCI.

To build the project:
* Install SUMO following the guide at [https://sumo.dlr.de/wiki/Downloads](https://sumo.dlr.de/wiki/Downloads)
    * You can use 
    	`sudo add-apt-repository ppa:sumo/stable`  
    	`sudo apt update`  
    	`sudo apt install sumo sumo-tools sumo-doc`  
    * Be careful: in the future the previous commands will install updated version of SUMO which are not ensured to work with this scripts (that are tested wit **v-1.2.0**)

* Clone this repository in your pc.
    
* Configure waf to build the new modules with "<ns3-folder>./waf configure --build-profile=optimized --enable-examples --enable-tests --enable-sudo" (add here what you want to enable) - Using the optimized profile allows to speed up the simulation time
* Build ns3 with "<ns3-folder>./waf build"

**Important**
The final project path-tree should be like:

    automotive/
               doc/
               examples/
                        sumo-files/
                                  img/
               helper/
               model/
                    asn1/
               test/
    traci/
          doc/
          examples/
          model/
    traci-applications/
                       examples/
                       helper/
                       model/

automotive/ contains all the application related files. Inside sumo-files you can find the SUMO map, trace and even images.
traci/ and traci-applications/ contains all the logic to link ns-3 and SUMO.
It is important to keep the folder tree that way, otherwise the simulation won't work properly.


**Simple V2I example**
All the commands here shown are issued in the folder ~/ns-allinone-3.29/ns-3.29

To run the program:
`./waf --run "v2i-lte-sandbox"` or
`./waf --run "v2i-80211p-sandbox"`

*  Nodes are created in the ns3 simulation as vehicle enters the SUMO simulation
*  A full LTE or WAVE stack is implemented at lower layers (depending on which example is run)

In this example, every vehicle that enters the scenario will start sending CAM (in plain text) with freq 10 hz. The server will receive them and with probability 1/100 will reply with a DENM (in plain text). To switch to ASN.1 format, use the command --asn=true.
The mobility trace is managed by the file automotive/example/sumo-files/cars.rou.xml -> please note that the very first line of this file are used to determine the number of UE to be generated in the simulation and is very important to update it if the numebr of vehicles changes.

**Important**
If using the LTE version in this very simple toy case, it is possible to connect at most 23 UEs to the enB (due to LENA limitation). There are two workarounds:
1) By using the command `--ns3::LteEnbRrc::SrsPeriodicity=[value]"`where [value]=0, 2, 5, 10, 20, 40, 80, 160, 320. In this way you can add more UEs. Example: `./waf --run "v2i-lte-sandbox --ns3::LteEnbRrc::SrsPeriodicity=160"`
2) By working with SUMO, you can for example create a closed topology (e.g. a ring) and use tools such as rerouters to re-use the vehicles and let them inside the simulation.

**List of most important commands**
* --realtime				           [bool] decide to run the simulation using the realtime scheduler or not
* --sim-time                   [double] simulation time
* --sumo-gui                   [bool] decide to show sumo-gui or not
* --server-aggregate-output	   [bool] if true, the server will print every second a report on the number of DENM sent and CAM received correctly
* --sumo-updates 			         [double] frequency of SUMO updates
* --send-cam 				           [bool] enable vehicles to send CAM
* --asn                        [bool] if true, CAMs and DENMs are encoded and decoded using ASN.1 
* --cam-intertime              [double] CAM dissemination intertime
