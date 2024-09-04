// Rawsock library, licensed under GPLv2
// Version 0.3.4
#include "rawsock.h"
#include "rawsock_lamp.h"
#include "minirighi_udp_checksum.h"
#include <sys/time.h>
#include <string.h>

/**
	\brief Populate a LaMP header

	This function can be used to populate a LaMP header (_struct lamphdr_).

	The user shall specify an already existing LaMP header structure to be filled in, the control field value (the **full** one), as _ctrl_,
	the identification value and the sequence number of the current packet.

	\note Like all the other functions inside the Rawsock library, it already takes care of byte ordering.

	\note In order to increase the sequence number inside an existing LaMP header, it is not necessary to repeatedly call this function.
	Use, instead, lampHeadIncreaseSeq().

	\warning **Important**: the timestamp is initialized to **0**. However, you should set a timestamp for any non timestampless or control packet
	(i.e. ACK and INIT). This can be achieved manually thanks to lampHeadSetTimestamp() (to be used for transmissions over normal sockets) or 
	automatically when calling rawLampSend() (to be used for transmissions over raw sockets). It is always advised to call, in the manual case,
	lampHeadSetTimestamp() as the last operation before sending the data.

	\param[in,out]		lampHeader 		Pointer to the LaMP header structure.
	\param[in]  		ctrl 			Full control field value (including the first 4 bits equal to _0xA_) to be placed inside the header.
	\param[in]  		id   			Identification field value.
	\param[in]  		seq   			(Initial) sequence number to be put inside the LaMP header.

	\return None.
**/
void lampHeadPopulate(struct lamphdr *lampHeader, unsigned char ctrl, unsigned short id, unsigned short seq) {
	lampHeader->reserved=PROTO_LAMP;
	lampHeader->ctrl=(uint8_t) ctrl;
	lampHeader->id=htons(id);
	lampHeader->seq=htons(seq);
	lampHeader->len=0;

	lampHeader->sec=0;
	lampHeader->usec=0;
}

/**
	\brief Set the INIT type field ("length or packet type") inside a LaMP header

	This function can be used to set the "length or packet type" field inside an already populated LaMP header.

	As it can be used to set an **INIT type**, this function has an effect only if the headers carries an
	"INIT" packet type and a valid index (see [INIT_PINGLIKE_INDEX](\ref INIT_PINGLIKE_INDEX) and [INIT_UNIDIR_INDEX](\ref INIT_UNIDIR_INDEX))
	is specified, otherwise it has no effect.

	\note Like all the other functions inside the Rawsock library, it already takes care of byte ordering.

	\param[in,out]		initLampHeader 		Pointer to the LaMP header structure, already populated by lampHeadPopulate().
	\param[in]			mode_index			INIT type index to be used (see [INIT_PINGLIKE_INDEX](\ref INIT_PINGLIKE_INDEX) and [INIT_UNIDIR_INDEX](\ref INIT_UNIDIR_INDEX))

	\return None.
**/
void lampHeadSetConnType(struct lamphdr *initLampHeader, uint16_t mode_index) {
	// If packet type is not INIT, ignore any operation on the LaMP header
	if(initLampHeader->ctrl==CTRL_CONN_INIT && IS_INIT_INDEX_VALID(mode_index)) {
		initLampHeader->len=htons(mode_index);
	}
}

/**
	\brief Set the FOLLOWUP_CTRL type field ("length or packet type") inside a LaMP header

	This function can be used to set the "length or packet type" field inside an already populated LaMP header.

	As it can be used to set a **FOLLOWUP_CTRL type**, this function has an effect only if the headers carries a
	"FOLLOWUP_CTRL" packet type and a valid type is specified, otherwise it has no effect.

	\note Like all the other functions inside the Rawsock library, it already takes care of byte ordering.

	\param[in,out]		followupLampHeader 		Pointer to the LaMP header structure, already populated by lampHeadPopulate().
	\param[in]			followup_type			FOLLOWUP_CTRL type to be used (see for instance [FOLLOWUP_REQUEST](\ref FOLLOWUP_REQUEST))

	\return None.
**/
void lampHeadSetFollowupCtrlType(struct lamphdr *followupLampHeader, uint16_t followup_type) {
	// If packet type is not FOLLOWUP_CTRL, ignore any operation on the LaMP header
	if(followupLampHeader->ctrl==CTRL_FOLLOWUP_CTRL && IS_FOLLOWUP_CTRL_TYPE_VALID(followup_type)) {
		followupLampHeader->len=htons(followup_type);
	}
}

