/*
 * file:	display.c
 * purpose:	to display the flow data in a usable format
 * created:	03-12-2007
 * creator:	rian shelley
 */


#include <GL/glut.h>
#include <GL/glu.h>
#include <sys/time.h>
#include <pthread.h>
#include "../shared/flowdata.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pixelmapping.h"
#include <netdb.h>
#include "video.h"
#include "subnets.h"
#include <unistd.h>

#define IMGWIDTH 1024
#define IMGHEIGHT 1024
#define MAPWIDTH 256
#define MAPHEIGHT 256
#define DISPLAYSTRINGSIZE 128
#define MENUBUFLEN 120
#define MAXSUBNETS 1024
//#define FULLSCREEN 1


int numsubnets=0;
subnet subnets[MAXSUBNETS];


#define BLOCKWIDTH IMGWIDTH/MAPWIDTH
#define BLOCKHEIGHT IMGHEIGHT/MAPHEIGHT

int FADERATE = 2;
int JUMPRATE = 100;
int SHOW_IN = 1;
int SHOW_OUT = 1;
int mapping = 0;
const char moviefile[] = "output.avi";

struct timeval inittime;
volatile int die=0;
volatile int dead=0;
pthread_mutex_t imglock = PTHREAD_MUTEX_INITIALIZER;

unsigned char imgdata[IMGWIDTH*IMGHEIGHT][COLORDEPTH] = {{0}};

int width, height;
float scalex, scaley;
char displaystring[DISPLAYSTRINGSIZE];
int displayx=0, displayy=0;
unsigned int serverip = 23597484;
char videostate=0;

/* prototypes */
void idle();
void display();
void* collect_data(void* p);
void initbuffer();
void reshape (int w, int h);
void mousemove(int x, int y);
void keyup(unsigned char k, int x, int y);
inline void setpixel(int w, unsigned char c, unsigned char v, char in);
inline unsigned char getpixel(int w, unsigned char c, char in);

unsigned short fwd_netblock(unsigned short);
unsigned short rev_netblock(unsigned short);
void update_menubar();
inline unsigned short mappixel(unsigned short ip);
/**
 * Where execution begins. Duh.
 * */
int main(int argc, char** argv)
{
	videoinit();
	int i;
	/* initialize the server, if it is given */
	if (argc >= 2)
	{
		struct hostent* he = gethostbyname(argv[1]);
		memcpy(&serverip, he->h_addr_list[0], sizeof(serverip));
	}
	/* computing the forward mapping from the reverse mapping */
	for (i = 0; i < 65536; i++)
		forwardmap[reversemap[i]] = i;
	
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	gettimeofday(&inittime, 0);
	initbuffer();
	glutInit(&argc, argv);
	glutInitWindowSize(IMGWIDTH,IMGHEIGHT);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("simple");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape); 
	/* glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); */
	glutIdleFunc(idle);
	/* glutKeyboardFunc(keydown); */
	glutKeyboardUpFunc(keyup); 
	/* glutSpecialFunc(specialkeydown); */
	/* glutSpecialUpFunc(specialkeyup); */
	glutPassiveMotionFunc(mousemove);

#ifdef FULLSCREEN
	/* switch to fullscreen mode */
	glutFullScreen();
#endif

	/* start up our thread that listens for data */
	int e;
	pthread_t pid;
	if ((e = pthread_create(&pid, NULL, collect_data, NULL)))
		printf("Error while creating thread: %s", strerror(e));
	else 
		glutMainLoop();
	return 0;
}

/*
 * function:	displaymsg()
 * purpose:		to display a string on the screen 
 * recieves:	a string
 */
void displaymsg(const char* s)
{
	strncpy(displaystring, s, DISPLAYSTRINGSIZE);
}



/*
 * function:	pulldata()
 * purpose:		to let the server know if we want more data. on first call, will also request subnets
 * recieves:	whether to start the data (1), or stop the data(0)
 */
