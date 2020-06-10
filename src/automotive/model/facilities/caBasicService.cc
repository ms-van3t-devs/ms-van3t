#include "caBasicService.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("CABasicService");

  CABasicService::CABasicService()
  {
    m_station_id = ULONG_MAX;
    m_stationtype = LONG_MAX;
    m_socket_tx=NULL;
    m_real_time=false;

    // Setting a default value of m_T_CheckCamGen_ms equal to 100 ms (i.e. T_GenCamMin_ms)
    m_T_CheckCamGen_ms=T_GenCamMin_ms;

    m_prev_heading=-1;
    m_prev_speed=-1;
    m_prev_distance=-1;

    m_T_GenCam_ms=T_GenCamMax_ms;

    lastCamGen=-1;
    lastCamGenLowFrequency=-1;
    lastCamGenSpecialVehicle=-1;

    // Set to 3 as described by the ETSI EN 302 637-2 V1.3.1 standard
    m_N_GenCamMax=3;
    m_N_GenCam=0;

    m_vehicle=true;

    // CAM generation interval for RSU ITS-Ss (default: 1 s)
    m_RSU_GenCam_ms=1000;

    m_cam_sent=0;
  }

  CABasicService::CABasicService(unsigned long fixed_stationid,long fixed_stationtype,CURRENT_VDP_TYPE vdp, bool real_time, bool is_vehicle)
  {
    CABasicService();
    m_station_id = (StationID_t) fixed_stationid;
    m_stationtype = (StationType_t) fixed_stationtype;

    m_T_CheckCamGen_ms=T_GenCamMin_ms;
    m_vdp=vdp;
    m_real_time=real_time;

    m_vehicle=is_vehicle;
  }

  CABasicService::CABasicService(unsigned long fixed_stationid,long fixed_stationtype,CURRENT_VDP_TYPE vdp, bool real_time, bool is_vehicle, Ptr<Socket> socket_tx)
  {
    CABasicService(fixed_stationid,fixed_stationtype,vdp,real_time,is_vehicle);

    m_socket_tx=socket_tx;
  }


  void
  CABasicService::startCamDissemination()
  {
    if(m_vehicle)
      {
        m_event_camDisseminationStart = Simulator::Schedule (Seconds(0), &CABasicService::initDissemination, this);
      }
    else
      {
        m_event_camDisseminationStart = Simulator::Schedule (Seconds (0), &CABasicService::RSUDissemination, this);
      }
  }

  void
  CABasicService::startCamDissemination(double desync_s)
  {
    if(m_vehicle)
      {
        m_event_camDisseminationStart = Simulator::Schedule (Seconds (desync_s), &CABasicService::initDissemination, this);
      }
    else
      {
        m_event_camDisseminationStart = Simulator::Schedule (Seconds (desync_s), &CABasicService::RSUDissemination, this);
      }
  }

  void
  CABasicService::receiveCam (Ptr<Socket> socket)
  {
    Ptr<Packet> packet;
    CAM_t *decoded_cam;
    Address from;

    packet = socket->RecvFrom (from);

    uint8_t *buffer = new uint8_t[packet->GetSize ()];
    packet->CopyData (buffer, packet->GetSize ()-1);


    /* Try to check if the received packet is really a DENM */
    if (buffer[1]!=FIX_CAMID)
      {
        NS_LOG_ERROR("Warning: received a message which has messageID '"<<buffer[1]<<"' but '2' was expected.");
        return;
      }

    /** Decoding **/
    void *decoded_=NULL;
    asn_dec_rval_t decode_result;

    do {
      decode_result = asn_decode(0, ATS_UNALIGNED_BASIC_PER, &asn_DEF_CAM, &decoded_, buffer, packet->GetSize ()-1);
    } while(decode_result.code==RC_WMORE);

    if(decode_result.code!=RC_OK || decoded_==NULL) {
        NS_LOG_ERROR("Warning: unable to decode a received CAM.");
        return;
      }

    decoded_cam = (CAM_t *) decoded_;

    m_CAReceiveCallback(decoded_cam,from);
  }

  void
  CABasicService::initDissemination()
  {
    generateAndEncodeCam();
    m_event_camCheckConditions = Simulator::Schedule (MilliSeconds(m_T_CheckCamGen_ms), &CABasicService::checkCamConditions, this);
  }

  void
  CABasicService::RSUDissemination()
  {
    generateAndEncodeCam();
    m_event_camRsuDissemination = Simulator::Schedule (MilliSeconds(m_RSU_GenCam_ms), &CABasicService::RSUDissemination, this);
  }

  void
  CABasicService::checkCamConditions()
  {
    int64_t now=computeTimestampUInt64 ()/NANO_TO_MILLI;
    CABasicService_error_t cam_error;
    bool condition_verified=false;
    static bool dyn_cond_verified=false;

    // If no initial CAM has been triggered before checkCamConditions() has been called, throw an error
    if(m_prev_heading==-1 || m_prev_speed==-1 || m_prev_distance==-1)
      {
        NS_FATAL_ERROR("Error. checkCamConditions() was called before sending any CAM and this is not allowed.");
      }
    /*
     * ETSI EN 302 637-2 V1.3.1 chap. 6.1.3 condition 1) (no DCC)
     * One of the following ITS-S dynamics related conditions is given:
    */

    /* 1a)
     * The absolute difference between the current heading of the originating
     * ITS-S and the heading included in the CAM previously transmitted by the
     * originating ITS-S exceeds 4°;
    */
    double head_diff = m_vdp.getHeadingValue () - m_prev_heading;
    head_diff += (head_diff>180.0) ? -360.0 : (head_diff<-180.0) ? 360.0 : 0.0;
    if (head_diff > 4.0 || head_diff < -4.0)
      {
        cam_error=generateAndEncodeCam ();
        if(cam_error==CAM_NO_ERROR)
          {
            m_N_GenCam=0;
            condition_verified=true;
            dyn_cond_verified=true;
          } else {
            NS_LOG_ERROR("Cannot generate CAM. Error code: "<<cam_error);
          }
      }

    /* 1b)
     * the distance between the current position of the originating ITS-S and
     * the position included in the CAM previously transmitted by the originating
     * ITS-S exceeds 4 m;
    */
    double pos_diff = m_vdp.getTravelledDistance () - m_prev_distance;
    if (!condition_verified && (pos_diff > 4.0 || pos_diff < -4.0))
      {
        cam_error=generateAndEncodeCam ();
        if(cam_error==CAM_NO_ERROR)
          {
            m_N_GenCam=0;
            condition_verified=true;
            dyn_cond_verified=true;
          } else {
            NS_LOG_ERROR("Cannot generate CAM. Error code: "<<cam_error);
          }
      }

    /* 1c)
     * he absolute difference between the current speed of the originating ITS-S
     * and the speed included in the CAM previously transmitted by the originating
     * ITS-S exceeds 0,5 m/s.
    */
    double speed_diff = m_vdp.getSpeedValue () - m_prev_speed;
    if (!condition_verified && (speed_diff > 0.5 || speed_diff < -0.5))
      {
        cam_error=generateAndEncodeCam ();
        if(cam_error==CAM_NO_ERROR)
          {
            m_N_GenCam=0;
            condition_verified=true;
            dyn_cond_verified=true;
          } else {
            NS_LOG_ERROR("Cannot generate CAM. Error code: "<<cam_error);
          }
      }

    /* 2)
     * The time elapsed since the last CAM generation is equal to or greater than T_GenCam
    */
    if(!condition_verified && (now-lastCamGen>=m_T_GenCam_ms))
      {
         cam_error=generateAndEncodeCam ();
         if(cam_error==CAM_NO_ERROR)
           {

             if(dyn_cond_verified==true)
               {
                 m_N_GenCam++;
                 if(m_N_GenCam>=m_N_GenCamMax)
                   {
                     m_N_GenCam=0;
                     m_T_GenCam_ms=T_GenCamMax_ms;
                     dyn_cond_verified=false;
                   }
               }
           } else {
             NS_LOG_ERROR("Cannot generate CAM. Error code: "<<cam_error);
           }
      }

    m_event_camCheckConditions = Simulator::Schedule (MilliSeconds(m_T_CheckCamGen_ms), &CABasicService::checkCamConditions, this);
  }

  CABasicService_error_t
  CABasicService::generateAndEncodeCam()
  {
    CAM_t *cam;
    CURRENT_VDP_TYPE::CAM_mandatory_data_t cam_mandatory_data;
    CABasicService_error_t errval=CAM_NO_ERROR;

    // Optional CAM data pointers
    AccelerationControl_t *accelerationcontrol=NULL;
    LanePosition_t *laneposition=NULL;
    SteeringWheelAngle_t *steeringwheelangle=NULL;
    LateralAcceleration_t *lateralacceleration=NULL;
    VerticalAcceleration_t *verticalacceleration=NULL;
    PerformanceClass_t *performanceclass=NULL;
    CenDsrcTollingZone_t *tollingzone=NULL;

    RSUContainerHighFrequency_t* rsu_container=NULL;

    Ptr<Packet> packet;
    asn_encode_to_new_buffer_result_t encode_result;
    int64_t now;

    if(m_vehicle==false)
      {
        rsu_container=m_vdp.getRsuContainerHighFrequency();

        if(rsu_container==NULL)
          {
            NS_LOG_ERROR("Cannot send RSU CAM: the current VDP does not provide any RSU High Frequency Container.");
            return CAM_NO_RSU_CONTAINER;
          }
      }

    /* Collect data for mandatory containers */
    cam=(CAM_t*) calloc(1, sizeof(CAM_t));
    if(cam==NULL)
      {
        return CAM_ALLOC_ERROR;
      }

    cam_mandatory_data=m_vdp.getCAMMandatoryData();

    /* Fill the header */
    cam->header.messageID = FIX_CAMID;
    cam->header.protocolVersion = FIX_PROT_VERS;
    cam->header.stationID = m_station_id;

    /*
     * Compute the generationDeltaTime, "computed as the time corresponding to the
     * time of the reference position in the CAM, considered as time of the CAM generation.
     * The value of the generationDeltaTime shall be wrapped to 65 536. This value shall be set as the
     * remainder of the corresponding value of TimestampIts divided by 65 536 as below:
     * generationDeltaTime = TimestampIts mod 65 536"
    */
    cam->cam.generationDeltaTime = compute_timestampIts () % 65536;

    /* Fill the basicContainer's station type */
    cam->cam.camParameters.basicContainer.stationType = m_stationtype;

    if(m_vehicle==true)
      {
        /* Fill the basicContainer */
        cam->cam.camParameters.basicContainer.referencePosition.altitude = cam_mandatory_data.altitude;
        cam->cam.camParameters.basicContainer.referencePosition.latitude = cam_mandatory_data.latitude;
        cam->cam.camParameters.basicContainer.referencePosition.longitude = cam_mandatory_data.longitude;
        cam->cam.camParameters.basicContainer.referencePosition.positionConfidenceEllipse = cam_mandatory_data.posConfidenceEllipse;

        /* Fill the highFrequencyContainer */
        cam->cam.camParameters.highFrequencyContainer.present = HighFrequencyContainer_PR_basicVehicleContainerHighFrequency;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.heading = cam_mandatory_data.heading;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.speed = cam_mandatory_data.speed;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.driveDirection = cam_mandatory_data.driveDirection;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleLength = cam_mandatory_data.VehicleLength;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.vehicleWidth = cam_mandatory_data.VehicleWidth;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.longitudinalAcceleration = cam_mandatory_data.longAcceleration;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.curvature = cam_mandatory_data.curvature;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.curvatureCalculationMode = cam_mandatory_data.curvature_calculation_mode;
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.yawRate = cam_mandatory_data.yawRate;

        // Manage optional data
        accelerationcontrol = m_vdp.getAccelerationControl();
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.accelerationControl = accelerationcontrol;

        laneposition = m_vdp.getLanePosition();
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.lanePosition = laneposition;

        steeringwheelangle = m_vdp.getSteeringWheelAngle();
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.steeringWheelAngle = steeringwheelangle;

        lateralacceleration=m_vdp.getLateralAcceleration();
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.lateralAcceleration = lateralacceleration;

        verticalacceleration=m_vdp.getVerticalAcceleration();
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.verticalAcceleration = verticalacceleration;

        performanceclass=m_vdp.getPerformanceClass();
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.performanceClass = performanceclass;

        tollingzone=m_vdp.getCenDsrcTollingZone();
        cam->cam.camParameters.highFrequencyContainer.choice.basicVehicleContainerHighFrequency.cenDsrcTollingZone = tollingzone;
      }
   else
      {
        /* Fill the basicContainer */
        /* There is still no full RSU support in this release */
        cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeConfidence = AltitudeValue_unavailable;
        cam->cam.camParameters.basicContainer.referencePosition.altitude.altitudeValue = AltitudeValue_unavailable;
        cam->cam.camParameters.basicContainer.referencePosition.latitude = Latitude_unavailable;
        cam->cam.camParameters.basicContainer.referencePosition.longitude = Longitude_unavailable;
        cam->cam.camParameters.basicContainer.referencePosition.positionConfidenceEllipse.semiMajorConfidence = SemiAxisLength_unavailable;
        cam->cam.camParameters.basicContainer.referencePosition.positionConfidenceEllipse.semiMinorConfidence = SemiAxisLength_unavailable;
        cam->cam.camParameters.basicContainer.referencePosition.positionConfidenceEllipse.semiMajorOrientation = HeadingValue_unavailable;
        /* Fill the highFrequencyContainer */
        cam->cam.camParameters.highFrequencyContainer.present = HighFrequencyContainer_PR_rsuContainerHighFrequency;
        cam->cam.camParameters.highFrequencyContainer.choice.rsuContainerHighFrequency = *rsu_container;
      }

    // Store all the "previous" values used in checkCamConditions()
    m_prev_distance=m_vdp.getTravelledDistance ();
    m_prev_speed=m_vdp.getSpeedValue ();
    m_prev_heading=m_vdp.getHeadingValue ();

    LowFrequencyContainer_t *lowfrequencycontainer=m_vdp.getLowFrequencyContainer();

    if(lowfrequencycontainer!=NULL)
      {
        // Send a low frequency container only if at least 500 ms have passed since the last CAM with a low frequency container
        if(lastCamGenLowFrequency==-1 ||(computeTimestampUInt64 ()-lastCamGenLowFrequency)>=500)
          {
            cam->cam.camParameters.lowFrequencyContainer = lowfrequencycontainer;
            lastCamGenLowFrequency=computeTimestampUInt64 ();
          }
      }

    SpecialVehicleContainer_t *specialvehiclecontainer=m_vdp.getSpecialVehicleContainer();

    if(specialvehiclecontainer!=NULL)
      {
        // Send a low frequency container only if at least 500 ms have passed since the last CAM with a low frequency container
        if(lastCamGenSpecialVehicle==-1 ||(computeTimestampUInt64 ()-lastCamGenSpecialVehicle)>=500)
          {
            cam->cam.camParameters.specialVehicleContainer = specialvehiclecontainer;
            lastCamGenSpecialVehicle=computeTimestampUInt64 ();
          }
      }

    /* Construct CAM and pass it to the lower layers (now UDP, in the future BTP and GeoNetworking, then UDP) */
    /** Encoding **/
    char errbuff[ERRORBUFF_LEN];
    size_t errlen=sizeof(errbuff);

    if(asn_check_constraints(&asn_DEF_CAM,(CAM_t *)cam,errbuff,&errlen) == -1) {
        NS_LOG_ERROR("Unable to validate the ASN.1 contraints for the current CAM."<<std::endl);
        NS_LOG_ERROR("Details: " << errbuff << std::endl);
        errval=CAM_ASN1_UPER_ENC_ERROR;
        goto error;
    }

    encode_result = asn_encode_to_new_buffer(NULL,ATS_UNALIGNED_BASIC_PER,&asn_DEF_CAM, cam);
    if (encode_result.result.encoded==-1)
      {
        errval=CAM_ASN1_UPER_ENC_ERROR;
        goto error;
      }

    packet = Create<Packet> ((uint8_t*) encode_result.buffer, encode_result.result.encoded+1);
    if(m_socket_tx->Send (packet)==-1)
      {
        errval=CAM_CANNOT_SEND;
        goto error;
      }

    m_cam_sent++;

    // Store the time in which the last CAM (i.e. this one) has been generated and successfully sent
    now=computeTimestampUInt64 ()/NANO_TO_MILLI;
    m_T_GenCam_ms=now-lastCamGen;
    lastCamGen = now;

    error:
    // Free all the previously allocated memory
    if(m_vehicle==true)
      {
        // After encoding, we can free the previosly allocated optional data
        if(accelerationcontrol) m_vdp.vdpFree(accelerationcontrol);
        if(laneposition) m_vdp.vdpFree(laneposition);
        if(steeringwheelangle) m_vdp.vdpFree(steeringwheelangle);
        if(lateralacceleration) m_vdp.vdpFree(lateralacceleration);
        if(verticalacceleration) m_vdp.vdpFree(verticalacceleration);
        if(performanceclass) m_vdp.vdpFree(performanceclass);
        if(tollingzone) m_vdp.vdpFree(tollingzone);
      }
    else
      {
        if(rsu_container) m_vdp.vdpFree(rsu_container);
      }

    if(lowfrequencycontainer) m_vdp.vdpFree(lowfrequencycontainer);
    if(specialvehiclecontainer) m_vdp.vdpFree(specialvehiclecontainer);

    if(cam) free(cam);

    return errval;
  }

  uint64_t
  CABasicService::terminateDissemination()
  {
    Simulator::Remove(m_event_camCheckConditions);
    Simulator::Remove(m_event_camDisseminationStart);
    Simulator::Remove(m_event_camRsuDissemination);
    return m_cam_sent;
  }

  int64_t
  CABasicService::computeTimestampUInt64()
  {
    int64_t int_tstamp=0;

    if (!m_real_time)
      {
        int_tstamp=Simulator::Now ().GetNanoSeconds ();
      }
    else
      {
        struct timespec tv;

        clock_gettime (CLOCK_MONOTONIC, &tv);

        int_tstamp=tv.tv_sec*1e9+tv.tv_nsec;
      }
    return int_tstamp;
  }
}
