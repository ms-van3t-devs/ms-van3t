// Example of receiver program using Rawsock_lib
// Rawsock_lib, licensed under GPLv2

// This program is an example program to receive and display the content of any UDP packet on a wireless interface, using raw sockets
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/wireless.h>
#include "Rawsock_lib/rawsock.h" /// Rawsock_lib is included here
#include <linux/if_packet.h>

#define NO_FLAGS 0

int main (int argc, char **argv) {
	int sFd;
	ssize_t rcv_bytes;
	byte_t packet[ETHERMTU]; // Packet buffer with size = Ethernet MTU
	
	struct ether_header* etherHeader=NULL;
	struct iphdr *IPheader=NULL;
	struct udphdr *udpHeader=NULL;
	byte_t *payload=NULL;
	
	int ret_wlanl_val;
	char devname[IFNAMSIZ]={0};
	int ifindex;
	struct sockaddr_ll addrll;

	size_t payloadsize;

	socklen_t AddrLen=sizeof(struct sockaddr);

	// Already get the pointers to the headers, given the buffer which will contain the packet
	payload=UDPgetpacketpointers(packet,&etherHeader,&IPheader,&udpHeader);

	// Look for and bind to wireless interface, other than creating the raw socket
	ret_wlanl_val=wlanLookup(devname,&ifindex,NULL,NULL,0,WLANLOOKUP_WLAN);
	if(ret_wlanl_val<=0) {
		fprintf(stderr,"wlanLookup() error.\n");
		rs_printerror(stderr,ret_wlanl_val);
		exit(EXIT_FAILURE);
	}

	sFd=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sFd==-1) {
		perror("Cannot create socket: socket() error");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout,"Using interface: %s - index: 0x%02x - number of VIFs: %d\n",devname,ifindex,ret_wlanl_val);

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

	fprintf(stdout,"Ready to receive datagrams.\n\n");
	while(1) {
		// Receive datagram (blocking)
		rcv_bytes = recvfrom(sFd,packet,ETHERMTU,NO_FLAGS,(struct sockaddr *)&addrll,&AddrLen);

		// Go on only if it is a datagram of interest (in our case if it is UDP)
		if (ntohs(etherHeader->ether_type)!=ETHERTYPE_IP) { 
			continue;
		}
		if (IPheader->protocol!=IPPROTO_UDP) {
			continue;
		}

		// Validate checksum (combined mode: IP+UDP): if it is wrong, discard packet
		payloadsize=UDPgetpayloadsize(udpHeader);
		if(!validateEthCsum(packet, udpHeader->check, &(IPheader->check), CSUM_UDPIP, (void *) &payloadsize)) {
			fprintf(stderr,"Wrong checksum! Packet will be discarded.\n");
			continue;
		}

		if(rcv_bytes==-1){
			perror("An error occurred in receive last message");
			fprintf(stderr,"The execution will be terminated now.\n");
			break;
		}

		// Print payload and source IP address
		fprintf(stdout,"Received a new packet from %s\n",inet_ntoa(*(struct in_addr*)&IPheader->saddr));
		display_packetc("Received a new packet with payload:", payload, payloadsize);
	}

	close(sFd);

	return 0;
}