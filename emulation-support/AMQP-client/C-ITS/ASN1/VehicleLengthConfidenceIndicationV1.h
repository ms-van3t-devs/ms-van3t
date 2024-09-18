/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ITS-Container"
 * 	found in "/mnt/EVO/ASN1-C-ITS/ITS-Container.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_VehicleLengthConfidenceIndicationV1_H_
#define	_VehicleLengthConfidenceIndicationV1_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeEnumerated.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum VehicleLengthConfidenceIndicationV1 {
	VehicleLengthConfidenceIndicationV1_noTrailerPresent	= 0,
	VehicleLengthConfidenceIndicationV1_trailerPresentWithKnownLength	= 1,
	VehicleLengthConfidenceIndicationV1_trailerPresentWithUnknownLength	= 2,
	VehicleLengthConfidenceIndicationV1_trailerPresenceIsUnknown	= 3,
	VehicleLengthConfidenceIndicationV1_unavailable	= 4
} e_VehicleLengthConfidenceIndicationV1;

/* VehicleLengthConfidenceIndicationV1 */
typedef long	 VehicleLengthConfidenceIndicationV1_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_VehicleLengthConfidenceIndicationV1_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_VehicleLengthConfidenceIndicationV1;
extern const asn_INTEGER_specifics_t asn_SPC_VehicleLengthConfidenceIndicationV1_specs_1;
asn_struct_free_f VehicleLengthConfidenceIndicationV1_free;
asn_struct_print_f VehicleLengthConfidenceIndicationV1_print;
asn_constr_check_f VehicleLengthConfidenceIndicationV1_constraint;
ber_type_decoder_f VehicleLengthConfidenceIndicationV1_decode_ber;
der_type_encoder_f VehicleLengthConfidenceIndicationV1_encode_der;
xer_type_decoder_f VehicleLengthConfidenceIndicationV1_decode_xer;
xer_type_encoder_f VehicleLengthConfidenceIndicationV1_encode_xer;
oer_type_decoder_f VehicleLengthConfidenceIndicationV1_decode_oer;
oer_type_encoder_f VehicleLengthConfidenceIndicationV1_encode_oer;
per_type_decoder_f VehicleLengthConfidenceIndicationV1_decode_uper;
per_type_encoder_f VehicleLengthConfidenceIndicationV1_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _VehicleLengthConfidenceIndicationV1_H_ */
#include "asn_internal.h"