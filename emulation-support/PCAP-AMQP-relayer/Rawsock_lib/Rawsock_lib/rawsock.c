// Rawsock library, licensed under GPLv2
// Version 0.3.4
#include "rawsock.h"
#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>
#include <linux/wireless.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include "ipcsum_alth.h"
#include "minirighi_udp_checksum.h"

static uint64_t swap64(uint64_t unsignedvalue, uint32_t (*swap_byte_order)(uint32_t)) {
	#if __BYTE_ORDER == __BIG_ENDIAN
	return hostu64;
	#elif __BYTE_ORDER == __LITTLE_ENDIAN
	uint32_t low;
	uint32_t high;
	high=(uint32_t) (unsignedvalue>>32);
	low=(uint32_t) (unsignedvalue & ((1ULL << 32) - 1));
	return ((uint64_t) swap_byte_order(low))<<32 | ((uint64_t) swap_byte_order(high));
	#else
	#error "The system seems to be neither little endian nor big endian..." 
	#endif
}

static uint8_t is_if_tun(int sFd,char *devname) {
	struct ethtool_drvinfo drvinfo;
	struct ifreq ethtool_ifr;

	drvinfo.cmd=ETHTOOL_GDRVINFO;
	strncpy(ethtool_ifr.ifr_name,devname,IFNAMSIZ);
	ethtool_ifr.ifr_data=(void *)&drvinfo;

	return ioctl(sFd,SIOCETHTOOL,&ethtool_ifr)!=-1 && strncmp(drvinfo.bus_info,"tun",ETHTOOL_BUSINFO_LEN)==0;
}

/**
	\brief Prepare a *macaddr_t* variable

	Allocates a six element byte array to store any
	MAC address, using the *macaddr_t* custom type.

	\warning Every MAC address allocated with this function
	is initialized to FF:FF:FF:FF:FF:FF.

	\return It returns the pointer to the allocated MAC address, in the form of a [macaddr_t](\ref macaddr_t) custom type. [MAC_NULL](\ref MAC_NULL) is returned if it was not possibile to allocate the required memory.
**/
macaddr_t prepareMacAddrT() {
	macaddr_t mac;
	int i;

	mac=malloc(MAC_ADDR_SIZE*sizeof(uint8_t));

	if(mac) {
		for(i=0;i<MAC_ADDR_SIZE;i++) {
			mac[i]=0xFF;
		}
	}

	return mac;
}

/**
	\brief Free a *macaddr_t* variable

	Frees a previously allocated MAC address variable (*macaddr_t*).

	\param[in] 	mac 	A previously allocated [macaddr_t](\ref macaddr_t) variable/pointer.

	\return None.
**/
void freeMacAddrT(macaddr_t mac) {
	free(mac);
}

/**
	\brief Get the MAC address type

	Starting from a MAC address type,
	its type is returned (unicast, multicast, broadcast).

	\warning As soon as a new MAC address is allocated, its type
	is set to the equivalent of *MAC_NULL* (i.e. a *NULL* pointer). You can use 
	this to check if an address was properly allocated, by calling `if(macAddrTypeGet(mac)!=MAC_NULL)`.

	\param[in]	mac 	A previously allocated [macaddr_t](\ref macaddr_t) variable/pointer.

	\return The MAC address tupe is returned: either [MAC_NULL](\ref MAC_NULL), [MAC_UNICAST](\ref MAC_UNICAST), [MAC_MULTICAST](\ref MAC_MULTICAST), [MAC_BROADCAST](\ref MAC_BROADCAST) or [MAC_ZERO](\ref MAC_ZERO). They are all threated as _unsigned int_.
**/
unsigned int macAddrTypeGet(macaddr_t mac) {
	if(mac!=NULL) {
		if(mac[0]==0x01) {
			return MAC_MULTICAST;
		} else if(mac[0]==0xFF && mac[1]==0xFF && mac[2]==0xFF && mac[3]==0xFF && mac[4]==0xFF && mac[5]==0xFF) {
			return MAC_BROADCAST;
		} else if(mac[0]==0x00 && mac[1]==0x00 && mac[2]==0x00 && mac[3]==0x00 && mac[4]==0x00 && mac[5]==0x00) {
			return MAC_ZERO;
		} else {
			return MAC_UNICAST;
		}
	} else {
		return MAC_NULL;
	}
}

