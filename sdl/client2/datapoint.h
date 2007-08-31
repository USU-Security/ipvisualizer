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

void datapointdrawpixel(struct datapoint* dp, unsigned char imgdata[][COLORDEPTH], int x, int y, int width);

