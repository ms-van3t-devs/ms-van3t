/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ETSI-ITS-CDD"
 * 	found in "ETSI-ITS-CDD.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_AccelerationChange_H_
#define	_AccelerationChange_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeEnumerated.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AccelerationChange {
	AccelerationChange_accelerate	= 0,
	AccelerationChange_decelerate	= 1
} e_AccelerationChange;

/* AccelerationChange */
typedef long	 AccelerationChange_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_AccelerationChange_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_AccelerationChange;
extern const asn_INTEGER_specifics_t asn_SPC_AccelerationChange_specs_1;
asn_struct_free_f AccelerationChange_free;
asn_struct_print_f AccelerationChange_print;
asn_constr_check_f AccelerationChange_constraint;
ber_type_decoder_f AccelerationChange_decode_ber;
der_type_encoder_f AccelerationChange_encode_der;
xer_type_decoder_f AccelerationChange_decode_xer;
xer_type_encoder_f AccelerationChange_encode_xer;
jer_type_encoder_f AccelerationChange_encode_jer;
oer_type_decoder_f AccelerationChange_decode_oer;
oer_type_encoder_f AccelerationChange_encode_oer;
per_type_decoder_f AccelerationChange_decode_uper;
per_type_encoder_f AccelerationChange_encode_uper;
per_type_decoder_f AccelerationChange_decode_aper;
per_type_encoder_f AccelerationChange_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _AccelerationChange_H_ */
#include "asn_internal.h"