/**
	\brief Automatically look for available WLAN, non-WLAN or loopback interfaces.
	
	This function can be used to automatically look for available and ready WLAN or non-WLAN (depending on _mode_)
	interfaces in the system.

	From version 0.2.0 it is also possible to specify [WLANLOOKUP_LOOPBACK](\ref WLANLOOKUP_LOOPBACK) as _index_
	to look for the first available lookback interface, instead of WLAN/non-WLAN ones.

	When only one interface is available and `0` is specified as index, 
	that interface name is returned inside `devname`. 
	Then, if the other three arguments are not NULL, the interface index,
	the corresponding source MAC address (if available) and
	the corresponding source IP address (if available) are respectively returned.

	If more than one interface is present, the number of available interfaces of the specified type (WLAN/non-WLAN)
	is returned by the function and _index_ is used to point to a specific interface 
	(for instance `index=1` can be used to point to a possible `wlan1` interface when _mode_ is [WLANLOOKUP_WLAN](\ref WLANLOOKUP_WLAN)).

	To print the available indeces, the user can use vifPrinter().

	\param[out] 	devname 	Name of the WLAN interface.
	\param[out]		ifindex 	Interface index corresponding to _devname_ (filled in only if non-NULL).
	\param[out]		mac     	Interface (source) MAC address (filled in only if non-NULL / non-[MAC_NULL](\ref MAC_NULL)).
	\param[out]		srcIP		Interface (source) IPv4 address (filled in only if non-NULL and returned inside a _struct in_addr_, which should be available in the calling module).
	\param[in]		index       Integer index used to point to a specific interface of the specified type (i.e. using the specified _mode_), when more than one is available (or equal to [WLANLOOKUP_LOOPBACK](\ref WLANLOOKUP_LOOPBACK) to look for the first available loopback interface).
	\param[in] 		mode 		Operating mode: [WLANLOOKUP_WLAN](\ref WLANLOOKUP_WLAN) to look for available WLAN interfaces only, [WLANLOOKUP_NONWLAN](\ref WLANLOOKUP_NONWLAN) to look for available non-WLAN/Ethernet interfaces only

	\return Number of WLAN/non-WLAN interfaces that were found, <b> > 0 </b>, or, in case of error, a [rawsockerr_t](\ref rawsockerr_t) error:
	- *ERR_WLAN_NOIF* -> no interfaces found
	- *ERR_WLAN_SOCK* -> cannot create socket to look for available and running interfaces
	- *ERR_WLAN_GETIFADDRS* -> error in calling `getifaddrs()`
	- *ERR_WLAN_INDEX* -> invalid index value
	- *ERR_WLAN_GETSRCMAC* -> unable to get source MAC address (if requested)
	- *ERR_WLAN_GETIFINDEX* -> unable to get source interface index (if requested)
	- *ERR_WLAN_GETSRCMAC* -> unable to get source IP address (if requested)
**/
rawsockerr_t wlanLookup(char *devname, int *ifindex, macaddr_t mac, struct in_addr *srcIP, int index, int mode) {
	// Variables for wlan interfaces detection
	int sFd=-1;
	// struct ifaddrs used to look for available interfaces and bind to a wireless interface
	struct ifaddrs *ifaddr_head, *ifaddr_it;
	// struct ifreq to check whether an interface is wireless or not. The ifr_name field is used to specify which device to affect.
	struct ifreq wifireq;
	// Pointers to manage the list containing all valid wireless interfaces
	struct iflist *iflist_head=NULL; // Head
	struct iflist *curr_ptr=NULL; // Current element
	struct iflist *iflist_it=NULL; // To iterate the list
	struct iflist *iflist_u=NULL; // To free the list
	int ifno=0;
	int ioctl_result=0; // Result of the call to ioctl(sFd,SIOCGIWNAME,&wifireq)
	int return_value=1; // Return value: >0 ok - # of found interfaces, <=0 error 
								 // (=0 for no WLAN interfaces, =-1 for socket error, =-2 for getifaddrs error)
							     // (=-3 for wrong index, =-4 unable to get MAC address, =-5 unable to get ifindex)

	// Linked list nodes to store the WLAN interfaces
	struct iflist {
		struct iflist *next;
		struct ifaddrs *ifaddr_ptr;
	};

	// Open socket (needed)
	sFd=socket(AF_INET,SOCK_DGRAM,0); // Any socket should be fine (to be better investigated!)
	if(sFd==-1) {
		return_value=ERR_WLAN_SOCK;
		goto sock_error_occurred;
	}

	// Getting all interface addresses
	if(getifaddrs(&ifaddr_head)==-1) {
		return_value=ERR_WLAN_GETIFADDRS;
		goto getifaddrs_error_occurred;
	}

	// Looking for wlan interfaces
	memset(&wifireq,0,sizeof(wifireq));
	// Iterating over the interfaces linked list
	for(ifaddr_it=ifaddr_head;ifaddr_it!=NULL;ifaddr_it=ifaddr_it->ifa_next) {
		if(index==-1 && ifaddr_it->ifa_addr!=NULL && (ifaddr_it->ifa_flags & IFF_LOOPBACK)) {
			// This is the first loopback interface
			strncpy(devname,ifaddr_it->ifa_name,IFNAMSIZ);

			// One loopback interface is returned
			return_value=1;

			break;
		} else {
			if(ifaddr_it->ifa_addr!=NULL && (ifaddr_it->ifa_addr->sa_family == AF_PACKET || (ifaddr_it->ifa_addr->sa_family == AF_INET && is_if_tun(sFd,ifaddr_it->ifa_name)))) {
				// fprintf(stdout,"Checking interface %s for use.\n",ifaddr_it->ifa_name);
				// IFNAMSIZ is defined by system libraries and it "defines the maximum buffer size needed to hold an interface name, 
				//  including its terminating zero byte"
				// This is done because (from man7.org) "normally, the user specifies which device to affect by setting
	            //  ifr_name to the name of the interface"
				strncpy(wifireq.ifr_name,ifaddr_it->ifa_name,IFNAMSIZ);

				// Trying to get the Wireless Extensions (a socket descriptor must be specified to ioctl())
				ioctl_result=ioctl(sFd,SIOCGIWNAME,&wifireq);

				// Current interface should be considered
				if((ioctl_result!=-1 && mode==WLANLOOKUP_WLAN) || (ioctl_result==-1 && mode==WLANLOOKUP_NONWLAN)) {
					if(ifaddr_it->ifa_addr!=NULL && (ifaddr_it->ifa_flags & IFF_UP) && !(ifaddr_it->ifa_flags & IFF_LOOPBACK)) {
						// If the interface is up, add it to the head of the "iflist" (it is not added to the tail in order to
						//  avoid defining an extra pointer)
						ifno++;
						curr_ptr=malloc(sizeof(struct iflist));
						curr_ptr->ifaddr_ptr=ifaddr_it;
						if(iflist_head==NULL) {
							iflist_head=curr_ptr;
							iflist_head->next=NULL;
						} else {
							curr_ptr->next=iflist_head;
							iflist_head=curr_ptr;
						}
					}
				}
			}
		}
	}

	// Scan the list of returned interfaces only if lookback was not specified
	if(index!=-1) {
		// No wireless interfaces found (the list is empty)
		if(iflist_head==NULL) {
			return_value=0;
			goto error_occurred;
		} else if(ifno==1) {
			// Only one wireless interface found
			if(index>=1) {
				return_value=ERR_WLAN_INDEX;
				goto error_occurred;
			}
			strncpy(devname,iflist_head->ifaddr_ptr->ifa_name,IFNAMSIZ);
		} else {
			// Multiple interfaces found -> use the value of 'index'
			if(index>=ifno) {
				return_value=ERR_WLAN_INDEX;
				goto error_occurred;
			}
			return_value=ifno; // Return the number of interfaces found
			// Iterate the list until the chosen interface is reached
			iflist_it=iflist_head;
			while(index<=ifno-2) {
				iflist_it=iflist_it->next;
				index++;
			}
			strncpy(devname,iflist_it->ifaddr_ptr->ifa_name,IFNAMSIZ);
		}
	}

	// Get MAC address of the interface (if requested by the user with a non-NULL mac)
	if(mac!=NULL) {
		strncpy(wifireq.ifr_name,devname,IFNAMSIZ); 
		if(ioctl(sFd,SIOCGIFHWADDR,&wifireq)!=-1) {
			memcpy(mac,wifireq.ifr_hwaddr.sa_data,MAC_ADDR_SIZE);
		} else {
			return_value=ERR_WLAN_GETSRCMAC;
			goto error_occurred;
		}
	}

	// Get interface index of the interface (if requested by the user with a non-NULL ifindex)
	if(ifindex!=NULL) {
		strncpy(wifireq.ifr_name,devname,IFNAMSIZ);
		if(ioctl(sFd,SIOCGIFINDEX,&wifireq)!=-1) {
			*ifindex=wifireq.ifr_ifindex;
		} else {
			return_value=ERR_WLAN_GETIFINDEX;
			goto error_occurred;
		}
	}

	// Get the IP address of the interface (if requested by the user with a non-NULL struct in_addr)
	if(srcIP!=NULL) {
		strncpy(wifireq.ifr_name,devname,IFNAMSIZ);
		wifireq.ifr_addr.sa_family = AF_INET;
		if(ioctl(sFd,SIOCGIFADDR,&wifireq)!=-1) {
			*srcIP=((struct sockaddr_in*)&wifireq.ifr_addr)->sin_addr;
		} else {
			return_value=ERR_WLAN_GETSRCIP;
			// No need to go to error_occurred, as we are already there
		}
	}

	error_occurred:
	// iflist and the other list are no more useful -> free them
	freeifaddrs(ifaddr_head);
	for(iflist_it=iflist_head;iflist_it!=NULL;iflist_it=iflist_u) {
		iflist_u=iflist_it->next;
		free(iflist_it);
	}

	getifaddrs_error_occurred:
	// Close socket
	close(sFd);

	sock_error_occurred:
	return return_value;
}

