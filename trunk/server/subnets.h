
#ifndef SUBNETS_H
#define SUBNETS_H

typedef struct subnet_t {
	unsigned int base;
	unsigned char mask;
} subnet;

#define SUBSTRUCT_SIZE 5

int getsubnets(subnet* subnets, int max, const char* site, const char* webpage, const char* authentication);



#endif /* !SUBNETS_H */
