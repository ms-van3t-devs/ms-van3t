#!/bin/bash

dirarray=( src/antenna/
src/dsdv/
src/lte/
src/network/
src/aodv/
src/dsr/
src/nix-vector-routing/
src/applications/
src/energy/
src/olsr/
src/traffic-control/
src/fd-net-device/
src/openflow/
src/uan/
src/bridge/
src/flow-monitor/
src/point-to-point/
src/virtual-net-device/
src/brite/
src/internet/
src/point-to-point-layout/
src/visualizer/
src/buildings/
src/internet-apps/
src/propagation/
src/wave/
src/click/
src/lr-wpan/
src/sixlowpan/
src/wifi/
src/config-store/
src/spectrum/
src/wimax/
src/core/
src/mesh/
src/stats/
src/mobility/
src/tap-bridge/
src/csma/
src/mpi/
src/test/
src/csma-layout/
src/netanim/
src/topology-read/
.settings/
bindings/
contrib/
doc/
examples/
log/
patch/
scratch/
utils/
waf-tools/
CHANGES.html
Makefile
README
READMEpatch.md
RELEASE_NOTES
premake5.lua
test.py
testpy.supp
tools.lua
utils.py
waf
waf.bat
wscript
wutils.py )

i=0

while [ $i -lt ${#dirarray[@]} ]; do
	echo "${dirarray[$i]}"
	git rm -r --cached ${dirarray[$i]}
	i=$((i+1))
done

echo "Done. Bye."