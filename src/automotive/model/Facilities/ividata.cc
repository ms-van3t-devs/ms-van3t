#include "ividata.h"
//#include "iviService.h"
#include "ns3/asn_application.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/Setter.hpp"
#include "ns3/Encoding.hpp"
#include "ns3/SetOf.hpp"
#include "ns3/SequenceOf.hpp"
#include "ns3/BitString.hpp"
#include "ns3/View.hpp"
#include "ns3/Utils.hpp"

namespace ns3 {


  NS_LOG_COMPONENT_DEFINE("ividata");

  iviData::iviData()
  {
    m_optional.glc = IVIDataItem<IVI_glc_t>(false);
    m_optional.gic = IVIDataItem<IVI_gic_t>(false);
    m_optional.rcc = IVIDataItem<IVI_rcc_t>(false);
    m_optional.tc = IVIDataItem<IVI_tc_t>(false);
    m_optional.lac = IVIDataItem<IVI_lac_t>(false);

  }
void
iviData::setIvimHeader(long messageID, long protocolVersion, unsigned long stationID)
{
  NS_LOG_FUNCTION(this);
  m_header.messageID=messageID;
  m_header.protocolVersion=protocolVersion;
  m_header.stationID=stationID;
}

void
iviData::setOptionalPresent (bool glc, bool gic, bool rcc, bool tc, bool lac)
{
  m_optional.glc = IVIDataItem<IVI_glc>(glc);
  m_optional.gic = IVIDataItem<IVI_gic>(gic);
  m_optional.rcc = IVIDataItem<IVI_rcc>(rcc);
  m_optional.tc = IVIDataItem<IVI_tc>(tc);
  m_optional.lac = IVIDataItem<IVI_lac>(lac);
}

std::vector<bool>
iviData::getOptionalPresent ()
{
  std::vector<bool> presents;

  bool gicb,glcb,rccb,tcb,lacb;
  glcb = m_optional.glc.isAvailable ();
  gicb = m_optional.gic.isAvailable ();
  rccb = m_optional.rcc.isAvailable ();
  tcb = m_optional.tc.isAvailable ();
  lacb = m_optional.lac.isAvailable ();

  presents.push_back (glcb);
  presents.push_back (gicb);
  presents.push_back (rccb);
  presents.push_back (tcb);
  presents.push_back (lacb);

  return presents;
}

void
iviData::pushIvimGicPart(IVI_gic_part_t gic_part){
   IVI_gic_t gic = m_optional.gic.getData ();
   gic.GicPart.push_back (gic_part);
   m_optional.gic = IVIDataItem<IVI_gic_t>(gic);
}

void
iviData::setIvimlac(long layoutID, std::vector<IVI_lac_comp> layComp){
  IVI_lac lac;
  lac.layoutId = layoutID;
  lac.IVIlacComp = layComp;
  m_optional.lac = IVIDataItem<IVI_lac_t>(lac);
}


}
