/** \file 
	IP checksum calculation alternative header - from Linux kernel 4.19.1 (not an original work)

	This header file gives access to a function (ip_fast_csum()) that can be used to
	compute, in an efficient and reliable way, the IPv4 checksum to be put inside the corresponding field of the IPv4 header.

	It represents an alternative user-space header to access the Linux kernel ip_fast_csum() function.

	This is not an original work: this function comes from the [Linux kernel 4.19.1](https://elixir.bootlin.com/linux/v4.19.1/source/lib/checksum.c#L110), 
	released under GNU GPL version 2. There is no modification to its code.

	The function, even though it is used internally in the main Rawsock module, is available through a separate
	header in order to enable any application to use it separately in user space, when needed.

	\version Rawsock library verion: 0.3.4; Linux kernel version: 4.19.1
	\author Jorge Cwik, <jorge@laser.satlink.net>
	\author Arnt Gulbrandsen, <agulbra@nvg.unit.no>
	\author Tom May, <ftom@netcom.com>
	\author Andreas Schwab, <schwab@issan.informatik.uni-dortmund.de>
	\author Lots of code moved from tcp.c and ip.c; see those files, inside the Linux kernel code, for more names.
	\copyright This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
**/

#ifndef IPCSUM_ALTH_INCLUDED
#define IPCSUM_ALTH_INCLUDED

#include <linux/types.h>

// This is all taken from Linux kernel 4.19.1 (this is not original work)
__sum16 ip_fast_csum(const void *iph, unsigned int ihl);

#endif
