/*
 * file:	flowdata.h
 * purpose:	contains the struct that stores the data
 * created:	03-09-3007
 * creator:	rian shelley
 */


#ifndef FLOWDATA_H
#define FLOWDATA_H


/* the mask to watch */
#define NETMASK 0XFFFF0000
/* the base ip */
#define NETBASE ((129<<24)|(123<<16)|(0 << 8)|(0))
/* how often the client should check in */
#define HEARTBEAT 1
/* how long until we drop a client */
#define TIMEOUT 5
/* the maximum number of clients we should support */
#define MAXCLIENTS 10
/* the port number */
#define PORT 17843
/* this should be set to the MTU */
#define BUFFERSIZE 1400
/* our current version */
#define VERSION 0
#define SIZEOF_PACKETHEADER 4
struct packetheader {
	unsigned int version:16;
	unsigned int packettype:8;
	unsigned int reserved:8;
	unsigned char data[BUFFERSIZE-SIZEOF_PACKETHEADER];
};

/* the packet types */
#define PKT_FLOW 0
#define PKT_SUBNET 1





typedef struct t_flowdata {
	/* 16 bits for the last two octets of the ip */
	unsigned short ip;
	/* 11 bits (represents up to 2048) for the packet size */
	unsigned int packetsize:11;
	/* 2 bits for the packet type */
	unsigned int packet:2;
	/* 2 bits reserved */
	unsigned int reserved:2;
	/* 1 bit for incoming/outgoing */
	unsigned int incoming:1;
} flowdata;

#define MAXINDEX ((BUFFERSIZE-SIZEOF_PACKETHEADER)/sizeof(flowdata))

struct flowpacket {
	unsigned short count;
	unsigned short mask;
	unsigned int base;

	flowdata data[MAXINDEX];
};

enum packettype {OTHER, TCP, UDP};

typedef struct subnetword_t {
	unsigned int base:16;
	unsigned int reserved:11;
	unsigned int mask:5;
} subnetword;

struct subnetpacket {
	unsigned short count;
	unsigned short requestnum;
	subnetword subnets[MAXINDEX];
};

struct flowrequest {
	char flowon;
};

inline int flowpacketsize(struct flowpacket* f);
inline int subnetpacketsize(struct subnetpacket* s);

#endif /* !FLOWDATA_H */
