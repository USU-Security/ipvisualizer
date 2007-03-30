
#ifndef SUBNETS_H
#define SUBNETS_H

typedef struct subnet_t {
	unsigned int base;
	unsigned char mask;
} subnet;

#define SUBSTRUCT_SIZE 5

typedef enum iptype_t {
	ALLOCATED,
	UNALLOCATED
} iptype;

int getsubnets(subnet* subnets, int max, const char* site, const char* webpage, const char* authentication);
int comparenets(subnet* subnets, int subnetsize, unsigned int ip1, unsigned int ip2, iptype* type);



#endif //!SUBNETS_H
