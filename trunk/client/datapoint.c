/*
 * implementation of the functions used to deal with a datapoint
 */

#include "datapoint.h"

/*
 * These increase the value of the data point as defined in JUMPRATE
 */
void datapointtcp(struct datapoint * dp, int direction, unsigned int rate)
{
	if (direction) 
	{
		if (dp->tcpin + rate > 255)
			dp->tcpin = 255;
		else
			dp->tcpin += rate;
	}
	else
	{	
		if (dp->tcpout + rate > 255)
			dp->tcpout = 255;
		else
			dp->tcpout += rate;
	}
}
void datapointudp(struct datapoint * dp, int direction, unsigned int rate)
{
	if (direction) 
	{
		if (dp->udpin + rate > 255)
			dp->udpin = 255;
		else
			dp->udpin += rate;
	}
	else
	{	
		if (dp->udpout + rate > 255)
			dp->udpout = 255;
		else
			dp->udpout += rate;
	}

}
void datapointother(struct datapoint * dp, int direction, unsigned int rate)
{
	if (direction) 
	{
		if (dp->otherin + rate > 255)
			dp->otherin = 255;
		else
			dp->otherin += rate;
	}
	else
	{	
		if (dp->otherout + rate > 255)
			dp->otherout = 255;
		else
			dp->otherout += rate;
	}

}

/*
 * this one works a little differently. we want to display full intensity immediately, then drop off fast
 */
void datapointdrop(struct datapoint * dp, unsigned int rate)
{
	dp->drop = rate;
}


/*
 * this will decrease the intensity
 */
void datapointfade(struct datapoint* dp, int faderate)
{
	/* exponential decay. looks better */
	dp->tcpin *= (1-faderate*.0002);
	dp->udpin *= (1-faderate*.0002);
	dp->otherin *= (1-faderate*.0002);
	dp->tcpout *= (1-faderate*.0002);
	dp->udpout *= (1-faderate*.0002);
	dp->otherout *= (1-faderate*.0002);
	//dp->drop *= .5; /* we want this one to fade faster */
	dp->drop *= (1-faderate*.0075); /* we want this one to fade faster */

	if ((!dp->drop) && dp->msg)
	{
		/* oops, most of these will be the same set of rules. im so stupid */
		/*free(dp->msg); */ 
		dp->msg = 0;
	}
}

/* 
 * this function will draw a bitmap representing the current pixel
 */
void datapointdrawpixel(struct datapoint* dp, unsigned char* imgdata, int x, int y, int width)
{
	int i, j;
	x*=BLOCKWIDTH;
	y*=BLOCKHEIGHT;
	float ratio = (float)BLOCKWIDTH/BLOCKHEIGHT;
	
	for (i = x+1; i < x + BLOCKWIDTH; i++) 
	{
		for (j = y+1; j < y + BLOCKHEIGHT; j++) 
		{
			if (i-x > (j-y)*ratio) 
			{
				imgdata[(i+j*width)*COLORDEPTH + 2] = dp->udpin;
				imgdata[(i+j*width)*COLORDEPTH + 1] = dp->tcpin;
				imgdata[(i+j*width)*COLORDEPTH + 0] = dp->otherin;
			} 
			else 
			{
				imgdata[(i+j*width)*COLORDEPTH + 2] = dp->udpout;
				imgdata[(i+j*width)*COLORDEPTH + 1] = dp->tcpout;
				imgdata[(i+j*width)*COLORDEPTH + 0] = dp->otherout;
			}
			float d1 = (i-x) - (j-y)*ratio;
			float d2 = (i-x) - (BLOCKHEIGHT-(j-y))*ratio;
			if ((d1 > -1 && d1 < 1) || (d2 > -1 && d2 < 1)) 
			{
				if (imgdata[(i+j*width)*COLORDEPTH+2] + dp->drop > 255)
					imgdata[(i+j*width)*COLORDEPTH+2] = 255;
				else
					imgdata[(i+j*width)*COLORDEPTH+2] += dp->drop;
				if (imgdata[(i+j*width)*COLORDEPTH+1] + dp->drop > 255)
					imgdata[(i+j*width)*COLORDEPTH+1] = 255;
				else
					imgdata[(i+j*width)*COLORDEPTH+1] += dp->drop;
				if (imgdata[(i+j*width)*COLORDEPTH+0] + dp->drop > 255)
					imgdata[(i+j*width)*COLORDEPTH+0] = 255;
				else
					imgdata[(i+j*width)*COLORDEPTH+0] += dp->drop;
			}
		}
	}
}

