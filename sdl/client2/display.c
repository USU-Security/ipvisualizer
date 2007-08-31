/*
 * file:	display.c
 * purpose:	to display the flow data in a usable format
 * created:	03-12-2007
 * creator:	rian shelley
 */


#include <SDL/SDL.h>
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
#include "text.h"
#include "constants.h"
//#define FULLSCREEN 1


int FADERATE = 2;
int JUMPRATE = 100;
/* this needs the defines given above. retarted, i know */
#include "datapoint.h"


int numsubnets=0;
subnet subnets[MAXSUBNETS];

int SHOW_IN = 1;
int SHOW_OUT = 1;
int mapping = 0;
const char moviefile[] = "output.avi";

struct timeval inittime;
volatile int die=0;
volatile int dead=0;
pthread_mutex_t imglock = PTHREAD_MUTEX_INITIALIZER;

unsigned char imgdata[IMGWIDTH*IMGHEIGHT][COLORDEPTH] = {{0}};
struct datapoint pointdata[MAPWIDTH*MAPHEIGHT];

int width, height;
float scalex, scaley;
char displaystring[DISPLAYSTRINGSIZE];
int displayx=0, displayy=0;
unsigned int serverip = 23597484;
SDL_Surface * screen;
char videostate=0;
const char* rules[65536] = {0};

/* prototypes */
int  idle();
void display();
void* collect_data(void* p);
void initbuffer();
void reshape (int w, int h);
void mousemove(int x, int y);
void keyup(unsigned char k, int x, int y);

unsigned short fwd_netblock(unsigned short);
unsigned short rev_netblock(unsigned short);
void update_menubar();
inline unsigned short mappixel(unsigned short ip);
void sdl_eventloop();

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	gettimeofday(&inittime, 0);
	initbuffer();
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	/* get a video mode */
	
	screen = SDL_SetVideoMode(512, 512, 32, SDL_RESIZABLE|SDL_OPENGL);
	
	//screen = SDL_SetVideoMode(512, 512, 32, SDL_RESIZABLE|SDL_OPENGL);
	//screen = SDL_SetVideoMode(modes[0]->w, modes[0]->h, 32,       SDL_FULLSCREEN|SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_OPENGL);

	if (!screen) 
	{
		printf("Unable to create the screen\n");
		return 1;
	}
	
//	reshape(modes[0]->w, modes[0]->h);	
	reshape(screen->w, screen->h);	

	/* start up our thread that listens for data */
	int e;
	pthread_t pid;
	if ((e = pthread_create(&pid, NULL, collect_data, NULL)))
		printf("Error while creating thread: %s", strerror(e));
	else 
		sdl_eventloop();

	SDL_Quit();
	return 0;
}

/*
 * function:	sdl_eventloop()
 * purpose:		to listen for sdl events
 */
