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
 * This file contains a definition for a data point. We will use this to figure
 * out how to draw a pixel. 
 */
#include "constants.h"

#define DATAPOINT_MSG_SIZE 128

struct datapoint {
	unsigned char tcpin;
	unsigned char udpin;
	unsigned char otherin;
	unsigned char tcpout;
	unsigned char udpout;
	unsigned char otherout;
	unsigned char drop;
	const char* msg;
};

void datapointtcp(struct datapoint * dp, int direction, unsigned int rate);
void datapointudp(struct datapoint * dp, int direction, unsigned int rate);
void datapointother(struct datapoint * dp, int direction, unsigned int rate);
void datapointdrop(struct datapoint * dp, unsigned int rate);

void datapointfade(struct datapoint * dp, int faderate);

void datapointdrawpixel(struct datapoint* dp, unsigned char* imgdata, int x, int y, int width);

