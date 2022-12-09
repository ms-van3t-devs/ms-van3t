#ifndef IVIDATA_H
#define IVIDATA_H

#include "asn_utils.h"
#include "ITSSOriginatingTableEntry.h"
#include "ITSSReceivingTableEntry.h"
#include "ns3/core-module.h"
#include "ns3/socket.h"
#include "ns3/btp.h"
#include "ns3/btpHeader.h"
#include "ns3/btpdatarequest.h"
#include <string>
#include <functional>
#include <mutex>
#include <queue>

namespace ns3{


  class iviData
  {
  public:
    iviData();
    typedef struct _header {
      long messageID;
      long protocolVersion;
      unsigned long stationID;
    } iviDataHeader;



    typedef struct _mandatory {      
      long providerIdentifier;
      int countryCode;
      long identificationNumber;
      IVIDataItem<long> timeStamp;
      IVIDataItem<long> validFrom;
      IVIDataItem<long> validTo;
      IVIDataItem<std::vector<long>> connectedIviStructures;
      long iviStatus;
    } iviDataMandatory;

    typedef struct IVI_posConfidenceEllipse {
      long semiMajorConfidence;
      long semiMinorConfidence;
      long semiMajorOrientation;
    } IVI_PosConfidenceEllipse_t;

    typedef struct IVI_glc_ReferencePosition{
      long latitude;
      long longitude;
      IVI_PosConfidenceEllipse_t positionConfidenceEllipse;
      IVIValueConfidence<long,long> altitude;
    } IVI_glc_ReferencePosition_t;

    typedef struct IVI_glcP_deltaPosition {
      long deltaLat;
      long deltaLong;
    }IVI_glcP_deltaPosition_t;

    typedef struct IVI_glcP_deltaPositionWA {
      long deltaLat;
      long deltaLong;
      long deltaAltitude;
    }IVI_glcP_deltaPositionWA_t;

    typedef struct IVI_glcP_absPosition {
      long lat;
      long lon;
    }IVI_glcP_absPosition_t;

    typedef struct IVI_glcP_absPositionWA {
      long lat;
      long lon;
      long altitude;
    }IVI_glcP_absPositionWA_t;

    typedef struct IVI_glcP_PolygonalLine {
      IVIDataItem<std::vector<IVI_glcP_deltaPosition_t>> deltaPositions;
      IVIDataItem<std::vector<IVI_glcP_deltaPositionWA_t>> deltaPositionsWA;
      IVIDataItem<std::vector<IVI_glcP_absPosition_t>> absPositions;
      IVIDataItem<std::vector<IVI_glcP_absPositionWA_t>> absPositionsWA;
    }IVI_glcP_PolygonalLine_t;

    typedef struct IVI_glcP_segment{
      IVI_glcP_PolygonalLine_t line;
      IVIDataItem<long> laneWidth;
    }IVI_glcP_segment_t;

    typedef struct IVI_glcP_cSegment {
      long zoneID;
      long laneNumber;
      long laneWidth;
      IVIDataItem<long> offsetDist;
      IVIDataItem<IVI_glcP_deltaPositionWA_t> offsetPosition;
    }IVI_glcP_cSegment_t;

    typedef struct IVI_glcP_zone {
      IVIDataItem<IVI_glcP_segment_t> segment;
      IVIDataItem<IVI_glcP_PolygonalLine_t> area;
      IVIDataItem<IVI_glcP_cSegment_t> computedSegment;
    }IVI_glcP_zone_t;

    typedef struct IVI_glc_part{
      long zoneId;
      IVIDataItem<long> laneNumber;
      IVIDataItem<long> zoneExtension;
      IVIDataItem<long> zoneHeading;
      IVIDataItem<IVI_glcP_zone_t> zone;
    }IVI_glc_part_t;

    typedef struct IVI_glc{
      IVI_glc_ReferencePosition referencePosition;
      IVIDataItem<IVIValueConfidence<long,long>> referencePositionHeading;
      IVIDataItem<IVIValueConfidence<long,long>> referencePositionSpeed;
      IVIDataItem<long> referencePositionTime;
      std::vector<IVI_glc_part> GlcPart;
    } IVI_glc_t;

    typedef struct gicPartRS{
      //Only ISO14823 supported
      IVIDataItem<long> RS_trafficSignPictogram;
      IVIDataItem<long> RS_publicFacilitiesPictogram;
      IVIDataItem<long> RS_ambientOrRoadConditionPictogram;
      long RS_nature;
      long RS_serialNumber;
      //SPE
      IVIDataItem<long> RS_spm;
      IVIDataItem<long> RS_mns;
      IVIDataItem<long> RS_unit;
    } gicPartRS_t;

    typedef struct IVI_gic_part{
      IVIDataItem<std::vector<long>> relevanceZoneIds;
      IVIDataItem<std::vector<long>> detectionZoneIds;
      IVIDataItem<long> direction;
      long iviType;
      std::vector<gicPartRS_t> RS;
    }IVI_gic_part_t;

    typedef struct IVI_gic{
      std::vector<IVI_gic_part_t> GicPart;
    } IVI_gic_t;

