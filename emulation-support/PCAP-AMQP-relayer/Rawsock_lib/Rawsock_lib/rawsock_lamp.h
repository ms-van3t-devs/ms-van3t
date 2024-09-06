/** \file 
	Extra <b>La</b>tency <b>M</b>easurement <b>P</b>rotocol (custom protocol) support, developed in Politecnico di Torino.

	This file represents an additional module of the Rawsock library, adding support for a custom L7 protocol, which
	can be encapsulated inside any packet and transported using any protocol, providing a tool to measure latency
	between wireless (and wired) devices. We decided to call it <i>LaMP</i> (Latency Measurement Protocol).

	This file uses rawsock.h, but it is not included within the main module, allowing the user to include the Rawsock library
	without forcing him/her to include also the LaMP module, if he/she does not need it.

	The version number of this module is set to be the same as the main Rawsock library version number.

	\version 0.3.4
	\date 2020-04-24
	\copyright Licensed under GPLv2
**/
#ifndef RAWSOCK_LAMP_H_INCLUDED
#define RAWSOCK_LAMP_H_INCLUDED

#include "rawsock.h"
#include <linux/if_packet.h>
#include <sys/time.h>

#define PROTO_LAMP 0xAA /**< **LaMP reserved field value**: the reserved field of LaMP must always be checked against this constant, as it represents the value that every LaMP packet should contain inside the reserved field. It allows to distinguish LaMP packets with respect to non-LaMP payloads. */
#define PROTO_LAMP_CTRL_MASK 0xA0 /**< **LaMP control reserved field mask**: every LaMP packet should contain 0xA as MSB of the control field, as extension of the reserved field itself. You can use this mask to check if the MSB is really 0xA (as rquired by LaMP); however, it is highly suggested to use   */

#define CTRL_PINGLIKE_REQ 0xA0 /**< **LaMP type (full control field)**: Ping-like (bidirectional) request **/
#define CTRL_PINGLIKE_REPLY 0xA1 /**< **LaMP type (full control field)**: Ping-like (bidirectional) reply **/
#define CTRL_PINGLIKE_ENDREQ 0xA2 /**< **LaMP type (full control field)**: Ping-like (bidirectional) end request **/
#define CTRL_PINGLIKE_ENDREPLY 0xA3 /**< **LaMP type (full control field)**: Ping-like (bidirectional) end reply **/
#define CTRL_UNIDIR_CONTINUE 0xA4 /**< **LaMP type (full control field)**: Unidirectional continue packet (there will be more packets after the current one) **/
#define CTRL_UNIDIR_STOP 0xA5 /**< **LaMP type (full control field)**: Unidirectional stop packet (last packet of the current session) **/
#define CTRL_UNIDIR_REPORT 0xA6 /**< **LaMP type (full control field)**: Report packet (payload should contain user-defined data to be transmitted containing latency related statistics) **/
#define CTRL_ACK 0xA7 /**< **LaMP type (full control field)**: Generic Acknowledgment (user can specify an additional custom payload, but it is not suggested to do so) **/
#define CTRL_CONN_INIT 0xA8 /**< **LaMP type (full control field)**: Connection INIT packet (initial handshake INIT packet) **/
#define CTRL_PINGLIKE_REQ_TLESS 0xA9 /**< **LaMP type (full control field)**: Ping-like (bidirectional) request - timestampless **/
#define CTRL_PINGLIKE_REPLY_TLESS 0xAA /**< **LaMP type (full control field)**: Ping-like (bidirectional) reply - timestampless **/
#define CTRL_PINGLIKE_ENDREQ_TLESS 0xAB /**< **LaMP type (full control field)**: Ping-like (bidirectional) end request - timestampless **/
#define CTRL_PINGLIKE_ENDREPLY_TLESS 0xAC /**< **LaMP type (full control field)**: Ping-like (bidirectional) end reply - timestampless **/
#define CTRL_FOLLOWUP_CTRL 0xAD /**< **LaMP type (full control field)**: Follow up control message (request, deny or accept) **/
#define CTRL_FOLLOWUP_DATA 0xAE /**< **LaMP type (full control field)**: Follow up data message (time delta message) **/

// Follow-up types
#define FOLLOWUP_REQUEST	0x0000 /**< **LaMP follow-up control type**: follow-up request (default request type: 0x00) **/
#define FOLLOWUP_DENY		0x0100 /**< **LaMP follow-up control type**: follow-up deny **/
#define FOLLOWUP_ACCEPT		0x0200 /**< **LaMP follow-up control type**: follow-up accept **/
#define FOLLOWUP_UNKNOWN  	0xFFFF /**< **LaMP follow-up control type**: bad or unknown request **/