/**
	\brief Set the timestamp inside a LaMP Header

	This function can be used to set the timestamp (seconds and microseconds fields) inside the specified LaMP header.

	If tStampPtr is NULL, the timestamp is set to the time instant in which this function was called and started its execution.
	If it is not NULL, it will be set using the values stored inside the specified struct timeval.

	<b>A realtime clock is used</b> (i.e. the one used for _gettimeofday()_).

	This function has no effect if the packet is timestampless (i.e. if the control field is indicating a "TLESS" type)

	\note Like all the other functions inside the Rawsock library, it already takes care of byte ordering.

	\param[in,out]		lampHeader 		Pointer to the LaMP header structure.
	\param[in]			tStampPtr		Pointer to a struct timeval to store a custom timestamp, or NULL to use the current time

	\return None.
**/
void lampHeadSetTimestamp(struct lamphdr *lampHeader, struct timeval *tStampPtr) {
	struct timeval currtime;

	if(lampHeader->ctrl!=CTRL_PINGLIKE_REQ_TLESS && lampHeader->ctrl!=CTRL_PINGLIKE_REPLY_TLESS && lampHeader->ctrl!=CTRL_PINGLIKE_ENDREQ_TLESS && lampHeader->ctrl!=CTRL_PINGLIKE_ENDREPLY_TLESS) {
		if(tStampPtr==NULL) {
			gettimeofday(&currtime,NULL); // Set timestamp as very last operation, only if it is not a TLESS packet

			lampHeader->sec=hton64((uint64_t) currtime.tv_sec);
			lampHeader->usec=hton64((uint64_t) currtime.tv_usec);
		} else {
			lampHeader->sec=hton64((uint64_t) tStampPtr->tv_sec);
			lampHeader->usec=hton64((uint64_t) tStampPtr->tv_usec);
		}
	}
}

/**
	\brief Increase the sequence number inside a LaMP header

	This function can be used to cyclically increase (up to 65535 and then back to 0) the sequence number inside an existing LaMP
	header of an existing LaMP packet.

	The function takes as input the pointer to a LaMP header (i.e. to a memory area where a _struct lamphdr_ is stored).

	\param[in,out]		inpacket_headerptr 		Pointer to the LaMP header structure, in which the sequence number will be increased by 1.

	\return None.
**/
void lampHeadIncreaseSeq(struct lamphdr *inpacket_headerptr) {
	uint16_t seq=htons(inpacket_headerptr->seq); // Get current sequence number in the proper bit order
	// Take into account ciclicity in the sequence numbers
	if(inpacket_headerptr->seq==UINT16_MAX) {
		seq=0;
	} else {
		seq++;
	}
	inpacket_headerptr->seq=ntohs(seq);
}

/**
	\brief Set the control field of a LaMP header to "Unidirectional stop"

	This function can be used to set the the control field inside an existing LaMP
	header of an existing LaMP packet, to "Unidirectional stop".

	The function takes as input the pointer to a LaMP header (i.e. to a memory area where a _struct lamphdr_ is stored).

	\param[in,out]		lampHeader 		Pointer to the LaMP header structure.

	\return None.
**/
void lampSetUnidirStop(struct lamphdr *lampHeader) {
	lampHeader->ctrl=CTRL_UNIDIR_STOP;
}

/**
	\brief Set the control field of a LaMP header to "Ping-like (bidirectional) end request"

	This function can be used to set the the control field inside an existing LaMP
	header of an existing LaMP packet, to "Ping-like (bidirectional) end request".

	The function takes as input the pointer to a LaMP header (i.e. to a memory area where a _struct lamphdr_ is stored).

	\param[in,out]		lampHeader 		Pointer to the LaMP header structure.

	\return None.
**/
void lampSetPinglikeEndreq(struct lamphdr *lampHeader) {
	lampHeader->ctrl=CTRL_PINGLIKE_ENDREQ;
}