void sdl_eventloop()
{
	SDL_Event event;
	while (!dead)
	{ 
		if (SDL_PollEvent(&event)) 
		{
			switch (event.type)
			{
			case SDL_KEYUP:
				keyup(event.key.keysym.sym, 0, 0);				
				/* keypresses */
				break;
			case SDL_QUIT:
				dead= 1;
				break;
			case SDL_VIDEORESIZE:
				screen = SDL_SetVideoMode(event.resize.w, event.resize.h,  32, SDL_RESIZABLE|SDL_OPENGL);
				reshape(event.resize.w, event.resize.h);
				break;
			case SDL_MOUSEMOTION:
				mousemove(event.motion.x, event.motion.y);
				break;
			default:
				break;
			}
		}
		else 
		{
			/* idle will sleep if it can tell we have lots of time to spare */
			if (idle())
				display();		
		}
	}
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
		ph.packettype = PKT_FWRULE;
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		sendto(sock, &ph, SIZEOF_PACKETHEADER, 0, (struct sockaddr*)&sin, sizeof(sin));
		printf("requesting firewall rule data\n");
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
		

void togglefullscreen() 
{
	static int fullscreen = 0;
	if (fullscreen) 
	{
		screen = SDL_SetVideoMode(512, 512, 32, SDL_RESIZABLE|SDL_OPENGL);
		reshape(512, 512); 
		fullscreen = 0;
	}
	else
	{
		SDL_Rect ** modes = SDL_ListModes(NULL, SDL_FULLSCREEN);

		if (!modes) 
		{
			printf("unable to obtain a list of video modes\n");
		}
		else 
		{
			screen = SDL_SetVideoMode(modes[0]->w, modes[0]->h, 32, SDL_RESIZABLE|SDL_OPENGL|SDL_FULLSCREEN);
			reshape(modes[0]->w, modes[0]->h);
			fullscreen = 1;
		}
	}
	SDL_WM_GrabInput(SDL_GRAB_OFF);
}


void keyup(unsigned char k, int x, int y)
{
	/* get the keyboard state */
	Uint8 *keys = SDL_GetKeyState(NULL);
	switch (k) {
	case SDLK_m: /* ip mapping */
		snprintf(displaystring, DISPLAYSTRINGSIZE, "Changed Mapping");
		mapping++;
		initbuffer();
		break;
	case SDLK_f: /* fade rate */
		if (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) {
			FADERATE-=3;
			FADERATE = (FADERATE + 256) % 256;
			snprintf(displaystring, DISPLAYSTRINGSIZE, 
				"Fade Rate set to %i", FADERATE);
		} else {
			FADERATE+=3;
			FADERATE = FADERATE % 256;
			snprintf(displaystring, DISPLAYSTRINGSIZE, 
				"Fade Rate set to %i", FADERATE);
		}
		break;
	case SDLK_j: /* jump rate */
		if (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) {
			JUMPRATE -= 10;
			JUMPRATE = (JUMPRATE+256) % 256;
			snprintf(displaystring, DISPLAYSTRINGSIZE, 
				"Jump Rate changed to %i",JUMPRATE );
		} else {
			JUMPRATE += 10;
			JUMPRATE = JUMPRATE % 256;
			snprintf(displaystring, DISPLAYSTRINGSIZE, 
				"Jump Rate changed to %i",JUMPRATE );
		}
		break;
	case SDLK_c: /* clear the screen */
		initbuffer();
		break;
	case 'I':
		if (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) {
			SHOW_IN = 0;
			snprintf(displaystring, DISPLAYSTRINGSIZE, 
				"Not showing incoming packets" );
		} else {
			SHOW_IN = 1;
			snprintf(displaystring, DISPLAYSTRINGSIZE, 
				"Showing incoming packets" );
		}
		break;
	case SDLK_o:
		if (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) {
			SHOW_OUT = 0;
			snprintf(displaystring, DISPLAYSTRINGSIZE, 
		 		"Not showing outgoing packets" );
		} else {
			SHOW_OUT = 1;
			snprintf(displaystring, DISPLAYSTRINGSIZE, 
				"Showing outgoing packets" );
		}
		break;
	case SDLK_q:
		videoon(0);
		exit(0);
		break;
	case SDLK_v:
		if (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) 
			videoon(1);
		else
			videoon(0);
		break;
	case SDLK_RETURN:
		if (keys[SDLK_RALT] || keys[SDLK_LALT])
			togglefullscreen();
		break;
	default:
		printf("Not handling key 0x%2X '%s'\n", (int)k, SDL_GetKeyName(k));
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
	
	unsigned short mappixel = x|((MAPHEIGHT - y - 1)<<8);
	unsigned short ip = unmappixel(mappixel);
	if (pointdata[mappixel].msg)
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"129.123.%i.%i %s", (ip & 0xff00)>>8, ip & 0xff, pointdata[mappixel].msg );
	else 
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
	memset(pointdata, 0, sizeof(pointdata));
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
		datapointfade(&pointdata[i], FADERATE);
	}
	pthread_mutex_unlock(&imglock);
	/* unlock */

}


/**
 * where idling happens
 * returns:	true if the screen should be redrawn
 */
int idle()
{
	static float ti = 0;
	static int counter =0;
	if (dead)
	{
		/* our child died. the anguish is too much*/
		exit(0);
	}

	double now = timer();

	if (now - ti < FRAMERATE)
		/* sleep the remaining time */
		usleep(((FRAMERATE - (timer()-ti)) * 1000000));
	
	fade();
	ti = timer();
	counter++;
	if (counter >= 24 * HEARTBEAT) 
	{
		pulldata(1);
		counter = 0;
		//int i;
		//for (i = 0; i < 1024; i++)
		//	datapointdrop(&pointdata[i], 255);
	}
	return 1;
	 
	return 0;
}

