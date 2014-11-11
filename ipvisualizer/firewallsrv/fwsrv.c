/*
    Copyright 2008 Utah State University    

    This file is part of ipvisualizer.

    ipvisualizer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ipvisualizer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ipvisualizer.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * purpose:	to read pflog0'ed  packets using libpcap, and to chunk data together
 * 		and deliver it to somebody
 * created:	08/13/2007
 * creator:	rian shelley
 */

#include <stdio.h>
#include <pcap.h>
//#include <features.h>
#include <sys/time.h>
#include "structs.h"
#include "mygettime.h"
/* #include "tree.h" */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "../shared/flowdata.h"

/* the minimum elapsed time between packets, in milliseconds. this corresponds
 * to a sustainable 24 frames per second */
#define MINRATE 41



/* the snaplen. 80 bytes should be sufficient */
#define SNAPLEN 80

/* The pflog stuff includes a custom header. */
#define PFLOG_HEADER_SIZE 48 
#define PFLOG_RULE_OFFSET 36
#define T_IP 0x0800
#define T_TCP 6
#define T_UDP 17


unsigned long long lastupdate = 0;

struct packetheader flowbuffer;
struct fwflowpacket *buffer;
struct subnetpacket* subnets;
struct packetheader cachedsubnets;
unsigned int clients[MAXCLIENTS] = {0};
int sendsock;
int numclients = 0;

/*
 * function:	flushbuffer()
 * purpose:	to flush data to a client socket
 * recieves:	a client struct
 */
inline void flushbuffer() {
	int i;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	//printf("Flushing the buffer\n");
	for (i = 0; i < MAXCLIENTS; i++) {
		if (clients[i]) {
			sin.sin_addr.s_addr = clients[i];
			int len = sendto(sendsock, &flowbuffer, 
					fwflowpacketsize(buffer) + SIZEOF_PACKETHEADER, 
					0, (struct sockaddr*)&sin, sizeof(sin));

			if (len < 0) {
				printf("error writing %i bytes: %s\n",
					fwflowpacketsize(buffer) + SIZEOF_PACKETHEADER, 
					strerror(errno));
			} else {
			}
		}
	}
	
	lastupdate = gettime();
	buffer->count = 0;
}

/*
 * function:	report()
 * purpose:		to report a packet
 * recieves:	the source ip, the destination ip, the packet size, 
 * 				the packet type
 */
void report(unsigned int src, unsigned int dst, unsigned int rule) {
	if ((src & NETMASK) == NETBASE)
		buffer->data[buffer->count].ip = src & ~NETMASK;
	else if ((dst & NETMASK) == NETBASE)
		buffer->data[buffer->count].ip = dst & ~NETMASK;
	else 
		/* ignore the packet */
		return;
	buffer->data[buffer->count].rule = rule;
/*
	printf("saw flow from xx.xx.%i.%i from rule #%i\n",
		(buffer->data[buffer->count].ip & 0x0000ff00) >> 8,
		 buffer->data[buffer->count].ip & 0x000000ff, rule);
// */

	buffer->count++;
	if (buffer->count >= MAXINDEX || lastupdate + MINRATE < gettime())
		flushbuffer();
}

/*
 * function:	addclient()
 * purpose:		to add a new client
 * recieves:	the ip of a client, in network byte order
 * returns:		whether or not the client was added
 */
int addclient(unsigned int ip)
{
	int i;
	for (i = 0; i < MAXCLIENTS; i++)
	{
		if (clients[i] == ip)
		{
			return 1;
		}
		else if (!clients[i])
		{
			printf("added a client\n");
			clients[i] = ip;
			numclients++;
			return 1;
		} 
	}
	return 0;
}

/*
 * function:	initnetworkbyhost()
 * purpose:		to set up a socket to stream to a client
 * recieves:	a string representing the person who wants the data
 * returns:		success or failure
 */
int initnetworkbyhost(const char* host)
{
	/* see if we have room */
	if (numclients >= MAXCLIENTS)
		return 0;

	struct hostent * he = gethostbyname(host);
	if (he <= 0) 
		return 0;
	
	//memcpy(&addr.sin_addr, he->h_addr_list[0], sizeof(struct in_addr));
	return addclient(*(int*)he->h_addr_list[0]);
}


/*
 * function:	main
 * purpose:	to set stuff up and get things rolling. duh. 
 */
int main(int argc, char* argv[])
{
	char ebuff[PCAP_ERRBUF_SIZE];
	int i;

	for (i = 1; i < argc; i++)
		initnetworkbyhost(argv[i]);

	/* set up our buffers */
	flowbuffer.version = VERSION;
	flowbuffer.packettype = PKT_FIREWALL;
	buffer = (struct fwflowpacket*)flowbuffer.data;

	pcap_t* handle = pcap_open_live("pflog0", SNAPLEN, 1, MINRATE, ebuff);
	//pcap_t* handle = pcap_open_offline("/var/log/ulogd.pcap", ebuff);
	if (!handle) {
		fprintf(stderr, "Couldn't sniff on device %s: %s\n", 
			argv[1], ebuff);
		return 2;
	}

	struct pcap_pkthdr* header;
	const u_char* packet;
	const struct sniff_ethernet *ethernet; /* The ethernet header */
	const struct sniff_ip *ip; /* The IP header */


	/* create a socket we can send with */
	sendsock = socket(AF_INET, SOCK_DGRAM, 0);

	//printf("Sending stuff. yay\n");
	for(;;) {
		int result;
		if(1 == (result = pcap_next_ex(handle, &header, &packet))) {
			//printf("Got a packet....\n");
			//ethernet = (struct sniff_ethernet*)(packet); 
			/* verify that the packet is an ip packet */
			//if (ntohs(ethernet->ether_type) == T_IP) {
				
				ip = (struct sniff_ip*)(packet+PFLOG_HEADER_SIZE);
				/* grab the protocol */
				report(ntohl(ip->ip_src), ntohl(ip->ip_dst), ntohl(*((unsigned int*)(packet + PFLOG_RULE_OFFSET))));
			//}
			//else 
			//{
			//	printf("Doesn't seem to be an ip packet:\n");
			//	int i; 
			//	for (i = 0; i < header->caplen; i++)
			//	{
			//		if (i % 4 == 0)
			//			printf("\n");
			//		printf("%02X ", (int)*(packet + i));
			//	}
			//	printf("\n");
			//}
		} else if (result == 0) {
			/* supposed to be if the timeout expires, but may not need this */
			//printf("pcap timer expired\n");
		}
		/* do other misc stuff here */
		/* like check for udp packets */
	}

	pcap_close(handle);

	return 0;
}
