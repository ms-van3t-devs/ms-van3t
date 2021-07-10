 #ifndef ASN_UTILS_H
#define ASN_UTILS_H

#include "ns3/simulator.h"
#include "ns3/DENM.h"
#include "ns3/Seq.hpp"
#include <stdint.h>
#include <string>

#define FIX_DENMID          0x01
#define FIX_CAMID           0x02
#define DECI                10
#define CENTI               100
#define MILLI               1000
#define MICRO               1000000
#define DOT_ONE_MICRO       10000000
#define NANO_TO_MILLI       1000000
#define NANO_TO_CENTI       10000000

#define DEG_2_RAD(val) ((val)*M_PI/180.0)


//Epoch time at 2004-01-01
#define TIME_SHIFT 1072915200000

/* Maximum length of an asn1c error message (when decoding fails with respect to certain constraints) */
#define ERRORBUFF_LEN       128

namespace ns3
{
  long compute_timestampIts (bool real_time);
  double haversineDist(double lat_a, double lon_a, double lat_b, double lon_b);

  uint8_t setByteMask(uint8_t mask);
  uint8_t setByteMask(uint16_t mask, unsigned int i);
  uint8_t setByteMask(uint32_t mask, unsigned int i);
  uint8_t getFromMask(uint8_t mask);
  inline uint16_t getFromMasks(uint8_t mask0,uint8_t mask1){return getFromMask(mask0) | (getFromMask(mask1)<<8);}
  inline uint32_t getFromMasks(uint8_t mask0,uint8_t mask1,uint8_t mask2){return getFromMask(mask0) | (getFromMask(mask1)<<8) | (getFromMask(mask2)<<16);}


  typedef struct _pathHistoryDeltas {

    long timestamp;
    long deltaTime;
    long deltaCoverage;
    long heading;
  }PathHistoryDeltas_t;

  template <class T>
  class DENDataItem
  {
      private:
        bool m_available;
        T m_dataitem;

      public:
        DENDataItem(T data): m_dataitem(data) {m_available=true;}
        DENDataItem(bool availability) {m_available=availability;}
        DENDataItem() {m_available=false;m_dataitem = {};}
        T getData() {return m_dataitem;}
        bool isAvailable() {return m_available;}
        void setData(T data) {m_dataitem=data; m_available = true;}
        T getDataRef() {return &m_dataitem;}
  };

  template <class V = int, class C = int>
  class DENValueConfidence
  {
      private:
        V m_value;
        C m_confidence;

      public:
        DENValueConfidence() {}
        DENValueConfidence(V value,C confidence):
          m_value(value), m_confidence(confidence) {}

        V getValue() {return m_value;}
        C getConfidence() {return m_confidence;}
        void setValue(V value) {m_value=value;}
        void setConfidence(C confidence) {m_confidence=confidence;}
  };

  typedef struct DEN_PosConfidenceEllipse {
    long semiMajorConfidence;
    long semiMinorConfidence;
    long semiMajorOrientation;
  } DEN_PosConfidenceEllipse_t;

  typedef struct DEN_ActionID{
    unsigned long originatingStationID;
    long sequenceNumber;
  }DEN_ActionID_t;

  typedef struct DEN_ReferencePosition{
    long latitude;
    long longitude;
    DEN_PosConfidenceEllipse_t positionConfidenceEllipse;
    DENValueConfidence<long,long> altitude;
  } DEN_ReferencePosition_t;

  typedef struct DEN_DeltaReferencePosition {
    long deltaLatitude;
    long deltaLongitude;
    long deltaAltitude;
  } DEN_DeltaReferencePosition_t;

  typedef struct DEN_VehicleIdentification{
    DENDataItem<std::string> wMInumber; // OCTET_STRING !
    DENDataItem<std::string> vDS; // OCTET_STRING !
  }DEN_VehicleIdentification_t;


  typedef struct DEN_EventPoint{
          DEN_DeltaReferencePosition_t	 eventPosition;
          DENDataItem<long>	eventDeltaTime;
          long	 informationQuality;
  } DEN_EventPoint_t;

  typedef struct DEN_PathPoint{
    DEN_DeltaReferencePosition_t pathPosition;
    DENDataItem<long> pathDeltaTime;
  }DEN_PathPoint_t;


  typedef struct _DEN_ImpactReductionCont{
    long	 heightLonCarrLeft;
    long	 heightLonCarrRight;
    long	 posLonCarrLeft;
    long	 posLonCarrRight;
    std::vector<long>	 positionOfPillars; // //////////////!!
    long	 posCentMass;
    long	 wheelBaseVehicle;
    long	 turningRadius;
    long	 posFrontAx;
    long	 vehicleMass;
    long	 requestResponseIndication;
    uint32_t	 positionOfOccupants; // BIT_STRING 20 bits

  } DEN_ImpactReductionContainer_t;

  typedef struct _DEN_DangerousGoodsExtended {
          long	 dangerousGoodsType;
          long	 unNumber;
          bool	 elevatedTemperature;
          bool	 tunnelsRestricted;
          bool	 limitedQuantity;
          DENDataItem<std::string>	emergencyActionCode; // OCTET_STRING !
          DENDataItem<std::string>	phoneNumber; // OCTET_STRING !
          DENDataItem<std::string>	companyName; // OCTET_STRING !
  } DEN_DangerousGoodsExtended_t;


  typedef struct DEN_RoadWorksContainerExtended{
    DENDataItem<uint8_t> lightBarSirenInUse;
    DENDataItem<uint8_t> innerhardShoulderStatus;
    DENDataItem<uint8_t> outerhardShoulderStatus;
    DENDataItem<uint16_t> drivingLaneStatus;
    DENDataItem<std::vector<long>> restriction;
    DENDataItem<long> speedLimit;
    DENDataItem<long> causeCode;
    DENDataItem<long> subCauseCode;
    DENDataItem<std::vector<DEN_ReferencePosition_t>> recommendedPath;
    DENDataItem<DEN_DeltaReferencePosition_t> startingPointSpeedLimit;
    DENDataItem<long> trafficFlowRule;
    DENDataItem<std::vector<DEN_ActionID_t>> referenceDenms;
  }DEN_RoadWorksContainerExtended_t;


  typedef struct DEN_StationaryVehicleContainer{

    DENDataItem<long> stationarySince;
    DENDataItem<long> causeCode;
    DENDataItem<long> subCauseCode;
    DENDataItem<DEN_DangerousGoodsExtended_t> carryingDangerousGoods;
    DENDataItem<long> numberOfOccupants;
    DENDataItem<DEN_VehicleIdentification_t> vehicleIdentification;
    DENDataItem<uint8_t> energyStorageType; // BIT_STRING 7 bits

  } DEN_StationaryVehicleContainer_t;

}

#endif // ASN_UTILS_H

