#include "opencda-sensor.h"
#include <cmath>
namespace ns3 {


  OpenCDASensor::OpenCDASensor()
  {
    m_id = 0;
    m_string_id = "";
    m_opencda_client = NULL;

  }
  OpenCDASensor::OpenCDASensor(Ptr<OpenCDAClient> opencda_client, std::string id)
  {
    m_id = std::stoi(id);
    m_string_id = id;
    m_opencda_client = opencda_client;

  }
  OpenCDASensor::OpenCDASensor(Ptr<OpenCDAClient> opencda_client, int id)
  {
    m_id = id;
    m_string_id = std::to_string (id);
    m_opencda_client = opencda_client;

  }
  OpenCDASensor::~OpenCDASensor()
  {
    Simulator::Cancel(m_event_updateDetectedObjects);
  }

  void
  OpenCDASensor::updateDetectedObjects ()
  {
    if(m_opencda_client->hasCARLALDM (m_id))
      {
        carla::Objects objects = m_opencda_client->getDetectedObjects(m_id);
        carla::Vehicle egoVehicle = m_opencda_client->GetManagedActorById (m_id);
        std::vector<uint64_t> carla_ids;
        std::vector<LDM::returnedVehicleData_t> LDM_POs, LDM_CVs;


        // First we insert all entries of received data (CAM or CPM) so they get matched with the entries in OpenCDA's LDM
        if(m_LDM->getAllPOs (LDM_POs))
          {
            std::vector<LDM::returnedVehicleData_t>::iterator it;
            for(it = LDM_POs.begin (); it != LDM_POs.end ();it++)
              {
                if(it->vehData.perceivedBy.getData () != m_id)
                  {
                    if(m_lastInserted.find (it->vehData.stationID) == m_lastInserted.end())
                      {
                        insertObject (it->vehData);
                        m_lastInserted[it->vehData.stationID] = it->vehData.timestamp_us;
                      }
                    else if(m_lastInserted[it->vehData.stationID] < it->vehData.timestamp_us)
                      {
                        insertObject (it->vehData);
                        m_lastInserted[it->vehData.stationID] = it->vehData.timestamp_us;
                      }
                    // If the timestamp in m_lastInserted is the same as current one, we skip
                  }

              }
          }
        if(m_LDM->getAllCVs (LDM_CVs))
          {
            std::vector<LDM::returnedVehicleData_t>::iterator it;
            for(it = LDM_CVs.begin (); it != LDM_CVs.end ();it++)
              {
                if(m_lastInserted.find (it->vehData.stationID) == m_lastInserted.end())
                  {
                    insertObject (it->vehData);
                    m_lastInserted[it->vehData.stationID] = it->vehData.timestamp_us;
                  }
                else if(m_lastInserted[it->vehData.stationID] < it->vehData.timestamp_us)
                  {
                    insertObject (it->vehData);
                    m_lastInserted[it->vehData.stationID] = it->vehData.timestamp_us;
                  }
                // If the timestamp in m_lastInserted is the same as current one, we skip
              }
          }

        // Once all entries from V2X messages are inserted and matched in OpenCDA's LDM, we sync both LDMs
        for (int i=0;i<objects.objects_size ();i++)
           {
             carla::Object obj = objects.objects (i);
             carla_ids.push_back ((uint64_t) obj.id());
             LDM::returnedVehicleData_t retveh = {0};
             LDM::LDM_error_t retval = m_LDM->lookup(obj.id (),retveh);

             if (retval==LDM::LDM_ITEM_NOT_FOUND || (retval==LDM::LDM_OK && retveh.vehData.detected))
               {
                 vehicleData_t objectData = {0};
                  objectData.detected = obj.detected ();
                  objectData.ID = std::to_string(obj.id ());
                  objectData.stationID = obj.id ();

                  objectData.elevation = AltitudeValue_unavailable;

                  objectData.speed_ms = sqrt(pow(obj.speed ().x (),2)+pow(obj.speed ().y (),2));
                  //objectData.heading = atan2(obj.speed ().y (),obj.speed ().x ()) * (180.0 / M_PI);
                  objectData.heading = obj.yaw ();
                  double yawDiff = obj.yaw () - egoVehicle.heading ();
                  if(yawDiff < 0)
                    objectData.angle.setData ((yawDiff+360)*DECI);
                  else
                    objectData.angle.setData (yawDiff*DECI);

                  objectData.timestamp_us = obj.timestamp ()*1000; //timestamp from CARLA is in milliseconds
                  objectData.camTimestamp = objectData.timestamp_us;
                  objectData.vehicleWidth = OptionalDataItem<long>(long (obj.width ()*DECI));
                  objectData.vehicleLength = OptionalDataItem<long>(long (obj.length ()*DECI));
                  objectData.xDistance = OptionalDataItem<long>(long (obj.dx()*CENTI));//X distance in centimeters
                  objectData.yDistance = OptionalDataItem<long>(long (obj.dy()*CENTI));//Y Distance in centimeters

                  carla::Vector objPos = m_opencda_client->getGeo (obj.transform ().location ().x (), obj.transform ().location ().y ());
                  objectData.lat = objPos.x ();
                  objectData.lon = objPos.y ();

                  objectData.xSpeed = OptionalDataItem <long>((long) (obj.speed ().x () - egoVehicle.speed ().x ())*DECI);
                  objectData.ySpeed = OptionalDataItem <long>((long) (obj.speed ().y () - egoVehicle.speed ().y ())*DECI);
                  objectData.longitudinalAcceleration = OptionalDataItem <long> (long (sqrt(pow(obj.acceleration ().x (),2)+pow(obj.acceleration ().y (),2))));
                  objectData.confidence = long (obj.confidence ()*CENTI); //Distance based confidence
                  objectData.perceivedBy = OptionalDataItem<long> ((long) obj.perceivedby ());

                  objectData.stationType = StationType_unknown;

                  if (retveh.vehData.lastCPMincluded.isAvailable ())
                    objectData.lastCPMincluded.setData(retveh.vehData.lastCPMincluded.getData());

                  if(retveh.vehData.associatedCVs.isAvailable ())
                    objectData.associatedCVs = OptionalDataItem<std::vector<long>>(retveh.vehData.associatedCVs.getData ());

                  retval = m_LDM->insert(objectData);

//                  if(objectData.detected)
//                    {
//                      std::cout << "Vehicle "
//                                << m_id << " new LDM object:"
//                                << "[" << objectData.stationID
//                                << "] -- > Heading: " << objectData.heading
//                                << ", Pos: [" << obj.transform ().location ().x ()<<", "<< obj.transform ().location ().y ()
//                                << "], Speed: " << objectData.speed_ms << std::endl;
//                    }

                  if(retval!=LDM::LDM_OK && retval!=LDM::LDM_UPDATED) {
                      std::cerr << "Warning! Insert on the database for detected object " << objectData.ID << "failed!" << std::endl;
                  }
               }
           }
        // After sync, we delete from ns-3 LDM leftover detected objects
        if(m_LDM->getAllPOs (LDM_POs))
          {
            std::vector<LDM::returnedVehicleData_t>::iterator it;
            for(it = LDM_POs.begin (); it != LDM_POs.end ();it++)
              {
                if(std::find(carla_ids.begin (), carla_ids.end (), it->vehData.stationID) == carla_ids.end ())
                  m_LDM->remove (it->vehData.stationID); // We asume that this Perceived Objects has been matched with another one
              }
          }
         m_event_updateDetectedObjects = Simulator::Schedule(MilliSeconds (100),&OpenCDASensor::updateDetectedObjects,this);
      }
  }