/**
	\brief Print information about available interfaces
	
	This function can be used to automatically print some information
	about the interfaces which are up and available in the system.

	The **"Interface internal index"** value can be used as a reference
	for the _index_ value to be inserted in wlanLookup(), as last argument,
	when more than one wireless interface is available on the system.

	\param[out] 	stream 		File stream to print to (a file, _stdout_ or _stderr_)

	\return **0** if no error occurred, or, in case of error, a [rawsockerr_t](\ref rawsockerr_t) error:
	- *ERR_VIFPRINTER_SOCK* -> cannot create socket to look for available interfaces
	- *ERR_VIFPRINTER_GETIFADDRS* -> error in calling `getifaddrs()`
**/
rawsockerr_t vifPrinter(FILE *stream) {
	int sFd=-1;
	struct ifaddrs *ifaddr_head, *ifaddr_it;
	struct ifreq wifireq;
	int wlan_ifno=0;
	int nonwlan_ifno=0;

	// Open socket (needed)
	sFd=socket(AF_INET,SOCK_DGRAM,0); // Any socket should be fine (to be better investigated!)
	if(sFd==-1) {
		return ERR_VIFPRINTER_SOCK;
	}

	// Getting all interface addresses
	if(getifaddrs(&ifaddr_head)==-1) {
		close(sFd);
		return ERR_VIFPRINTER_GETIFADDRS;
	}

	fprintf(stream,"Interface name   | Interface type | Interface internal index\n"
		 "--------------   | -------------- | ------------------------\n");

	// Looking for wlan interfaces
	memset(&wifireq,0,sizeof(wifireq));
	// Iterating over the interfaces linked list
	for(ifaddr_it=ifaddr_head;ifaddr_it!=NULL;ifaddr_it=ifaddr_it->ifa_next) {
		if(ifaddr_it->ifa_addr!=NULL && (ifaddr_it->ifa_flags & IFF_LOOPBACK) && ifaddr_it->ifa_addr->sa_family==AF_PACKET) {
			fprintf(stream,"%s\t\t | %-14s | %s\t\n",ifaddr_it->ifa_name,"Loopback","(lo)");
		} else {
			if(ifaddr_it->ifa_addr!=NULL && ifaddr_it->ifa_addr->sa_family==AF_PACKET) {
				strncpy(wifireq.ifr_name,ifaddr_it->ifa_name,IFNAMSIZ); 
				if(ioctl(sFd,SIOCGIWNAME,&wifireq)!=-1) {
					if(ioctl(sFd,SIOCGIFFLAGS,&wifireq)!=-1 && (wifireq.ifr_flags & IFF_UP)) {
						// If the interface is up, print the information related to such interface
						fprintf(stream,"%-*s | %-14s | (wlan) %d\t\n",IFNAMSIZ,ifaddr_it->ifa_name,"Wireless",wlan_ifno);
						wlan_ifno++;
					}
				} else {
					// Interface is not wireless
					if(ifaddr_it->ifa_addr!=NULL && (ifaddr_it->ifa_flags & IFF_UP)) {
						fprintf(stream,"%-*s | %-14s | (non-wlan) %d\t\n",IFNAMSIZ,ifaddr_it->ifa_name,"Non-wireless",nonwlan_ifno);
						nonwlan_ifno++;
					}
				}
			} else if(ifaddr_it->ifa_addr!=NULL && ifaddr_it->ifa_addr->sa_family==AF_INET && is_if_tun(sFd,ifaddr_it->ifa_name)) {
				fprintf(stream,"%-*s | %-14s | (non-wlan) %d\t\n",IFNAMSIZ,ifaddr_it->ifa_name,"tun (AF_INET)",nonwlan_ifno);
				nonwlan_ifno++;
			}
		}
	}

	return 1;
}

/**
	\brief Print more detailed error messages

	Given a stream (e.g. _stdout_ or _stderr_) and
	the error code, as a [rawsockerr_t](\ref rawsockerr_t) type,
	more detailed information is printed on the selected stream.

	This function essentialy works like _perror()_, but instead of describing
	the current value of _errno_, it describes what is stored inside _code_.

	It can be useful to print any error that may occur after a call to wlanLookup()
	or other compatible library functions.

	\param[in]	stream	_FILE_ stream to be used to print the requested error (it can be a file, _stdout_ or _stderr_).
	\param[in]	code    [rawsockerr_t](\ref rawsockerr_t) integer variable containing the error code (returned by another library function).

	\return None.
**/
void rs_printerror(FILE *stream,rawsockerr_t code) {
	switch(code) {
		case ERR_WLAN_NOIF:
			fprintf(stream,"wlanLookup: No interfaces found.\n");
		break;

		case ERR_WLAN_SOCK:
			fprintf(stream,"wlanLookup: socket creation error.\n");
		break;

		case ERR_WLAN_GETIFADDRS:
			fprintf(stream,"wlanLookup: getifaddrs() error.\n");
		break;

		case ERR_WLAN_INDEX:
			fprintf(stream,"wlanLookup: wrong index specified.\n");
		break;

		case ERR_WLAN_GETSRCMAC:
			fprintf(stream,"wlanLookup: unable to get source MAC address.\n");
		break;

		case ERR_WLAN_GETIFINDEX:
			fprintf(stream,"wlanLookup: unable to get interface index.\n");
		break;

		case ERR_WLAN_GETSRCIP:
			fprintf(stream,"wlanLookup: unable to get source IP address.\n");
		break;

		case ERR_IPHEAD_SOCK:
			fprintf(stream,"IP4headPopulateB: socket creation error.\n");
		break;

		case ERR_IPHEAD_NOSRCADDR:
			fprintf(stream,"IP4headPopulateB: unable to retrieve source IP address.\n");
		break;

		default:
			fprintf(stream,"No error.\n");
	}
}

/**
	\brief Display packet in hexadecimal form

	This function can be used to display a packet content in hexadecimal form,
	given the pointer to a buffer of [bytes](\ref byte_t) and its length (_len_).

	_text_ can be used to specify an additional text that will be printer before
	printing the actual packet content.

	\note _stdout_ is always used to print the requested information.

	\param[in]	text 	Additional text preceding the packet content
	\param[in]	packet  Buffer containing the whole packet to be printed
	\param[in]	len     Length, in bytes, of the buffer to be printed (i.e. number of elements of the [bytes](\ref byte_t) buffer to be printed)

	\return None.
**/
void display_packet(const char *text,byte_t *packet,unsigned int len) {
	int i;

	fprintf(stdout,"%s -> ",text);
	for(i=0;i<len;i++) {
		fprintf(stdout,"%02x ",packet[i]);
	}
	fprintf(stdout,"\n");
	fflush(stdout);
}

/**
	\brief Display packet in character form

	This function can be used to display a packet content in character form,
	given the pointer to a buffer of [bytes](\ref byte_t) and its length (_len_).

	_text_ can be used to specify an additional text that will be printer before
	printing the actual packet content.

	\note _stdout_ is always used to print the requested information.

	\param[in]	text 	Additional text preceding the packet content
	\param[in]	packet  Buffer containing the whole packet to be printed
	\param[in]	len     Length, in bytes, of the buffer to be printed (i.e. number of elements of the [bytes](\ref byte_t) buffer to be printed)

	\return None.
**/
void display_packetc(const char *text,byte_t *packet,unsigned int len) {
	int i;

	fprintf(stdout,"%s -> ",text);
	for(i=0;i<len;i++) {
		fprintf(stdout,"%c",packet[i]);
	}
	fprintf(stdout,"\n");
	fflush(stdout);
}