void pulldata(char start)
{
	static int sock = 0;
	struct packetheader ph;
	struct flowrequest* fr = (struct flowrequeset*)ph.data;
	
	fr->flowon = start;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sin.sin_addr.s_addr = serverip;
	ph.version = VERSION;
	if (!sock)
	{
		ph.packettype = PKT_SUBNET;
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		sendto(sock, &ph, SIZEOF_PACKETHEADER, 0, (struct sockaddr*)&sin, sizeof(sin));
		printf("requesting subnet data\n");
		//the response will come... later
	}
	ph.packettype = PKT_FLOW;
	sendto(sock, &ph, sizeof(fr) + SIZEOF_PACKETHEADER, 0, (struct sockaddr*)&sin, sizeof(sin));
}

void videoon(char b)
{
	if (b && !videostate) {
		videostate = startvideo(moviefile, IMGWIDTH, IMGHEIGHT);
	} else if (!b && videostate) {
		endvideo();
		videostate = 0;
	}
}
		

void keyup(unsigned char k, int x, int y)
{
	switch (k) {
	case 'm': /* ip mapping */
		snprintf(displaystring, DISPLAYSTRINGSIZE, "Changed Mapping");
		mapping++;
		initbuffer();
		break;
	case 'f': /* fade rate */
		FADERATE+=3;
		FADERATE = FADERATE % 256;
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"Fade Rate set to %i", FADERATE);
		break;
	case 'F': /* fade rate */
		FADERATE-=3;
		FADERATE = (FADERATE + 256) % 256;
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"Fade Rate set to %i", FADERATE);
		break;
	case 'j': /* jump rate */
		JUMPRATE += 10;
		JUMPRATE = JUMPRATE % 256;
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"Jump Rate changed to %i",JUMPRATE );
		break;
	case 'J': /* jump rate */
		JUMPRATE -= 10;
		JUMPRATE = (JUMPRATE+256) % 256;
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"Jump Rate changed to %i",JUMPRATE );
		break;
	case 'c': /* clear the screen */
		initbuffer();
		break;
	case 'i':
		SHOW_IN = 1;
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"Showing incoming packets" );
		break;
	case 'I':
		SHOW_IN = 0;
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"Not showing incoming packets" );
		break;
	case 'o':
		SHOW_OUT = 1;
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"Showing outgoing packets" );
		break;
	case 'O':
		SHOW_OUT = 0;
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"Not showing outgoing packets" );
		break;
	case 'q':
	case 'Q':
		videoon(0);
		exit(0);
		break;
	case 'V':
		videoon(1);
		break;
	case 'v':
		videoon(0);
		break;
	}		
	mappixel(0); 
	update_menubar();
}


/*
 * function:	mappixel()
 * purpose:		maps from an ip to a pixel. 
 * recieves:	ip
 * returns:		pixel
 */
inline unsigned short mappixel(unsigned short ip)
{
	switch (mapping) {
	case 0: /* linear mapping */
		return ip;
	case 1: /* fractal mapping */
		return forwardmap[ip];
	case 2:
		return fwd_netblock(ip);
	}	
	/* reset to linear */
	mapping = 0;
	return ip;
}
inline unsigned short unmappixel(unsigned short ip)
{
	switch (mapping) {
	case 0: /*linear*/
		return ip;
	case 1:
		return reversemap[ip];
	case 2:
		return rev_netblock(ip);
	}
	mapping = 0;
	return ip;
}

/*
 * function:	mousemove()
 * purpose:	callback that gets called when the mouse moves
 * recieves:	the x and y coordinate of the mouse. woot. woot
 */
void mousemove(int x, int y)
{
	/* unmap the ip */
	x *= scalex;
	y *= scaley;
	y += .5;
	if (y < MAPHEIGHT/2)
		displayy = 0;
	else
		displayy =  IMGHEIGHT - 13;
	
	unsigned short ip = unmappixel(x|((MAPHEIGHT - y - 1)<<8));
	snprintf(displaystring, DISPLAYSTRINGSIZE, 
		"129.123.%i.%i", (ip & 0xff00)>>8, ip & 0xff);
}