/**
	\brief Set the control field of a LaMP header to "Ping-like (bidirectional) end request (timestampless)"

	This function can be used to set the the control field inside an existing LaMP
	header of an existing LaMP packet, to "Ping-like (bidirectional) end request (timestampless)".

	The function takes as input the pointer to a LaMP header (i.e. to a memory area where a _struct lamphdr_ is stored).

	\param[in,out]		lampHeader 		Pointer to the LaMP header structure.

	\return None.
**/
void lampSetPinglikeEndreqTless(struct lamphdr *lampHeader) {
	lampHeader->ctrl=CTRL_PINGLIKE_ENDREQ_TLESS;
}

/**
	\brief Set the control field of a LaMP header to "Ping-like (bidirectional) end request"

	This function can be used to set the the control field inside an existing LaMP
	header of an existing LaMP packet, to "Ping-like (bidirectional) end request".

	This function will look at the already existing control field and act accordingly:

	1. If the control field corresponds to [CTRL_PINGLIKE_REQ](\ref CTRL_PINGLIKE_REQ), [CTRL_PINGLIKE_ENDREQ](\ref CTRL_PINGLIKE_ENDREQ) is set
	2. If the control field corresponds to [CTRL_PINGLIKE_REQ_TLESS](\ref CTRL_PINGLIKE_REQ_TLESS), [CTRL_PINGLIKE_ENDREQ_TLESS](\ref CTRL_PINGLIKE_ENDREQ_TLESS) is set
	3. Otherwise, the control field remains unmodified.

	The function takes as input the pointer to a LaMP header (i.e. to a memory area where a _struct lamphdr_ is stored).

	\param[in,out]		lampHeader 		Pointer to the LaMP header structure.

	\return None.
**/
void lampSetPinglikeEndreqAll(struct lamphdr *lampHeader) {
	if(lampHeader->ctrl==CTRL_PINGLIKE_REQ) {
		lampHeader->ctrl=CTRL_PINGLIKE_ENDREQ;
	} else if(lampHeader->ctrl==CTRL_PINGLIKE_REQ_TLESS) {
		lampHeader->ctrl=CTRL_PINGLIKE_ENDREQ_TLESS;
	}
}

/**
	\brief Combine LaMP payload and header
	
	This function combines a LaMP (optional) payload and header, the latter in the form of a [struct lamphdr](\ref lamhdr) structure.

	It can be used if an additional payload have to be inserted inside a LaMP packet. If the user does not want to insert any payload,
	he can directly send the LaMP header structure as payload of any other protocol, encapsulating LaMP, without the need of calling lampEncapsulate().

	The user shall specify a buffer in which the full packet will be put, the LaMP header, the payload
	(<i>byte_t *data</i>) and its size in bytes.

	\warning The _packet_ buffer should be already allocated, and it should be big enough to contain both the header **and** 
	the payload. It is possible to use the macros defined in rawsock_lamp.h, such as [LAMP_HDR_PAYLOAD_SIZE(LaMP_payload_size)](\ref LAMP_HDR_PAYLOAD_SIZE), 
	to find the required size starting from an higher layer payload size and then call a **malloc()** with that size to allocate the memory for _packet_.

	\note The "length" field of the LaMP header is automatically set.

	\param[out] 	packet 		Packet buffer (should be already allocated) that will contain the full LaMP packet (LaMP header + payload)
	\param[in]		lampHeader 	LaMP header, as [struct lamphdr](\ref lamhdr). Should be filled in with [lampHeadPopulate()](\ref lampHeadPopulate()) before being passed to this function.
	\param[in]		data    	Buffer containing the payload.
	\param[in]		payloadsize	Size, in _bytes_, of the payload.

	\return None.
**/
void lampEncapsulate(byte_t *packet, struct lamphdr *lampHeader, byte_t *data, size_t payloadsize) {
	lampHeader->len=htons(payloadsize);

	memcpy(packet,lampHeader,sizeof(struct lamphdr));
	memcpy(packet+sizeof(struct lamphdr),data,payloadsize);
}

