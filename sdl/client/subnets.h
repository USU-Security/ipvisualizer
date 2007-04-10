
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
