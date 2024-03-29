/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ITS-Container"
 * 	found in "/mnt/EVO/ASN1-C-ITS/ITS-Container.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_ItineraryPathV1_H_
#define	_ItineraryPathV1_H_


#include "asn_application.h"

/* Including external dependencies */
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ReferencePositionV1;

/* ItineraryPathV1 */
typedef struct ItineraryPathV1 {
	A_SEQUENCE_OF(struct ReferencePositionV1) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ItineraryPathV1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ItineraryPathV1;
extern asn_SET_OF_specifics_t asn_SPC_ItineraryPathV1_specs_1;
extern asn_TYPE_member_t asn_MBR_ItineraryPathV1_1[1];
extern asn_per_constraints_t asn_PER_type_ItineraryPathV1_constr_1;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ReferencePositionV1.h"

#endif	/* _ItineraryPathV1_H_ */
#include "asn_internal.h"