// Follow-up request types
#define FOLLOWUP_REQUEST_T_APP		0x0000 /**< **LaMP follow-up request type**: application timestamps **/
#define FOLLOWUP_REQUEST_T_KRN_RX 	0x0001 /**< **LaMP follow-up request type**: kernel rx timestamps **/
#define FOLLOWUP_REQUEST_T_KRN 		0x0002 /**< **LaMP follow-up request type**: kernel timestamps **/
#define FOLLOWUP_REQUEST_T_HW 		0x0003 /**< **LaMP follow-up request type**: hardware timestamps **/

// Macro to check if a given "payload length or packet type" field, specified as 'idx', contains a valid FOLLOWUP_REQUEST or not, no matter the request type
#define IS_FOLLOWUP_REQUEST(lenortype_field) ((lenortype_field & 0xFF00)==0x0000) /**< **LaMP Test macro**: it checks, given a "payload length or packet type" field of a FOLLOWUP_CTRL packet, specified as 'lenortype_field', if it corresponds to a valid FOLLOWUP_REQUEST or not, no matter the request type (i.e. it checks if the first byte is set to _0x00_) */

// Macros that accounts for more than one packet at once (to be used inside if-else statements)
#define IS_CTRL_PINGLIKE_REQ(ctrl) (ctrl==CTRL_PINGLIKE_REQ || ctrl==CTRL_PINGLIKE_REQ_TLESS) /**< **Multi-type LaMP type check macro**: checks if the control field of a LaMP header, specified as _ctrl_, corresponds to any kind of ping-like request */
#define IS_CTRL_PINGLIKE_REPLY(ctrl) (ctrl==CTRL_PINGLIKE_REPLY || ctrl==CTRL_PINGLIKE_REPLY_TLESS) /**< **Multi-type LaMP type check macro**: checks if the control field of a LaMP header, specified as _ctrl_, corresponds to any kind of ping-like reply */
#define IS_CTRL_PINGLIKE_ENDREQ(ctrl) (ctrl==CTRL_PINGLIKE_ENDREQ || ctrl==CTRL_PINGLIKE_ENDREQ_TLESS) /**< **Multi-type LaMP type check macro**: checks if the control field of a LaMP header, specified as _ctrl_, corresponds to any kind of ping-like end request */
#define IS_CTRL_PINGLIKE_ENDREPLY(ctrl) (ctrl==CTRL_PINGLIKE_ENDREPLY || ctrl==CTRL_PINGLIKE_ENDREPLY_TLESS) /**< **Multi-type LaMP type check macro**: checks if the control field of a LaMP header, specified as _ctrl_, corresponds to any kind of ping-like end reply */

#define INIT_PINGLIKE_INDEX 0x0001 /**< **INIT type field value**: ping-like. */
#define INIT_UNIDIR_INDEX 0x0002 /**< **INIT type field value**: undirectional. */

#define IS_INIT_INDEX_VALID(idx) (idx == INIT_PINGLIKE_INDEX || idx == INIT_UNIDIR_INDEX) /**< **LaMP Test macro**: checks whether the given INIT type field value (which can be extracted, for instance, from a received LaMP header and specified as _idx_) is valid or not. */
#define IS_INIT(ctrl) (ctrl == CTRL_CONN_INIT) /**< **LaMP Test macro**: checks if the given control field value (specified as _ctrl_) is corresponding to "Connection INIT". */
#define IS_FOLLOWUP_CTRL(ctrl) (ctrl == CTRL_FOLLOWUP_CTRL) /**< **LaMP Test macro**: checks if the given control field value (specified as _ctrl_) is corresponding to "Follow-up control". */
#define IS_FOLLOWUP_CTRL_REQ_TYPE_VALID(idx) (idx == FOLLOWUP_REQUEST_T_APP || idx == FOLLOWUP_REQUEST_T_KRN_RX || idx == FOLLOWUP_REQUEST_T_KRN || idx == FOLLOWUP_REQUEST_T_HW || (idx >= 0x00F0 && idx<= 0x00FF)) /**< **LaMP Test macro**: checks, given the full 16-bits long "payload length or packet type" field, whether it contains a valid follow-up request type or not (user defined values are included). */
#define IS_FOLLOWUP_CTRL_TYPE_VALID(idx) (idx == FOLLOWUP_REQUEST || idx == FOLLOWUP_DENY || idx == FOLLOWUP_ACCEPT || IS_FOLLOWUP_CTRL_REQ_TYPE_VALID(idx)) /**< **LaMP Test macro**: checks whether the given FOLLOWUP_CTRL type field value (which can be extracted, for instance, from a received LaMP header and specified as _idx_) is valid or not. */
#define IS_UNIDIR(ctrl) (ctrl == CTRL_UNIDIR_CONTINUE || ctrl == CTRL_UNIDIR_STOP) /**< **LaMP Test macro**: checks, though the specified (as _ctrl_) control field value, if the current packet is undirectional. */
#define IS_PINGLIKE(ctrl) (ctrl == CTRL_PINGLIKE_REQ || ctrl == CTRL_PINGLIKE_REPLY || ctrl == CTRL_PINGLIKE_ENDREQ || ctrl == CTRL_PINGLIKE_ENDREPLY || ctrl == CTRL_PINGLIKE_REQ_TLESS || ctrl == CTRL_PINGLIKE_REPLY_TLESS || ctrl == CTRL_PINGLIKE_ENDREQ_TLESS || ctrl == CTRL_PINGLIKE_ENDREPLY_TLESS) /**< **LaMP Test macro**: checks, though the specified (as _ctrl_) control field value, if the current packet is ping-like. */
#define IS_LAMP(reserved, ctrl) (reserved==PROTO_LAMP && (ctrl & PROTO_LAMP_CTRL_MASK)==PROTO_LAMP_CTRL_MASK) /**< **LaMP Test macro**: _important macro:_ you can use this to check if a received packet is really encapsulating LaMP, after trying to extract the reserved (_reserved_) and control (_ctrl_) fields from it (threating the first bytes as if they were a LaMP header). */

