/** \file 
	This file represents the main header file of the Rawsock library.

	This file, represeting the main header file of the Rawsock library, should be included in your project everytime 
	you want to use the library to simplify and enhance the use of Linux raw sockets.
	It supports, as of now, IPv4 and UDP.

	\version 0.3.4
	\date 2020-01-24
	\copyright Licensed under GPLv2
**/
#ifndef RAWSOCK_H_INCLUDED
#define RAWSOCK_H_INCLUDED

#include <net/ethernet.h>
#include <linux/udp.h>	
#include <linux/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef __ANDROID__
	#define udphdr __kernel_udphdr
#endif

#define MAC_FILE_PATH_SIZE 23 /**< Reserved for future use. */
#define MAC_ADDR_SIZE 6 /**< Size of a MAC address, in _bytes_. You can use this definition to declare your own *uint8_t*, *unsigned char* or *byte_t* array, with *MAC_ADDR_SIZE* elements, to store any MAC address. For instance: `uint8_t macaddress[MAC_ADDR_SIZE];`*/

// Errors
#define ERR_WLAN_NOIF 0 /**< __wlanLookup() error definition__: no WLAN interfaces found. */
#define ERR_WLAN_SOCK -1 /**< __wlanLookup() error definition__: socket creation error. */
#define ERR_WLAN_GETIFADDRS -2 /**< __wlanLookup() error definition__: getifaddrs() (to obtain interfaces list) error. */
#define ERR_WLAN_INDEX -3 /**< __wlanLookup() error definition__: wrong index specified. */
#define ERR_WLAN_GETSRCMAC -4 /**< __wlanLookup() error definition__: unable to get source MAC address (if requested). */
#define ERR_WLAN_GETIFINDEX -5 /**<  __wlanLookup() error definition__: unable to get source interface index (if requested). */
#define ERR_WLAN_GETSRCIP -6 /**<  __wlanLookup() error definition__: unable to get source interface index (if requested). */

#define ERR_IPHEAD_SOCK -10 /**< __[IP4headPopulate*()](\ref IP4headPopulate) error definition__: internal socket creation error. */
#define ERR_IPHEAD_NOSRCADDR -11 /**< __[IP4headPopulate*()](\ref IP4headPopulate) error definition__: unable to retrieve current device IP address. */

#define ERR_VIFPRINTER_SOCK -20 /**< __vifPrinter() error definition__: socket creation error. */
#define ERR_VIFPRINTER_GETIFADDRS -21 /**< __vifPrinter() error definition__: getifaddrs() (to obtain interfaces list) error. */

// wlanLookup() modes
#define WLANLOOKUP_WLAN 0 /**< __wlanLookup() mode definition__: look for wireless interfaces only. */
#define WLANLOOKUP_NONWLAN 1 /**< __wlanLookup() mode definition__: look for non-wireless interfaces only. */

// Names
#define MAC_NULL 0x00 /**< __MAC type definition (managed by prepareMacAddrT(), macAddrTypeGet() and freeMacAddrT())__: NULL MAC-address (NULL pointer returned). */
#define MAC_BROADCAST 0x01 /**< __MAC type definition (managed by prepareMacAddrT(), macAddrTypeGet() and freeMacAddrT())__: broadcast (FF:FF:FF:FF:FF:FF) MAC address. */
#define MAC_UNICAST 0x02 /**< __MAC type definition (managed by prepareMacAddrT(), macAddrTypeGet() and freeMacAddrT())__: unicast MAC address. */
#define MAC_MULTICAST 0x03 /**< __MAC type definition (managed by prepareMacAddrT(), macAddrTypeGet() and freeMacAddrT())__: multicast (01:_xx_:_xx_:_xx_:_xx_:_xx_) MAC address. */
#define MAC_ZERO 0x04 /**< __MAC type definition (managed by prepareMacAddrT(), macAddrTypeGet() and freeMacAddrT())__: all-zero (00:00:00:00:00:00) MAC address (no MAC is set). */

