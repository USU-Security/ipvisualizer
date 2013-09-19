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
#include "../shared/flowdata.h"

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
