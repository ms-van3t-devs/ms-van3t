#include "ETSImessageHandler.h"


ETSImessageHandler::ETSImessageHandler() = default;
ETSImessageHandler::~ETSImessageHandler() = default;

int ETSImessageHandler::decodeMessage(uint8_t *buffer, size_t buflen, etsiDecodedData_t &decoded_data) {
    bool isGeoNet = true;

    if(buflen<=0) {
        return -1;
    }

    void *decoded_=nullptr;
    asn_dec_rval_t decode_result;

    GeoNet geonet;
    btp BTP;
    GNDataIndication_t gndataIndication;
    BTPDataIndication_t btpDataIndication;

    auto retval = geonet.decodeGN(buffer,&gndataIndication);

    if( (retval != GN_OK) && (retval != GN_BEACON)) {
        std::cerr << "[WARN] [Decoder] Warning: GN unable to decode a received packet." << std::endl;
        return -1;
    }
    if (retval == GN_BEACON) {
        return 1;
    }
    if(BTP.decodeBTP(gndataIndication,&btpDataIndication)!= BTP_OK)
    {
        std::cerr << "[WARN] [Decoder] Warning: BTP unable to decode a received packet." << std::endl;
        return -1;
    }

//    std::cout << "[INFO] [Decoder] ETSI packet content :" << std::endl;
//    for(uint32_t i=0;i<btpDataIndication.lenght;i++) {
//        std::printf("%02X ",btpDataIndication.data[i]);
//    }
//    std::cout << std::endl;

    decoded_data.gnTimestamp = gndataIndication.SourcePV.TST;

    if(btpDataIndication.destPort == CA_PORT) {
        decoded_data.type = ETSI_DECODED_CAM;
        asn1cpp::Seq<CAM> decoded_cam;
        std::string packetContent((char *) btpDataIndication.data, (int) btpDataIndication.lenght);
        decoded_cam = asn1cpp::uper::decodeASN(packetContent, CAM);
        if(!bool(decoded_cam)) {
            std::cout << "Warning: unable to decode a received CAM." << std::endl;
        }
        else{
            //std::cout << "CAM decoded" << std::endl;
            decoded_data.decoded_cam = decoded_cam;
//            std::cout << "Station ID: " << asn1cpp::getField(decoded_cam->header.stationId,long) << ", "
//            << " latitude: " << (double) asn1cpp::getField(decoded_cam->cam.camParameters.basicContainer.referencePosition.latitude,long) / DOT_ONE_MICRO<< ", "
//            << " longitude: " << (double) asn1cpp::getField(decoded_cam->cam.camParameters.basicContainer.referencePosition.longitude,long) / DOT_ONE_MICRO<< ", "
//            << " speed: " << (double) asn1cpp::getField(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue,long) / DECI<< "[ms], "
//            << " heading: " << (double) asn1cpp::getField(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue,long) / DECI<< "[°], "
//            << " length: " << (double) asn1cpp::getField(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthValue,long) / DECI<< "[m], "
//            << " width: " << (double) asn1cpp::getField(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth,long) / DECI<< "[m], "
//            << std::endl;

            nlohmann::basic_json cam_json;
            cam_json["type"] = "CAM";
            cam_json["stationID"] = asn1cpp::getField(decoded_cam->header.stationId,long);
            cam_json["stationType"] = asn1cpp::getField(decoded_cam->cam.camParameters.basicContainer.stationType,long);
            cam_json["timestamp"] = asn1cpp::getField(decoded_cam->cam.generationDeltaTime,long);
            cam_json["referencePosition"]["altitude"] = asn1cpp::getField(decoded_cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeValue,long);
            cam_json["referencePosition"]["latitude"] = (double) asn1cpp::getField(decoded_cam->cam.camParameters.basicContainer.referencePosition.latitude,long);
            cam_json["referencePosition"]["longitude"] = (double) asn1cpp::getField(decoded_cam->cam.camParameters.basicContainer.referencePosition.longitude,long);
            cam_json["highFrequencyContainer"]["heading"] = (double) asn1cpp::getField(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading.headingValue,long);
            if(cam_json["highFrequencyContainer"]["heading"] > 1800)
            {
                cam_json["highFrequencyContainer"]["heading"] = (long) cam_json["highFrequencyContainer"]["heading"] - 3600;
            }
            cam_json["highFrequencyContainer"]["speed"] = (double) asn1cpp::getField(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed.speedValue,long);
            cam_json["highFrequencyContainer"]["vehicleLength"] = (double) asn1cpp::getField(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthValue,long);
            cam_json["highFrequencyContainer"]["vehicleWidth"] = (double) asn1cpp::getField(decoded_cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth,long);

            decoded_data.json_msg = cam_json;
            std::cout << "CAM decoded" << std::endl;
            std::cout << cam_json.dump(4) << std::endl;
        }
    }
    else if(btpDataIndication.destPort == CP_PORT)
    {

        decoded_data.type = ETSI_DECODED_CPM;
        asn1cpp::Seq<CPM> decoded_cpm;
        std::string packetContent((char *) btpDataIndication.data, (int) btpDataIndication.lenght);
        decoded_cpm = asn1cpp::uper::decodeASN(packetContent, CPM);
        if(!bool(decoded_cpm)) {
            std::cout << "Warning: unable to decode a received CPM." << std::endl;
        }
        else{
            decoded_data.decoded_cpm = decoded_cpm;
            nlohmann::basic_json cpm_json, referencePosition, stationData;
            //request["stationID"] = std::stol(m_id.substr (3));
            cpm_json["type"] = "CPM";
            cpm_json["stationID"] = asn1cpp::getField(decoded_cpm->header.stationId,long);

            referencePosition["altitude"] = asn1cpp::getField(decoded_cpm->payload.managementContainer.referencePosition.altitude.altitudeValue,long);
            referencePosition["longitude"] = asn1cpp::getField(decoded_cpm->payload.managementContainer.referencePosition.longitude,long);
            referencePosition["latitude"] = asn1cpp::getField(decoded_cpm->payload.managementContainer.referencePosition.latitude,long);
            cpm_json["referencePosition"] = referencePosition;

            //For every PO inside the CPM, if any
            int wrappedContainer_size = asn1cpp::sequenceof::getSize(decoded_cpm->payload.cpmContainers);
            for (int i=0; i<wrappedContainer_size; i++)
            {
                auto wrappedContainer = asn1cpp::sequenceof::getSeq(decoded_cpm->payload.cpmContainers,WrappedCpmContainer,i);
                WrappedCpmContainer__containerData_PR present = asn1cpp::getField(wrappedContainer->containerData.present,WrappedCpmContainer__containerData_PR);
                if(present == WrappedCpmContainer__containerData_PR_PerceivedObjectContainer) {
                    auto POcontainer = asn1cpp::getSeq(wrappedContainer->containerData.choice.PerceivedObjectContainer,
                                                       PerceivedObjectContainer);
                    int PObjects_size = asn1cpp::sequenceof::getSize(POcontainer->perceivedObjects);
                    auto jsonObjects = nlohmann::json::array();
                    for (int j = 0; j < PObjects_size; j++) {
                        auto PO_seq = asn1cpp::makeSeq(PerceivedObject);
                        PO_seq = asn1cpp::sequenceof::getSeq(POcontainer->perceivedObjects, PerceivedObject, j);

                        auto jsonPO = nlohmann::json::object();
                        jsonPO["ObjectID"] = asn1cpp::getField(PO_seq->objectId, long);
                        jsonPO["Heading"] = asn1cpp::getField(PO_seq->angles->zAngle.value, double);
                        if (jsonPO["Heading"] > 1800) {
                            jsonPO["Heading"] = (long) jsonPO["Heading"] - 3600;
                        }
                        jsonPO["xSpeed"] = asn1cpp::getField(PO_seq->velocity->choice.cartesianVelocity.xVelocity.value,
                                                             long);
                        jsonPO["ySpeed"] = asn1cpp::getField(PO_seq->velocity->choice.cartesianVelocity.yVelocity.value,
                                                             long);
                        jsonPO["vehicleWidth"] = asn1cpp::getField(PO_seq->objectDimensionY->value, long);
                        jsonPO["vehicleLength"] = asn1cpp::getField(PO_seq->objectDimensionX->value, long);
                        jsonPO["xDistance"] = asn1cpp::getField(PO_seq->position.xCoordinate.value, long);
                        jsonPO["yDistance"] = asn1cpp::getField(PO_seq->position.yCoordinate.value, long);
                        jsonPO["timestamp"] =
                                asn1cpp::getField(decoded_cpm->payload.managementContainer.referenceTime, long) -
                                asn1cpp::getField(PO_seq->measurementDeltaTime, long);
                        jsonObjects.push_back(jsonPO);
                    }
                    cpm_json["perceivedObjects"] = jsonObjects;
                }
            }
            decoded_data.json_msg = cpm_json;

            std::cout << "CPM decoded" << std::endl;
            std::cout << cpm_json.dump(4) << std::endl;
        }
    }
    // Only CAMs and CPMs are supported for the time being
    else {
        decoded_data.type = ETSI_DECODED_ERROR;
        return -1;
    }
    return 0;
}


long ETSImessageHandler::getTimestamp() {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
}