  void
  OpenCDASensor::insertObject (vehicleData_t vehData)
  {
    carla::ObjectIn toSend;
    carla::Object* object = new carla::Object();
    carla::Vector* speed = new carla::Vector();
    carla::Vector* acc = new carla::Vector();
    carla::Vector* location = new carla::Vector();
    carla::Vector cartesian;
    carla::Rotation* rotation = new carla::Rotation();
    carla::Transform* transform = new carla::Transform();
    object->set_id (vehData.stationID);
    object->set_dx (((double) vehData.xDistance.getData ())/CENTI);
    object->set_dy (((double) vehData.yDistance.getData ())/CENTI);
    acc->set_x (0.0);
    acc->set_y (0.0);
    acc->set_z (0.0);
    object->set_allocated_acceleration (acc); //TODO
    speed->set_x (vehData.speed_ms * cos(vehData.heading * M_PI / 180.0));
    speed->set_y (vehData.speed_ms * sin(vehData.heading * M_PI / 180.0));
    speed->set_z (0.0);
    object->set_allocated_speed (speed);
    object->set_length (((double) vehData.vehicleLength.getData ())/DECI);
    object->set_width (((double) vehData.vehicleWidth.getData ())/DECI);
    object->set_onsight (false); // We always insert either objects from CPM or CAMs
    object->set_tracked (false);
    object->set_timestamp (vehData.timestamp_us/1000);
    object->set_confidence (vehData.confidence.getData ());

    if (vehData.heading > 180)
      object->set_yaw (vehData.heading - 360);
    else
      object->set_yaw (vehData.heading);

    object->set_detected(vehData.detected);

    carla::Vector pos = m_opencda_client->getCartesian (vehData.lon, vehData.lat);
    location->set_x (pos.x ());
    location->set_y (pos.y ());
//    if(vehData.detected)
//      {
//        std::cout << "Vehicle "
//                  << m_id << " insertObject:"
//                  << "[" << vehData.stationID << "] -- > Heading: " << vehData.heading << ", Pos: [" << pos.x ()<<", "<< pos.y () << "], Speed: " << vehData.speed_ms << std::endl;
//      }
    location->set_z (pos.z ());
    rotation->set_pitch (0);
    rotation->set_yaw (vehData.heading);
    rotation->set_roll (0);
    transform->set_allocated_location (location);
    transform->set_allocated_rotation (rotation);
    object->set_allocated_transform (transform);

    toSend.set_allocated_object (object);

    if(vehData.detected && vehData.perceivedBy.isAvailable ())
      {
        toSend.set_fromid (vehData.perceivedBy.getData ());
      }
    else if(!vehData.detected)
      {
        toSend.set_fromid (vehData.stationID);
      }
    else
      {
        toSend.set_fromid (m_id);
      }

    toSend.set_egoid (m_id);
    m_opencda_client->InsertObject (toSend);
  }
}
