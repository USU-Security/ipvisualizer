
#include "flowdata.h"
#include <stdlib.h>
#include <string.h>

/* returns the number of bytes that are valid */
inline int flowpacketsize(struct flowpacket* f)
{
	return(2*sizeof(unsigned short) + sizeof(flowdata) * f->count);
}

/* returns the number of bytes that are valid */
inline int fwflowpacketsize(struct fwflowpacket* f)
{
	return(2*sizeof(unsigned short) + sizeof(fwflowdata) * f->count);
}

inline int subnetpacketsize(struct subnetpacket* s)
{
	return(2*sizeof(unsigned short) + sizeof(subnetword) * s->count);
}

/* callee has the duty to free any allocated data */
inline void readrulepacket(void* buffer, struct fwrulepacket* r) 
{
	r->num = *((unsigned short*)buffer);
	buffer+=2;
	r->max = *((unsigned short*)buffer);
	buffer+=2;
	r->length = *((unsigned short*)buffer);
	buffer+=2;

	/* we have to be careful. the buffer will be up to BUFFERSIZE-PACKETHEADER-RULEPACKETSIZE */
	if (r->length > BUFFERSIZE-SIZEOF_PACKETHEADER-RULEPACKETSIZE)
		r->length = BUFFERSIZE-SIZEOF_PACKETHEADER-RULEPACKETSIZE;
	r->string = malloc(r->length+1);
	memcpy(r->string, buffer, r->length);
	r->string[r->length] = 0;
}

int writerulepacket(void* buffer, unsigned short num, unsigned short max, const char* string)
{
	*((unsigned short*)buffer) = num;
	buffer+=2;
	*((unsigned short*)buffer) = max;
	buffer+=2;
	
	unsigned short size = strlen(string);
	size = size < BUFFERSIZE-SIZEOF_PACKETHEADER-RULEPACKETSIZE?size:BUFFERSIZE-SIZEOF_PACKETHEADER-RULEPACKETSIZE;
	*((unsigned short*)buffer) = size;
	buffer+=2;

	memcpy(buffer, string, size);
	return (size + 6);
}

