/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ETSI-ITS-CDD"
 * 	found in "ETSI-ITS-CDD.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_VruClusterProfiles_H_
#define	_VruClusterProfiles_H_


#include "asn_application.h"

/* Including external dependencies */
#include "BIT_STRING.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum VruClusterProfiles {
	VruClusterProfiles_pedestrian	= 0,
	VruClusterProfiles_bicyclist	= 1,
	VruClusterProfiles_motorcyclist	= 2,
	VruClusterProfiles_animal	= 3
} e_VruClusterProfiles;

/* VruClusterProfiles */
typedef BIT_STRING_t	 VruClusterProfiles_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_VruClusterProfiles_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_VruClusterProfiles;
asn_struct_free_f VruClusterProfiles_free;
asn_struct_print_f VruClusterProfiles_print;
asn_constr_check_f VruClusterProfiles_constraint;
ber_type_decoder_f VruClusterProfiles_decode_ber;
der_type_encoder_f VruClusterProfiles_encode_der;
xer_type_decoder_f VruClusterProfiles_decode_xer;
xer_type_encoder_f VruClusterProfiles_encode_xer;
jer_type_encoder_f VruClusterProfiles_encode_jer;
oer_type_decoder_f VruClusterProfiles_decode_oer;
oer_type_encoder_f VruClusterProfiles_encode_oer;
per_type_decoder_f VruClusterProfiles_decode_uper;
per_type_encoder_f VruClusterProfiles_encode_uper;
per_type_decoder_f VruClusterProfiles_decode_aper;
per_type_encoder_f VruClusterProfiles_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _VruClusterProfiles_H_ */
#include "asn_internal.h"
