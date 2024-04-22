/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ETSI-ITS-CDD"
 * 	found in "/mnt/EVO/asn1c-forks/output/CPM-all.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_DangerousSituationSubCauseCode_H_
#define	_DangerousSituationSubCauseCode_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeInteger.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum DangerousSituationSubCauseCode {
	DangerousSituationSubCauseCode_unavailable	= 0,
	DangerousSituationSubCauseCode_emergencyElectronicBrakeEngaged	= 1,
	DangerousSituationSubCauseCode_preCrashSystemEngaged	= 2,
	DangerousSituationSubCauseCode_espEngaged	= 3,
	DangerousSituationSubCauseCode_absEngaged	= 4,
	DangerousSituationSubCauseCode_ebEngaged	= 5,
	DangerousSituationSubCauseCode_brakeWarningEngaged	= 6,
	DangerousSituationSubCauseCode_collisionRiskWarningEngaged	= 7
} e_DangerousSituationSubCauseCode;

/* DangerousSituationSubCauseCode */
typedef long	 DangerousSituationSubCauseCode_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_DangerousSituationSubCauseCode_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_DangerousSituationSubCauseCode;
asn_struct_free_f DangerousSituationSubCauseCode_free;
asn_struct_print_f DangerousSituationSubCauseCode_print;
asn_constr_check_f DangerousSituationSubCauseCode_constraint;
ber_type_decoder_f DangerousSituationSubCauseCode_decode_ber;
der_type_encoder_f DangerousSituationSubCauseCode_encode_der;
xer_type_decoder_f DangerousSituationSubCauseCode_decode_xer;
xer_type_encoder_f DangerousSituationSubCauseCode_encode_xer;
jer_type_encoder_f DangerousSituationSubCauseCode_encode_jer;
oer_type_decoder_f DangerousSituationSubCauseCode_decode_oer;
oer_type_encoder_f DangerousSituationSubCauseCode_encode_oer;
per_type_decoder_f DangerousSituationSubCauseCode_decode_uper;
per_type_encoder_f DangerousSituationSubCauseCode_encode_uper;
per_type_decoder_f DangerousSituationSubCauseCode_decode_aper;
per_type_encoder_f DangerousSituationSubCauseCode_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _DangerousSituationSubCauseCode_H_ */
#include "asn_internal.h"