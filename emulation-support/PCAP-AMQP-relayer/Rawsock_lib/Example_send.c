// Example of sender program using Rawsock_lib
// Rawsock_lib, licensed under GPLv2

/* 
	This example program has been used to send periodic broadcast UDP packets over a wireless interface, as Rawsock_lib also provides a function 
	to specifically look for wireless interfaces, under Linux (wlanLookup()).

	This code is using "timers that notify via file descriptors" (timerfd) together with poll() to correctly send packets at the specified time
	intervals.

	It has been successfully used, after cross-compilation, within the OpenWrt-V2X platform (on a PC Engines APU1D board), available on GitHub: 
	https://github.com/francescoraves483/OpenWrt-V2X/tree/OpenWrt-V2X-18.06.1

	This program accepts as arguments: the port to be used for broadcast transmissions, the period to use for sending each message (in seconds, 
	values less than 1 are accepted) and the payload of the message (your program can be then enhanced to use a different payload, not specified by 
	the user through the terminal).
*/
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <poll.h>
#include "Rawsock_lib/rawsock.h"
#include <linux/if_packet.h>

#define MAX_LEN 1470 // Maximum allowed payload length

#define SEC_TO_NANOSEC 1000000000 // Constant for conversion between seconds and nanoseconds
#define INDEFINITE_BLOCK -1
#define NO_FLAGS 0
#define SRCPORT 46772 // Source port to be used
#define START_ID 11349 // Initial ID for the first packet
#define INCR_ID 0 // ID increment for each successive packet (all packets will have the same ID in this case)

