#!/bin/bash

if [ ! -f "./src/automotive/model/ASN1/currmode.txt" ]; then
    echo "Error. File ./src/automotive/model/ASN1/currmode.txt not found."

    exit 1
fi

currmode=$(head -n 1 "./src/automotive/model/ASN1/currmode.txt")


if [ "$currmode" == "v2" ]; then
    echo "Current versions (v2): CAM v1.4.1 and DENM v1.3.1"

    read -p "Do you want to switch to 'v1' (yes/no)? " yesno

    if [ "$yesno" == "yes" ]; then
        sed -i -E "s|(.*)('/model/ASN1/asn1-v2/DigitalMap.c'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/ASN1/asn1-v2/NumericString.c'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/ASN1/asn1-v2/OpeningDaysHours.c'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/ASN1/asn1-v2/PhoneNumber.c'.*)|\1#\2|g" ./src/automotive/wscript

        sed -i -E "s|(.*)('/model/ASN1/asn1-v2/DigitalMap.h'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/ASN1/asn1-v2/NumericString.h'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/ASN1/asn1-v2/OpeningDaysHours.h'.*)|\1#\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)('/model/ASN1/asn1-v2/PhoneNumber.h'.*)|\1#\2|g" ./src/automotive/wscript

        sed -i -E "s/(.*)asn1-v2(.*)/\1asn1-v1\2/g" ./src/automotive/wscript

        echo "v1" > ./src/automotive/model/ASN1/currmode.txt

        echo "Successfully switched to: 'v1' (CAM v1.3.2 and DENM v1.2.2)"
    elif [ "$yesno" == "no" ]; then
        echo "Operation aborted."
    else
        echo "Error: $yesno is not a valid answer."
        echo "Operation aborted."
    fi
elif [ "$currmode" == "v1" ]; then
    echo "Current versions (v1): CAM v1.3.2 and DENM v1.2.2"

    read -p "Do you want to switch to 'v2' (yes/no)? " yesno

    if [ "$yesno" == "yes" ]; then
        sed -i -E "s|(.*)#('/model/ASN1/asn1-v1/DigitalMap.c'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/ASN1/asn1-v1/NumericString.c'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/ASN1/asn1-v1/OpeningDaysHours.c'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/ASN1/asn1-v1/PhoneNumber.c'.*)|\1\2|g" ./src/automotive/wscript

        sed -i -E "s|(.*)#('/model/ASN1/asn1-v1/DigitalMap.h'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/ASN1/asn1-v1/NumericString.h'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/ASN1/asn1-v1/OpeningDaysHours.h'.*)|\1\2|g" ./src/automotive/wscript
        sed -i -E "s|(.*)#('/model/ASN1/asn1-v1/PhoneNumber.h'.*)|\1\2|g" ./src/automotive/wscript

        sed -i -E "s/(.*)asn1-v1(.*)/\1asn1-v2\2/g" ./src/automotive/wscript

        echo "v2" > ./src/automotive/model/ASN1/currmode.txt

        echo "Successfully switched to: 'v2' (CAM v1.4.1 and DENM v1.3.1)"
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