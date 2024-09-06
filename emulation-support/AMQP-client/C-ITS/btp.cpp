//
// Created by carlos on 08/12/23.
//

#include "btp.h"
#include <iostream>

btp::btp() = default;

btp::~btp() = default;

btpError_e
btp::decodeBTP(GNDataIndication_t dataIndication, BTPDataIndication_t* btpDataIndication) {
    btpHeader header;


    btpDataIndication->data = dataIndication.data;

    header.removeHeader(btpDataIndication->data);
    btpDataIndication->data += 4;

    btpDataIndication->BTPType = dataIndication.upperProtocol;

    if((header.getDestPort ()!= CA_PORT) && (header.getDestPort ()!= CP_PORT))
    {
        std::cerr << "[ERROR] [Decoder] BTP port not supported: "<< header.getDestPort () << std::endl;
        return BTP_ERROR;
    }

    btpDataIndication->destPort = header.getDestPort ();

    if(btpDataIndication->BTPType == BTP_A)
    {
        btpDataIndication->sourcePort = header.getSourcePort ();
        btpDataIndication->destPInfo = 0;
    }
    else if(btpDataIndication->BTPType == BTP_B)  //BTP-B
    {
        btpDataIndication->destPInfo = header.getDestPortInfo ();
        btpDataIndication->sourcePort = 0;
    }
    else
    {
        std::cerr << "[ERROR] [Decoder] Incorrect transport protocol " << std::endl;
        return BTP_ERROR;
    }

    btpDataIndication->GnAddress = dataIndication.GnAddressDest;
    btpDataIndication->GNTraClass = dataIndication.GNTraClass;
    btpDataIndication->GNRemPLife = dataIndication.GNRemainingLife;
    btpDataIndication->GNPositionV = dataIndication.SourcePV;
    btpDataIndication->data = dataIndication.data + 4;
    btpDataIndication->lenght = dataIndication.lenght - 4;

    return BTP_OK;
}

GNDataRequest_t
btp::encodeBTP(BTPDataRequest_t dataRequest) {
    GNDataRequest_t GnDataRequest = {};
    btpHeader header;
    GnDataRequest._messagePort = dataRequest.destPort;

    if(dataRequest.BTPType==BTP_A) // BTP-A
    {
        GnDataRequest.upperProtocol = BTP_A;
        header.addHeader(dataRequest.destPort, dataRequest.sourcePort, dataRequest.data);
    }
    else    //  BTP-B
    {
        GnDataRequest.upperProtocol = BTP_B;
        header.addHeader(dataRequest.destPort, dataRequest.destPInfo, dataRequest.data);

    }

    //Filling the GN-dataRequest
    GnDataRequest.GNType = dataRequest.GNType;
    GnDataRequest.GnAddress = dataRequest.GnAddress;
    GnDataRequest.GNCommProfile = dataRequest.GNCommProfile;
    GnDataRequest.GNRepInt = dataRequest.GNRepInt;
    GnDataRequest.GNMaxRepTime = dataRequest.GNMaxRepInt;
    GnDataRequest.GNMaxLife = dataRequest.GNMaxLife;
    GnDataRequest.GNMaxHL = dataRequest.GNMaxHL;
    GnDataRequest.GNTraClass = dataRequest.GNTraClass;
    GnDataRequest.data = dataRequest.data;
    GnDataRequest.lenght = dataRequest.lenght + 4;

    return GnDataRequest;
}