/**
	\brief Send LaMP packet over a raw socket, automatically setting some fields such as the timestamp (when needed)

	This function can be used to send a LaMP packet over a raw socket, automating some operations such as the 
	insertion of the timestamp, which is performed as the last possible operation before sending.

	<b>It replaces any call to <i>sendto()</i></b>.

	This function allows the user to specify a certain protocol, which will be used to properly place the timestamp, when needed,
	inside the full packet. All the protocols defined inside [protocol_t](\ref protocol_t) are supported by this function.

	\warning This function can only be used when sending LaMP data over **raw** sockets. That's why the pointer to the full packet to be sent is called _ethernetpacket_.

	\param[in] 	descriptor 				Socket descriptor related to the raw socket to be used to send the packet.
	\param[in] 	addrll 					Socket address structure (*struct sockaddr_ll*, i.e. the same structure you would pass to a call to <i>sendto()</i>).
	\param[in]  inpacket_headerptr 		Pointer to the LaMP header **inside** the full packet, passed as _ethernetpacket_ (a future improvement will remove the necessity of passing this pointer).
	\param[in] 	ethernetpacket 			Pointer to the buffer storing the **whole** packet to be sent (i.e. the same buffer you would pass to a call to <i>sendto()</i>).
	\param[in] 	finalpacketsize 		Size of the whole packet (i.e. the same size you would pass to a call to <i>sendto()</i>).
	\param[in] 	end_flag 				End flag value: see [endflag_t](\ref endflag_t).
	\param[in] 	llprot 					Protocol type, using the [protocol_t](\ref protocol_t) definition inside rawsock.h.

	\return It returns **1** if the packet was successfully sent, **0** otherwise.
**/
int rawLampSend(int descriptor, struct sockaddr_ll addrll, struct lamphdr *inpacket_headerptr, byte_t *ethernetpacket, size_t finalpacketsize, endflag_t end_flag, protocol_t llprot) {
	struct timeval currtime;
	struct udphdr *inpacket_headerptr_udp;
	struct iphdr *inpacket_headerptr_ipv4;
	size_t packetsize;

	if(IS_UNIDIR(inpacket_headerptr->ctrl) && end_flag==FLG_STOP) {
		inpacket_headerptr->ctrl=CTRL_UNIDIR_STOP;
	} else if(IS_PINGLIKE(inpacket_headerptr->ctrl) && end_flag==FLG_STOP) {
		if(inpacket_headerptr->ctrl==CTRL_PINGLIKE_REQ) {
			inpacket_headerptr->ctrl=CTRL_PINGLIKE_ENDREQ;
		} else if(inpacket_headerptr->ctrl==CTRL_PINGLIKE_REQ_TLESS) {
			inpacket_headerptr->ctrl=CTRL_PINGLIKE_ENDREQ_TLESS;
		}
	}

	if(IS_UNIDIR(inpacket_headerptr->ctrl) || inpacket_headerptr->ctrl==CTRL_PINGLIKE_REQ || inpacket_headerptr->ctrl==CTRL_PINGLIKE_ENDREQ) {
		gettimeofday(&currtime,NULL); // Set timestamp as very last operation, only if it is not a ping-like reply

		inpacket_headerptr->sec=hton64((uint64_t) currtime.tv_sec);
		inpacket_headerptr->usec=hton64((uint64_t) currtime.tv_usec);
	}

	// Compute again the checksum depending on the lower layer protocol (UDP is supported as of now)
	switch(llprot) {
		case UDP:
			// Try to obtain the UDP header pointer by subtracting a certain offset to the LaMP inpacket_headerptr
			inpacket_headerptr_udp=(struct udphdr *) ((byte_t *)inpacket_headerptr-sizeof(struct udphdr));
			// Try to obtain the IPv4 header pointer by subtracting a certain offset to the LaMP inpacket_headerptr
			inpacket_headerptr_ipv4=(struct iphdr *) ((byte_t *)inpacket_headerptr_udp-sizeof(struct iphdr));

			inpacket_headerptr_udp->check=0;
			if(IS_INIT(inpacket_headerptr->ctrl) || IS_FOLLOWUP_CTRL(inpacket_headerptr->ctrl)) {
				packetsize=sizeof(struct udphdr)+LAMP_HDR_SIZE();
			} else {
				packetsize=sizeof(struct udphdr)+LAMP_HDR_PAYLOAD_SIZE(ntohs(inpacket_headerptr->len));
			}

			inpacket_headerptr_udp->check=minirighi_udp_checksum(inpacket_headerptr_udp,packetsize,inpacket_headerptr_ipv4->saddr,inpacket_headerptr_ipv4->daddr);
		break;
		default: // case UNSET_P -> do nothing
		break;
	}

	return (sendto(descriptor,ethernetpacket,finalpacketsize,0,(struct sockaddr *)&addrll,sizeof(struct sockaddr_ll))!=finalpacketsize);
}