/**
	\brief Convert a 64-bit unsigned value between host and network byte order

	This function works like [htons()](https://linux.die.net/man/3/htons) and [htonl()](https://linux.die.net/man/3/htonl),
	but with 64-bit unsigned integers (i.e. *uint64_t*).

	\note *uint64_t* is defined inside *stdint.h* and *inttypes.h*.

	\note This function has no direct utility in IPv4/UDP packet management,
	but it can be used to manage any custom or non-custom 64 bit data field to be sent over the network.

	\param[in]	hostu64		Host byte order integer to be converted into network byte order

	\return 64-bit unsigned value value converted to network byte order
**/
uint64_t hton64 (uint64_t hostu64) {
	return swap64(hostu64,&htonl);
}

/**
	\brief Convert a 64-bit unsigned value between network and host byte order

	This function works like [ntohs()](https://linux.die.net/man/3/ntohs) and [ntohl()](https://linux.die.net/man/3/ntohl),
	but with 64-bit unsigned integers (i.e. *uint64_t*).

	\note *uint64_t* is defined inside *stdint.h* and *inttypes.h*. 

	\note This function has no direct utility in IPv4/UDP packet management,
	but it can be used to manage any custom or non-custom 64 bit data field to be sent over the network.

	\param[in]	netu64		Network byte order integer to be converted into host byte order

	\return 64-bit unsigned value value converted to host byte order
**/
uint64_t ntoh64 (uint64_t netu64) {
	return swap64(netu64,&ntohl);
}

/**
	\brief Populate broadcast Ethernet header (variant of etherheadPopulate())

	This function can be used to populate an ethernet header (_struct ether_header_)
	for broadcast transmissions (i.e. a broadcast MAC address is automatically inserted).

	The user shall specify the pointer to an already existing *ether_header* structure to be filled in,
	the source MAC and the EtherType (either one type in **net/ethernet.h** or one type in rawsock.h).

	\note Like all the other functions inside this library, it already takes care of byte ordering.

	\warning All the parameters passed to this function should be in **host** byte order, i.e. the "normal" one
	for the application.

	\param[in,out]	etherHeader 	Pointer to the Ethernet header structure, used in raw sockets (for instance, to send data over a <i>ETH_P_ALL</i> socket).
	\param[in]  mac 			[macaddr_t](\ref macaddr_t) variable containing the source MAC address.
	\param[in]  type 			[ethertype_t](\ref ethertype_t) variable containing the EtherType.

	\return None.
**/
void etherheadPopulateB(struct ether_header *etherHeader, macaddr_t mac, ethertype_t type) {
	unsigned char broadcastMAC[ETHER_ADDR_LEN]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	memcpy(etherHeader->ether_dhost,broadcastMAC,ETHER_ADDR_LEN);
	memcpy(etherHeader->ether_shost,mac,ETHER_ADDR_LEN);
	etherHeader->ether_type = htons(type);
}

/**
	\brief Populate Ethernet header

	This function can be used to populate an ethernet header (_struct ether_header_).

	A variant, etherheadPopulateB(), addressed specifically to broadcast transmissions, exists.

	The user shall specify the pointer to an already existing *ether_header* structure to be filled in,
	the source and destination MAC addresses and the EtherType (either one type in **net/ethernet.h** or 
	one type in rawsock.h).

	\note Like all the other functions inside this library, it already takes care of byte ordering.

	\warning All the parameters passed to this function should be in **host** byte order, i.e. the "normal" one
	for the application.

	\param[out]	etherHeader 	Pointer to the Ethernet header structure, used in raw sockets (for instance, to send data over a <i>ETH_P_ALL</i> socket).
	\param[in]  macsrc 			[macaddr_t](\ref macaddr_t) variable containing the source MAC address.
	\param[in]  macdst   		[macaddr_t](\ref macaddr_t) variable containing the destination MAC address.
	\param[in]  type 			[ethertype_t](\ref ethertype_t) variable containing the EtherType.

	\return None.
**/
void etherheadPopulate(struct ether_header *etherHeader, macaddr_t macsrc, macaddr_t macdst, ethertype_t type) {
	memcpy(etherHeader->ether_dhost,macdst,ETHER_ADDR_LEN);
	memcpy(etherHeader->ether_shost,macsrc,ETHER_ADDR_LEN);
	etherHeader->ether_type = htons(type);
}

/**
	\brief Combine Ethernet SDU and PCI
	
	This function combines an Ethernet SDU and PCI, the latter in the form of a *struct ether_header* Linux structure.

	The user shall specify a buffer in which the full packet will be put, the ethernet header, the SDU
	(<i>byte_t *sdu</i>) and its size in bytes.

	\warning The _packet_ buffer should be already allocated, and it should be big enough to contain both the header **and** 
	the SDU. It is possible to use the macros defined in rawsock.h, such as [ETH_IP_UDP_PACKET_SIZE_S(udp_payload_size)](\ref ETH_IP_UDP_PACKET_SIZE_S),
	to find the required size starting from an higher layer payload size and then call a **malloc()** with that size to allocate the memory for _packet_.

	\param[out] 	packet 	Packet buffer (should be already allocated) that will contain the full packet (SDU+PCI).
	\param[in]		header 	Ethernet header, as *struct ether_header*. Should be filled in with [etherheadPopulate*()](\ref etherheadPopulate()) before being passed to this function.
	\param[in]		sdu     Buffer containing the SDU.
	\param[in]		sdusize	Size, in _bytes_, of the SDU.

	\return The full packet size (SDU+PCI), in _bytes_, is returned.
**/
size_t etherEncapsulate(byte_t *packet,struct ether_header *header,byte_t *sdu,size_t sdusize) {
	size_t packetsize=sizeof(struct ether_header)+sdusize;

	memcpy(packet,header,sizeof(struct ether_header));
	memcpy(packet+sizeof(struct ether_header),sdu,sdusize);

	return packetsize;
}

/**
	\brief Retrieve source MAC address field from Ethernet header

	Simple function to retrieve the source MAC address field from a *struct ether_header*,
	and copy it inside the _macsrc_ variable.

	\warning Always be sure to pass a non-NULL _macsrc_ to this function. In any case, if a 
	NULL _macsrc_ is passed, the function simply does nothing.

	\param[in]	etherHeader 	*struct ether_header*, already filled in or extracted from a received packet.
	\param[out] macsrc 			Source MAC address, filled in by the function.
**/
void getSrcMAC(struct ether_header *etherHeader, macaddr_t macsrc) {
	if(macsrc) {
		memcpy(macsrc,etherHeader->ether_shost,ETHER_ADDR_LEN);
	}
}

