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

#ifndef SUBNETS_H
#define SUBNETS_H

#define SUBSTRUCT_SIZE 5

#include "../shared/flowdata.h"


typedef struct subnet_t {
	unsigned int base;
	unsigned char mask;
} subnet;

typedef enum iptype_t {
	ALLOCATED,
	UNALLOCATED
} iptype;

int comparenets(subnet* subnets, int subnetsize, unsigned int ip1, unsigned int ip2, iptype* type);



#endif //!SUBNETS_H