/**
 * if the size of the window changes
 */
void reshape (int w, int h)
{
	width = w;
	height = h;
	scalex = (float)MAPWIDTH/w;
	scaley = (float)MAPHEIGHT/h;
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glPixelZoom((float)w/IMGWIDTH, (float)h/IMGHEIGHT);
	gluOrtho2D(0, IMGWIDTH, 0, IMGHEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


/*
 * function:	lineleft()
 * purpose:		to draw a line to the left of a pixel
 * recieves:	the x and y coords, and the blue, green and red components
 */
inline void lineleft(int i, int j, unsigned char b, unsigned char g, unsigned char r)
{
	imgdata[(i*4) + (j*4)*IMGWIDTH][0] = b;
	imgdata[(i*4) + (j*4)*IMGWIDTH][1] = g;
	imgdata[(i*4) + (j*4)*IMGWIDTH][2] = r;
	imgdata[(i*4) + ((j*4)+1)*IMGWIDTH][0] = b;
	imgdata[(i*4) + ((j*4)+1)*IMGWIDTH][1] = g;
	imgdata[(i*4) + ((j*4)+1)*IMGWIDTH][2] = r;
	imgdata[(i*4) + ((j*4)+2)*IMGWIDTH][0] = b;
	imgdata[(i*4) + ((j*4)+2)*IMGWIDTH][1] = g;
	imgdata[(i*4) + ((j*4)+2)*IMGWIDTH][2] = r;
	imgdata[(i*4) + ((j*4)+3)*IMGWIDTH][0] = b;
	imgdata[(i*4) + ((j*4)+3)*IMGWIDTH][1] = g;
	imgdata[(i*4) + ((j*4)+3)*IMGWIDTH][2] = r;
#if COLORDEPTH == 4
	imgdata[(i*4) + ((j*4)+3)*IMGWIDTH][3] = 255;
	imgdata[(i*4) + ((j*4)+2)*IMGWIDTH][3] = 255;
	imgdata[(i*4) + ((j*4)+1)*IMGWIDTH][3] = 255;
	imgdata[(i*4) + (j*4)*IMGWIDTH][3] = 255;
#endif
}

/*
 * function:	lineup() 
 * purpose:     to draw a line above a pixel
 * recieves:    the x and y coords, and the blue, green and red components
 */
inline void lineup(int i, int j, unsigned char b, unsigned char g, unsigned char r)
{
	imgdata[(i*4) + (j*4)*IMGWIDTH][0] = b;
	imgdata[(i*4) + (j*4)*IMGWIDTH][1] = g;
	imgdata[(i*4) + (j*4)*IMGWIDTH][2] = r;
	imgdata[((i*4)+1) + (j*4)*IMGWIDTH][0] = b;
	imgdata[((i*4)+1) + (j*4)*IMGWIDTH][1] = g;
	imgdata[((i*4)+1) + (j*4)*IMGWIDTH][2] = r;
	imgdata[((i*4)+2) + (j*4)*IMGWIDTH][0] = b;
	imgdata[((i*4)+2) + (j*4)*IMGWIDTH][1] = g;
	imgdata[((i*4)+2) + (j*4)*IMGWIDTH][2] = r;
	imgdata[((i*4)+3) + (j*4)*IMGWIDTH][0] = b;
	imgdata[((i*4)+3) + (j*4)*IMGWIDTH][1] = g;
	imgdata[((i*4)+3) + (j*4)*IMGWIDTH][2] = r;
#if COLORDEPTH == 4	
	imgdata[((i*4)+3) + (j*4)*IMGWIDTH][3] = 255;
	imgdata[((i*4)+2) + (j*4)*IMGWIDTH][3] = 255;
	imgdata[((i*4)+1) + (j*4)*IMGWIDTH][3] = 255;
	imgdata[(i*4) + (j*4)*IMGWIDTH][3] = 255;
#endif
}


/*
 * function:	outlinesubnets()
 * purpose:		to outline subnets on the map
 */
void outlinesubnets()
{
	int i, j;
	iptype t;
	for (i = 1; i < MAPWIDTH; i++)
	{
		for (j = 1; j < MAPHEIGHT; j++)
		{
			unsigned int ip = i + (j<< 8);
			ip = unmappixel(ip) + NETBASE;
			unsigned int ipleft = i-1 + (j << 8);
			ipleft = unmappixel(ipleft) + NETBASE;
			unsigned int ipup = i + ((j - 1)<< 8);
			ipup = unmappixel(ipup) + NETBASE;

			if (!comparenets(subnets, numsubnets, ip, ipleft, &t)) {
				if (t == ALLOCATED)
					lineleft(i, j, 128,128,128);
				else
					lineleft(i, j, 32,32,0);
			}
			if (!comparenets(subnets, numsubnets, ip, ipup, &t)) {
				if (t == ALLOCATED) 
					lineup(i, j, 128,128,128);
				else
					lineup(i, j, 32,32,0);
			}
		}
	}
	for (i = 1; i < MAPWIDTH; i++)
	{
		j = 0;
		unsigned int ip = i + (j<< 8);
		ip = unmappixel(ip) + NETBASE;
		unsigned int ipleft = i-1 + (j << 8);
		ipleft = unmappixel(ipleft) + NETBASE;

		if (!comparenets(subnets, numsubnets, ip, ipleft, &t)) {
			if (t == ALLOCATED) 
				lineleft(i, j, 128,128,128);
			else
				lineleft(i, j, 32,32,0);
		}
	}
	for (j = 1; j < MAPHEIGHT; j++)
	{
		i = 0;
		unsigned int ip = i + (j<< 8);
		ip = unmappixel(ip) + NETBASE;
		unsigned int ipup = i + ((j - 1)<< 8);
		ipup = unmappixel(ipup) + NETBASE;

		if (!comparenets(subnets, numsubnets, ip, ipup, &t)) {
			if (t == ALLOCATED) 
				lineup(i, j, 128,128,128);
			else
				lineup(i, j, 32, 32, 0);
		}
	}
}



/* 
 * function:	initbuffer()
 * purpose:		to initialize the buffer to something
 */
void initbuffer()
{
	int i;
	for (i = 0; i < IMGWIDTH*IMGHEIGHT; i++){
		imgdata[i][0] = 0;
		imgdata[i][1] = 0;
		imgdata[i][2] = 0;
#if COLORDEPTH==4
		imgdata[i][3] = 255;
#endif
	}
	outlinesubnets();
}


/*
 * function:	timer()
 * purpose:		to give us the time since the program began 
 */
double timer()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	tv.tv_sec -= inittime.tv_sec;
	tv.tv_usec -= inittime.tv_usec;
	return (double)tv.tv_sec + (.000001 * tv.tv_usec);
}

/*
 * function:	fade()
 * purpose:		to gradually fade the image after every pixel spike
 */
void fade()
{
	int i, j;
	/* lock */
	pthread_mutex_lock(&imglock);
	for (i = 0; i < MAPWIDTH*MAPHEIGHT; i++)
	{
		for (j = 0; j < 3; j++)
		{
			/* exponential decay */
			if (getpixel(i, j, 0))
				setpixel(i, j, getpixel(i, j, 0) * 1-FADERATE*.0002, 0);
			if (getpixel(i, j, 1))
				setpixel(i, j, getpixel(i, j, 1) * 1-FADERATE*.0002, 1);
#if 0
			if (!imgdata[i][j])
				continue;
			if (imgdata[i][j] < FADERATE)
				imgdata[i][j] = 0;
			else
				imgdata[i][j]-= FADERATE;
#endif
		}
	}
	pthread_mutex_unlock(&imglock);
	/* unlock */

}


/**
 * where idling happens
 */
void idle ()
{
	static float ti = 0;
	static int counter =0;
	if (dead)
	{
		/* our child died. the anguish is too much*/
		exit(0);
	}

	if (timer() - ti > .041666666666666666666)
	{
		fade();
		glutPostRedisplay();
		ti = timer();
		counter++;
		if (counter >= 24 * HEARTBEAT) 
		{
			pulldata(1);
			counter = 0;
		}
	}
}

void renderstring(const char* s)
{
	while (*s)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *(s++));
}

