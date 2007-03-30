/*
 * file:	flowserv.c
 * purpose:	to sniff for packets using libpcap, and to chunk data together
 * 		and deliver it to somebody
 * created:	03-09-2007
 * creator:	rian shelley
 */

#include <stdio.h>
#include <pcap.h>
#include <features.h>
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

/* the minimum elapsed time between packets, in milliseconds. this corresponds
 * to a sustainable 24 frames per second */
#define MINRATE 41



/* the snaplen. 80 bytes should be sufficient */
#define SNAPLEN 80

/* MAXINDEX has meaning in here... */
#include "flowdata.h"

#define T_IP 0x0800
#define T_TCP 6
#define T_UDP 17


unsigned long long lastupdate = 0;
struct flowpacket buffer;

unsigned int clients[MAXCLIENTS] = {0};
unsigned int timeouts[MAXCLIENTS] = {0};
int sendsock;
int numclients = 0;

void delclient(unsigned int ip);

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
	for (i = 0; i < MAXCLIENTS; i++) {
		if (clients[i]) {
			if (timeouts[i] < time(0)) {
				delclient(clients[i]);
			} else {
				//int len = write(sock[i], &buffer, flowpacketsize(&buffer));
				sin.sin_addr.s_addr = clients[i];
				int len = sendto(sendsock, &buffer, flowpacketsize(&buffer), 0, 
								(struct sockaddr*)&sin, sizeof(sin));

				if (len < 0) {
					printf("error writing %i bytes: %s\n",
						flowpacketsize(&buffer),
						strerror(errno));
				} else {
				}
			}
		}
	}
	
	lastupdate = gettime();
	buffer.count = 0;
}

/*
 * function:	report()
 * purpose:		to report a packet
 * recieves:	the source ip, the destination ip, the packet size, 
 * 				the packet type
 */
void report(unsigned int src, unsigned int dst, unsigned short size, enum packettype pt) {
	if ((src & NETMASK) == NETBASE)
	{
		buffer.data[buffer.count].incoming = 0;
		buffer.data[buffer.count].ip = src & ~NETMASK;
	}
	else if ((dst & NETMASK) == NETBASE)
	{
		buffer.data[buffer.count].incoming = 1;
		buffer.data[buffer.count].ip = dst & ~NETMASK;
	}
	buffer.data[buffer.count].packet = (unsigned int)pt;
	buffer.data[buffer.count].packetsize = size;
/*
	printf("saw flow from xx.xx.%i.%i\n",
		(buffer.data[buffer.count].ip & 0x0000ff00) >> 8,
		 buffer.data[buffer.count].ip & 0x000000ff);
*/

	buffer.count++;
	if (buffer.count >= MAXINDEX || lastupdate + MINRATE < gettime())
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
			timeouts[i] = time(0) + TIMEOUT;
			break;
		}
		else if (!clients[i])
		{
			printf("added a client\n");
			clients[i] = ip;
			timeouts[i] = time(0) + TIMEOUT; 
			numclients++;
			break;
		} 
	}
}

/*
 * function:	delclient()
 * purpose:		to delete a client
 * recieves:	the ip of a client, in network byte order
 */
void delclient(unsigned int ip)
{
	int i;
	for (i = 0; i < MAXCLIENTS; i++)
	{
		if (clients[i] == ip)
		{
			printf("removed a client\n");
			clients[i] = 0;
			numclients--;
			break;
		}
	}
}

/*
 * function:	initnetworkbyhost()
 * purpose:		to set up a socket to stream to a client
 * recieves:	a string representing the person who wants the data
 * returns:		success or failure
 */
int initnetworkbyhost(const char* host)
{
	int s;

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
 * function:	srvlisten()
 * purpose:		to return a socket that is listening for connections
 * returns:		a socket. didn't i just say that?
 */
int srvlisten()
{
	int s;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);

	addr.sin_addr.s_addr = INADDR_ANY;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)))
	{
		close(s);
		return 0;
	}
	return s;
}

/*
 * function:	checklisten()
 * purpose:		to check the socket for a client wishing data
 * recieves:	listening socket
 */
void checklisten(int listen)
{
	struct sockaddr_in addr;
	struct flowrequest fr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;
	socklen_t fromlen = sizeof(addr);
	int err;
	if ((err = recvfrom(listen, &fr, sizeof(fr), MSG_DONTWAIT, 
		(struct sockaddr*)&addr, &fromlen)) > 0)
	{
		if (fr.flowon) 
			addclient(addr.sin_addr.s_addr);
		else 
			delclient(addr.sin_addr.s_addr);
	}
	else 
	{
		printf("\t%s\n", strerror(errno));
	}
}

/*
 * function:	main
 * purpose:	to set stuff up and get things rolling. duh. 
 */
int main(int argc, char* argv[])
{
	char packetbuffer[SNAPLEN];
	char ebuff[PCAP_ERRBUF_SIZE];

	if (argc < 2) {
		fprintf(stderr, "usage: %s <network-device> [hostname+]\n", argv[0]);
		return 1;
	}

	/* open up the sockets */
	int i;
	for (i = 2; i < argc; i++) 
	{
		if (!initnetworkbyhost(argv[i]))
			printf("Error binding to %s: %s\n", argv[i], strerror(errno));
	}

	pcap_t* handle = pcap_open_live(argv[1], SNAPLEN, 1, MINRATE, ebuff);

	if (!handle) {
		fprintf(stderr, "Couldn't sniff on device %s: %s\n", 
			argv[1], ebuff);
		return 2;
	}

	struct pcap_pkthdr* header;
	const u_char* packet;
	const struct sniff_ethernet *ethernet; /* The ethernet header */
	const struct sniff_ip *ip; /* The IP header */


	/* start listening for clients */
	int listensock;
	if (!(listensock = srvlisten()))
		return -1; /* die if we can't listen */
	/* create a socket we can send with */
	sendsock = socket(AF_INET, SOCK_DGRAM, 0);


	for(;;) {
		int result;
		if(1 == (result = pcap_next_ex(handle, &header, &packet))) {
			ethernet = (struct sniff_ethernet*)(packet);
			/* verify that the packet is an ip packet */
			if (ntohs(ethernet->ether_type) == T_IP) {
				ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
				/* grab the protocol */
				enum packettype pt;
				if (ip->ip_p == T_TCP)
					pt = TCP;
				else if (ip->ip_p == T_UDP)
					pt = UDP;
				else
					pt = OTHER;
				report(ntohl(ip->ip_src), ntohl(ip->ip_dst), header->len, pt);
			} else {
			}

		} else if (result == 0) {
			/* supposed to be if the timeout expires, but may not need this */
		}
		/* do other misc stuff here */
		/* like check for udp packets */
		checklisten(listensock);
	}

	pcap_close(handle);

	return 0;
}
