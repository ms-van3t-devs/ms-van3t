/** \file 
	UDP checksum calculation utility - from the Minirighi IA-32 Operating System

	This header file gives access to a function (minirighi_udp_checksum()) that can be used to
	compute the UDP checksum to be put inside the corresponding field of the UDP header.

	This is not an original work: this function comes from the [Minirighi IA-32 Operating System](http://minirighi.sourceforge.net/html/udp_8c.html), 
	released under GNU GPL. Except for the name, there is no modification to its code.

	Moreover, the original definition can be found at line 29 of file **udp.c** of the Minirighi system.

	The function, even though it is used internally in the main Rawsock module, is available through a separate
	header in order to enable any application to use it separately, when needed.

	\version Rawsock library verion: 0.3.4
	\author Andrea Righi <drizzt@inwind.it>
	\date 2003-01-21 Andrea Righi: Suppressed deferencing type-punned pointer warning.
	\copyright Licensed under GPLv2, (c) 2003 Andrea Righi
**/

#ifndef MINIRIGHI_UDP_CHECKSUM_H_INCLUDED
#define MINIRIGHI_UDP_CHECKSUM_H_INCLUDED
	
#include <inttypes.h>
#include <stdlib.h>
#include <netinet/in.h>

uint16_t minirighi_udp_checksum(const void *buff, size_t len, in_addr_t src_addr, in_addr_t dest_addr);

#endif
