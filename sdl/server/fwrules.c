/*
 * purpose:	to retrieve the firewall rules 
 * created:	08/15/2007
 * creator: rian shelley
 */

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "fwrules.h"
#include "sockutils.h"


void freefwrules(struct fwrule* head)
{
	struct fwrule* tmp;
	while (head)
	{
		tmp = head;
		head = head->next;
		free(tmp);
	}
}


/*
 * this function takes a site, a webpage, and authentication data, and returns
 * the head of a linked list that contains all of the rules, along with the
 * number of rules read
 */
struct fwrule* getfwrules(const char* site, const char* webpage, const char* authentication, int* numrules)
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
		return 0;
	}

	memcpy(&serv_addr.sin_addr, he->h_addr_list[0], sizeof(struct in_addr));


	char buffer[128] = {0};
	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	write(sock, "GET ", 4);
	write(sock, webpage, strlen(webpage));
	write(sock, " HTTP 1.1\n", 10);
	snprintf(buffer, 128, "Host: %s\n", site);
	write(sock, buffer, strlen(buffer));
	write(sock, "Authorization: Basic ", 21);
	write(sock, authentication, strlen(authentication));
	write(sock, "\n\n", 2);

	readstring(sock, buffer, 128);
	if (!strstr(buffer, "200")) /* check for OK response */
	{
		fprintf(stderr, "Unable to retrieve firewall rules\n");
		close(sock);
		return 0;
	}
	/* read through the header crap */
	while (readstring(sock, buffer, 128));
	
	struct fwrule head;
	head.next = 0;
	
	struct fwrule* cur = &head;

	int size=0;
	*numrules=0;
	while ((size = readunixstring(sock, buffer, 128)) > 0)
	{
		cur->next = malloc(sizeof(struct fwrule));
		cur = cur->next;
		cur->next = 0;
		cur->rule = strdup(buffer);
		(*numrules)++;
	}
		
	return head.next;
}

