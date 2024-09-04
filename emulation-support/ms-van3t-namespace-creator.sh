#!/bin/bash

function cleanup {
    set +e

    sudo ip link del veth1root
    sudo ip netns del ns1
}
trap cleanup EXIT

set -x
set -e

# Create a new network namespace
sudo ip netns add ns1

# Add a new veth interface
sudo ip link add veth1root type veth peer name veth1ns
sudo ip link set veth1ns netns ns1

# Enable the veth interface on both root and newly created namespace
sudo ip netns exec ns1 ip link set dev veth1ns up
sudo ip link set dev veth1root up

# Set IP address to the namespace's end of the virtual interface
sudo ip netns exec ns1 ip addr add 10.10.7.1/24 dev veth1ns

# Set the root namespace IP as default gateway (optional)
sudo ip netns exec ns1 ip route add default via 10.10.7.254 dev veth1ns

# Set that interface to promiscuous mode
sudo ip netns exec ns1 ip link set veth1ns promisc on

# Set IP address to the root's end of the virtual interface
sudo ip addr add 10.10.7.254/24 dev veth1root

# Enable loopback on the namespace (for SUMO and TraCI)
sudo ip netns exec ns1 ip link set dev lo up

echo "Namespace created."
read -p "Press ENTER to delete current configuration and the namespace..."