/**
	\brief Populate IP version 4 header

	This function can be used to populate an Ipv4 header (_struct iphdr_).

	Two variants exist: 
	- IP4headPopulateS(), to specify a *struct in_addr* destination IP address instead of a human-readable string
	- IP4headPopulateB(), to populate an IPv4 header for broadcast transmissions (a broadcast destination IP address is automatically inserted)

	The user shall specify an already existing IP header structure to be filled in, the interface name inside _devname_ (for instance _wlan0_), 
	the destination IP address as a human-readable string (e.g. `"192.168.1.180"`), the TOS value, the fragment offset value, the TTL, 
	the protocol and the IP header flags (reserved, DF, MF).

	If <i>struct ipaddrs *addrs</i> is non-NULL, this function, other than returning possible errors ([ERR_IPHEAD_SOCK](\ref ERR_IPHEAD_SOCK) 
	or [ERR_IPHEAD_NOSRCADDR](\ref ERR_IPHEAD_NOSRCADDR)) as return value, fills the structure pointed by _addrs_ with the destination and source
	IP addresses.

	\note Like all the other functions inside this library, it already takes care of byte ordering.

	\warning All the parameters passed to this function should be in **host** byte order, i.e. the "normal" one
	for the application.

	\warning This function does **not** fill the ID field: an additional call to IP4headAddID() is required if you do not want to send a packet with **0** ID,
	for which, instead, it is not necessary to call IP4headAddID().

	\param[in,out]	IPhead 		Pointer to the IPv4 header structure, used in raw sockets.
	\param[in]  devname 		Name of the interface which will be used to send the packet (it is used to automatically retrieve the source IP address)
	\param[in]  destIP   		Destination IP address, a string containing the address in a human-readable format.
	\param[in]  tos   			Type of Service (ToS) field.
	\param[in]  frag_offset   	IPv4 fragment offset field.
	\param[in]  ttl 	   		Time To Live (TTL) field.
	\param[in]  protocol   		Higher layer protocol field.
	\param[in]  flags 			IPv4 flags (reserved, DF, MF): should be filled in using the macros: [FLAG_NOFRAG_MASK](\ref FLAG_NOFRAG_MASK), [FLAG_RESERVED_MASK](\ref FLAG_RESERVED_MASK) and [FLAG_MOREFRAG_MASK](\ref FLAG_MOREFRAG_MASK).
	\param[out]  addrs 			Pointer to the [ipaddrs](\ref ipaddrs) structure to be filled in with the destination and source IP addresses, or NULL if no structure has to be filled in.

	\return **0** if the header was filled in properly, or, in case of error, a [rawsockerr_t](\ref rawsockerr_t) error:
	- *ERR_IPHEAD_SOCK* -> internal socket creation error: unable to set source IP address inside the header
	- *ERR_IPHEAD_NOSRCADDR* -> cannot retrieve the source IP address to be inserted inside the header
**/
rawsockerr_t IP4headPopulate(struct iphdr *IPhead, char *devname, char *destIP, unsigned char tos,unsigned short frag_offset, unsigned char ttl, unsigned char protocol,unsigned int flags,struct ipaddrs *addrs) {
	struct in_addr destIPAddr;
	int sFd; // To get the current IP address
	struct ifreq wifireq;

	IPhead->ihl=BASIC_IHL;
	IPhead->version=IPV4;
	IPhead->tos=(__u8) tos;
	IPhead->frag_off=htons(frag_offset);
	IPhead->frag_off=(IPhead->frag_off) | flags;
	IPhead->ttl=(__u8) ttl;
	IPhead->protocol=(__u8) protocol;
	inet_pton(AF_INET,destIP,(struct in_addr *)&destIPAddr);
	IPhead->daddr=destIPAddr.s_addr;

	// Get own IP address
	sFd=socket(AF_INET,SOCK_DGRAM,0);
	if(sFd==-1) {
		return ERR_IPHEAD_SOCK;
	}
	strncpy(wifireq.ifr_name,devname,IFNAMSIZ);
	wifireq.ifr_addr.sa_family = AF_INET;
	if(ioctl(sFd,SIOCGIFADDR,&wifireq)!=0) {
		close(sFd);
		return ERR_IPHEAD_NOSRCADDR;
	}
	close(sFd);
	IPhead->saddr=((struct sockaddr_in*)&wifireq.ifr_addr)->sin_addr.s_addr;
	if(addrs!=NULL) {
		addrs->src=IPhead->saddr;
		addrs->dst=IPhead->daddr;
	}

	// Initialize checksum to 0
	IPhead->check=0;

	// Initialize ID to 0
	IPhead->id=0;

	return 0;
}

/**
	\brief Populate IP version 4 header with *struct in_addr* addresses (variant of IP4headPopulate())

	This function can be used to populate an IPv4 header (_struct iphdr_).

	The user shall specify an already existing IP header structure to be filled in, the interface name inside _devname_ (for instance _wlan0_), 
	the destination IP address, stored inside a *struct in_addr*, the TOS value, the fragment offset value, the TTL, 
	the protocol and the IP header flags (reserved, DF, MF).

	If <i>struct ipaddrs *addrs</i> is non-NULL, this function, other than returning possible errors ([ERR_IPHEAD_SOCK](\ref ERR_IPHEAD_SOCK) 
	or [ERR_IPHEAD_NOSRCADDR](\ref ERR_IPHEAD_NOSRCADDR)) as return value, fills the structure pointed by _addrs_ with the destination and source
	IP addresses.

	\note Like all the other functions inside this library, it already takes care of byte ordering.

	\warning All the parameters passed to this function should be in **host** byte order, i.e. the "normal" one
	for the application.

	\warning This function does **not** fill the ID field: an additional call to IP4headAddID() is required if you do not want to send a packet with **0** ID,
	for which, instead, it is not necessary to call IP4headAddID().

	\param[in,out]	IPhead 		 	Pointer to the IPv4 header structure, used in raw sockets.
	\param[in]  devname 		Name of the interface which will be used to send the packet (it is used to automatically retrieve the source IP address)
	\param[in]  destIP   		Destination IP address, stored inside the *s_addr* field of the specified *struct in_addr*.
	\param[in]  tos   			Type of Service (ToS) field.
	\param[in]  frag_offset   	IPv4 fragment offset field.
	\param[in]  ttl 	   		Time To Live (TTL) field.
	\param[in]  protocol   		Higher layer protocol field.
	\param[in]  flags 			IPv4 flags (reserved, DF, MF): should be filled in using the macros: [FLAG_NOFRAG_MASK](\ref FLAG_NOFRAG_MASK), [FLAG_RESERVED_MASK](\ref FLAG_RESERVED_MASK) and [FLAG_MOREFRAG_MASK](\ref FLAG_MOREFRAG_MASK).
	\param[out]  addrs 			Pointer to the [ipaddrs](\ref ipaddrs) structure to be filled in with the destination and source IP addresses, or NULL if no structure has to be filled in.

	\return **0** if the header was filled in properly, or, in case of error, a [rawsockerr_t](\ref rawsockerr_t) error:
	- *ERR_IPHEAD_SOCK* -> internal socket creation error: unable to set source IP address inside the header
	- *ERR_IPHEAD_NOSRCADDR* -> cannot retrieve the source IP address to be inserted inside the header
**/
rawsockerr_t IP4headPopulateS(struct iphdr *IPhead, char *devname, struct in_addr destIP, unsigned char tos,unsigned short frag_offset, unsigned char ttl, unsigned char protocol,unsigned int flags,struct ipaddrs *addrs) {
	int sFd; // To get the current IP address
	struct ifreq wifireq;

	IPhead->ihl=BASIC_IHL;
	IPhead->version=IPV4;
	IPhead->tos=(__u8) tos;
	IPhead->frag_off=htons(frag_offset);
	IPhead->frag_off=(IPhead->frag_off) | flags;
	IPhead->ttl=(__u8) ttl;
	IPhead->protocol=(__u8) protocol;
	IPhead->daddr=destIP.s_addr;

	// Get own IP address
	sFd=socket(AF_INET,SOCK_DGRAM,0);
	if(sFd==-1) {
		return ERR_IPHEAD_SOCK;
	}
	strncpy(wifireq.ifr_name,devname,IFNAMSIZ);
	wifireq.ifr_addr.sa_family = AF_INET;
	if(ioctl(sFd,SIOCGIFADDR,&wifireq)!=0) {
		close(sFd);
		return ERR_IPHEAD_NOSRCADDR;
	}
	close(sFd);
	IPhead->saddr=((struct sockaddr_in*)&wifireq.ifr_addr)->sin_addr.s_addr;
	if(addrs!=NULL) {
		addrs->src=IPhead->saddr;
		addrs->dst=IPhead->daddr;
	}

	// Initialize checksum to 0
	IPhead->check=0;

	// Initialize ID to 0
	IPhead->id=0;

	return 0;
}