// Useful constants
// Additional EtherTypes
#define ETHERTYPE_GEONET 0x8947 /**< __Additional EtherType definition__: GeoNetworking, as defined in ETSI EN 302 636-4-1. */
#define ETHERTYPE_WSMP 0x88DC /**< __Additional EtherType definition__: WAVE Short Message Protocol, as defined in IEEE Std 1609.3-2016. */

// Special wlanLookup index values
#define WLANLOOKUP_LOOPBACK -1 /**< __wlanLookup() special [index](\ref wlanLookup) value definition__: use *WLANLOOKUP_LOOPBACK* to search for loopback IF instead of WLAN IF. */

// IP constants
#define BASIC_IHL 5 /**< __IPv4 constant__: basic Internet Header Length, without any options. */
#define IPV4 4 /**< __IPv4 constant__: basic Internet Header Length, in _bytes_ without any options. */
#define BASIC_UDP_TTL 64 /**< __IPv4 constant__: some sort of "default" TTL which can be used to generate IPv4/UDP packets with a TTL of _64_. */

// UDP constant
#define UDPHEADERLEN 8 /**< __UDP constant__: number of bytes inside the UDP header. */

// Useful masks
#define FLAG_NOFRAG_MASK (1<<6) /**< __IPv4 flags mask__: Mask to set the _Don't Fragment_ (DF) IPv4 flag, in the [flags](\ref IP4headPopulate) argument of [IP4headPopulate*()](\ref IP4headPopulate) \note It can be OR-ed with other masks. */
#define FLAG_RESERVED_MASK (1<<7) /**< __IPv4 flags mask__: Mask to set the _Reserved_ IPv4 flag, in the [flags](\ref IP4headPopulate) argument of [IP4headPopulate*()](\ref IP4headPopulate) \note It can be OR-ed with other masks. */
#define FLAG_MOREFRAG_MASK (1<<5) /**< __IPv4 flags mask__: Mask to set the _More Fragments_ (MF) IPv4 flag, in the [flags](\ref IP4headPopulate) argument of [IP4headPopulate*()](\ref IP4headPopulate) \note It can be OR-ed with other masks. */

// Checksum protocols, to be used inside the validateEthCsum() function 
//  (0x00->0x7F should be simple types, 0x80->0xFF should be combined types)
#define CSUM_IP 0x00 /**< __Simple validateEthCsum() checksum type (_csum_t_)__: compute IPv4 checksum by specifying *CSUM_IP* as _type_. */
#define CSUM_UDP 0x01 /**< __Simple validateEthCsum() checksum type (_csum_t_)__: compute UDP checksum by specifying *CSUM_UDP* as _type_. */
#define CSUM_UDPIP 0x80 /**< __Combined validateEthCsum() checksum type (_csum_t_)__: compute IPv4 and UDP checksums by specifying *CSUM_UDPIP* as _type_. \warning If you are extending the library and you want to add another combined type, always use a value from 0x81 to 0xFF, as 0x00 to 0x7F should be simple types, and 0x80 to 0xFF combined ones, to keep things clear. */

// Useful macros for printing MAC addresses inside the printf() familty of functions
#define PRI_MAC "%02x:%02x:%02x:%02x:%02x:%02x" /**< Useful macro specifier for printing MAC addresses inside the _printf()_ familty of functions. *PRI_MAC* works as a single specifier for the whole address, like PRIu<i>xx</i> in _inttypes.h_ for printing _xx_ bits integers, but without the leading `%`. See also the strictly related [MAC_PRINTER](\ref MAC_PRINTER) macro. */
#define MAC_PRINTER(mac_array) mac_array[0], mac_array[1], mac_array[2], mac_array[3], mac_array[4], mac_array[5] /**< *MAC_PRINTER(address-variable)* should be used in combination with [PRI_MAC](\ref PRI_MAC) to specify the variable containing the MAC address. For instance, if _addr_ is a variable of type [macaddr_t](\ref macaddr_t), it is possible to print the corresponding address with `printf("Address: " PRI_MAC "\n",MAC_PRINTER(addr))`. \warning No check is performed to ensure that a NULL pointer ([MAC_NULL](\ref MAC_NULL)) is not passed to *MAC_PRINTER*. The check must be manually performed to avoid a segmentation fault.*/

