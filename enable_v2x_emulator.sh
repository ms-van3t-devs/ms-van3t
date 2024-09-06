#!/bin/bash

sudo chown root.root build/src/fd-net-device/ns3-dev-raw-sock-creator*
sudo chmod 4755 build/src/fd-net-device/ns3-dev-raw-sock-creator*
NS3_FILE="ns3"
sed -i '/if username == "root":/ s/^/# /' "$NS3_FILE"
sed -i '/raise Exception("Refusing to run as root. --enable-sudo will request your password when needed")/ s/^/# /' "$NS3_FILE"

sudo apt update 
sudo apt install nlohmann-json3-dev
wget https://archive.apache.org/dist/qpid/proton/0.38.0/qpid-proton-0.38.0.tar.gz
tar zxvf qpid-proton-0.38.0.tar.gz
cd qpid-proton-0.38.0 && mkdir build && cd build 
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DSYSINSTALL_BINDINGS=ON -DBUILD_PYTHON=ON -DBUILD_RUBY=OFF -DBUILD_GO=OFF && make all -j$(nproc)
sudo make install