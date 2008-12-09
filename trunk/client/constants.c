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
 * initializes variables that used to be constants from the config file
 */
#include "../shared/config.h"
unsigned int IMGWIDTH; /* 1024 */
unsigned int IMGHEIGHT; /* 1024 */
unsigned int MAPWIDTH; /* 256 */
unsigned int MAPHEIGHT; /* 256 */
float BLOCKWIDTH; /* IMGWIDTH/MAPWIDTH */
float BLOCKHEIGHT; /* IMGHEIGHT/MAPHEIGHT */
unsigned int a_shift;
unsigned int b_mask;
unsigned int c_mask_rev;
unsigned int b_shift_rev;
unsigned int b_mask_rev;
unsigned int localip;
unsigned int localmask;
unsigned short serverport;


/* initializes the unconstants according the size of the screen */
void screensize(int w, int h)
{
	IMGWIDTH = w;
	IMGHEIGHT = h;
	BLOCKWIDTH = (float)IMGWIDTH/MAPWIDTH;
	BLOCKHEIGHT = (float)IMGHEIGHT/MAPHEIGHT;

	/* these are needed to compute the fwd and rev_netblock functions; */
	unsigned int netblock_width = MAPWIDTH/16;
	b_mask = (netblock_width - 1) << 8;
	/*             0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 */
	int log_2[] = {0,0,1,0,2,0,0,0,3,0,0,0,0,0,0,0,4};
	a_shift = log_2[netblock_width] + 8; //assumes that netblock_width isnt greater than 16

	c_mask_rev = (netblock_width - 1) << 4;
	b_shift_rev = log_2[netblock_width] + 4;
	b_mask_rev = 0xf << b_shift_rev;
/*	
	printf("a_shift: 0x%04x\n", a_shift);
	printf("b_mask: 0x%04x\n", b_mask);
	printf("c_mask_rev: 0x%04x\n", c_mask_rev);
	printf("b_shift_rev: 0x%04x\n", b_shift_rev);
	printf("b_mask_rev: 0x%04x\n", b_mask_rev);
*/

}
void setnet(unsigned int base, unsigned int mask)
{
	localmask = mask;
	localip = base;
	int cidr = masktocidr(localmask);
	int half = (32 - cidr)/2;
	MAPHEIGHT = 1 << half;
	if (cidr % 2)
		MAPWIDTH = MAPHEIGHT << 1;
	else
		MAPWIDTH = MAPHEIGHT;
	serverport = config_int(CONFIG_PORT);
	//these depend on the size of the screen
	screensize(1024, 1024);

}
void initunconstants()
{
	unsigned int i, m;
	//these depend on the size of the allocation
	config_net(CONFIG_LOCALNET, &i, &m);
	setnet(i, m);
}
	
