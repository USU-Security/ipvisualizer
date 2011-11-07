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

#include "flowdata.h"
#include <stdlib.h>
#include <string.h>

/* returns the number of bytes that are valid */
inline int flowpacketsize(struct flowpacket* f)
{
	return(4*sizeof(unsigned short) + sizeof(flowdata) * f->count);
}

/* returns the number of bytes that are valid */
inline int fwflowpacketsize(struct fwflowpacket* f)
{
	return(4*sizeof(unsigned short) + sizeof(fwflowdata) * f->count);
}

/* returns the number of bytes that are valid */
inline int vflowpacketsize(struct verbosefirewall* f)
{
	return(SIZEOF_VERBOSEHEADER + SIZEOF_VERBOSEDATA * f->count);
}

inline int subnetpacketsize(struct subnetpacket* s)
{
#ifdef VERBOSE_PACKET
	return(2*sizeof(unsigned short) + 2*sizeof(unsigned int) + sizeof(subnetword) * s->count);
#else
	return(2*sizeof(unsigned short) + sizeof(subnetword) * s->count);
#endif
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

