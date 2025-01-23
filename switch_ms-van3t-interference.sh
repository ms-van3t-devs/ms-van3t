#!/bin/bash

# Usage:
# To enable interference management:
# ./switch_ms-van3t-interference.sh on
#
# To disable interference management:
# ./switch_ms-van3t-interference.sh off

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 {on|off}"
    exit 1
fi

case "$1" in
    on)
        echo "Enabling Interference Management..."
        cp src/automotive/model/TxTracker/channel_files/modified/ipv4-address-generator.cc src/internet/model/
        cp src/automotive/model/TxTracker/channel_files/modified/multi-model-spectrum-channel.cc src/spectrum/model/
        cp src/automotive/model/TxTracker/channel_files/modified/multi-model-spectrum-channel.h src/spectrum/model/
        cp src/automotive/model/TxTracker/channel_files/modified/yans-wifi-channel.cc src/wifi/model/
        cp src/automotive/model/TxTracker/channel_files/modified/yans-wifi-channel.h src/wifi/model/
        cp src/automotive/model/TxTracker/channel_files/modified/yans-wifi-phy.h src/wifi/model/
        ;;
    off)
        echo "Disabling Interference Management..."
        cp src/automotive/model/TxTracker/channel_files/normal/ipv4-address-generator.cc src/internet/model/
        cp src/automotive/model/TxTracker/channel_files/normal/multi-model-spectrum-channel.cc src/spectrum/model/
        cp src/automotive/model/TxTracker/channel_files/normal/multi-model-spectrum-channel.h src/spectrum/model/
        cp src/automotive/model/TxTracker/channel_files/normal/yans-wifi-channel.cc src/wifi/model/
        cp src/automotive/model/TxTracker/channel_files/normal/yans-wifi-channel.h src/wifi/model/
        cp src/automotive/model/TxTracker/channel_files/normal/yans-wifi-phy.h src/wifi/model/
        ;;
    *)
        echo "Invalid argument: $1"
        echo "Usage: $0 {on|off}"
        exit 1
        ;;
esac

echo "Finished!"