/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "ITS-Container"
 * 	found in "ITS_Container.asn1"
 * 	`asn1c -fincludes-quoted -gen-PER`
 */

#ifndef	_EmbarkationStatus_H_
#define	_EmbarkationStatus_H_


#include "asn_application.h"

/* Including external dependencies */
#include "BOOLEAN.h"

#ifdef __cplusplus
extern "C" {
#endif

/* EmbarkationStatus */
typedef BOOLEAN_t	 EmbarkationStatus_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EmbarkationStatus;
asn_struct_free_f EmbarkationStatus_free;
asn_struct_print_f EmbarkationStatus_print;
asn_constr_check_f EmbarkationStatus_constraint;
ber_type_decoder_f EmbarkationStatus_decode_ber;
der_type_encoder_f EmbarkationStatus_encode_der;
xer_type_decoder_f EmbarkationStatus_decode_xer;
xer_type_encoder_f EmbarkationStatus_encode_xer;
per_type_decoder_f EmbarkationStatus_decode_uper;
per_type_encoder_f EmbarkationStatus_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _EmbarkationStatus_H_ */
#include "asn_internal.h"