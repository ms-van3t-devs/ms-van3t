# ms-van3t UDP mode namespace creator script

This folder contains a script which can be used to create (and delete) a new network namespace (named `ns1`), which can be very helpful for setting up the communication between ms-van3t (in UDP emulation mode) and the UDP->AMQP relayer in "loopback", i.e. using the same device and on the same OS.

In order to run ms-van3t and make it communicate with the relayer "in loopback", you need to:
- Enable the +x permission on the script (needed only once): `chmod +x ms-van3t-namespace-creator.sh`
- Launch `./ms-van3t-namespace-creator.sh`; then, leave this terminal always open and open another terminal/tab for the other points
- Launch the relayer with the UDP socket waiting for messages from ms-van3t
- Start ms-van3t in UDP emulation mode (inside the `ns1` namespace) with: `sudo ip netns exec ns1 ./waf --run "v2x-emulator --interface=veth1ns --subnet=10.10.7.0 --gateway=10.10.7.254 --udp=10.10.7.254:20000 --sumo-netns=ns1"`
- When you finish testing and you want to delete the namespace (undoing the modifications applied by the script), simply open the terminal which was left open, with the script, and press ENTER.

# UDP->AMQP 1.0.b relayer

This folder contains the UDP->AMQP 1.0.b relayer source file and the Makefile to compile the whole program.

The relayer will receive the CAM  and CPM messages from ms-van3t, and forward them to an AMQP broker, using a topic selected by the user.

In order to compile the relayer, you can use the Makefile included in this directory. You can thus compile the relayer executable simply with `make`.
You can then launch the UDP->AMQP relayer with: `./UDPAMQPrelayer`.

If no options are specified, the relayer will try to connect to `127.0.0.1:5672` and use as a default topic name `topic://test`.

You can also specify a custom URL or topic/queue name with, respectively, the `--url` and `--queue` options.

This relayer has been tested with an [Apache ActiveMQ "Classic"](https://activemq.apache.org/components/classic/download/) broker (version 5).

The relayer relies on the [TCLAP library](http://tclap.sourceforge.net/) in order to parse the command line options.

# Emulator example with AMQP relayer 

In order to use the `v2x-emulator` example with the UDP->AMQP relayer 3 terminals need to be opened 

Terminal window 1 (~/ms-van3t/ns-3-dev/emulation-support): `./ms-van3t-namespace-creator.sh`

Terminal window 2 (~/ms-van3t/ns-3-dev/emulation-support/UDP-AMQP-relayer): `./UDPAMQPrelayer --url 127.0.0.1:5672 --queue topic://tests`

Terminal window 3 - to be launched to start a test (~/ms-van3t/ns-3-dev): `sudo ip netns exec ns1 ./waf --run "v2x-emulator --interface=veth1ns --subnet=10.10.7.0 --gateway=10.10.7.254 --udp=10.10.7.254:20000 --sumo-netns=ns1 --sim-time=500 --sumo-gui=false" `