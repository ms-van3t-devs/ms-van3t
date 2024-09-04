#!/bin/bash

sudo chown root.root build/src/fd-net-device/ns3-dev-raw-sock-creator*
sudo chmod 4755 build/src/fd-net-device/ns3-dev-raw-sock-creator*
NS3_FILE="ns3"
sed -i '/if username == "root":/ s/^/# /' "$NS3_FILE"
sed -i '/raise Exception("Refusing to run as root. --enable-sudo will request your password when needed")/ s/^/# /' "$NS3_FILE"