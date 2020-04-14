#ifndef UTILS_H
#define UTILS_H

#define SPEED_OF_LIGHT      299792458.0
#define OFFSET_X            0
#define OFFSET_Y            0

//ASN.1 utils
#define FIX_PROT_VERS       1
#define FIX_CAMID           2
#define FIX_DENMID          1
#define DEF_LATITUDE        90000001
#define DEF_LONGITUDE       1800000001
#define DEF_LENGTH          1022
#define DEF_WIDTH           62
#define DEF_SPEED           16383
#define DEF_ACCELERATION    161
#define DEF_HEADING         3601
#define DECI                10
#define CENTI               100
#define MICRO               1000000
#define DOT_ONE_MICRO       10000000

//Epoch time at 2004-01-01
#define TIME_SHIFT 1072915200000

namespace ns3 {
  typedef struct _den_data_t
  {
     int proto;
     int actionid;
     int messageid;
     int sequence;
     long stationid;
     long originatingstationid;
     long detectiontime;
     long referencetime;
     long stationtype;
     int validity;
  }den_data_t;
  typedef struct _ca_data_t
  {
     long timestamp;
     long type;
     long latitude;
     long longitude;
     long altitude_value;
     long altitude_conf;
     long speed_value;
     long speed_conf;
     long heading_value;
     long heading_conf;
     long longAcc_value;
     long longAcc_conf;
     long length_value;
     long length_conf;
     long width;
     long id;
     int proto;
     int messageid;
  }ca_data_t;
}

#endif // UTILS_H

