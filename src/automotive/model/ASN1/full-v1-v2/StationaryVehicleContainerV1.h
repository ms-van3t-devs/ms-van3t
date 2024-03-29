/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "DENM-PDU-Descriptions"
 * 	found in "/mnt/EVO/ASN1-C-ITS/DENM-PDU-Descriptions-1.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_StationaryVehicleContainerV1_H_
#define	_StationaryVehicleContainerV1_H_


#include "asn_application.h"

/* Including external dependencies */
#include "StationarySinceV1.h"
#include "NumberOfOccupantsV1.h"
#include "EnergyStorageTypeV1.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CauseCodeV1;
struct DangerousGoodsExtendedV1;
struct VehicleIdentificationV1;

/* StationaryVehicleContainerV1 */
typedef struct StationaryVehicleContainerV1 {
	StationarySinceV1_t	*stationarySince	/* OPTIONAL */;
	struct CauseCodeV1	*stationaryCause	/* OPTIONAL */;
	struct DangerousGoodsExtendedV1	*carryingDangerousGoods	/* OPTIONAL */;
	NumberOfOccupantsV1_t	*numberOfOccupants	/* OPTIONAL */;
	struct VehicleIdentificationV1	*vehicleIdentification	/* OPTIONAL */;
	EnergyStorageTypeV1_t	*energyStorageType	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} StationaryVehicleContainerV1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_StationaryVehicleContainerV1;
extern asn_SEQUENCE_specifics_t asn_SPC_StationaryVehicleContainerV1_specs_1;
extern asn_TYPE_member_t asn_MBR_StationaryVehicleContainerV1_1[6];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "CauseCodeV1.h"
#include "DangerousGoodsExtendedV1.h"
#include "VehicleIdentificationV1.h"

#endif	/* _StationaryVehicleContainerV1_H_ */
#include "asn_internal.h"