int main (int argc, char **argv) {
	int sFd;
	int broadPerm=1;
	int broadPort;
	size_t len;
	int clockFd;

	// Timer management variables
	struct itimerspec new_value;
	long nanosec;
	double d_sec;
	double d_time;
	struct pollfd timerMon;

	// Junk variable (needed to clear the timer event with read())
	unsigned long long junk;

	// wlanLookup() variables, for interface name, source MAC address and return value
	char devname[IFNAMSIZ]={0}; // It will contain the used interface name
	macaddr_t srcmacaddr=prepareMacAddrT();
	int ret_wlanl_val;

	// Ethernet header and packet container
	struct ether_header etherHeader;
	byte_t *ethernetpacket;

	// IP header
	struct iphdr ipHeader;
	// IP address (src+dest) structure
	struct ipaddrs ipaddrs;
	// IP packet container
	byte_t *ippacket;
	// id to be inserted in the id field on the IP header
	unsigned int id=START_ID;

	// UDP header
	struct udphdr udpHeader;
	// UDP packet container
	byte_t *udppacket;

	// sockaddr_ll (device-independent physical-layer address)
	struct sockaddr_ll addrll;
	// Index of the interface which is used (and returned by wlanLookup())
	int ifindex;

	// Final packet size
	size_t finalpktsize;

	// Check command line arguments
	if(argc!=4) {
	    fprintf(stderr,"Error. Expected five parameters.\nCorrect usage: <%s> <broadcast port> <time interval> <payload>.\n",argv[0]);
	    exit(EXIT_FAILURE);
	}

	// Get payload length
	len=strlen(argv[3]);
	if(len>MAX_LEN) {			
		fprintf(stderr,"Error. Payload string has length %zu, which is over the allowed maximum (%d)",len,MAX_LEN);
	       exit(EXIT_FAILURE);
	}

	// Open socket
	sFd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if(sFd==-1) {
		perror("socket() error");
		exit(EXIT_FAILURE);
	}

	// Look for available WLAN interfaces (wlan0 should be returned in the APU/ALIX boards)
	ret_wlanl_val=wlanLookup(devname,&ifindex,srcmacaddr,NULL,0,WLANLOOKUP_WLAN);
	if(ret_wlanl_val<=0) {
		fprintf(stderr,"wlanLookup() error.\n");
		rs_printerror(stderr,ret_wlanl_val);
		close(sFd);
		exit(EXIT_FAILURE);
	}

	// Check if wlanLookup() was able to properly write the source MAC address (which is normally initialized to broadcast)
	if(macAddrTypeGet(srcmacaddr)==MAC_BROADCAST) {
		fprintf(stderr,"Could not retrieve source MAC address.\n");
		close(sFd);
		exit(EXIT_FAILURE);
	}

	// Prepare sockaddr_ll structure
	memset(&addrll,0,sizeof(addrll));
	addrll.sll_ifindex=ifindex;
	addrll.sll_family=AF_PACKET;
	addrll.sll_protocol=htons(ETH_P_ALL);

	// Bind to the wireless interface
	if(bind(sFd,(struct sockaddr *) &addrll,sizeof(addrll))<0) {
		perror("Cannot bind to interface: bind() error");
  		close(sFd);
  		exit(EXIT_FAILURE);
	}

	// This works only with AF_INET sockets, so it should not be used here
	// if(setsockopt(sFd,SOL_SOCKET,SO_BINDTODEVICE,devname,strlen(devname))==-1) {
	// 	perror("setsockopt() for SO_BINDTODEVICE error");
	// 	close(sFd);
	// 	exit(EXIT_FAILURE);
	// }

	// Set broadcast permission using socket layer options (is this really needed here?)
	if(setsockopt(sFd,SOL_SOCKET,SO_BROADCAST,(void *) &broadPerm,sizeof(broadPerm))!=0) {
		perror("setsockopt() for SO_BROADCAST error");
		close(sFd);
		exit(EXIT_FAILURE);
	}

	broadPort=atoi(argv[1]); // Read port from command line

	// Preparing headers (it can be done here since they won't change during the execution, in this specific case)
	//unsigned char dstaddr[6]={0xD8,0x61,0x62,0x07,0x9D,0xA8}; <- example of use with non broadcast transmission
	//etherheadPopulate(&etherHeader, srcmacaddr, dstaddr, ETHERTYPE_IP); <- example of use with non broadcast transmission
	etherheadPopulateB(&etherHeader, srcmacaddr, ETHERTYPE_IP);
	IP4headPopulateB(&ipHeader, devname, 0, 0, BASIC_UDP_TTL, IPPROTO_UDP, FLAG_NOFRAG_MASK, &ipaddrs);
	//IP4headPopulate(&ipHeader, devname, "10.10.6.103", 0, 0, BASIC_UDP_TTL, IPPROTO_UDP, FLAG_NOFRAG_MASK, &ipaddrs); <- example of use with non broadcast transmission
	UDPheadPopulate(&udpHeader, SRCPORT, broadPort);

	// Allocating packet buffers
	udppacket=malloc(UDP_PACKET_SIZE(argv[3]));
	ippacket=malloc(IP_UDP_PACKET_SIZE(argv[3]));
	ethernetpacket=malloc(ETH_IP_UDP_PACKET_SIZE(argv[3]));

	// Create monotonic (increasing) timer
	clockFd=timerfd_create(CLOCK_MONOTONIC,NO_FLAGS);
	if(clockFd==-1) {
		perror("timerfd_create() error");
		close(sFd);
		exit(EXIT_FAILURE);
	}

	// Get time from command line and convert it to sec and nanosec
	d_time=strtod(argv[2],NULL);
	if(d_time==0) {
		perror("strtod() to get time interval returned an error");
		close(sFd);
		exit(EXIT_FAILURE);
	}

	nanosec=SEC_TO_NANOSEC*modf(d_time,&d_sec);
	new_value.it_value.tv_nsec=nanosec;
	new_value.it_value.tv_sec=(time_t)d_sec;
	new_value.it_interval.tv_nsec=nanosec;
	new_value.it_interval.tv_sec=(time_t)d_sec;
	// time_t is long in my case, but being implementation dependant, I decided to print just d_sec as double with no decimal digits
	fprintf(stdout,"Sending period set to %.0f second(s) and %.3f microsecond(s).\n",d_sec,nanosec/1000.0);

	// Fill pollfd structure
	timerMon.fd=clockFd;
	timerMon.revents=0;
	timerMon.events=POLLIN;

	// Start timer
	if(timerfd_settime(clockFd,NO_FLAGS,&new_value,NULL)==-1) {
		perror("timerfd_settime() error");
		close(sFd);
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout,"Timer successfully started. Sending triggered.\n\n");
	}

	while(1) {
		// poll waiting for events happening on the timer descriptor (i.e. wait for timer expiration)
		if(poll(&timerMon,1,INDEFINITE_BLOCK)>0) {
			// "Clear the event" by performing a read() on a junk variable
			read(clockFd,&junk,sizeof(junk));
			
			// Prepare datagram
			IP4headAddID(&ipHeader,(unsigned short) id);
			id+=INCR_ID;
			UDPencapsulate(udppacket,&udpHeader,argv[3],strlen(argv[3]),ipaddrs);
			// 'IP4headAddTotLen' may also be skipped since IP4Encapsulate already takes care of filling the length field
			// IP4headAddTotLen(&ipHeader, IP_UDP_PACKET_SIZE(argv[5]));
			IP4Encapsulate(ippacket, &ipHeader, udppacket, UDP_PACKET_SIZE(argv[3]));
			finalpktsize=etherEncapsulate(ethernetpacket, &etherHeader, ippacket, IP_UDP_PACKET_SIZE(argv[3]));

			// Send datagram
			if(sendto(sFd,ethernetpacket,finalpktsize,NO_FLAGS,(struct sockaddr *)&addrll,sizeof(struct sockaddr_ll))!=finalpktsize) {
				perror("sendto() for sending broadcasted data failed");
				fprintf(stderr,"The program will be terminated now");
				break;
			}
		}
	}

	freeMacAddrT(srcmacaddr); // freeing a macaddr_t structure, using a function provided with the Rawsock_lib library
	free(udppacket);
	free(ethernetpacket);
	free(ippacket);
	close(sFd);
	close(clockFd);

	return 0;
}