/**
	\brief Populate broadcast IP version 4 header (variant of IP4headPopulate())

	This function can be used to populate an IPv4 header (_struct iphdr_) for broadcast transmissions.

	The user shall specify an already existing IP header structure to be filled in, the interface name inside _devname_ (for instance _wlan0_), 
	the TOS value, the fragment offset value, the TTL, the protocol and the IP header flags (reserved, DF, MF).

	A broadcast destination IP address is automatically inserted (i.e. _255.255.255.255_).

	If <i>struct ipaddrs *addrs</i> is non-NULL, this function, other than returning possible errors ([ERR_IPHEAD_SOCK](\ref ERR_IPHEAD_SOCK) 
	or [ERR_IPHEAD_NOSRCADDR](\ref ERR_IPHEAD_NOSRCADDR)) as return value, fills the structure pointed by _addrs_ with the destination and source
	IP addresses.

	\note Like all the other functions inside this library, it already takes care of byte ordering.

	\warning All the parameters passed to this function should be in **host** byte order, i.e. the "normal" one
	for the application.

	\warning This function does **not** fill the ID field: an additional call to IP4headAddID() is required if you do not want to send a packet with **0** ID,
	for which, instead, it is not necessary to call IP4headAddID().

	\param[in,out]	IPhead 		 	Pointer to the IPv4 header structure, used in raw sockets.
	\param[in]  devname 		Name of the interface which will be used to send the packet (it is used to automatically retrieve the source IP address)
	\param[in]  tos   			Type of Service (ToS) field.
	\param[in]  frag_offset   	IPv4 fragment offset field.
	\param[in]  ttl 	   		Time To Live (TTL) field.
	\param[in]  protocol   		Higher layer protocol field.
	\param[in]  flags 			IPv4 flags (reserved, DF, MF): should be filled in using the macros: [FLAG_NOFRAG_MASK](\ref FLAG_NOFRAG_MASK), [FLAG_RESERVED_MASK](\ref FLAG_RESERVED_MASK) and [FLAG_MOREFRAG_MASK](\ref FLAG_MOREFRAG_MASK).
	\param[out] addrs 			Pointer to the [ipaddrs](\ref ipaddrs) structure to be filled in with the destination and source IP addresses, or NULL if no structure has to be filled in.

	\return **0** if the header was filled in properly, or, in case of error, a [rawsockerr_t](\ref rawsockerr_t) error:
	- *ERR_IPHEAD_SOCK* -> internal socket creation error: unable to set source IP address inside the header
	- *ERR_IPHEAD_NOSRCADDR* -> cannot retrieve the source IP address to be inserted inside the header
**/
rawsockerr_t IP4headPopulateB(struct iphdr *IPhead, char *devname,unsigned char tos,unsigned short frag_offset, unsigned char ttl, unsigned char protocol,unsigned int flags,struct ipaddrs *addrs) {
	struct in_addr broadIPAddr;
	int sFd; // To get the current IP address
	struct ifreq wifireq;

	IPhead->ihl=BASIC_IHL;
	IPhead->version=IPV4;
	IPhead->tos=(__u8) tos;
	IPhead->frag_off=htons(frag_offset);
	IPhead->frag_off=(IPhead->frag_off) | flags;
	IPhead->ttl=(__u8) ttl;
	IPhead->protocol=(__u8) protocol;
	inet_pton(AF_INET,"255.255.255.255",(struct in_addr *)&broadIPAddr);
	IPhead->daddr=broadIPAddr.s_addr;

	// Get own IP address
	sFd=socket(AF_INET,SOCK_DGRAM,0);
	if(sFd==-1) {
		return ERR_IPHEAD_SOCK;
	}
	strncpy(wifireq.ifr_name,devname,IFNAMSIZ);
	wifireq.ifr_addr.sa_family = AF_INET;
	if(ioctl(sFd,SIOCGIFADDR,&wifireq)!=0) {
		close(sFd);
		return ERR_IPHEAD_NOSRCADDR;
	}
	close(sFd);
	IPhead->saddr=((struct sockaddr_in*)&wifireq.ifr_addr)->sin_addr.s_addr;
	if(addrs!=NULL) {
		addrs->src=IPhead->saddr;
		addrs->dst=IPhead->daddr;
	}

	// Initialize checksum to 0
	IPhead->check=0;

	// Initialize ID to 0
	IPhead->id=0;

	return 0;
}

/**
	\brief Add ID to a given IPv4 header

	Since it can be useful, after preparing a certain header, to update quite often the IPv4 ID (depending on the application and on the use case, though),
	this function allows to perform the aformentioned operation, inserting a given _id_ inside an already populated IPv4 header (_IPhead_).

	\note Since IP4headPopulate() and its variants only initialize the identification field to `0`, a call to this function is required to set a specific
	ID inside an IPv4 header, before sending any packet over raw sockets.

	\param[in,out]	IPhead 	IPv4 header structure (_struct iphdr_) in which the ID field has to be set.
	\param[in]  	id 		16-bit IP identification value

	\return None.
**/
void IP4headAddID(struct iphdr *IPhead, unsigned short id) {
	IPhead->id=htons(id);
}

/**
	\brief Add Total Length to a given IPv4 header

	This function can be used to force a certain length of the entire packet inside the IPv4 header _Total Length_ field.

	\note This function should be used only if, for any reason, the packet to be sent will not be processed by IP4Encapsulate().
	In all the other cases, when combining the IPv4 SDU and PCI with IP4Encapsulate() the _Total Length_ field should be automatically
	set by the aforementioned function.

	\param[in,out]	IPhead 	IPv4 header structure (_struct iphdr_) in which the _Total Length_ field has to be set.
	\param[in]  	len 	Length, in _bytes_, to be inserted inside the IPv4 header.

	\return None.
**/
void IP4headAddTotLen(struct iphdr *IPhead, unsigned short len) {
	IPhead->tot_len=htons(len);
}

