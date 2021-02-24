#ifndef VDP_H
#define VDP_H

#include "ns3/CAM.h"
#include "asn_utils.h"
#include <float.h>

namespace ns3
{
  class VDP
  {
    public:
      typedef struct CAM_mandatory_data {
        Speed_t speed;
        Longitude_t longitude;
        Latitude_t latitude;
        Altitude_t altitude;
        PosConfidenceEllipse_t posConfidenceEllipse;
        LongitudinalAcceleration_t longAcceleration;
        Heading_t heading;
        DriveDirection_t driveDirection;
        Curvature_t curvature;
        CurvatureCalculationMode_t curvature_calculation_mode;
        VehicleLength_t VehicleLength;
        VehicleWidth_t VehicleWidth;
        YawRate_t yawRate;
      } CAM_mandatory_data_t;

      typedef struct VDP_position_cartesian {
        double x,y,z;
      } VDP_position_cartesian_t;

      typedef struct VDP_position_latlon {
        double lat,lon,alt;
      } VDP_position_latlon_t;

      virtual CAM_mandatory_data_t getCAMMandatoryData() = 0;

      // These methods are used by the CAM generation frequency management mechanism,
      // as mandated by ETSI, and they should return values which are not already
      // converted to the ETSI measurements units for CAM messages
      // For instance, the speed is encoded into a CAM in terms of [0.1 m/s], and it
      // is correct that getCAMMandatoryData() should return a speed already converted to
      // [0.1 m/s].
      // However, as these functions are used in the aforementioned mechanism inside the
      // CA Basic Service implementation, they need to return normal units, i.e.
      // [m/s] for speed, [m] for the travelled distance and [deg] for heading
      // That's why they return non-ASN1 types but normal 'double' values
      virtual double getSpeedValue() = 0;
      virtual double getTravelledDistance() = 0;
      virtual double getHeadingValue() = 0;

      // These methods shall return a position either cartesian or geodetic (needed mainly by the GeoNetworking module)
      // getPositionLatLon() shall return the current position of the object as (Latitude,Longitude,Altitude)
      // getPositionXY() shall return the current position as cartesian projected coordinates (x,y,z)
      // If the Altitude (or z) is not available, it must be set to DBL_MAX
      // getXY() shall be a utility function converting from Lat,Lon to x,y on a projected coordinate system (e.g. using Transverse Mercator)
      virtual VDP_position_latlon_t getPosition() = 0;
      virtual VDP_position_cartesian_t getPositionXY() = 0;
      virtual VDP_position_cartesian_t getXY(double lon, double lat) = 0;

      // These methods refer to optional fields in mandatory containers
      // If the information is not provided, they should be implemented
      // such that NULL is returned for each unavailable information
      // The derived class shall manage the memory allocation for all
      // these methods, including memory cleanup
      virtual AccelerationControl_t *getAccelerationControl() = 0;
      virtual LanePosition_t *getLanePosition() = 0;
      virtual SteeringWheelAngle_t *getSteeringWheelAngle() = 0;
      virtual LateralAcceleration_t *getLateralAcceleration() = 0;
      virtual VerticalAcceleration_t *getVerticalAcceleration() = 0;
      virtual PerformanceClass_t *getPerformanceClass() = 0;
      virtual CenDsrcTollingZone_t *getCenDsrcTollingZone() = 0;

      // As all the above methods, returning a pointer, perform
      // a memory allocation, the derived class should implement
      // a way to let the CA Basic Service free the memory after
      // encoding (with ASN.1) a certain optional field
      virtual void vdpFree(void* optional_field) = 0;

      // Optional container methods. As before, they can return NULL
      // if these containers are unavailable
      virtual RSUContainerHighFrequency_t *getRsuContainerHighFrequency() = 0;
      virtual LowFrequencyContainer_t *getLowFrequencyContainer() = 0;
      virtual SpecialVehicleContainer_t *getSpecialVehicleContainer() = 0;

      void setFixedVehicleLength(VehicleLength_t vehicle_length)
      {
         m_vehicle_length=vehicle_length;
      }

      void setFixedVehicleWidth(VehicleWidth_t vehicle_width)
      {
         m_vehicle_width=vehicle_width;
      }

      protected:
        VehicleLength_t m_vehicle_length;
        VehicleWidth_t m_vehicle_width;
  };
}
#endif // VDP_H