// Useful macros for reading MAC addresses inside the scanf() familty of functions
#define SCN_MAC "%x:%x:%x:%x:%x:%x%*c" /**< Useful macro specifier for reading MAC addresses inside the _scanf()_ familty of functions. *SCN_MAC* works as a single specifier for the whole address, like SCNu<i>xx</i> in _inttypes.h_ for reading _xx_ bits integers, but without the leading `%`. See also the strictly related [MAC_SCANNER](\ref MAC_SCANNER) macro. */
#define MAC_SCANNER(mac_array) &mac_array[0], &mac_array[1], &mac_array[2], &mac_array[3], &mac_array[4], &mac_array[5] /**< *MAC_SCANNER(_address-variable_)* should be used in combination with [SCN_MAC](\ref SCN_MAC) to specify the variable containing the MAC address (without `&`, as it is already added by *MAC_SCANNER*). For instance, if _addr_ is an allocated variable of type [macaddr_t](\ref macaddr_t), it is possible to store an address inside _addr_ with `scanf(SCN_MAC,MAC_SCANNER(addr))`. \warning No check is performed to ensure that a NULL pointer ([MAC_NULL](\ref MAC_NULL)) is not passed to *MAC_SCANNER*. The check must be manually performed to avoid a segmentation fault (the variable should be already allocated with prepareMacAddrT()).*/

// Size definitions (macros)
#define UDP_PACKET_SIZE(data) sizeof(struct udphdr)+sizeof(data)  /**< __Size definition__: given *data*, as any variable, the UDP payload size containing the specified *data* is calculated and returned in _bytes_. */
#define IP_UDP_PACKET_SIZE(data) sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(data) /**< __Size definition__: given *data*, as any variable, the IPv4 + UDP payload size (with basic IHL, i.e. no options) containing the specified *data* is calculated and returned in _bytes_. */
#define ETH_IP_UDP_PACKET_SIZE(data) sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(data) /**< __Size definition__: given *data*, as any variable, the IPv4 + UDP payload size (with basic IHL, i.e. no options), **including struct ether_header**, containing the specified *data* is calculated and returned in _bytes_. */

#define UDP_PACKET_SIZE_S(size) sizeof(struct udphdr)+size /**< __Size definition__: given *size*, in _bytes_, the UDP payload size containing a payload with the specified *size* is calculated and returned in _bytes_. */
#define IP_UDP_PACKET_SIZE_S(size) sizeof(struct iphdr)+sizeof(struct udphdr)+size /**< __Size definition__: given *size*, in _bytes_, the IPv4 + UDP payload size (with basic IHL, i.e. no options) containing a payload with the specified *size* is calculated and returned in _bytes_. */
#define ETH_IP_UDP_PACKET_SIZE_S(size) sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr)+size /**< __Size definition__: given *size*, in _bytes_, the IPv4 + UDP payload size (with basic IHL, i.e. no options), **including struct ether_header**, containing a payload with the specified *size* is calculated and returned in _bytes_. */

#ifndef BYTE_TYPE
#define BYTE_TYPE
typedef uint8_t byte_t; /**< Custom type to store a single byte. It should be defined only if it was not defined elsewhere. */
#endif

typedef uint8_t * macaddr_t; /**< Custom type to store a MAC address. It is better and should be managed with the provided prepareMacAddrT(), macAddrTypeGet() and freeMacAddrT() functions, to avoid messing up with pointer. */
typedef unsigned short ethertype_t; /**< Custom type which can be used to store the _EtherType_. */
typedef int rawsockerr_t; /**< Custom type to store errors returned by the library functions. */
typedef unsigned char csumt_t; /**< Custom type to store checksum types to be passed to validateEthCsum(). */
typedef __sum16 csum16_t; /**< Custom type to store checksums (it may be a bit clearer that using directly <i>__sum16</i>, although <i>__sum16</i> is perfectly fine too). */
/**
	\brief Structure to store a couple of source and destinaion IPv4 addresses.

	This structure can be used to store a couple of IPv4 address (source + destination), in the *in_addr_t* format.
**/
struct ipaddrs {
	in_addr_t src; /**< Source IPv4 address container.*/
	in_addr_t dst; /**< Destination IPv4 address container.*/
};

