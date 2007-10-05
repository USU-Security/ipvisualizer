/*
 * file:	ulogserv2.c
 * purpose:	to read logged packets via netlink, and the libipulog libary
 * created:	2007-10-04
 * creator:	rian shelley
 */

#include <stdlib.h>
#include <stdio.h>
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
#include <unistd.h>
#include "../subnetpassword.h"
#include "../shared/flowdata.h"

#include "libipulog/libipulog.h"

#define MAXLEN 1500


/* the minimum elapsed time between packets, in milliseconds. this corresponds
 *  * to a sustainable 24 frames per second */
#define MINRATE 41



/* the snaplen. 80 bytes should be sufficient */
#define SNAPLEN 80



#define T_IP 0x0800
#define T_TCP 6
#define T_UDP 17

unsigned long long lastupdate = 0;

struct packetheader flowbuffer;
struct flowpacket *buffer;
struct subnetpacket* subnets;
struct packetheader cachedsubnets;
unsigned int clients[MAXCLIENTS] = {0};
int sendsock;
int numclients = 0;


/*
 *  * function:    flushbuffer()
 *   * purpose:     to flush data to a client socket
 *    * recieves:    a client struct
 *     */
inline void flushbuffer() {
        int i;
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(PORT);
        for (i = 0; i < MAXCLIENTS; i++) {
                if (clients[i]) {
                        sin.sin_addr.s_addr = clients[i];
                        int len = sendto(sendsock, &flowbuffer,
                                        flowpacketsize(buffer) + SIZEOF_PACKETHEADER,
                                        0, (struct sockaddr*)&sin, sizeof(sin));

                        if (len < 0) {
                                printf("error writing %i bytes: %s\n",
                                        flowpacketsize(buffer) + SIZEOF_PACKETHEADER,
                                        strerror(errno));
                        } else {
                        }
                }
        }

        lastupdate = gettime();
        buffer->count = 0;
}


/*
 * function:    report()
 * purpose:             to report a packet
 * recieves:    the source ip, the destination ip, the packet size, 
 *                              the packet type
 */
void report(unsigned int src, unsigned int dst, unsigned short size, enum packettype pt) {
        if ((src & NETMASK) == NETBASE)
        {
                buffer->data[buffer->count].incoming = 0;
                buffer->data[buffer->count].ip = src & ~NETMASK;
        }
        else if ((dst & NETMASK) == NETBASE)
        {
                buffer->data[buffer->count].incoming = 1;
                buffer->data[buffer->count].ip = dst & ~NETMASK;
        }
        else
        {
                /* ignore the packet */
                return;
        }
        buffer->data[buffer->count].packet = (unsigned int)pt;
        buffer->data[buffer->count].packetsize = size;
/*
          printf("saw flow from xx.xx.%i.%i\n",
                          (buffer->data[buffer->count].ip & 0x0000ff00) >> 8,
                                           buffer->data[buffer->count].ip & 0x000000ff);
// */

        buffer->count++;
        if (buffer->count >= MAXINDEX || lastupdate + MINRATE < gettime())
                flushbuffer();
}


/*
 *  * function:    addclient()
 *   * purpose:             to add a new client
 *    * recieves:    the ip of a client, in network byte order
 *     * returns:             whether or not the client was added
 *      */
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
 *  * function:    initnetworkbyhost()
 *   * purpose:             to set up a socket to stream to a client
 *    * recieves:    a string representing the person who wants the data
 *     * returns:             success or failure
 *      */
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
        


int main(int argc, char* argv[])
{
	int i;
	unsigned char buffer[MAXLEN];
	/* initialize our list of static clients */
	for (i = 1; i < argc; i++)
		initnetworkbyhost(argv[i]);



	struct ipulog_handle *h = ipulog_create_handle(1, 150000);
	if (!h)
	{
		ipulog_perror(0);
		return 1;
	}
	const struct sniff_ip *ip; 
	while(1) 
	{
		int len = ipulog_read(h, buffer, MAXLEN, 1);
		if (len <= 0) 
		{
			ipulog_perror("ipulog_read returned a value less than 0");
			return 2;
		}
		/* do something with packet */
		printf("Read %i bytes:\n", len);
		for (i = 0; i < len; i++)
		{
			if (!(i%20))
				printf("\n");
			printf("%02hhX ", buffer[i]);
		}
		ulog_packet_msg_t *packet; 
		while(packet = ipulog_get_packet(h, buffer, len))
		{
			/*
			printf("\n");
			printf("mark: %u\n", packet->mark);
			printf("timestamp_sec: %u\n", packet->timestamp_sec);
			printf("timestamp_usec: %u\n", packet->timestamp_usec);
			printf("hook: %u\n", packet->hook);
			printf("indev_name: %s\n", packet->indev_name);
			printf("outdev_name: %s\n", packet->outdev_name);
			printf("data_len: %u\n", packet->data_len);
			printf("prefix: %s\n", packet->prefix);
			printf("mac: %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n", packet->mac[0], packet->mac[1], packet->mac[2], packet->mac[3], packet->mac[4], packet->mac[5]);
			printf("\t");
			*/
			for (i = 0; i < len; i++)
			{
				//if (!(i%20))
				//	printf("\n\t");
				//printf("%02hhX ", packet->payload[i]);
				ip = (struct sniff_ip*)(&(packet->payload[0]));
				enum packettype pt;
				if (ip->ip_p == T_TCP)
					pt= TCP;
				else if (ip->ip_p == T_UDP)
					pt=UDP;
				else
					pt = OTHER;
				report(ntohl(ip->ip_src), ntohl(ip->ip_dst), len, pt);
			}

		}
	}

}
