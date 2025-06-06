/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ETSI-ITS-CDD"
 * 	found in "ETSI-ITS-CDD.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_InterferenceManagementZones_H_
#define	_InterferenceManagementZones_H_


#include "asn_application.h"

/* Including external dependencies */
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct InterferenceManagementZone;

/* InterferenceManagementZones */
typedef struct InterferenceManagementZones {
	A_SEQUENCE_OF(struct InterferenceManagementZone) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} InterferenceManagementZones_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InterferenceManagementZones;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "InterferenceManagementZone.h"

#endif	/* _InterferenceManagementZones_H_ */
#include "asn_internal.h"
