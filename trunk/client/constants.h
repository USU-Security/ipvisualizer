
#define COLORDEPTH 4 
#define DISPLAYSTRINGSIZE 512
#define MENUBUFLEN 120
#define MAXSUBNETS 1024

/* not constants anymore, but too lazy to change the name of the include file*/
extern unsigned int IMGWIDTH; /* 1024 */
extern unsigned int IMGHEIGHT; /* 1024 */
extern unsigned int MAPWIDTH; /* 256 */
extern unsigned int MAPHEIGHT; /* 256 */
extern float BLOCKWIDTH; /* IMGWIDTH/MAPWIDTH */
extern float BLOCKHEIGHT; /* IMGHEIGHT/MAPHEIGHT */
extern unsigned int localip;
extern unsigned int localmask;
extern unsigned short serverport;

/* variables cached globally */
extern unsigned int a_shift;
extern unsigned int b_mask;
extern unsigned int c_mask_rev;
extern unsigned int b_shift_rev;
extern unsigned int b_mask_rev;

/* call this function to initialize these variables */
void initunconstants();