    typedef struct laneInfo{
      long laneNumber;
      long direction;
      long laneType;
      long laneStatus;
    } laneInfo_t;

    typedef struct IVI_rcc_part{

      std::vector<long> zoneID;
      long roadType;
      std::vector<laneInfo_t> laneInformation;

    }IVI_rcc_part_t;

    typedef struct IVI_rcc{
      std::vector<IVI_rcc_part_t> IVIrccPart;
    } IVI_rcc_t;

    typedef struct IVI_tc_text {
      IVIDataItem<long> layoutComponentId;
      int bitLanguage;
      std::string textCont;
    }IVI_tc_text_t;

    typedef struct IVI_tc_part{
      IVIDataItem<std::vector<long>> detectionZoneIds;
      std::vector<long> relevanceZoneIds;
      IVIDataItem<long> direction;
      IVIDataItem<std::vector<long>> driverAwarenessZoneIds;
      IVIDataItem<long> minimumAwarenessTime;
      IVIDataItem<std::vector<long>> applicableLanesPos;
      IVIDataItem<long> layoutId;
      IVIDataItem<long> preStoredlayoutId;
      IVIDataItem<std::vector<IVI_tc_text_t>> text;
      std::string data;
    }IVI_tc_part_t;

    typedef struct IVI_tc{
      std::vector<IVI_tc_part_t> IVItcPart;
    } IVI_tc_t;

    typedef struct IVI_lac_comp{
      long x;
      long y;
      long width;
      long height;
      long textScripting;
      long layoutComponentId;
    }IVI_lac_comp_t;

    typedef struct IVI_lac{
      long layoutId;
      IVIDataItem<long> height;
      IVIDataItem<long> width;
      std::vector<IVI_lac_comp> IVIlacComp;
    } IVI_lac_t;

    typedef struct _optional {
      IVIDataItem<IVI_glc_t> glc;
      IVIDataItem<IVI_gic_t> gic;
      IVIDataItem<IVI_rcc_t> rcc;
      IVIDataItem<IVI_tc_t> tc;
      IVIDataItem<IVI_lac_t> lac;
    } iviDataOptional;


    void setIvimHeader(long messageID, long protocolVersion, unsigned long stationID);
    void setIvimMessageID(long messageID){m_header.messageID=messageID;}
    void setIvimProtocolVersion(long protocolVersion){m_header.protocolVersion=protocolVersion;}
    void setIvimStationID(unsigned long stationID){m_header.stationID=stationID;}

    /* AppIVIM mandatory setters */
    void setIvimMandatory(iviDataMandatory mandatoryData){m_mandatory = mandatoryData;}
    void setIvimIviStatus(long iviStatus){m_mandatory.iviStatus = iviStatus;}

    /* AppIVIM optional setters */
    void setOptional(iviDataOptional optionalData){m_optional = optionalData;}

    void setOptionalPresent (bool glc, bool gic, bool rcc, bool tc, bool lac);

    // AppIVIM glc setter
    void setIvimGlc(IVI_glc_t glc){m_optional.glc = IVIDataItem<IVI_glc_t>(glc);}

    // AppIVIM gic setters
    void setIvimGic(IVI_gic_t gic){m_optional.gic = IVIDataItem<IVI_gic_t>(gic);}
    void pushIvimGicPart(IVI_gic_part_t gic_part);

    // AppIVIM rcc setter
    void setIvimRcc(IVI_rcc_t rcc){m_optional.rcc = IVIDataItem<IVI_rcc_t>(rcc);}

    // AppIVIM tc setter
    void setIvimtc(IVI_tc_t textContainer){m_optional.tc = IVIDataItem<IVI_tc_t> (textContainer);}

    // AppIVIM lac setter
    void setIvimlac (IVI_lac lac){m_optional.lac = IVIDataItem<IVI_lac> (lac);}
    void setIvimlac (long layoutId, std::vector<IVI_lac_comp> layComp);

    /* Header getters */
    long getIvimHeaderMessageID() {return m_header.messageID;}
    long getIvimHeaderProtocolVersion() {return m_header.protocolVersion;}
    long getIvimHeaderStationID() {return m_header.stationID;}

    long getIvimStatus() {return m_mandatory.iviStatus;}


    /* Optional fields getters */
    std::vector<bool> getOptionalPresent ();

    // Glc getter
    IVI_glc getIvimGlc() {return m_optional.glc.getData();}
    // Gic getter
    IVI_gic getGic(){return m_optional.gic.getData ();}
    // Rcc getter
    IVI_rcc getRcc(){return m_optional.rcc.getData();}
    // Tc getter
    IVI_tc getTc(){return m_optional.tc.getData();}
    // Lac getter
    IVI_lac getLac(){return m_optional.lac.getData();}

    /* Container getters */
    iviDataHeader getIvimHeader_asn_types() {return m_header;}
    iviDataMandatory getIvimMandatory_asn_types() {return m_mandatory;}
    iviDataOptional getIvimOptional_asn_types() {return m_optional;}


  private:
    iviDataHeader m_header;
    iviDataMandatory m_mandatory;
    iviDataOptional m_optional;

  };

}

#endif // IVIDATA_H
