/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CAM-PDU-Descriptions"
 * 	found in "/mnt/EVO/ASN1-C-ITS/CAM-PDU-Descriptions-1.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_SafetyCarContainerV1_H_
#define	_SafetyCarContainerV1_H_


#include "asn_application.h"

/* Including external dependencies */
#include "LightBarSirenInUseV1.h"
#include "TrafficRuleV1.h"
#include "SpeedLimitV1.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CauseCodeV1;

/* SafetyCarContainerV1 */
typedef struct SafetyCarContainerV1 {
	LightBarSirenInUseV1_t	 lightBarSirenInUse;
	struct CauseCodeV1	*incidentIndication	/* OPTIONAL */;
	TrafficRuleV1_t	*trafficRule	/* OPTIONAL */;
	SpeedLimitV1_t	*speedLimit	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SafetyCarContainerV1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SafetyCarContainerV1;
extern asn_SEQUENCE_specifics_t asn_SPC_SafetyCarContainerV1_specs_1;
extern asn_TYPE_member_t asn_MBR_SafetyCarContainerV1_1[4];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "CauseCodeV1.h"

#endif	/* _SafetyCarContainerV1_H_ */
#include "asn_internal.h"
