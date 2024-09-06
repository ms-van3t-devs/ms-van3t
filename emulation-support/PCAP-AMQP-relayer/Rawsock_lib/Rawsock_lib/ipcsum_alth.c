// Rawsock library, licensed under GPLv2
// Version 0.3.4
#include "ipcsum_alth.h"

// This is all taken from Linux kernel 4.19.1 (this is not original work)
static inline unsigned short from32to16(unsigned int x) {
	/* add up 16-bit and 16-bit for 16+c bit */
	x = (x & 0xffff) + (x >> 16);
	/* add up carry.. */
	x = (x & 0xffff) + (x >> 16);
	return x;
}

// This is all taken from Linux kernel 4.19.1 (this is not original work)
static unsigned int do_csum(const unsigned char *buff, int len) {
	int odd;
	unsigned int result = 0;

	if (len <= 0)
		goto out;
	odd = 1 & (unsigned long) buff;
	if (odd) {
#ifdef __LITTLE_ENDIAN
		result += (*buff << 8);
#else
		result = *buff;
#endif
		len--;
		buff++;
	}
	if (len >= 2) {
		if (2 & (unsigned long) buff) {
			result += *(unsigned short *) buff;
			len -= 2;
			buff += 2;
		}
		if (len >= 4) {
			const unsigned char *end = buff + ((unsigned)len & ~3);
			unsigned int carry = 0;
			do {
				unsigned int w = *(unsigned int *) buff;
				buff += 4;
				result += carry;
				result += w;
				carry = (w > result);
			} while (buff < end);
			result += carry;
			result = (result & 0xffff) + (result >> 16);
		}
		if (len & 2) {
			result += *(unsigned short *) buff;
			buff += 2;
		}
	}
	if (len & 1)
#ifdef __LITTLE_ENDIAN
		result += *buff;
#else
		result += (*buff << 8);
#endif
	result = from32to16(result);
	if (odd)
		result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
out:
	return result;
}

/**
	\brief Calculate the IPv4 checksum (optimized for IP headers, which always checksum on 4 octet boundaries) 

	This function can be used to compute the IPv4 header checksum, given the whole header and the IHL field.

	__Example of use:__

		struct iphdr header;

		header.check=ip_fast_csum((__u8 *)&header, 5); // IHL = 5 word -> no options

	\param[in]	iph 		Pointer to the IPv4 header.
	\param[in] 	ihl 		Value of the IPv4 IHL (_Internet Header Length_).

	\return The result of the checksum calculation, ready to be inserted inside the _check_ field of the _struct iphdr_.
**/
__sum16 ip_fast_csum(const void *iph, unsigned int ihl)
{
	return (__sum16)~do_csum(iph,ihl*4);
}