/**
	\brief Extract relevant data from a LaMP packet

	This function can be used to extract all the relevant data, already converted to host byte order, from a LaMP packet.

	The function takes as input the pointer to the LaMP packet (i.e. to a memory area where a _struct lamphdr_, **plus the payload**, is stored).

	\param[in]		lampPacket 		Pointer to the LaMP packet buffer (with type [byte_t](\ref byte_t)).
	\param[out]		id 				Identification field (LaMP ID) value.
	\param[out]		seq 			Current sequence number inside the header.
	\param[out]		len  			Value stored inside the "length or INIT type" field.
	\param[out]     timestamp 		_struct timeval_ which is filled using the timestamp stored inside the LaMP header.
	\param[out]		payload 		If this pointer is non-NULL, it should be related to a memory area big enough to contain a possible LaMP payload. Then, the function will copy the payload contained inside the LaMP packet buffer to that memory area. Il the length field is **0** (or NULL is specified), no copy operation will be performed.

	\return None.
**/
void lampHeadGetData(byte_t *lampPacket, lamptype_t *type, unsigned short *id, unsigned short *seq, unsigned short *len, struct timeval *timestamp, byte_t *payload) {
	struct lamphdr *lampHeader=(struct lamphdr *) lampPacket;
	byte_t *payloadptr=(lampPacket+LAMP_HDR_SIZE());

	if(type) *type=(lampHeader->ctrl) & 0x0F;
	if(id) *id=ntohs(lampHeader->id);
	if(seq) *seq=ntohs(lampHeader->seq);
	if(len) *len=ntohs(lampHeader->len);
	if(timestamp) {
		timestamp->tv_sec=(time_t) ntoh64(lampHeader->sec);
		timestamp->tv_usec=(suseconds_t) ntoh64(lampHeader->usec);
	}

	if(payload && lampHeader->len!=0x00) {
		memcpy(payload,payloadptr,lampHeader->len);
	}
}

/**
	\brief Get pointers to header and payload in a LaMP packet

	This function can be used to obtain, given a certain buffer containing a full LaMP packet, including its payload,
	the pointers to the header (i.e. to the [struct lamphdr](\ref lamphdr) and payload sections 

	Example of call: 

		payload=lampGetPacketPointers(packet,&lampHeader);

	with:

		struct lamphdr *lampHeader;
		byte_t *payload;

	\warning No memory is allocated by this function! It will return pointers inside the original _pktbuf_ buffer, by doing the proper arithmetics.

	\warning **Never use this function on a LaMP packet without a payload.** It will work, but the returned payload pointer will point to a memory area which is just
	after the one storing the _struct lamphdr_, and that, if no payload was received, is not belonging to the application callling lampGetPacketPointers(), possibly causing 
	a segmentation fault or an undefined behaviour when trying to access or write it.
	
	\param[in] 	pktbuf 		Packet buffer, containing a full valid LaMP packet (the checksum or even the reserved field can be wrong, "valid" means here "that can be LaMP").
	\param[out]	lampHeader  This pointer will be written by the function, storing the pointer to the LaMP header inside the _pktbuf_ packet buffer.
	
	\return The pointer to the payload (of type [byte_t](\ref byte_t)) is returned by the function. If _pktbuf_ is a valid pointer, it should never happen that the returned pointer is NULL.
**/
byte_t *lampGetPacketPointers(byte_t *pktbuf,struct lamphdr **lampHeader) {
	byte_t *payload;

	*lampHeader=(struct lamphdr*) pktbuf;
	payload=pktbuf+sizeof(struct lamphdr);

	return payload;
}