/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "IVI"
 * 	found in asn1/ISO19321.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example -R`
 */

#ifndef	_IviManagementContainer_H_
#define	_IviManagementContainer_H_


#include "asn_application.h"

/* Including external dependencies */
#include "Provider.h"
#include "IviIdentificationNumber.h"
#include "TimestampIts.h"
#include "IviStatus.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct IviIdentificationNumbers;
struct ConnectedDenms;

/* IviManagementContainer */
typedef struct IviManagementContainer {
	Provider_t	 serviceProviderId;
	IviIdentificationNumber_t	 iviIdentificationNumber;
	TimestampIts_t	*timeStamp;	/* OPTIONAL */
	TimestampIts_t	*validFrom;	/* OPTIONAL */
	TimestampIts_t	*validTo;	/* OPTIONAL */
	struct IviIdentificationNumbers	*connectedIviStructures;	/* OPTIONAL */
	IviStatus_t	 iviStatus;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct ConnectedDenms	*connectedDenms;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IviManagementContainer_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IviManagementContainer;
extern asn_SEQUENCE_specifics_t asn_SPC_IviManagementContainer_specs_1;
extern asn_TYPE_member_t asn_MBR_IviManagementContainer_1[8];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "IviIdentificationNumbers.h"
#include "ConnectedDenms.h"

#endif	/* _IviManagementContainer_H_ */
#include "asn_internal.h"
