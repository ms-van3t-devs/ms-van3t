#!/bin/bash

if [ ! -f "./src/automotive/currmode.txt" ]; then
    echo "Error. File ./src/automotive/currmode.txt not found."

    exit 1
fi

currmode=$(head -n 1 "./src/automotive/currmode.txt")


if [ "$currmode" == "modern" ]; then
    echo "Current versions (modern): CAM v1.4.1 and DENM v1.3.1"

    read -p "Do you want to switch to 'legacy' mode (yes/no)? " yesno

    if [ "$yesno" == "yes" ]; then
        sed -i -E "s|(.*)('/model/asn1-modern/DigitalMap.c'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/asn1-modern/NumericString.c'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/asn1-modern/OpeningDaysHours.c'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/asn1-modern/PhoneNumber.c'.*)|\1#\2|g" ./src/automotive/wscript

        sed -i -E "s|(.*)('/model/asn1-modern/DigitalMap.h'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/asn1-modern/NumericString.h'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/asn1-modern/OpeningDaysHours.h'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/asn1-modern/PhoneNumber.h'.*)|\1#\2|g" ./src/automotive/wscript

        sed -i -E "s/(.*)asn1-modern(.*)/\1asn1-legacy\2/g" ./src/automotive/wscript

        echo "legacy" > ./src/automotive/currmode.txt

        echo "Successfully switched to: 'legacy' mode (CAM v1.3.2 and DENM v1.2.2)"
    elif [ "$yesno" == "no" ]; then
        echo "Operation aborted."
    else
        echo "Error: $yesno is not a valid answer."
        echo "Operation aborted."
    fi
elif [ "$currmode" == "legacy" ]; then
    echo "Current versions (legacy): CAM v1.3.2 and DENM v1.2.2"

    read -p "Do you want to switch to 'modern' mode (yes/no)? " yesno

    if [ "$yesno" == "yes" ]; then
        sed -i -E "s|(.*)#('/model/asn1-legacy/DigitalMap.c'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/asn1-legacy/NumericString.c'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/asn1-legacy/OpeningDaysHours.c'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/asn1-legacy/PhoneNumber.c'.*)|\1\2|g" ./src/automotive/wscript

        sed -i -E "s|(.*)#('/model/asn1-legacy/DigitalMap.h'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/asn1-legacy/NumericString.h'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/asn1-legacy/OpeningDaysHours.h'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/asn1-legacy/PhoneNumber.h'.*)|\1\2|g" ./src/automotive/wscript

        sed -i -E "s/(.*)asn1-legacy(.*)/\1asn1-modern\2/g" ./src/automotive/wscript

        echo "modern" > ./src/automotive/currmode.txt

        echo "Successfully switched to: 'modern' mode (CAM v1.4.1 and DENM v1.3.1)"
    elif [ "$yesno" == "no" ]; then
        echo "Operation aborted."
    else
        echo "Error: $yesno is not a valid answer."
        echo "Operation aborted."
    fi
else
    echo "Error. File ./src/automotive/currmode.txt seems to be corrupted."

    exit 1
fi