/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * Edited by Marco Malinverno, Politecnico di Torino (marco.malinverno@polito.it)
*/
#include <errno.h>

#include "appClient.h"

extern "C"
{
  #include "asn1/CAM.h"
  #include "asn1/DENM.h"
}

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("appClient");

  NS_OBJECT_ENSURE_REGISTERED(appClient);

  long retValue(double value, int defValue, int fix, int fixNeeded)
  {
      if(fix<fixNeeded)
          return defValue;
      else
          return value;
  }

  TypeId
  appClient::GetTypeId (void)
  {
    static TypeId tid =
        TypeId ("ns3::appClient")
        .SetParent<Application> ()
        .SetGroupName ("Applications")
        .AddConstructor<appClient> ()
        .AddAttribute ("LonLat",
            "If it is true, position are sent through lonlat (not XY).",
            BooleanValue (true),
            MakeBooleanAccessor (&appClient::m_lon_lat),
            MakeBooleanChecker ())
        .AddAttribute ("CAMIntertime",
            "Time between two consecutive CAMs",
            DoubleValue(0.1),
            MakeDoubleAccessor (&appClient::m_cam_intertime),
            MakeDoubleChecker<double> ())
        .AddAttribute ("PrintSummary",
            "To print summary at the end of simulation",
            BooleanValue(false),
            MakeBooleanAccessor (&appClient::m_print_summary),
            MakeBooleanChecker ())
        .AddAttribute ("SendCam",
            "If it is true, the branch sending the CAM is activated.",
            BooleanValue (true),
            MakeBooleanAccessor (&appClient::m_send_cam),
            MakeBooleanChecker ())
        .AddAttribute ("RealTime",
            "To compute properly timestamps",
            BooleanValue(false),
            MakeBooleanAccessor (&appClient::m_real_time),
            MakeBooleanChecker ())
        .AddAttribute ("NodePrefix",
            "The prefix used to idefy vehicles in SUMO.",
            StringValue ("veh"),
            MakeStringAccessor (&appClient::m_veh_prefix),
            MakeStringChecker ())
        .AddAttribute ("Client",
            "TraCI client for SUMO",
            PointerValue (0),
            MakePointerAccessor (&appClient::m_client),
            MakePointerChecker<TraciClient> ());
        return tid;
  }

  appClient::appClient ()
  {
    NS_LOG_FUNCTION(this);
    m_client = nullptr;
    m_print_summary = true;
    m_already_print = false;

    m_cam_sent = 0;
    m_denm_received = 0;
  }

  appClient::~appClient ()
  {
    NS_LOG_FUNCTION(this);

  }

  void
  appClient::DoDispose (void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose ();
  }

  void
  appClient::StartApplication (void)
  {
    NS_LOG_FUNCTION(this);
    m_id = m_client->GetVehicleId (this->GetNode ());

    // Schedule CAM dissemination
    if (m_send_cam)
       m_sendCamEvent = Simulator::Schedule (Seconds (m_cam_intertime), &appClient::TriggerCam, this);
  }

  void
  appClient::StopApplication ()
  {
    NS_LOG_FUNCTION(this);
    Simulator::Remove(m_sendCamEvent);

    if (m_print_summary && !m_already_print)
      {
        std::cout << "INFO-" << m_id
                  << ",CAM-SENT:" << m_cam_sent
                  << ",DENM-RECEIVED:" << m_denm_received
                  << std::endl;
        m_already_print=true;
      }
  }

  void
  appClient::StopApplicationNow ()
  {
    NS_LOG_FUNCTION(this);
    StopApplication ();
  }

  void
  appClient::TriggerCam()
  {
    /* Build CAM data */
    ca_data_t cam;

    /* Generation delta time [ms since 2004-01-01]. In case the scheduler is not real time, we have to use simulation time,
     * otherwise timestamps will be not reliable */
    long timestamp;
    if(m_real_time)
      {
        timestamp = compute_timestampIts ()%65536;
      }
    else
      {
        struct timespec tv = compute_timestamp ();
        timestamp = (tv.tv_nsec/1000000)%65536;
      }
    cam.timestamp = timestamp;

    cam.type = StationType_passengerCar;

    /* Positions - the standard is followed only if m_lonlat is true */
    libsumo::TraCIPosition pos = m_client->TraCIAPI::vehicle.getPosition(m_id);
    if (m_lon_lat)
        pos = m_client->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);

    //altitude [0,01 m]
    cam.altitude_conf = AltitudeConfidence_unavailable;
    cam.altitude_value = AltitudeValue_unavailable;
    //latitude WGS84 [0,1 microdegree]
    cam.latitude = (long)retValue(pos.y*DOT_ONE_MICRO,DEF_LATITUDE,0,0);
    //longitude WGS84 [0,1 microdegree]
    cam.longitude = (long)retValue(pos.x*DOT_ONE_MICRO,DEF_LONGITUDE,0,0);

    /* Heading WGS84 north [0.1 degree] */
    double angle = m_client->TraCIAPI::vehicle.getAngle (m_id);
    cam.heading_value = (double)retValue (angle*DECI,DEF_HEADING,0,0);
    cam.heading_conf = HeadingConfidence_unavailable;

    /* Speed [0.01 m/s] */
    double speed=m_client->TraCIAPI::vehicle.getSpeed(m_id);
    cam.speed_value = (long)retValue(speed*CENTI,DEF_SPEED,0,0);
    cam.speed_conf = SpeedConfidence_unavailable;

    /* Acceleration [0.1 m/s^2] */
    double acc=m_client->TraCIAPI::vehicle.getAcceleration (m_id);
    cam.longAcc_value = (long)retValue(acc*DECI,DEF_ACCELERATION,0,0);
    cam.longAcc_conf = AccelerationConfidence_unavailable;

    /* Length and width of car [0.1 m] */
    double veh_length = m_client->TraCIAPI::vehicle.getLength (m_id);
    cam.length_value = (double)retValue (veh_length*DECI,DEF_LENGTH,0,0);
    cam.length_conf = VehicleLengthConfidenceIndication_unavailable;
    double veh_width = m_client->TraCIAPI::vehicle.getWidth (m_id);
    cam.width = (long)retValue (veh_width*DECI,DEF_WIDTH,0,0);

    /* Proto version, id and msg id */
    cam.proto = FIX_PROT_VERS;
    cam.id = std::stol (m_id.substr (3));
    cam.messageid = FIX_CAMID;

    Ptr<CAMSender> app = GetNode()->GetApplication (0)->GetObject<CAMSender> ();
    app->SendCam (cam);

    m_cam_sent++;

    m_sendCamEvent = Simulator::Schedule (Seconds (m_cam_intertime), &appClient::TriggerCam, this);
  }

  void
  appClient::receiveDENM (den_data_t denm)
  {
    /* This function extracts only one parameter from the DENM:
     * if 0, the vehicle slows down by setting the max speed to 6.94m/s (25km/h)
     * if 1, the vehicle maximum speed is set to 30m/s (108km/h)
     * Additionaly, for visualization purposes, the vehicles change color everytime they change zone
     */
    m_denm_received++;
    std::cout << "DENM received by " << m_id << std::endl;

    int speedmode = denm.sequence;

    if (speedmode==0)
      {
        libsumo::TraCIColor orange;
        orange.r=255;orange.g=99;orange.b=71;orange.a=255;
        m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, 6.94);
        m_client->TraCIAPI::vehicle.setColor (m_id,orange);
      }
    else if (speedmode==1)
      {
        libsumo::TraCIColor green;
        green.r=50;green.g=205;green.b=50;green.a=255;
        m_client->TraCIAPI::vehicle.setMaxSpeed (m_id, 27.77);
        m_client->TraCIAPI::vehicle.setColor (m_id,green);
      }

  }

  struct timespec
  appClient::compute_timestamp ()
  {
    struct timespec tv;
    if (!m_real_time)
      {
        double nanosec =  Simulator::Now ().GetNanoSeconds ();
        tv.tv_sec = 0;
        tv.tv_nsec = nanosec;
      }
    else
      {
        clock_gettime (CLOCK_MONOTONIC, &tv);
      }
    return tv;
  }

  long
  appClient::compute_timestampIts ()
  {
    /* To get millisec since  2004-01-01T00:00:00:000Z */
    auto time = std::chrono::system_clock::now(); // get the current time
    auto since_epoch = time.time_since_epoch(); // get the duration since epoch
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch); // convert it in millisecond since epoch

    long elapsed_since_2004 = millis.count() - TIME_SHIFT; // in TIME_SHIFT we saved the millisec from epoch to 2004-01-01
    return elapsed_since_2004;
  }
}





