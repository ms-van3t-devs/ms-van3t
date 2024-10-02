/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

* Created by:
    *  Carlos Mateo Risma Carletti, Politecnico di Torino (carlosrisma@gmail.com)
*/


#include "opencda-sensor.h"
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
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

  double
  OpenCDASensor::insertCV(vehicleData_t vehData) {

    double ret = m_opencda_client->InsertCV(createCARLAObjectIn(vehData));
    m_lastInserted[vehData.stationID] = vehData.timestamp_us;

    return ret;
  }

  double OpenCDASensor::insertPOs(std::vector<vehicleData_t> POs, uint64_t fromId) {

    carla::ObjectsIn objectsIn;
    for (auto it = POs.begin(); it != POs.end(); it++) {
        carla::Object object;
        carla::Vector speed;
        carla::Vector acc;
        carla::Vector location;
        carla::Rotation rotation;
        carla::Transform transform;

        object.set_id(it->stationID);
        object.set_dx(static_cast<double>(it->xDistance.getData()));
        object.set_dy(static_cast<double>(it->yDistance.getData()));

        acc.set_x(0.0);
        acc.set_y(0.0);
        acc.set_z(0.0);
        object.set_allocated_acceleration(&acc);  // Temporary assignment

        speed.set_x(it->speed_ms * cos(it->heading * M_PI / 180.0));
        speed.set_y(it->speed_ms * sin(it->heading * M_PI / 180.0));
        speed.set_z(0.0);
        object.set_allocated_speed(&speed);  // Temporary assignment

        object.set_length(static_cast<double>(it->vehicleLength.getData()) / DECI);
        object.set_width(static_cast<double>(it->vehicleWidth.getData()) / DECI);
        object.set_onsight(false);
        object.set_tracked(false);
        object.set_timestamp(it->timestamp_us / 1000);
        object.set_confidence(it->confidence.getData());

        if (it->heading > 180) {
            object.set_yaw(it->heading - 360);
        } else {
            object.set_yaw(it->heading);
        }

        object.set_detected(it->detected);

        carla::Vector pos = m_opencda_client->getCartesian(it->lon, it->lat);
        location.set_x(pos.x());
        location.set_y(pos.y());
        location.set_z(pos.z());
        rotation.set_pitch(0);
        rotation.set_yaw(it->heading);
        rotation.set_roll(0);
        transform.set_allocated_location(&location);  // Temporary assignment
        transform.set_allocated_rotation(&rotation);  // Temporary assignment
        object.set_allocated_transform(&transform);  // Temporary assignment

        objectsIn.add_cpmobjects()->CopyFrom(object);

        // Remove temporary assignments to avoid memory management issues
        object.release_acceleration();
        object.release_speed();
        transform.release_location();
        transform.release_rotation();
        object.release_transform();

        m_lastInserted[it->stationID] = it->timestamp_us;
    }

    // Print values of cpmobjects for debugging
    // for (int i = 0; i < objectsIn.cpmobjects_size(); i++) {
    //     carla::Object obj = objectsIn.cpmobjects(i);
    //     std::cout << "Object " << i << ":\n";
    //     std::cout << "ID: " << obj.id() << "\n";
    //     std::cout << "dx: " << obj.dx() << "\n";
    //     std::cout << "dy: " << obj.dy() << "\n";
    //     std::cout << "acceleration: (" << obj.acceleration().x() << ", " << obj.acceleration().y() << ", " << obj.acceleration().z() << ")\n";
    //     std::cout << "speed: (" << obj.speed().x() << ", " << obj.speed().y() << ", " << obj.speed().z() << ")\n";
    //     std::cout << "length: " << obj.length() << "\n";
    //     std::cout << "width: " << obj.width() << "\n";
    //     std::cout << "onsight: " << obj.onsight() << "\n";
    //     std::cout << "tracked: " << obj.tracked() << "\n";
    //     std::cout << "timestamp: " << obj.timestamp() << "\n";
    //     std::cout << "confidence: " << obj.confidence() << "\n";
    //     std::cout << "yaw: " << obj.yaw() << "\n";
    //     std::cout << "detected: " << obj.detected() << "\n";
    //     std::cout << "location: (" << obj.transform().location().x() << ", " << obj.transform().location().y() << ", " << obj.transform().location().z() << ")\n";
    //     std::cout << "rotation: (" << obj.transform().rotation().pitch() << ", " << obj.transform().rotation().yaw() << ", " << obj.transform().rotation().roll() << ")\n";
    // }

    objectsIn.set_fromid(fromId);
    objectsIn.set_egoid(m_id);
    double ret = m_opencda_client->InsertObjects(objectsIn);
    return ret;

  }

  void
  OpenCDASensor::updateDetectedObjects ()
  {
    if(m_opencda_client->hasCARLALDM (m_id))
      {
        // carla::Objects objects = m_opencda_client->getDetectedObjects(m_id);
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

        carla::Objects objects = m_opencda_client->getDetectedObjects(m_id);
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
                  objectData.xSpeedAbs = OptionalDataItem<long>(long (obj.speed ().x ()*CENTI));
                  objectData.ySpeedAbs = OptionalDataItem<long>(long (obj.speed ().y ()*CENTI));
                  //objectData.heading = atan2(obj.speed ().y (),obj.speed ().x ()) * (180.0 / M_PI);
                  objectData.heading = obj.yaw ();
                  objectData.angle.setData (objectData.heading*DECI);
                  // double yawDiff = obj.yaw () - egoVehicle.heading ();
                  // if(yawDiff < 0)
                  //   objectData.angle.setData ((yawDiff+360)*DECI);
                  // else
                  //   objectData.angle.setData (yawDiff*DECI);

                  objectData.timestamp_us = obj.timestamp ()*1000; //timestamp from CARLA is in milliseconds
                  objectData.camTimestamp = objectData.timestamp_us;
                  objectData.vehicleWidth = OptionalDataItem<long>(long (obj.width ()*DECI));
                  objectData.vehicleLength = OptionalDataItem<long>(long (obj.length ()*DECI));
                  objectData.xDistAbs = OptionalDataItem<long>(long (obj.dx()*CENTI));//X distance in centimeters
                  objectData.yDistAbs = OptionalDataItem<long>(long (obj.dy()*CENTI));//Y Distance in centimeters


                  objectData.x = obj.transform ().location ().x ();
                  objectData.y = obj.transform ().location ().y ();
                  carla::Vector objPos = m_opencda_client->getGeo (obj.transform ().location ().x (), obj.transform ().location ().y ());
                  objectData.lat = objPos.x ();
                  objectData.lon = objPos.y ();

                  objectData.xSpeed = OptionalDataItem <long>((long) (obj.speed ().x () - egoVehicle.speed ().x ())*CENTI);
                  objectData.ySpeed = OptionalDataItem <long>((long) (obj.speed ().y () - egoVehicle.speed ().y ())*CENTI);
                  objectData.speed_ms = sqrt(pow(obj.speed ().x () - egoVehicle.speed ().x (),2)+pow(obj.speed ().y () - egoVehicle.speed ().y (),2));
                  objectData.longitudinalAcceleration = OptionalDataItem <long> (long (sqrt(pow(obj.acceleration ().x (),2)+pow(obj.acceleration ().y (),2))));
                  objectData.confidence = long (obj.confidence ()*CENTI); //Distance based confidence

                  // if retval is OK and perceivedBy is different from m_id, we update the perceivedBy field
                  if(retval==LDM::LDM_OK && (retveh.vehData.perceivedBy.getData () != m_id || obj.perceivedby () == -1) ) {
                    objectData.perceivedBy = retveh.vehData.perceivedBy;
                  }
                  else {
                    objectData.perceivedBy = OptionalDataItem<long> ((long) obj.perceivedby ());
                  }

                  if (objectData.perceivedBy.getData() == -1) {
                    continue;
                    }
                  //objectData.perceivedBy = OptionalDataItem<long> ((long) obj.perceivedby ());

                  objectData.stationType = StationType_unknown;

                  if (retveh.vehData.lastCPMincluded.isAvailable ())
                    objectData.lastCPMincluded.setData(retveh.vehData.lastCPMincluded.getData());

                  if(retveh.vehData.associatedCVs.isAvailable ())
                    objectData.associatedCVs = OptionalDataItem<std::vector<long>>(retveh.vehData.associatedCVs.getData ());

                  objectData.GTaccuracy = OptionalDataItem<double> (m_opencda_client->getGTaccuracy(obj.transform ().location ().x (),obj.transform ().location ().y (),obj.length (), obj.width (), obj.yaw (), obj.id ()));

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
        if(m_GUI)
          {
            updateGUI ();
          }

         m_event_updateDetectedObjects = Simulator::Schedule(MilliSeconds (100),&OpenCDASensor::updateDetectedObjects,this);
      }
  }

  void OpenCDASensor::insertObject(vehicleData_t vehData) {
    carla::ObjectIn toSend;
    carla::Object* object = toSend.mutable_object();

    object->set_id(vehData.stationID);
    object->set_dx(((double)vehData.xDistance.getData()) / CENTI);
    object->set_dy(((double)vehData.yDistance.getData()) / CENTI);

    carla::Vector* acc = object->mutable_acceleration();
    acc->set_x(0.0);
    acc->set_y(0.0);
    acc->set_z(0.0);

    carla::Vector* speed = object->mutable_speed();
    speed->set_x(vehData.speed_ms * cos(vehData.heading * M_PI / 180.0));
    speed->set_y(vehData.speed_ms * sin(vehData.heading * M_PI / 180.0));
    speed->set_z(0.0);

    object->set_length(((double)vehData.vehicleLength.getData()) / DECI);
    object->set_width(((double)vehData.vehicleWidth.getData()) / DECI);
    object->set_onsight(false); // We always insert either objects from CPM or CAMs
    object->set_tracked(false);
    object->set_timestamp(vehData.timestamp_us / 1000);
    object->set_confidence(vehData.confidence.getData());

    if (vehData.heading > 180) {
        object->set_yaw(vehData.heading - 360);
    } else {
        object->set_yaw(vehData.heading);
    }

    object->set_detected(vehData.detected);

    carla::Vector pos = m_opencda_client->getCartesian(vehData.lon, vehData.lat);

    carla::Transform* transform = object->mutable_transform();
    carla::Vector* location = transform->mutable_location();
    carla::Rotation* rotation = transform->mutable_rotation();

    location->set_x(pos.x());
    location->set_y(pos.y());
    location->set_z(pos.z());

    rotation->set_pitch(0);
    rotation->set_yaw(vehData.heading);
    rotation->set_roll(0);

    if (vehData.detected && vehData.perceivedBy.isAvailable()) {
        toSend.set_fromid(vehData.perceivedBy.getData());
    } else if (!vehData.detected) {
        toSend.set_fromid(vehData.stationID);
    } else {
        toSend.set_fromid(m_id);
    }

    toSend.set_egoid(m_id);
    m_opencda_client->InsertObject(toSend);
}




  carla::ObjectIn OpenCDASensor::createCARLAObjectIn(vehicleData_t vehData) {
    carla::ObjectIn toSend;
    carla::Object* object = toSend.mutable_object();

    object->set_id(vehData.stationID);
    object->set_dx(((double)vehData.xDistance.getData()) / CENTI);
    object->set_dy(((double)vehData.yDistance.getData()) / CENTI);

    carla::Vector* acc = object->mutable_acceleration();
    acc->set_x(0.0);
    acc->set_y(0.0);
    acc->set_z(0.0);

    carla::Vector* speed = object->mutable_speed();
    speed->set_x(vehData.speed_ms * cos(vehData.heading * M_PI / 180.0));
    speed->set_y(vehData.speed_ms * sin(vehData.heading * M_PI / 180.0));
    speed->set_z(0.0);

    object->set_length(((double)vehData.vehicleLength.getData()) / DECI);
    object->set_width(((double)vehData.vehicleWidth.getData()) / DECI);
    object->set_onsight(false); // We always insert either objects from CPM or CAMs
    object->set_tracked(false);
    object->set_timestamp(vehData.timestamp_us / 1000);
    object->set_confidence(vehData.confidence.getData());

    if (vehData.heading > 180) {
        object->set_yaw(vehData.heading - 360);
    } else {
        object->set_yaw(vehData.heading);
    }

    object->set_detected(vehData.detected);

    carla::Vector pos = m_opencda_client->getCartesian(vehData.lon, vehData.lat);

    carla::Transform* transform = object->mutable_transform();
    carla::Vector* location = transform->mutable_location();
    carla::Rotation* rotation = transform->mutable_rotation();

    location->set_x(pos.x());
    location->set_y(pos.y());
    location->set_z(pos.z());

    rotation->set_pitch(0);
    rotation->set_yaw(vehData.heading);
    rotation->set_roll(0);

    if (vehData.detected && vehData.perceivedBy.isAvailable()) {
        toSend.set_fromid(vehData.perceivedBy.getData());
    } else if (!vehData.detected) {
        toSend.set_fromid(vehData.stationID);
    } else {
        toSend.set_fromid(m_id);
    }

    toSend.set_egoid(m_id);
    return toSend;
}



  carla::Object OpenCDASensor::createCARLAObject(vehicleData_t vehData) {
    carla::Object object;
    object.set_id(vehData.stationID);
    object.set_dx(((double)vehData.xDistance.getData()) / CENTI);
    object.set_dy(((double)vehData.yDistance.getData()) / CENTI);

    carla::Vector* acc = object.mutable_acceleration();
    acc->set_x(0.0);
    acc->set_y(0.0);
    acc->set_z(0.0);

    carla::Vector* speed = object.mutable_speed();
    speed->set_x(vehData.speed_ms * cos(vehData.heading * M_PI / 180.0));
    speed->set_y(vehData.speed_ms * sin(vehData.heading * M_PI / 180.0));
    speed->set_z(0.0);

    object.set_length(((double)vehData.vehicleLength.getData()) / DECI);
    object.set_width(((double)vehData.vehicleWidth.getData()) / DECI);
    object.set_onsight(false);
    object.set_tracked(false);
    object.set_timestamp(vehData.timestamp_us / 1000);
    object.set_confidence(vehData.confidence.getData());

    if (vehData.heading > 180) {
      object.set_yaw(vehData.heading - 360);
    } else {
      object.set_yaw(vehData.heading);
    }

    object.set_detected(vehData.detected);

    carla::Vector pos = m_opencda_client->getCartesian(vehData.lon, vehData.lat);
    carla::Transform* transform = object.mutable_transform();
    carla::Vector* location = transform->mutable_location();
    carla::Rotation* rotation = transform->mutable_rotation();

    location->set_x(pos.x());
    location->set_y(pos.y());
    location->set_z(pos.z());
    rotation->set_pitch(0);
    rotation->set_yaw(vehData.heading);
    rotation->set_roll(0);

    return object;
  }


  // Function to calculate the intersection area of two polygons
  double
  OpenCDASensor::calculateIoU(const cv::RotatedRect& rect1, const cv::RotatedRect& rect2) {
    std::vector<cv::Point2f> intersection;
    int result = cv::rotatedRectangleIntersection(rect1, rect2, intersection);

    if (result == cv::INTERSECT_NONE) {
      return 0.0; // No intersection
    } else if (result == cv::INTERSECT_PARTIAL || result == cv::INTERSECT_FULL) {
      double intersectionArea = cv::contourArea(intersection);
      double unionArea = rect1.size.area() + rect2.size.area() - intersectionArea;
      return intersectionArea / unionArea;
    } else {
      // Handle unexpected result
      return 0.0;
    }
  }

  void OpenCDASensor::enableGUI(bool visualize) {
    m_GUI = true;
    m_pldm = false;
    m_visualize = visualize;
    m_cv_image = cv::Mat::zeros(1000, 2000, CV_8UC3);
    std::string windowName = "LDM vehicle " + m_string_id;

    if(m_visualize) {
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);
        cv::imshow(windowName, m_cv_image);
    }

    // Create folder for simulation frames if it does not exist
    struct stat info;
    if (stat("sim_frames_LDM", &info) != 0 || !(info.st_mode & S_IFDIR)) {
      std::string command = "mkdir sim_frames_LDM";
      system(command.c_str());
    }

    // Create subfolder
    std::string subfolderPath = "sim_frames_LDM/vehicle_" + m_string_id;
    if (stat(subfolderPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
      std::string command = "mkdir " + subfolderPath;
      system(command.c_str());
    }

    m_count = 0;
  }
  void
  OpenCDASensor::updateGUI () {
    if (m_GUI) {
        double x_center = 1700;
        double y_center = 500;
        double scale_x = 8;
        double scale_y = 8;
        auto egoVehicle = m_opencda_client->GetManagedActorById(m_id);
        double x_offset = egoVehicle.location().x();
        double y_offset = egoVehicle.location().y();

        m_cv_image.setTo(cv::Scalar(0, 0, 0));

      std::map<int, cv::RotatedRect> PO_boxes, GT_boxes;

        std::string windowName = "LDM vehicle " + m_string_id;

        std::vector<LDM::returnedVehicleData_t> LDM_POs, LDM_CVs;
        if (m_LDM->getAllPOs(LDM_POs)) {
            for (auto &it : LDM_POs) {
                carla::Vector pos = m_opencda_client->getCartesian(it.vehData.lon, it.vehData.lat);
                double x = (pos.x() - x_offset) * scale_x + x_center;
                double y = (pos.y() - y_offset) * scale_y + y_center;
                double length = it.vehData.vehicleLength.getData() * scale_x / 10;
                double width = it.vehData.vehicleWidth.getData() * scale_y / 10;
                double heading = it.vehData.heading;

                // Define the rotated rectangle
                cv::RotatedRect rotatedRect(cv::Point2f(x, y), cv::Size2f(length, width), heading);
                PO_boxes[it.vehData.stationID] = rotatedRect;
                cv::Point2f vertices[4];
                rotatedRect.points(vertices);

                // Draw the rotated rectangle
                for (int j = 0; j < 4; j++) {
                    cv::line(m_cv_image, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 0, 255), 2);
                }

              // Find the top-left corner of the rotated rectangle
              cv::Point2f topLeft = vertices[0];
              for (int j = 1; j < 4; j++) {
                if (vertices[j].x < topLeft.x || (vertices[j].x == topLeft.x && vertices[j].y < topLeft.y)) {
                  topLeft = vertices[j];
                }
              }

              // Draw the text at the top-left corner
              cv::putText(m_cv_image, std::to_string(it.vehData.stationID), topLeft, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 255), 1);
            }
        }
        if (m_LDM->getAllCVs(LDM_CVs)) {
            for (auto &it : LDM_CVs) {
                carla::Vector pos = m_opencda_client->getCartesian(it.vehData.lon, it.vehData.lat);
                double x = (pos.x() - x_offset) * scale_x + x_center;
                double y = (pos.y() - y_offset) * scale_y + y_center;
                double length = it.vehData.vehicleLength.getData() * scale_x / 10;
                double width = it.vehData.vehicleWidth.getData() * scale_y / 10;
                double heading = it.vehData.heading;

                // Define the rotated rectangle
                cv::RotatedRect rotatedRect(cv::Point2f(x, y), cv::Size2f(length, width), heading);
                cv::Point2f vertices[4];
                rotatedRect.points(vertices);

                // Draw the rotated rectangle
                for (int j = 0; j < 4; j++) {
                    cv::line(m_cv_image, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 255, 0), 2);
                }

              // Find the top-left corner of the rotated rectangle
              cv::Point2f topLeft = vertices[0];
              for (int j = 1; j < 4; j++) {
                if (vertices[j].x < topLeft.x || (vertices[j].x == topLeft.x && vertices[j].y < topLeft.y)) {
                  topLeft = vertices[j];
                }
              }

              // Draw the text at the top-left corner
              cv::putText(m_cv_image, std::to_string(it.vehData.stationID), topLeft, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 255, 0), 1);
            }
        }

      auto GTactors = m_opencda_client->GetManagedHostIds();
      for (int i = 0; i < GTactors.actorid_size(); i++) {
        carla::Vehicle vehicle = m_opencda_client->GetManagedActorById(GTactors.actorid(i));
        double x = (vehicle.location().x() - x_offset) * scale_x + x_center;
        double y = (vehicle.location().y() - y_offset) * scale_y + y_center;
        double length = 4.8 * scale_x;
        double width = 2.1 * scale_y;
        double heading = vehicle.heading();

        // Define the rotated rectangle
        cv::RotatedRect rotatedRect(cv::Point2f(x, y), cv::Size2f(length, width), heading);
        GT_boxes[GTactors.actorid(i)] = rotatedRect;
        cv::Point2f vertices[4];
        rotatedRect.points(vertices);

        // Draw the rotated rectangle
        for (int j = 0; j < 4; j++) {
          cv::line(m_cv_image, vertices[j], vertices[(j + 1) % 4], cv::Scalar(128, 128, 128), 1);
        }

        // Draw the text (ID) inside the rectangle
        std::string text = std::to_string(GTactors.actorid(i));
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = 0.4;
        int thickness = 1;
        int baseline = 0;

        // Get the text size
        cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
        baseline += thickness;

        // Center the text
        cv::Point textOrg((x - textSize.width / 2), (y + textSize.height / 2));

        // Put the text inside the rectangle
        cv::putText(m_cv_image, text, textOrg, fontFace, fontScale, cv::Scalar(128, 128, 128), thickness);
      }

        auto time = Simulator::Now().GetSeconds();
        cv::putText(m_cv_image, "Time: " + std::to_string(time), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);

        // Show list of POs with their IDs, accuracy and respPMID

        // if (LDM_POs.size() > 0) {
        //     int i = 0;
        //     int text_x_offset = 10;
        //     for (auto &it : LDM_POs) {
        //         std::string po_string = "PO ID: " + std::to_string(it.vehData.stationID)
        //             + " / Accuracy: " + std::to_string(it.vehData.GTaccuracy.getData())
        //             + " / RespPMID: " + std::to_string(it.vehData.respPMID.getData());
        //         int offset = 60 + 30 * i;
        //         cv::putText(m_cv_image, po_string, cv::Point(text_x_offset, offset), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
        //         i++;
        //         if(i > 4) {
        //           text_x_offset += 500;
        //           i = 0;
        //       }
        //     }
        // }


      int i =0;
      int text_x_offset = 10;
      double avg_accuracy = 0.0;
      for (const auto& PO : PO_boxes) {
        int id = PO.first;
        if (GT_boxes.find(id) != GT_boxes.end()) {

          double IoU = calculateIoU(PO.second, GT_boxes[id]);
          avg_accuracy += IoU;
          //std::cout << "ID: " << id << " IoU: " << IoU << std::endl;
          int offset = 60 + 30 * i;
          std::string po_string = "PO ID: " + std::to_string(id)
              + " / Accuracy: " + std::to_string(IoU);
          cv::putText(m_cv_image, po_string, cv::Point(text_x_offset, offset), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
          i++;
          if(i > 4) {
            text_x_offset += 500;
            i = 0;
          }
        }
      }
      avg_accuracy /= PO_boxes.size();
      //m_csv_file << time << "," << avg_accuracy << "\n";
      std::cout << "Time: " << time << " Avg. Accuracy: " << avg_accuracy << std::endl;

        if(m_visualize) {
            cv::imshow(windowName, m_cv_image);
        }
        std::string filename = "sim_frames_LDM/vehicle_" + m_string_id + "/" + std::to_string(m_count) + ".png";
        cv::imwrite(filename, m_cv_image);
        m_count++;
        cv::waitKey(1);
    }
  }

}
