/*
 * file:	subnets.c
 * purpose:	to retrieve the subnet information and convert it to a binary form
 * created:	03/23/2007
 * creator: rian shelley
 */

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include "subnets.h"
#include <string.h>

/*
 * sub:         readstring()
 * purpose:     to read a string, the slow way. reads till it finds a newline, discards it. 
 * recieves:    the file descriptor, a pointer to a character array, and its size
 * returns:     how many characters were read
 */
int readstring(int fd, char* buffer, int size)
{
	int len=0;

	/* read characters until read burps or we hit a newline */
	while (len < size-2)
	{
		if (read(fd, &buffer[len], 1) < 1)
			return -1;
		if (len > 0 && buffer[len-1] == '\x0d' && buffer[len] == '\x0a')
		{
			len--;
			break;
		}
		len++;
	} 	
	buffer[len] = 0;
	fprintf(stderr, "returning '%s'\n", buffer);
	return len;
}




/*
 * function:	getsubnets()
 * purpose:		to retrieve a list of subnets from a web page
 * recieves:	a pointer to an array of subnets, the size of the array,
 * 				the server to get the list from, the path to the page,
 * 				and the authorization required to log into the website as a
 * 				username:password mime-encoded string. 
 */
int getsubnets(subnet subnets[], int max, const char* site, const char* webpage, const char* authentication)
{

	int sock;
	struct sockaddr_in serv_addr;
	struct hostent *he;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(80);
	if ((he = gethostbyname(site)) <= 0)
	{
		fprintf(stderr, "That is a bad ip address or hostname\n");
		return -2;
	}

	memcpy(&serv_addr.sin_addr, he->h_addr_list[0], sizeof(struct in_addr));


	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	write(sock, "GET ", 4);
	write(sock, webpage, strlen(webpage));
	write(sock, " HTTP 1.1\n", 10);
	write(sock, "Authorization: Basic ", 21);
	write(sock, authentication, strlen(authentication));
	write(sock, "\n\n", 2);

	char buffer[80] = {0};
	readstring(sock, buffer, 80);
	if (!strstr(buffer, "200")) /* check for OK response */
	{
		fprintf(stderr, "Unable to get the subnet data\n");
		close(sock);
		return 0;
	}
	/* read through the header crap */
	while (readstring(sock, buffer, 80));

	int size=0;
	while (size < max && (read(sock, &subnets[size], SUBSTRUCT_SIZE) == SUBSTRUCT_SIZE))
		size++;
	return size;
}

/*
 * function:	comparenets
 * purpose:		to compare two ips to see if they are in the same subnet
 * recives:		the subnet array, its size, the two ips
 * returns:		true or false
 */
int comparenets(subnet* subnets, int subnetsize, unsigned int ip1, unsigned int ip2, iptype *type)
{
	/* TODO: subnets is sorted, do a binary search or something */
	int i;
	for (i = 0; i < subnetsize; i++) {
		if (subnets[i].base > ip1) 
			break;/* if the base is bigger than the ip then they all will be after this */
		if (subnets[i].base  == (ip1 & (~((1 << (32-subnets[i].mask)) - 1)))) {
			*type = ALLOCATED;
			return subnets[i].base  == (ip2 & (~((1 << (32-subnets[i].mask)) - 1)));
		}
	}
	for (i = 0; i < subnetsize; i++) {
		if (subnets[i].base > ip2) 
			break;/* if the base is bigger than the ip then they all will be after this */
		if (subnets[i].base  == (ip2 & (~((1 << (32-subnets[i].mask)) - 1)))) {
			*type = ALLOCATED;
			return 0;
		}
	}
	*type = UNALLOCATED;
	return 0;
}
