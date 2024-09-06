//
// Created by carlos on 08/12/23.
//

#include "geonet.h"
#include <iostream>

GeoNet::GeoNet() = default;
GeoNet::~GeoNet() = default;

bool
GeoNet::decodeLT (uint8_t lifeTime, double*seconds)
{
    uint8_t base,multiplier;

    base = lifeTime & 0x03;
    multiplier = (lifeTime & 0xFC) >> 2;

    switch (base)
    {
        case 0:
            *seconds = multiplier * 0.050;
            break;
        case 1:
            *seconds = multiplier * 1; // Put 1 just for completion
            break;
        case 2:
            *seconds = multiplier * 10.0;
            break;
        case 3:
            *seconds = multiplier * 100.0;
            break;
        default:
            return false;
            break;
    };

    return true;
}

gnError_e
GeoNet::decodeGN(unsigned char *packet, GNDataIndication_t* dataIndication)
{
    basicHeader basicH;
    commonHeader commonH;

    dataIndication->data = packet;

    basicH.removeHeader(dataIndication->data);
    dataIndication->data += 4;
    dataIndication->GNRemainingLife = basicH.GetLifeTime ();
    dataIndication->GNRemainingHL = basicH.GetRemainingHL ();

    //Basic Header Procesing according to ETSI EN 302 636-4-1 [10.3.3]
    //1)Check version field
    if(basicH.GetVersion() != m_GnPtotocolVersion && basicH.GetVersion() != 0)
    {
        //std::cerr<< "[ERROR] [Decoder] Incorrect version of GN protocol" << std::endl;
        return GN_VERSION_ERROR;

    }
    // This warning can be useful, but, as in the 5G-CARMEN tests all the packets have a GeoNetworking version equal to "0",
    // it has been commented out not to make the logs grow too much
    // else if(basicH.GetVersion() == 0) {
    // std::cerr<< "[WARN] [Decoder] Unexpected GeoNetworking version \"0\"" << std::endl;
    // }
    //2)Check NH field
    if(basicH.GetNextHeader()==2) //a) if NH=0 or NH=1 proceed with common header procesing
    {
        //Secured packet
        std::cerr << "[ERROR] [Decoder] Secured packet not supported" << std::endl;
        return GN_SECURED_ERROR;
    }
    if(!decodeLT(basicH.GetLifeTime(),&dataIndication->GNRemainingLife))
    {
        std::cerr << "[ERROR] [Decoder] Unable to decode lifetime field" << std::endl;
        return GN_LIFETIME_ERROR;
    }
    //Common Header Processing according to ETSI EN 302 636-4-1 [10.3.5]
    commonH.removeHeader(dataIndication->data);
    dataIndication->data += 8;
    dataIndication->upperProtocol = commonH.GetNextHeader (); //!Information needed for step 7
    dataIndication->GNTraClass = commonH.GetTrafficClass (); //!Information needed for step 7
    //1) Check MHL field
    if(commonH.GetMaxHopLimit() < basicH.GetRemainingHL())
    {
        std::cerr << "[ERROR] [Decoder] Max hop limit greater than remaining hop limit" << std::endl; //a) if MHL<RHL discard packet and omit execution of further steps
        return GN_HOP_LIMIT_ERROR;
    }
    //2) process the BC forwarding buffer, for now not implemented (SCF in traffic class disabled)
    //3) check HT field
    dataIndication->GNType = commonH.GetHeaderType();
    dataIndication->lenght = commonH.GetPayload ();

    switch(dataIndication->GNType)
    {
        case TSB:
            if((commonH.GetHeaderSubType ()==0)) dataIndication = processSHB(dataIndication);
            else {
                std::cerr << "[WARNING] [Decoder] GeoNet packet not supported" << std::endl;
                return GN_TYPE_ERROR;
            }
            break;
        default:
            std::cerr << "[WARNING] [Decoder] GeoNet packet not supported. GNType: " << static_cast<unsigned int>(dataIndication->GNType) << std::endl;
            return GN_TYPE_ERROR;
    }
    return GN_OK;
}


GNDataIndication_t*
GeoNet::processSHB (GNDataIndication_t* dataIndication)
{
    shbHeader shbH;

    shbH.removeHeader(dataIndication->data);
    dataIndication->data += 28;
    dataIndication->SourcePV = shbH.GetLongPositionV ();
    dataIndication->GNType = TSB;


    //7) Pass the payload to the upper protocol entity
    return dataIndication;
}

std::vector<unsigned char>
GeoNet::encodeGN(GNDataRequest_t dataRequest) {
    basicHeader basicH;
    commonHeader commonH;
    shbHeader shbH; //Only CAM and CPM supported for now;
    std::vector<unsigned char> dataConfirm;

    basicH.SetVersion (m_GnPtotocolVersion);
    basicH.SetNextHeader (1); // Common header
    basicH.SetLifeTime (encodeLT(dataRequest.GNMaxLife));
    if(dataRequest.GNMaxHL != 0)
    {
        basicH.SetRemainingHL (dataRequest.GNMaxHL);
    }
    else
    {
        basicH.SetRemainingHL (m_GnDefaultHopLimit);
    }

    commonH.SetNextHeader (dataRequest.upperProtocol);
    commonH.SetHeaderType(dataRequest.GNType);
    commonH.SetHeaderSubType(0);

    if(dataRequest.GNType == TSB)
    {
        if(dataRequest.GNMaxHL>1)
            commonH.SetHeaderSubType(1); //For now shouldn't happen
    }

    commonH.SetTrafficClass (dataRequest.GNTraClass);
    commonH.SetFlag(m_GnIsMobile);

    if(dataRequest.GNMaxHL != 0)// Equal to the remaining hop limit
    {
        commonH.SetMaxHL (dataRequest.GNMaxHL);
    }
    else
    {
        commonH.SetMaxHL(m_GnDefaultHopLimit);
    }

    commonH.SetPayload (dataRequest.lenght);


    switch(dataRequest.GNType)
    {
        // TODO: Add support for other GeoNetworking types
        case TSB:
            if(commonH.GetHeaderSubType ()==0) dataConfirm = encodeSHB (dataRequest,commonH,basicH);
            break;
        default:
            std::cout << "GeoNet packet not supported" << std::endl;
            dataConfirm = {};
    }

    return dataConfirm;
}

std::vector<unsigned char>
GeoNet::encodeSHB(GNDataRequest_t dataRequest, commonHeader commonHeader, basicHeader basicHeader) {
    shbHeader shbH;
    std::vector<unsigned char> dataConfirm;

    shbH.SetLongPositionV (m_epv);

    shbH.addHeader (dataRequest.data);
    commonHeader.addHeader (dataRequest.data);
    basicHeader.addHeader (dataRequest.data);

    return dataRequest.data;
}


uint8_t GeoNet::encodeLT(double seconds) {
    uint8_t base,multiplier,retval;
    //Encoding of lifeTime field for Basic Header as specified in ETSI EN 302 636-4-1 [9.6.4]
    if (seconds >= 630.0)
    {
        base = 0x03;
        multiplier = (uint8_t) (seconds / 100.0);
    }
    else if (seconds >= 63.0)
    {
        base = 0x02;
        multiplier = (uint8_t) (seconds / 10.0);
    }
    else if (seconds >= 3.15)
    {
        base = 0x01;
        multiplier = (uint8_t) (seconds / 1.0);
    }
    else
    {
        base = 0x00;
        multiplier = (uint8_t) (seconds / 0.050);
    }
    retval = (multiplier << 2) | base;
    return retval;
}