#define ETHERTYPE_LAMP ETH_P_802_EX1 /**< Local Experimental Ethertype should be used if LaMP is encapsulated directly inside a 802.11/Ethernet packet. You can use **ETHERTYPE_LAMP** for the sake of clarity (but **ETH_P_802_EX1** is perfectly fine too). */
#define MAX_LAMP_LEN (65535) /**< **LaMP size definition: maximum payload size a LaMP packet can bear. */

#define LAMP_HDR_PAYLOAD_SIZE_STR(data) sizeof(struct lamphdr)+strlen(data) /**< __Size definition__:  given *data*, as a string, the LaMP packet size (header + payload) containing the specified string is calculated and returned in _bytes_. */
#define LAMP_HDR_PAYLOAD_SIZE(size) sizeof(struct lamphdr)+size /**< __Size definition__:  given *size*, in _bytes_, the LaMP packet size (LaMP header + payload) containing a payload with the specified *size* is calculated and returned in _bytes_. This macro is similar to [UDP_PACKET_SIZE_S](\ref UDP_PACKET_SIZE_S), [IP_UDP_PACKET_SIZE_S](\ref IP_UDP_PACKET_SIZE_S) and [ETH_IP_UDP_PACKET_SIZE_S](\ref ETH_IP_UDP_PACKET_SIZE_S), defined in **rawsock.h** */
#define LAMP_HDR_SIZE() sizeof(struct lamphdr) /**< __Size definition__: this macro returns the size of a LaMP header. Since the underlying _struct lamphdr_ should be already "packed by design", this macro should be equivant to a call to ´sizeof(struct lamphdr)´, but a bit more compact. */

#define TYPE_TO_CTRL(field) (field | PROTO_LAMP_CTRL_MASK) /**< __Conversion macro__: Macro to convert a [lamptype_t](\ref lamptype_t) value (specified as _field_, 4 bits) to the corresponding full control field (_ctrl_) value (0xA + 4 type bits). */
#define CTRL_TO_TYPE(field) (field & 0x0F) /**< __Conversion macro__: Macro to convert a full control field (_ctrl_) value (0xA + 4 type bits) to the corresponding [lamptype_t](\ref lamptype_t) value (4 bits). */

#ifndef BYTE_TYPE
#define BYTE_TYPE
typedef unsigned char byte_t; /**< Custom type to store a single byte. It should be defined only if it was not defined elsewhere. */
#endif

/**
	\brief LaMP type enumerator

	LaMP type enumerator: it can be used to manage the different types of LaMP packets without having to deal with the upper 4 bits of the _control_ header field,
	which are always equal to _0xA_.

	The conversion from a control field value to the corresponding *lamptype_t* value, and vice versa, can be obtained through [TYPE_TO_CTRL](\ref TYPE_TO_CTRL) and [CTRL_TO_TYPE](\ref CTRL_TO_TYPE).
**/
typedef enum {
	PINGLIKE_REQ, 				/**< Ping-like (bidirectional) request */
	PINGLIKE_REPLY, 			/**< Ping-like (bidirectional) reply */
	PINGLIKE_ENDREQ,			/**< Ping-like (bidirectional) end request */
	PINGLIKE_ENDREPLY,			/**< Ping-like (bidirectional) end reply */
	UNIDIR_CONTINUE,			/**< Unidirectional continue packet (there will be more packets after the current one) */
	UNIDIR_STOP,				/**< Unidirectional stop packet (last packet of the current session) */
	REPORT,						/**< Report packet (payload should contain user-defined data to be transmitted containing latency related statistics) */
	ACK,						/**< Generic Acknowledgment (user can specify an additional custom payload, but it is not suggested to do so) */
	INIT,						/**< Connection INIT packet (initial handshake INIT packet) */
	PINGLIKE_REQ_TLESS,			/**< Ping-like (bidirectional) request - timestampless */
	PINGLIKE_REPLY_TLESS,		/**< Ping-like (bidirectional) reply - timestampless */
	PINGLIKE_ENDREQ_TLESS,		/**< Ping-like (bidirectional) end request - timestampless */
	PINGLIKE_ENDREPLY_TLESS, 	/**< Ping-like (bidirectional) end reply - timestampless */
	FOLLOWUP_CTRL, 				/**< Follow-up control message */
	FOLLOWUP_DATA 				/**< Follow-up data message */
} lamptype_t;

