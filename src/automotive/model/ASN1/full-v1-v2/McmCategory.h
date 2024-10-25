/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "MCM-PDU-Descriptions"
 * 	found in "MCM-base.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_McmCategory_H_
#define	_McmCategory_H_


#include "asn_application.h"

/* Including external dependencies */
#include "McmCategoryType.h"
#include "StationID.h"
#include "NativeInteger.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* McmCategory */
typedef struct McmCategory {
	McmCategoryType_t	 type;
	StationID_t	*objectID;	/* OPTIONAL */
	long	*referencedTrajectoryID;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} McmCategory_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_McmCategory;
extern asn_SEQUENCE_specifics_t asn_SPC_McmCategory_specs_1;
extern asn_TYPE_member_t asn_MBR_McmCategory_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _McmCategory_H_ */
#include "asn_internal.h"
