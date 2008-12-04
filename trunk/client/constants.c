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

void initunconstants()
{
	//these depend on the size of the allocation
	config_net(CONFIG_LOCALNET, &localip, &localmask);
	int cidr = masktocidr(localmask);
	int half = (32 - cidr)/2;
	MAPHEIGHT = 1 << half;
	if (cidr % 2)
		MAPWIDTH = MAPHEIGHT << 1;
	else
		MAPWIDTH = MAPHEIGHT;
	
	//these depend on the size of the screen
	screensize(1024, 1024);
}

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
	
