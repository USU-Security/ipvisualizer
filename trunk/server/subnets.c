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
#include <unistd.h>
#include "sockutils.h"
#include "base64.h"

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
	if (!(he = gethostbyname(site)))
	{
		fprintf(stderr, "That is a bad ip address or hostname\n");
		return -2;
	}


	memcpy(&serv_addr.sin_addr, he->h_addr_list[0], sizeof(struct in_addr));

	char auth[512] = {0};
	int authsize=512;
	base64encode(authentication, strlen(authentication), auth, &authsize);


	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	write(sock, "GET ", 4);
	write(sock, webpage, strlen(webpage));
	write(sock, " HTTP 1.1\n", 10);
	write(sock, "Authorization: Basic ", 21);
	write(sock, auth, authsize);
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

