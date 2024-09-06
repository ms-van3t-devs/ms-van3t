// Rawsock library, licensed under GPLv2
// Version 0.3.4
#include "minirighi_udp_checksum.h"

/**
	\brief Calculate the UDP checksum (calculated with the whole packet) 

	\note This documentation is coded to reflect exactly the one provided
	for the Minirighi Opearing System, available [here](http://minirighi.sourceforge.net/html/udp_8c.html).

	\param[in]	buff 		The UDP packet.
	\param[in] 	len 		The UDP packet length. 
	\param[in] 	src_addr  	The IP source address (in **network** format).
	\param[in] 	dest_addr  	The IP destination address (in **network** format).

	\return The result of the checksum.
**/
uint16_t minirighi_udp_checksum(const void *buff, size_t len, in_addr_t src_addr, in_addr_t dest_addr) {
	const uint16_t *buf=buff;
	uint16_t *ip_src=(void *)&src_addr, *ip_dst=(void *)&dest_addr;
	uint32_t sum;
	size_t length=len;

	sum = 0;
	while (len > 1) {
		sum += *buf++;
		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}

	if ( len & 1 )
		sum += *((uint8_t *)buf);

	sum += *(ip_src++);
	sum += *ip_src;
	sum += *(ip_dst++);
	sum += *ip_dst;

	sum += htons(IPPROTO_UDP);
	sum += htons(length);

	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ( (uint16_t)(~sum)  );
}