/*
 * where drawing happens
 */
void display()
{
	/*
	 * its a right-handed coordinate system, not that we care. 
	 * 0,0 is the bottom left-hand corner
	 */
	glRasterPos2i(0,0);

	/* insert lock here */
	pthread_mutex_lock(&imglock);
	if (videostate)
		writeframe(imgdata, IMGWIDTH, IMGHEIGHT);
#if (COLORDEPTH==4)
	glDrawPixels(IMGWIDTH, IMGHEIGHT, GL_BGRA, GL_UNSIGNED_BYTE, imgdata); 
#else
	glDrawPixels(IMGWIDTH, IMGHEIGHT, GL_BGR, GL_UNSIGNED_BYTE, imgdata); 
#endif
	pthread_mutex_unlock(&imglock);
	/* insert unlock here */
	
	glRasterPos2i(displayx,displayy);
	renderstring(displaystring);

	glutSwapBuffers();
}

/*
 * function:	setpixel()
 * purpose:		to set the color of a pixel
 * recieves:	which pixel, and which color component, and what intensity to set it at
 */
inline void setpixel(int w, unsigned char c, unsigned char v, char in)
{
	int x = w % MAPWIDTH;
	int y = w / MAPWIDTH;

	x*= BLOCKWIDTH;
	y*= BLOCKHEIGHT;
	int i, j;
	if (in) {
		for (i = x+1; i < x + BLOCKWIDTH; i++)
		{
			for (j = y+ (i - x); j < y + BLOCKHEIGHT; j++)
			{
				imgdata[j * IMGWIDTH + i][c] = v;
			}
		}
	}
	else 
	{
		for (i = x+1; i < x + BLOCKWIDTH; i++)
		{
			for (j = y + 1; j <= y +  (i - x); j++)
			{
				imgdata[j * IMGWIDTH + i][c] = v;
			}
		}
		
	}
}