inline void renderstring(const char* s)
{
/*	while (*s)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *(s++)); */
	/* gimp_image is a struct that contains our bitmap */
	/* the bitmap is ascii characters 32-127 arranged vertically */
	/*
	const int pixheight = gimp_image.height / (127-32);
	while (*s)
	{
		glRasterPos2d(x, y);
		x += gimp_image.width;
		glDrawPixels(gimp_image.width, 
					pixheight, 
					gimp_image.bytes_per_pixel == 4 ? GL_RGBA : GL_RGB,
					GL_UNSIGNED_BYTE,
					gimp_image.pixel_data + (*(s++) - 32) * (pixheight * gimp_image.width * gimp_image.bytes_per_pixel)
					);
	}
	*/
	/* attempt #4 */
	glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
	glPixelStorei( GL_UNPACK_SWAP_BYTES,  GL_FALSE );
	glPixelStorei( GL_UNPACK_LSB_FIRST,   GL_FALSE );
	glPixelStorei( GL_UNPACK_ROW_LENGTH,  0        );
	glPixelStorei( GL_UNPACK_SKIP_ROWS,   0        );
	glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0        );
	glPixelStorei( GL_UNPACK_ALIGNMENT,   1        );

	while (*s)
	{
		glBitmap(8, 13,
			0, 0, 
			8, 0,
			Fixed8x13Font[*(s++)-32] + 1
			);
	}
	glPopClientAttrib();
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
	/* set up the buffer */
	int i, j;
	for (i = 0; i < MAPWIDTH; i++)
		for (j = 0; j < MAPHEIGHT; j++)
			datapointdrawpixel(&pointdata[i + j*MAPWIDTH], imgdata, i, j, MAPWIDTH);
			
	if (videostate)
		writeframe(imgdata, IMGWIDTH, IMGHEIGHT);
#if (COLORDEPTH==4)
	glDrawPixels(IMGWIDTH, IMGHEIGHT, GL_BGRA, GL_UNSIGNED_BYTE, imgdata); 
#else
	glDrawPixels(IMGWIDTH, IMGHEIGHT, GL_BGR, GL_UNSIGNED_BYTE, imgdata); 
#endif
	pthread_mutex_unlock(&imglock);
	if (glGetError()) {
		printf("errors drawing to the screen\n");
		return 1;
	}
	/* insert unlock here */
	glRasterPos2i(displayx, displayy);	
	renderstring(displaystring);

	SDL_GL_SwapBuffers();
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

		/*
		   printf("jumping pixel for ip 129.123.%i.%i\n",
		   (fp.data[i].ip & 0x0000ff00) >> 8,
		   fp.data[i].ip & 0x000000ff);
		   */
		switch (fp->data[i].packet)
		{
		case UDP:
			datapointudp(&pointdata[pixel], fp->data[i].incoming, JUMPRATE);
			break;
		case TCP:
			datapointtcp(&pointdata[pixel], fp->data[i].incoming, JUMPRATE);
			break;
		case OTHER:
			datapointother(&pointdata[pixel], fp->data[i].incoming, JUMPRATE);
			break;
		}
		
	}
	pthread_mutex_unlock(&imglock);
	/* unlock it */

}
/*
 * function:	update_firewall()
 * purpose:		to update the image according to a given flowpacket
 * recieves:	the flow packet
 */
void update_firewall(struct fwflowpacket* fp) 
{
	int i;
	int max = fp->count;
	if (max > MAXINDEX)
		max = MAXINDEX; /* watch those externally induced overflows */
	/* lock it up */
	pthread_mutex_lock(&imglock);
	for (i = 0; i < max; i++)
	{
		int pixel = mappixel(fp->data[i].ip);

		/*
		   printf("jumping pixel for ip 129.123.%i.%i\n",
		   (fp.data[i].ip & 0x0000ff00) >> 8,
		   fp.data[i].ip & 0x000000ff);
		   */
		datapointdrop(&pointdata[pixel], 192);
		pointdata[pixel].msg = rules[fp->data[i].rule];
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
 * this function will interpret and use a rule packet
 */
void update_rule(void *buffer) 
{
	struct fwrulepacket rp;
	readrulepacket(buffer, &rp);
	if (rules[rp.num])
		free(rules[rp.num]);
	rules[rp.num] = rp.string;
	printf("Recieved rule %hu of %hu: %s\n", rp.num + 1, rp.max, rp.string);
}

/*
 * function:	collect_data()
 * purpose:		collects data to draw. this is a separate thread
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
		case PKT_FIREWALL:
			update_firewall((struct fwflowpacket*)ph.data);
			break;
		case PKT_FWRULE:
			update_rule(ph.data);
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

	/* glutSetWindowTitle(menubuf); */
	return;
}