/**
	\brief End flag type
	
	This enumerator can be used to tell rawLampSend() is the current packet whether the last one of the current session or not.

	If it is **FLG_STOP**, the current packet will be threated as last one, and its type will be forced to "ping-like (bidirectional) end request"
	or "ping-like (bidirectional) end request - timestampless" or "unidirectional stop" (depending on the current session type).

	If it is **FLG_CONTINUE** the current packet will not be threated as the last one, and its type won't be changed (it remains as specified before,
	for example by means of lampHeadPopulate().

	<b>FLG_NONE</b> can be used in all the cases in which the user does not need to specify if the current packet is the last one or not (e.g. ACK and INIT packets).

	Please see rawLampSend(), as the existence of this enumerator is directly linked to this function.
**/
typedef enum {
	FLG_CONTINUE, /**< Flag **CONTINUE**: this is not the last packet of the session */
	FLG_STOP, /**< Flag **STOP**: this is the last packet of the session */
	FLG_NONE /**< Flag **NONE**: the user does not want/need to specify any flag to rawLampSend() */
} endflag_t;

/**
	\brief Main LaMP packet header structure.

	This structure is the main LaMP packet header structure, to be used to prepare and read any LaMP packet.

	It contains all the required fields, and it can be used like other Linux structures, such as *struct ether_header*, *struct iphdr*, *struct udphdr* and so on.

	An application dealing with LaMP packets should declare at least once a _struct lamphdr_.
**/
struct lamphdr {
	uint8_t reserved; /**< Reserved field, 1 B: should always be set to _0xAA_. */
	uint8_t ctrl; /**< Control field, 1 B: the first 4 bits should always be set to _0xA_, while the second 4 bits are used to encode the packet type. */
	uint16_t id; /**< Identification field, 2 B: it is used to identify a certain LaMP session. */
	uint16_t seq; /**< Sequence field, 2 B: it is used to store cyclically increasing sequence numbers, up to 65535, that can be used to identify lost packets and to associate replies with requests. */
	uint16_t len; /**< Payload length or packet type, 2 B: it store the (optional) payload length, up to 65535 B, or, if the message type is INIT or FOLLOWUP, the type of the connection that should be established (pinglike or unidirectional) or the kind of follow-up message. */
	uint64_t sec; /**< 64-bit seconds timestamp, 8 B: it stores the seconds of the current packet timestamp. */
	uint64_t usec; /**< 64-bit microseconds timestamp, 8 B: it stores the microseconds of the current packet timestamp. */
};

void lampHeadPopulate(struct lamphdr *lampHeader, unsigned char ctrl, unsigned short id, unsigned short seq);
void lampHeadSetTimestamp(struct lamphdr *lampHeader, struct timeval *tStampPtr); // Sets the LaMP header timestamp (specify NULL as struct timeval *tStampPtr to use the current time instead of a custom timestamp) -> to be used with non-raw sockets, in which rawLampSend() cannot be used
void lampEncapsulate(byte_t *packet, struct lamphdr *lampHeader, byte_t *data, size_t payloadsize);
void lampSetUnidirStop(struct lamphdr *lampHeader);
void lampSetPinglikeEndreq(struct lamphdr *lampHeader);
void lampSetPinglikeEndreqTless(struct lamphdr *lampHeader);
void lampSetPinglikeEndreqAll(struct lamphdr *lampHeader);
void lampHeadSetConnType(struct lamphdr *initLampHeader, uint16_t mode_index);
void lampHeadSetFollowupCtrlType(struct lamphdr *followupLampHeader, uint16_t followup_type);

void lampHeadIncreaseSeq(struct lamphdr *inpacket_headerptr);
int rawLampSend(int descriptor, struct sockaddr_ll addrll, struct lamphdr *inpacket_headerptr, byte_t *ethernetpacket, size_t finalpacketsize, endflag_t end_flag, protocol_t llprot);

void lampHeadGetData(byte_t *lampPacket, lamptype_t *type, unsigned short *id, unsigned short *seq, unsigned short *len, struct timeval *timestamp, byte_t *payload);
byte_t *lampGetPacketPointers(byte_t *pktbuf,struct lamphdr **lampHeader);
#endif