/**
	\brief Combine IPv4 SDU and PCI
	
	This function combines an IPv4 SDU and PCI, the latter in the form of a *struct iphdr* Linux structure.

	The user shall specify a buffer in which the full packet will be put, the IPv4 header, the SDU
	(<i>byte_t *sdu</i>) and its size in bytes.

	\warning The _packet_ buffer should be already allocated, and it should be big enough to contain both the header **and** 
	the SDU. It is possible to use the macros defined in rawsock.h, such as [IP_UDP_PACKET_SIZE_S(udp_payload_size)](\ref IP_UDP_PACKET_SIZE_S), to find the required size starting from an higher layer payload size and then call a **malloc()** with that size to allocate the memory for _packet_.

	\param[out] 	packet 	Packet buffer (should be already allocated) that will contain the full IPv4 packet (SDU+PCI).
	\param[in]		header 	IPv4 header, as *struct iphdr*. Should be filled in with [IP4headPopulate*()](\ref IP4headPopulate()) before being passed to this function.
	\param[in]		sdu     Buffer containing the SDU.
	\param[in]		sdusize	Size, in _bytes_, of the SDU.

	\return The full packet size (SDU+PCI), in _bytes_, is returned.
**/
size_t IP4Encapsulate(byte_t *packet,struct iphdr *header,byte_t *sdu,size_t sdusize) {
	size_t packetsize=sizeof(struct iphdr)+sdusize;

	header->tot_len=htons(packetsize);
	header->check=0; // Reset to 0 in case of subsequent calls

	header->check=ip_fast_csum((__u8 *)header, BASIC_IHL);

	memcpy(packet,header,sizeof(struct iphdr));
	memcpy(packet+sizeof(struct iphdr),sdu,sdusize);

	return packetsize;
}

/**
	\brief Populate UDP header

	This function can be used to populate a UDP header (_struct udphdr_).

	The user shall specify an already existing UDP header structure, the source port
	and the destination port.

	\warning This function will initialize the checksum field to **0**. A subsequent call to UDPencapsulate(), specifying the payload you want to
	insert inside the UDP packet, is needed, since the latter is responsible for properly computing the UDP checksum, which, it may be important to
	recall, takes into account also the header and pseudo-header.

	\note Like all the other functions inside this library, it already takes care of byte ordering.

	\param[in,out]	UDPhead 		Pointer to the UDP header structure, used in raw sockets.
	\param[in]  sourceport 		Source port (16 bit unsigned value)
	\param[in]  destport   		Destination port (16 bit unsigned value)

	\return None.
**/
void UDPheadPopulate(struct udphdr *UDPhead, unsigned short sourceport, unsigned short destport) {
	UDPhead->source=htons(sourceport);
	UDPhead->dest=htons(destport);

	// Initialize checksum to 0
	UDPhead->check=0;
}

/**
	\brief Combine UDP payload and header
	
	This function combines a UDP payload and header, the latter in the form of a *struct udphdr* Linux structure.

	The user shall specify a buffer in which the full packet will be put, the UDP header, the payload
	(<i>byte_t *data</i>), its size in bytes and a [struct ipaddrs](\ref ipaddrs) containing the source and destination IP addresses.
	In particular, the latter is needed to properly compute the UDP checksum.

	\warning The _packet_ buffer should be already allocated, and it should be big enough to contain both the header **and** 
	the payload. It is possible to use the macros defined in rawsock.h, such as [UDP_PACKET_SIZE_S(udp_payload_size)](\ref UDP_PACKET_SIZE_S), 
	to find the required size starting from an higher layer payload size and then call a **malloc()** with that size to allocate the memory for _packet_.

	\param[out] 	packet 		Packet buffer (should be already allocated) that will contain the full IPv4 packet (SDU+PCI).
	\param[in]		header 		IPv4 header, as *struct iphdr*. Should be filled in with [IP4headPopulate*()](\ref IP4headPopulate()) before being passed to this function.
	\param[in]		data    	Buffer containing the data payload.
	\param[in]		payloadsize	Size, in _bytes_, of the payload.

	\return The full packet size (SDU+PCI), in _bytes_, is returned.
**/
size_t UDPencapsulate(byte_t *packet,struct udphdr *header,byte_t *data,size_t payloadsize,struct ipaddrs addrs) {
	size_t packetsize=sizeof(struct udphdr)+payloadsize;

	header->len=htons(packetsize);
	header->check=0; // Reset to 0 in case of subsequent calls

	memcpy(packet,header,sizeof(struct udphdr));
	memcpy(packet+sizeof(struct udphdr),data,payloadsize);

	header->check=minirighi_udp_checksum(packet,packetsize,addrs.src,addrs.dst);

	memcpy(packet,header,sizeof(struct udphdr));

	return packetsize;
}

/**
	\brief Get pointers to headers and payload in UDP packet buffer

	This function can be used to obtain, given a certain buffer containing a full UDP packet, the pointers to the headers 
	(i.e. to the *struct ether_header*, *struct iphdr* and *struct udphdr* structure respectively) and payload sections 

	Example of call: 

		payload=UDPgetpacketpointers(packet,&etherHeader,&IPheader,&udpHeader);

	with:

		struct ether_header *etherHeader;
		struct iphdr *IPheader;
		struct udphdr *udpHeader;
		byte_t *payload;

	If any argument is NULL, no pointer will be returned for that argument.

	\note You can use this function, after receiving a packet, to retrieve header specific data and parse the payload.

	\warning No memory is allocated by this function! It will return pointers inside the original _pktbuf_ buffer, by doing the proper arithmetics.
	
	\param[in] 	pktbuf 		Packet buffer, containing a full valid UDP packet (the checksum can be wrong, "valid" means here "that is really UDP"), including *struct ether_header*.
	\param[out]	etherHeader This pointer will be written by the function, storing the pointer to the ethernet header inside the _pktbuf_ packet buffer; as it is written by the function, a pointer to the pointer shall be passed to UDPgetpacketpointers() (see the example above)
	\param[out]	IPheader    This pointer will be written by the function, storing the pointer to the IPv4 header inside the _pktbuf_ packet buffer.
	\param[out]	UDPheader   This pointer will be written by the function, storing the pointer to the UDP header inside the _pktbuf_ packet buffer.
	
	\return The pointer to the payload (of type [byte_t](\ref byte_t)) is returned by the function. If _pktbuf_ is a valid pointer, it should never happen that the returned pointer is NULL. If _pktbuf_ is NULL, NULL will be returned.
**/
byte_t *UDPgetpacketpointers(byte_t *pktbuf,struct ether_header **etherHeader, struct iphdr **IPheader,struct udphdr **UDPheader) {
	byte_t *payload=NULL;

	if(pktbuf!=NULL) {
		if(etherHeader) *etherHeader=(struct ether_header*) pktbuf;
		if(IPheader) *IPheader=(struct iphdr*)(pktbuf+sizeof(struct ether_header));
		if(UDPheader) *UDPheader=(struct udphdr*)(pktbuf+sizeof(struct ether_header)+sizeof(struct iphdr));
		payload=(pktbuf+sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr));
	} else {
		payload=NULL;
	}

	return payload;
}

/**
	\brief Get UDP payload size, given a UDP header

	This function can be used to obtain the UDP payload size as *unsigned short (int)*, given a UDP header pointer.

	It basically extracts the length field and substracts the standard header length.

	\param[in]	UDPheader 	Pointer to a *struct udphdr*, containing the UDP header.

	\return It returns the length of the payload stored inside the packet corresponding to the UDP header passed as argument.
**/
unsigned short UDPgetpayloadsize(struct udphdr *UDPheader) {
	return (ntohs(UDPheader->len)-UDPHEADERLEN);
}

