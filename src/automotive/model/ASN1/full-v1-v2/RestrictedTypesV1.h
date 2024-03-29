/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ITS-Container"
 * 	found in "/mnt/EVO/ASN1-C-ITS/ITS-Container.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_RestrictedTypesV1_H_
#define	_RestrictedTypesV1_H_


#include "asn_application.h"

/* Including external dependencies */
#include "StationTypeV1.h"
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RestrictedTypesV1 */
typedef struct RestrictedTypesV1 {
	A_SEQUENCE_OF(StationTypeV1_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RestrictedTypesV1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RestrictedTypesV1;
extern asn_SET_OF_specifics_t asn_SPC_RestrictedTypesV1_specs_1;
extern asn_TYPE_member_t asn_MBR_RestrictedTypesV1_1[1];
extern asn_per_constraints_t asn_PER_type_RestrictedTypesV1_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _RestrictedTypesV1_H_ */
#include "asn_internal.h"