/*
 * function:	getpixel()
 * purpose:		to set the color of a pixel
 * recieves:	which pixel, and which color component, and what intensity to set it at
 */
inline unsigned char getpixel(int w, unsigned char c, char in)
{
	int x = w % MAPWIDTH;
	int y = w / MAPWIDTH;

	x*= BLOCKWIDTH;
	y*= BLOCKHEIGHT;
	x++;
	y++;
	if (in) 
		return imgdata[(y+BLOCKHEIGHT-2) * IMGWIDTH + x][c];
	return imgdata[y * IMGWIDTH + x+ BLOCKHEIGHT - 2][c];
}

/*
 * function:	update_image()
 * purpose:		to update the image according to a given flowpacket
 * recieves:	the flow packet
 */
void update_image(struct flowpacket* fp) 
{
	int i;
	int max = fp->count;
	if (max > MAXINDEX)
		max = MAXINDEX; /* watch those externally induced overflows */
	/* lock it up */
	pthread_mutex_lock(&imglock);
	for (i = 0; i < max; i++)
	{
		if (fp->data[i].incoming && !SHOW_IN)
			continue;
		if (!fp->data[i].incoming && !SHOW_OUT)
			continue;

		int pixel = mappixel(fp->data[i].ip);

		unsigned char b = getpixel(pixel, fp->data[i].packet, fp->data[i].incoming);
		/*
		   printf("jumping pixel for ip 129.123.%i.%i\n",
		   (fp.data[i].ip & 0x0000ff00) >> 8,
		   fp.data[i].ip & 0x000000ff);
		   */
		if (b < 255 - JUMPRATE) /* don't overflow */
			setpixel(pixel, fp->data[i].packet, getpixel(pixel, fp->data[i].packet, fp->data[i].incoming) + JUMPRATE, fp->data[i].incoming);
		else
			setpixel(pixel, fp->data[i].packet, 255, fp->data[i].incoming);
	}
	pthread_mutex_unlock(&imglock);
	/* unlock it */

}