/**
	\brief Validate the checksum of a raw "Ethernet" packet, i.e. of any packet containing a *struct ether_header* as first bytes

	This function can be used to valide the checksum of any Ethernet raw packet, which can be, for instance, received through
	a raw socket.

	A more detailed description is presented below, in the "Parameters" section.

	\warning Additional arguments are nedded, as of now, only when the type is **CSUM_UDP** or **CSUM_UDPIP**. In that case, you shall pass a pointer to a *size_t* variable
	containing the size of the **UDP payload**.
	

	\param[in]	packet 			Pointer to the **full** packet buffer for which the checksum has to be validated.
	\param[in]	csum 			Checksum value to be checked against the newly computed value, from _packet_ (see also [csum16_t](\ref csum16_t))
	\param[in]	combinedcsum  	This parameter is a pointer to a checksum value. It should be **NULL** for non combined checksum types, otherwise it should contain the pointer
								to the value of the second checksum to be checked (the ordering between _csum_ and _*combinedcsum_ is the same as the one in the checksum 
								type constant - for instance: **CSUM_UDPIP** requires _csum_ to be related to **UDP** and _*combinedcsum_ to **IPv4**).
								If NULL is specified for any combined type, the function will always return **false**.
	\param[in]  type   			Checksum protocol (see also [csum16_t](\ref csum16_t)): various protocols can be specified within the same function, in order to keep
								it easier to be extended as newer protocols will be implemented inside the library.
								The supported protocols are defined inside proper "Checksum protocols" constants in **rawsock.h**, starting with **CSUM_** 
								(e.g. [CSUM_IP](\ref CSUM_IP)).
								Both simple protocols and combined ones are supported: in the first case, only the checksum related to the
								specified protocol is checked (*csum16_t csum*), in the second case, two checksums, contained inside the same
								packet, are checked (_csum16_t csum_ and <i>csum16_t *combinedcsum</i>).
								This can be useful, for instance, to check both the UDP and IP checksums all at once; in case a combined mode is selected _*combinedcsum_ shall
								be non-NULL, otherwise the function will always return **false**.
	\param[in] args 			Additional protocol-specific arguments: these are typically dependant on the specified protocol. For instance,
								using IPv4, they are not needed and NULL can be passed; instead, when using UDP (or UDP+IP), they shall contain
								the pointer to a single value which is the payload length (this may avoid computing it multiple times, outside 
								and inside this function, when a variable containing it is already available).
								Everytime an additional argument is needed and it is not passed (i.e. NULL _args_), the function always returns **false**.

	\return Boolean value containing **true** if the packet contained a valid checksum, **false** otherwise (or in case of errors).
**/
bool validateEthCsum(byte_t *packet, csum16_t csum, csum16_t *combinedcsum, csumt_t type, void *args) {
	csum16_t currCsum;
	bool returnVal=false;
	void *headerPtr; // Generic header pointer
	void *payloadPtr; // Generic payload/SDU pointer
	size_t packetsize; // Used in UDP checksum calculation
	__sum16 storedCsum; // To store the current value of checksum, read from 'packet'

	// Directly return 'false' (as an error occurred) if a combined type is specified but combinedcsum is NULL
	if(type>=0x80 && combinedcsum==NULL) {
		return false;
	}

	// Discriminate the different protocols
	switch(type) {
		case CSUM_IP:
			headerPtr=(struct iphdr*)(packet+sizeof(struct ether_header));

			// Checksum should start with a value of 0x0000 in order to be correctly computed:
			//  set it to 0 and restore it, to avoid making a copy of the packet in memory
			storedCsum=((struct iphdr *) headerPtr)->check;
			((struct iphdr *) headerPtr)->check=0;

			currCsum=ip_fast_csum((__u8 *)headerPtr, ((struct iphdr *) headerPtr)->ihl);

			((struct iphdr *) headerPtr)->check=storedCsum;

			returnVal=(currCsum==csum);
		break;
		case CSUM_UDP:
		case CSUM_UDPIP:
			// payloadsize should be specified, otherwise 'false' will be always returneds
			if(args==NULL) {
				returnVal=false;
			} else {
				// Get packetsize
				packetsize=sizeof(struct udphdr)+*((size_t *) args);

				headerPtr=(struct iphdr*)(packet+sizeof(struct ether_header));
				payloadPtr=(struct udphdr*)(packet+sizeof(struct ether_header)+sizeof(struct iphdr));

				storedCsum=((struct udphdr *) payloadPtr)->check;
				((struct udphdr *) payloadPtr)->check=0;

				currCsum=minirighi_udp_checksum(payloadPtr,packetsize,((struct iphdr *)headerPtr)->saddr,((struct iphdr *)headerPtr)->daddr);

				((struct udphdr *) payloadPtr)->check=storedCsum;

				returnVal=(currCsum==csum);

				if(type==CSUM_UDPIP) {
					if(combinedcsum==NULL) {
						returnVal=false;
					} else {
						// Compute IP checksum
						storedCsum=((struct iphdr *) headerPtr)->check;
						((struct iphdr *) headerPtr)->check=0;

						currCsum=ip_fast_csum((__u8 *)headerPtr, ((struct iphdr *)headerPtr)->ihl);

						((struct iphdr *) headerPtr)->check=storedCsum;

						returnVal=returnVal && (currCsum==(*combinedcsum));
					}
				}
			}
		break;
		default:
			returnVal=false;
	}

	return returnVal;
}

/**
	\brief Test function: inject a checksum error in an IP packet

	This function can be used as a test function to specifically force a checksum error
	inside an already formed IP packet.

	The pointer to the full IP packet shall be passed (IP header+payload).

	\param[in] 	IPpacket 	Pointer to a [byte_t](\ref byte_t) buffer containing the full IP packet.

	\return None.
**/
void test_injectIPCsumError(byte_t *IPpacket) {
	// Get header pointer
	struct iphdr *IPheader=(struct iphdr *) IPpacket;

	if(IPpacket!=NULL) {
		// Change checksum, avoiding possible overflow situations
		if(IPheader->check!=0xFF) {
			IPheader->check=IPheader->check+1;
		} else {
			IPheader->check=0x00;
		}
	}
}

/**
	\brief Test function: inject a checksum error in an UDP packet

	This function can be used as a test function to specifically force a checksum error
	inside an already formed UDP packet.

	The pointer to the full UDP packet shall be passed (UDP header+payload - **not** including neither the Ethernet header nor the IPv4 one)

	\param[in] 	UDPpacket 	Pointer to a [byte_t](\ref byte_t) buffer containing the full UDP packet.

	\return None.
**/
void test_injectUDPCsumError(byte_t *UDPpacket) {
	// Get header pointer
	struct udphdr *UDPheader=(struct udphdr *) UDPpacket;

	if(UDPheader!=NULL) {
		// Change checksum, avoiding possible overflow situations
		if(UDPheader->check!=0xFF) {
			UDPheader->check=UDPheader->check+1;
		} else {
			UDPheader->check=0x00;
		}
	}
}
