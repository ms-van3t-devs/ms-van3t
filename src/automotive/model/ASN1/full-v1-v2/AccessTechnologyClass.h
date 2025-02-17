/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ETSI-ITS-CDD"
 * 	found in "ETSI-ITS-CDD.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_AccessTechnologyClass_H_
#define	_AccessTechnologyClass_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeEnumerated.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AccessTechnologyClass {
	AccessTechnologyClass_any	= 0,
	AccessTechnologyClass_itsg5Class	= 1,
	AccessTechnologyClass_ltev2xClass	= 2,
	AccessTechnologyClass_nrv2xClass	= 3
	/*
	 * Enumeration is extensible
	 */
} e_AccessTechnologyClass;

/* AccessTechnologyClass */
typedef long	 AccessTechnologyClass_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_AccessTechnologyClass_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_AccessTechnologyClass;
extern const asn_INTEGER_specifics_t asn_SPC_AccessTechnologyClass_specs_1;
asn_struct_free_f AccessTechnologyClass_free;
asn_struct_print_f AccessTechnologyClass_print;
asn_constr_check_f AccessTechnologyClass_constraint;
ber_type_decoder_f AccessTechnologyClass_decode_ber;
der_type_encoder_f AccessTechnologyClass_encode_der;
xer_type_decoder_f AccessTechnologyClass_decode_xer;
xer_type_encoder_f AccessTechnologyClass_encode_xer;
jer_type_encoder_f AccessTechnologyClass_encode_jer;
oer_type_decoder_f AccessTechnologyClass_decode_oer;
oer_type_encoder_f AccessTechnologyClass_encode_oer;
per_type_decoder_f AccessTechnologyClass_decode_uper;
per_type_encoder_f AccessTechnologyClass_encode_uper;
per_type_decoder_f AccessTechnologyClass_decode_aper;
per_type_encoder_f AccessTechnologyClass_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _AccessTechnologyClass_H_ */
#include "asn_internal.h"