/**
	\brief Protocol type enumerator

	Protocol type enumerator, useful to manage more than one protocol on a single program. As more protocols will be supported, this _enum_ will be updated accordingly.
**/
typedef enum {
	UNSET_P, 	/**< Unspecified protocol type */
	UDP, 		/**< UDP over IPv4 */
	AMQP_0_9,   /**< AMQP 0.9 (RabbitMQ) - not yet supported by the library */
	AMQP_1_0    /**< AMQP 1.0 (ActiveMQ) - not yet supported by the library */
} protocol_t;

// General utilities
rawsockerr_t wlanLookup(char *devname, int *ifindex, macaddr_t mac, struct in_addr *srcIP, int index, int mode);
rawsockerr_t vifPrinter(FILE *stream);
macaddr_t prepareMacAddrT();
unsigned int macAddrTypeGet(macaddr_t mac);
void freeMacAddrT(macaddr_t mac);
void rs_printerror(FILE *stream,rawsockerr_t code);
void display_packet(const char *text,byte_t *packet,unsigned int len);
void display_packetc(const char *text,byte_t *packet,unsigned int len);
uint64_t hton64 (uint64_t hostu64); // Like 'htonl()' but for 64-bits unsigned integers
uint64_t ntoh64 (uint64_t netu64); // Like 'ntohl()' but for 64-bits unsigned integers

// Ethernet level functions
void etherheadPopulateB(struct ether_header *etherHeader, macaddr_t mac, ethertype_t type);
void etherheadPopulate(struct ether_header *etherHeader, macaddr_t macsrc, macaddr_t macdst, ethertype_t type);
size_t etherEncapsulate(byte_t *packet,struct ether_header *header,byte_t *sdu,size_t sdusize);
void getSrcMAC(struct ether_header *etherHeader, macaddr_t macsrc);

// IP level functions
rawsockerr_t IP4headPopulateB(struct iphdr *IPhead, char *devname,unsigned char tos,unsigned short frag_offset, unsigned char ttl, unsigned char protocol,unsigned int flags,struct ipaddrs *addrs);
rawsockerr_t IP4headPopulateS(struct iphdr *IPhead, char *devname, struct in_addr destIP, unsigned char tos,unsigned short frag_offset, unsigned char ttl, unsigned char protocol,unsigned int flags,struct ipaddrs *addrs);
rawsockerr_t IP4headPopulate(struct iphdr *IPhead, char *devname, char *destIP, unsigned char tos,unsigned short frag_offset, unsigned char ttl, unsigned char protocol,unsigned int flags,struct ipaddrs *addrs);
void IP4headAddID(struct iphdr *IPhead, unsigned short id);
void IP4headAddTotLen(struct iphdr *IPhead, unsigned short len);
size_t IP4Encapsulate(byte_t *packet,struct iphdr *header,byte_t *sdu,size_t sdusize);

// UDP level functions
void UDPheadPopulate(struct udphdr *UDPhead, unsigned short sourceport, unsigned short destport);
size_t UDPencapsulate(byte_t *packet,struct udphdr *header,byte_t *data,size_t payloadsize,struct ipaddrs addrs);

// Receiving device functions
byte_t *UDPgetpacketpointers(byte_t *pktbuf,struct ether_header **etherHeader, struct iphdr **IPheader,struct udphdr **UDPheader);
unsigned short UDPgetpayloadsize(struct udphdr *UDPheader);
bool validateEthCsum(byte_t *packet, csum16_t csum, csum16_t *combinedcsum, csumt_t type, void *args);

// Test functions, to inject errors inside packets - should never be used under normal circumstances
void test_injectIPCsumError(byte_t *IPpacket);
void test_injectUDPCsumError(byte_t *UDPpacket);

#endif
