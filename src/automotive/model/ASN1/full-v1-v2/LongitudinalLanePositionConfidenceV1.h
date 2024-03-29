/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CPM-PDU-Descriptions"
 * 	found in "/mnt/EVO/ASN1-stuff/ASN1-C-ITS/CPMv1.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_LongitudinalLanePositionConfidenceV1_H_
#define	_LongitudinalLanePositionConfidenceV1_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeInteger.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum LongitudinalLanePositionConfidenceV1 {
	LongitudinalLanePositionConfidenceV1_zeroPointZeroOneMeter	= 1,
	LongitudinalLanePositionConfidenceV1_oneMeter	= 100,
	LongitudinalLanePositionConfidenceV1_outOfRange	= 101,
	LongitudinalLanePositionConfidenceV1_unavailable	= 102
} e_LongitudinalLanePositionConfidenceV1;

/* LongitudinalLanePositionConfidenceV1 */
typedef long	 LongitudinalLanePositionConfidenceV1_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_LongitudinalLanePositionConfidenceV1_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_LongitudinalLanePositionConfidenceV1;
asn_struct_free_f LongitudinalLanePositionConfidenceV1_free;
asn_struct_print_f LongitudinalLanePositionConfidenceV1_print;
asn_constr_check_f LongitudinalLanePositionConfidenceV1_constraint;
ber_type_decoder_f LongitudinalLanePositionConfidenceV1_decode_ber;
der_type_encoder_f LongitudinalLanePositionConfidenceV1_encode_der;
xer_type_decoder_f LongitudinalLanePositionConfidenceV1_decode_xer;
xer_type_encoder_f LongitudinalLanePositionConfidenceV1_encode_xer;
oer_type_decoder_f LongitudinalLanePositionConfidenceV1_decode_oer;
oer_type_encoder_f LongitudinalLanePositionConfidenceV1_encode_oer;
per_type_decoder_f LongitudinalLanePositionConfidenceV1_decode_uper;
per_type_encoder_f LongitudinalLanePositionConfidenceV1_encode_uper;
per_type_decoder_f LongitudinalLanePositionConfidenceV1_decode_aper;
per_type_encoder_f LongitudinalLanePositionConfidenceV1_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _LongitudinalLanePositionConfidenceV1_H_ */
#include "asn_internal.h"