/*
 * function:	updatesubnets()
 * purpose:		to update our list of subnets
 * recieves:	a poitner to a subnetpacket
 */
void updatesubnets(struct subnetpacket* sp)
{
	int i;
	int max = sp->count>MAXINDEX ? MAXINDEX : sp->count;
	numsubnets = max;
	printf("Recived information about %i subnets\n", numsubnets); 
	for (i = 0; i < numsubnets; i++)
	{
		subnets[i].base = sp->subnets[i].base + NETBASE;
		subnets[i].mask = sp->subnets[i].mask;
	}
	/* redraw the subnet stuff */
	outlinesubnets();

}

/*
 * function:	collect_data()
 * purpose:		collects data to draw. this is a separate thread because i didn't
 * 				want to use glut callback foo to do this, its too risky.
 */
void* collect_data(void* p)
{
	dead = 0;
	/* start listening for data */
	int s;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);

	addr.sin_addr.s_addr = INADDR_ANY;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	if (!s)
	{
		dead = 1;
		return 0;
	}

	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)))
	{
		dead = 1;
		return 0;
	}

	struct packetheader ph;
	int i;
	while (!die)
	{
		read(s, &ph, sizeof(ph));
		switch (ph.packettype)
		{
		case PKT_FLOW:	
			update_image((struct flowpacket*)ph.data);
			break;
		case PKT_SUBNET:
			updatesubnets((struct subnetpacket*)ph.data);
			break;
		default:
			printf("Recieved garbage packet of type %i (0x%2X)\n", (unsigned int)ph.packettype, (unsigned int)ph.packettype);
		}		
	}
	dead = 1;
	return 0;
}

unsigned short fwd_netblock(unsigned short num)
{
	unsigned short x_inner, y_inner, x_outer, y_outer, mapped;
	x_inner = num % 16;
	x_outer = num / 16 % 16;
	y_inner = num / 256 % 16;
	y_outer = num / 4096 % 16;

	mapped = y_outer * 4096 + x_outer * 256 + y_inner * 16 + x_inner;

	return mapped;
}

unsigned short rev_netblock(unsigned short num)
{
	unsigned short x_inner, y_inner, x_outer, y_outer, mapped;
	x_inner = num % 16;
	y_inner = num / 16 % 16;
	x_outer = num / 256 % 16;
	y_outer = num / 4096 % 16;

	mapped = y_outer * 4096 + y_inner * 256 + x_outer * 16 + x_inner;

	return mapped;
	
}

void update_menubar()
{
	char menubuf[MENUBUFLEN];
	char *map_string;

	switch(mapping)
	{
		case 0:
			map_string = "linear";
			break;
		case 1:
			map_string = "\"fractal\"";
			break;
		case 2:
			map_string = "blocks";
			break;
		default:
			map_string = "";
	}


	snprintf(menubuf, MENUBUFLEN, "Showing: %9s %9s - Mapping: %8s - Fade:"
			" %2d - Jump: %3d", SHOW_IN?"incoming":"",
			SHOW_OUT?"outgoing":"", map_string, FADERATE,
			JUMPRATE);

	glutSetWindowTitle(menubuf);
	return;
}
