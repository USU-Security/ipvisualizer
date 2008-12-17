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
 * file:	display.c
 * purpose:	to display the flow data in a usable format
 * created:	03-12-2007
 * creator:	rian shelley
 * blarg
 */

#include <SDL/SDL.h>
#include <GL/glu.h>
#include <sys/time.h>
#include <pthread.h>
#include "../shared/flowdata.h"
#include "../shared/config.h"
#include "../config.h" /* the generated includes */
#if HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT
const char moviefile[] = "output.avi";
#include "video.h"
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pixelmapping.h"
#include <netdb.h>
#include "subnets.h"
#include <unistd.h>
#include "text.h"
#include "constants.h"
#include <sched.h>
#include <errno.h>
//#define FULLSCREEN 1


int FADERATE = 2;
int JUMPRATE = 100;
/* this needs the defines given above. retarted, i know */
#include "datapoint.h"


int numsubnets=0;
subnet subnets[MAXSUBNETS];

int SHOW_IN = 1;
int SHOW_OUT = 1;
int mapping = 2;
int pausedisplay = 0;
int displayrules = 1;
int sock = 0;

struct timeval inittime;
volatile int die=0;
volatile int dead=0;
pthread_mutex_t imglock = PTHREAD_MUTEX_INITIALIZER;

unsigned char *imgdata = NULL;
struct datapoint* pointdata = NULL;

int width, height;
float scalex, scaley;
char displaystring[DISPLAYSTRINGSIZE];
int displayx=0, displayy=0;
unsigned int serverip = 0x0100007f; //default of 127.0.0.1: nonfunctional until ports change
SDL_Surface * screen;
char videostate=0;
const char* rules[65536] = {0};
#define DISPLAYLINES 120
#define DISPLAYWIDTH 120
#define DISPLAYFADE .05
struct t_displaytext 
{
	char log[DISPLAYLINES][DISPLAYWIDTH+30];
	float brightness[DISPLAYLINES];
	int dup[DISPLAYLINES];
	int dir[DISPLAYLINES];
	int scroll;
	int which;
} displaytext;
GLuint texture;
int highlighting = 0;
struct {
	int x1,x2, y1, y2;
} subnetarea, highlightarea = {0,0,0,0};
unsigned short selectedbase, selectedmask;
pthread_t pid;
struct ipaddress {
	union {
		unsigned int i;
		unsigned char a[4];
	} ip;
} mouseip;

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
void printdisplay(const char* s, unsigned short ip);
void allocatescreen();
void pulldata(char start);

/**
 * Where execution begins. Duh.
 * */
int main(int argc, char** argv)
{
	config_loadfile("/etc/ipvisualizer.conf");
	config_loadargs(argc, argv);
	//get values for the variables that used to be constants from the config.
	initunconstants();
	allocatescreen();
#if HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT
	videoinit();
#endif
	int i;
	
	struct hostent* he = gethostbyname(config_string(CONFIG_SERVER));
	if (!he)
	{
		printf("Unable to lookup %s. Please check the config file\n", config_string(CONFIG_SERVER));
		return 1;
	}
	memcpy(&serverip, he->h_addr_list[0], sizeof(serverip));

	/* computing the forward mapping from the reverse mapping */
	for (i = 0; i < 65536; i++)
		forwardmap[reversemap[i]] = i;
	
	/* initialize the onscreen log */
	for (i = 0; i < DISPLAYLINES; i++)
	{
		memset(&displaytext.log[i][0], 0, DISPLAYWIDTH);
		displaytext.brightness[i] = 0;
		displaytext.dup[i] = 0;
	}
	displaytext.which = DISPLAYLINES-1;

	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	/* get a video mode */
	
	screen = SDL_SetVideoMode(1024, 1024, 32, SDL_RESIZABLE|SDL_OPENGL);
	SDL_WM_SetCaption(config_string(CONFIG_LOCALNET), "ipvisualizer");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	gettimeofday(&inittime, 0);
	initbuffer();
	
	//screen = SDL_SetVideoMode(512, 512, 32, SDL_RESIZABLE|SDL_OPENGL);
	//screen = SDL_SetVideoMode(modes[0]->w, modes[0]->h, 32,       SDL_FULLSCREEN|SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_OPENGL);

	if (!screen) 
	{
		printf("Unable to create the screen: %s", SDL_GetError());
		return 1;
	}
	
//	reshape(modes[0]->w, modes[0]->h);	
	reshape(screen->w, screen->h);	

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* request the initial data */
	pulldata(1);

	/* start up our thread that listens for data */
	int e;
	if ((e = pthread_create(&pid, NULL, collect_data, NULL)))
		printf("Error while creating thread: %s", strerror(e));
	else 
		sdl_eventloop();

	SDL_Quit();
	return 0;
}

void findcommonsubnet(unsigned short ip1, unsigned short ip2, unsigned short* base, unsigned short* mask)
{
	*mask = 0xff00;
	while (*mask && ((*mask & ip1) != (*mask & ip2)))
		*mask <<= 1;
	*base = ip1 & *mask;
}

/* dynamically allocates structures that keep track of screen pixels */
void allocatescreen()
{
	/*
	unsigned char imgdata[IMGWIDTH*IMGHEIGHT][COLORDEPTH] = {{0}};
	struct datapoint pointdata[MAPWIDTH*MAPHEIGHT];
	*/
	if (imgdata)
		free(imgdata);
	imgdata = (unsigned char*)malloc(IMGWIDTH*IMGHEIGHT*COLORDEPTH);
	memset(imgdata, 0, IMGWIDTH * IMGHEIGHT * COLORDEPTH);
	if (pointdata)
		free(pointdata);
	pointdata = (struct datapoint*)malloc(MAPWIDTH*MAPHEIGHT*sizeof(struct datapoint));
}
void stop_thread()
{
	die = 1;
	while(!dead)
		sched_yield();
	void* ret;
	pthread_join(pid, &ret);
}
void restart_thread()
{
	dead = 0;
	die = 0;
	if (pthread_create(&pid, NULL, collect_data, NULL))
		exit(0);
}

void mousedown(SDL_MouseButtonEvent* e)
{
	if (e->button == SDL_BUTTON_LEFT || e->button == SDL_BUTTON_RIGHT) 
	{
		highlightarea.x2 = highlightarea.x1 = e->x /(float)width *IMGWIDTH;
		highlightarea.y2 = highlightarea.y1 = IMGHEIGHT - e->y / (float)height * IMGHEIGHT;
		highlighting = e->button;
	}
	else if (e->button == SDL_BUTTON_WHEELUP)
	{
		//zoom in... need to figure out whether to add a 1 or a zero based
		//on the position
		if (MAPWIDTH == MAPHEIGHT)
		{
			if (e->y < height/2)
			{
				unsigned int newmask = 0x80000000 | (localmask >> 1);
				unsigned int newbit = newmask & (~localmask);
				localip |= newbit;
				localmask = newmask;
			}
			else
			{
				localmask = 0x80000000 | (localmask >> 1);
			}
		}
		else
		{
			if (e->x > width/2)
			{
				unsigned int newmask = 0x80000000 | (localmask >> 1);
				unsigned int newbit = newmask & (~localmask);
				localip |= newbit;
				localmask = newmask;
			}
			else
			{
				localmask = 0x80000000 | (localmask >> 1);
			}
		}

		if (localmask != 0xffffff00)
        {
			stop_thread(); /* prevent race condition */
			setnet(localip|selectedbase, localmask | selectedmask);
			screensize(1024, 1024);
			allocatescreen();
			initbuffer();
			reshape(width, height);
			restart_thread();
		}
	}
	else if (e->button == SDL_BUTTON_WHEELDOWN)
	{
		/* only zoom out if we have bits left to discard */
		if ((localmask & 0xffff) != 0)
		{
			stop_thread();
			unsigned int newbase = localip & (localmask << 1);
			setnet(newbase, localmask << 1);
			screensize(1024, 1024);
			allocatescreen();
			initbuffer();
			reshape(width, height);
			restart_thread();
		}
	}
}
void mouseup(SDL_MouseButtonEvent* e)
{
	if (e->button == SDL_BUTTON_LEFT && highlighting == SDL_BUTTON_LEFT)
	{
		highlighting = 0;
		if (highlightarea.x1 != highlightarea.x2 && highlightarea.y1 != highlightarea.y2)
		{
			/* only zoom in if we aren't smaller than a /24 */
			if (localmask != 0xffffff00)
			{
				stop_thread(); /* prevent race condition */
				setnet(localip|selectedbase, localmask | selectedmask);
				screensize(1024, 1024);
				allocatescreen();
				initbuffer();
				reshape(width, height);
				restart_thread();
			}
		}
		else
		{
			/* a normal left click */
			if (config_string(CONFIG_LEFTCLICK))
			{
				printf("Beginning fork\n");
				if (fork() == 0)
				{
					printf("\tI am the child fork\n");
					char displaystring[32];
					snprintf(displaystring, 32, "%i.%i.%i.%i", (int)mouseip.ip.a[3], (int)mouseip.ip.a[2], (int)mouseip.ip.a[1], (int)mouseip.ip.a[0]);
					

					int l = strlen(config_string(CONFIG_LEFTCLICK)) + 33;
					char s[l];

					snprintf(s, l, config_string(CONFIG_LEFTCLICK), displaystring);
					//execlp("sh", "-c", s);
					if (-1 == system(s))
						printf("Unable to spawn child: %s", strerror(errno));
					_exit(0); /* exec failed, need to quit */
				}
				printf("\tI am the parent fork\n");
			}
			else
			{
				printf("An ip has been clicked using the left mouse button.\nIf leftclickcmd was specified in the config, it would be executed now.\n");
			}
		}
	}
	else if (e->button == SDL_BUTTON_RIGHT && highlighting == SDL_BUTTON_RIGHT)
	{
		highlighting = 0;
		unsigned int cidr = masktocidr(localmask | selectedmask);
		if ((highlightarea.x1 != highlightarea.x2 || highlightarea.y1 != highlightarea.y2) && cidr < 32)
		{
			if (config_string(CONFIG_RIGHTNET))
			{
				if (fork() == 0)
				{
					char displaystring[32];
					struct ipaddress c;
					c.ip.i = mouseip.ip.i & (localmask | selectedmask);


					snprintf(displaystring, 32, "%i.%i.%i.%i/%i", (int)c.ip.a[3], (int)c.ip.a[2], (int)c.ip.a[1], (int)c.ip.a[0], cidr);
					int l = strlen(config_string(CONFIG_RIGHTNET)) + 33;
					char s[l];
					snprintf(s, l, config_string(CONFIG_RIGHTNET), displaystring);
					if (-1 == system(s))
						printf("Unable to spawn child: %s", strerror(errno));
					_exit(0); /* exec failed, need to quit */
				}
			}
			else
			{
				printf("A network has been selected using the right mouse button.\nIf rightclicknet was specified in the config, it would be executed now.\n");
			}
		}
		else
		{
			if (config_string(CONFIG_RIGHTCLICK))
			{
				if (fork() == 0)
				{
					char displaystring[32];
					snprintf(displaystring, 32, "%i.%i.%i.%i", (int)mouseip.ip.a[3], (int)mouseip.ip.a[2], (int)mouseip.ip.a[1], (int)mouseip.ip.a[0]);
					int l = strlen(config_string(CONFIG_RIGHTCLICK)) + 33;
					char s[l];
					snprintf(s, l, config_string(CONFIG_RIGHTCLICK), displaystring);
					if (-1 == system(s))
						printf("Unable to spawn child: %s", strerror(errno));
					_exit(0); /* exec failed, need to quit */
				}
			}
			else
			{
				printf("An ip has been clicked using the right mouse button.\nIf rightclickcmd was specified in the config, it would be executed now.\n");
			}				
		}	
	}
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
			case SDL_MOUSEBUTTONDOWN:
				mousedown(&event.button);
				break;
			case SDL_MOUSEBUTTONUP:
				mouseup(&event.button);
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
	struct packetheader ph;
	struct flowrequest* fr = (struct flowrequest*)ph.data;
	
	fr->flowon = start;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(serverport);
	sin.sin_addr.s_addr = serverip;
	ph.version = VERSION;
	if (!sock)
	{
		ph.packettype = PKT_SUBNET;
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		sendto(sock, &ph, SIZEOF_PACKETHEADER, 0, (struct sockaddr*)&sin, sizeof(sin));
		printf("requesting subnet data\n");
		ph.packettype = PKT_FWRULE;
		sendto(sock, &ph, SIZEOF_PACKETHEADER, 0, (struct sockaddr*)&sin, sizeof(sin));
		printf("requesting firewall rule data\n");
		//the response will come... later
	}
	ph.packettype = PKT_FLOW;
	sendto(sock, &ph, sizeof(fr) + SIZEOF_PACKETHEADER, 0, (struct sockaddr*)&sin, sizeof(sin));
}

void videoon(char b)
{
#if HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT
	if (b && !videostate) {
		videostate = startvideo(moviefile, IMGWIDTH, IMGHEIGHT);
	} else if (!b && videostate) {
		endvideo();
		videostate = 0;
	}
#endif
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
#if HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT
	case SDLK_v:
		if (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) 
			videoon(1);
		else
			videoon(0);
		break;
#endif
	case SDLK_RETURN:
		if (keys[SDLK_RALT] || keys[SDLK_LALT])
			togglefullscreen();
		break;
	case SDLK_p: /* pause output */
		pausedisplay = !pausedisplay;
		break;
	case SDLK_r: /* show the scrolling rules */
		displayrules = !displayrules;
		break;
	default:
		printf("Not handling key 0x%2X '%s' (%d, %d)\n", (int)k, SDL_GetKeyName(k), x, y);
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
 * purpose:		callback that gets called when the mouse moves
 * recieves:	the x and y coordinate of the mouse. woot. woot
 */
void mousemove(int x, int y)
{
	highlightarea.x2 = x /(float)width *IMGWIDTH;
	highlightarea.y2 = IMGHEIGHT - y / (float)height * IMGHEIGHT;
	/* find out what ips they have highlighted */
	unsigned short ip1 = ((unsigned int)(highlightarea.x1/BLOCKWIDTH) + ((unsigned int)(highlightarea.y1/BLOCKHEIGHT) * MAPWIDTH));
	unsigned short ip2 = ((unsigned int)(highlightarea.x2/BLOCKWIDTH) + ((unsigned int)(highlightarea.y2/BLOCKHEIGHT) * MAPWIDTH));
	/* find a subnet that encapsulates them */
	ip1 = unmappixel(ip1);
	ip2 = unmappixel(ip2);
	findcommonsubnet(ip1, ip2, &selectedbase, &selectedmask);
	ip1 = selectedbase;
	ip2 = selectedbase | ~selectedmask;
	/* then convert back to screen coordinates */
	ip1 = mappixel(ip1);
	ip2 = mappixel(ip2);
	subnetarea.x1 = ip1 % MAPWIDTH * BLOCKWIDTH;
	subnetarea.y1 = ip1 / MAPHEIGHT * BLOCKWIDTH;
	subnetarea.x2 = ip2 % MAPWIDTH * BLOCKWIDTH;
	subnetarea.y2 = ip2 / MAPHEIGHT * BLOCKWIDTH;


	/* unmap the ip */
	x *= scalex;
	y *= scaley;
	y += .5;
	if (y < (int)MAPHEIGHT/2)
		displayy = 0;
	else
		displayy =  IMGHEIGHT - 13*((float)IMGHEIGHT/height);
	
	unsigned short mappixel = x + ((MAPHEIGHT - y - 1) * MAPWIDTH);
	unsigned int ip = unmappixel(mappixel);
	if ((ip & ~localmask) != ip)
		return;
	mouseip.ip.i = ip | localip;
	
	if (pointdata[mappixel].msg)
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"%i.%i.%i.%i %s", mouseip.ip.a[3], mouseip.ip.a[2], mouseip.ip.a[1], mouseip.ip.a[0], pointdata[mappixel].msg );
	else 
		snprintf(displaystring, DISPLAYSTRINGSIZE, 
			"%i.%i.%i.%i", mouseip.ip.a[3], mouseip.ip.a[2], mouseip.ip.a[1], mouseip.ip.a[0]);

}



/**
 * if the size of the window changes
 */
void reshape (int w, int h)
{
	width = w;
	height = h;
	if (displayy) /* need to recalculate this */
		displayy = IMGHEIGHT - 13*((float)IMGHEIGHT/height);
	scalex = (float)MAPWIDTH/w;
	scaley = (float)MAPHEIGHT/h;
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glPixelZoom((float)w/IMGWIDTH, (float)h/IMGHEIGHT);
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
	int k;
	for (k = 0; k < BLOCKHEIGHT; k++) 
	{
		imgdata[COLORDEPTH * ((i*(int)BLOCKWIDTH) + (j*(int)BLOCKHEIGHT+k)*IMGWIDTH)+0] = b;
		imgdata[COLORDEPTH * ((i*(int)BLOCKWIDTH) + (j*(int)BLOCKHEIGHT+k)*IMGWIDTH)+1] = g;
		imgdata[COLORDEPTH * ((i*(int)BLOCKWIDTH) + (j*(int)BLOCKHEIGHT+k)*IMGWIDTH)+2] = r;
#if COLORDEPTH == 4
		imgdata[COLORDEPTH * ((i*(int)BLOCKWIDTH) + (j*(int)BLOCKHEIGHT+k)*IMGWIDTH)+3] = 255;
#endif
	}
}

/*
 * function:	lineup() 
 * purpose:     to draw a line above a pixel
 * recieves:    the x and y coords, and the blue, green and red components
 */
inline void lineup(int i, int j, unsigned char b, unsigned char g, unsigned char r)
{
	int k;
	for (k = 0; k < BLOCKWIDTH; k++) 
	{
		imgdata[COLORDEPTH * ((int)(i*BLOCKWIDTH)+k + (int)(j*BLOCKHEIGHT)*IMGWIDTH)+0] = b;
		imgdata[COLORDEPTH * ((int)(i*BLOCKWIDTH)+k + (int)(j*BLOCKHEIGHT)*IMGWIDTH)+1] = g;
		imgdata[COLORDEPTH * ((int)(i*BLOCKWIDTH)+k + (int)(j*BLOCKHEIGHT)*IMGWIDTH)+2] = r;
#if COLORDEPTH == 4
		imgdata[COLORDEPTH * ((int)(i*BLOCKWIDTH)+k + (int)(j*BLOCKHEIGHT)*IMGWIDTH)+3] = 255;
#endif
	}
}


/*
 * function:	outlinesubnets()
 * purpose:		to outline subnets on the map
 */
void outlinesubnets()
{
	unsigned int i, j;
	if (numsubnets < 2) //if we only have one, or no subnets, not worth doing.
		return;
	iptype t;
	for (i = 1; i < MAPWIDTH; i++)
	{
		for (j = 1; j < MAPHEIGHT; j++)
		{
			unsigned int ip = i + (j * MAPWIDTH);
			ip = unmappixel(ip) | localip;
			unsigned int ipleft = i-1 + (j * MAPWIDTH);
			ipleft = unmappixel(ipleft) | localip;
			unsigned int ipup = i + ((j - 1) * MAPWIDTH);
			ipup = unmappixel(ipup) | localip;

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
		unsigned int ip = i + (j * MAPWIDTH);
		ip = unmappixel(ip) | localip;
		unsigned int ipleft = i-1 + (j * MAPWIDTH);
		ipleft = unmappixel(ipleft) | localip;

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
		unsigned int ip = i + (j * MAPWIDTH);
		ip = unmappixel(ip) | localip;
		unsigned int ipup = i + ((j - 1) * MAPWIDTH);
		ipup = unmappixel(ipup) | localip;

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
	unsigned int i;
	for (i = 0; i < IMGWIDTH*IMGHEIGHT; i++){
		imgdata[i*COLORDEPTH + 0] = 0;
		imgdata[i*COLORDEPTH + 1] = 0;
		imgdata[i*COLORDEPTH + 2] = 0;
#if COLORDEPTH==4
		imgdata[i*COLORDEPTH + 3] = 255;
#endif
	}
	memset(pointdata, 0, sizeof(struct datapoint) * MAPWIDTH * MAPHEIGHT);
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
	unsigned int i;
	/* lock */
	pthread_mutex_lock(&imglock);
	for (i = 0; i < MAPWIDTH*MAPHEIGHT; i++)
		datapointfade(&pointdata[i], FADERATE);
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
	
	if (!pausedisplay)
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

void printdisplay(const char* s, unsigned short ip )
{
	/* check for duplicate lines */
	if (displaytext.brightness[displaytext.which] > .1 &&  0 == strncmp(displaytext.log[displaytext.which], s, strlen(s)))
	{
		displaytext.dup[displaytext.which]++;
		if(ip != 0) 
			snprintf(displaytext.log[displaytext.which], DISPLAYWIDTH+29, "%s (129.123.%d.%d) (%d)", s, ip >> 8, ip & 255, displaytext.dup[displaytext.which]);
		else 
			snprintf(displaytext.log[displaytext.which], DISPLAYWIDTH+29, "%s (%d)", s, displaytext.dup[displaytext.which]);
	}
	else
	{
		displaytext.which--;
		displaytext.which = (displaytext.which + DISPLAYLINES) % DISPLAYLINES;
		strncpy(displaytext.log[displaytext.which], s, DISPLAYWIDTH-1);
		if(ip != 0) 
			snprintf(displaytext.log[displaytext.which], DISPLAYWIDTH+19, "%s (129.123.%d.%d)", s, ip >> 8, ip & 255);
		else 
			snprintf(displaytext.log[displaytext.which], DISPLAYWIDTH+19, "%s", s);

		displaytext.dup[displaytext.which] = 1;
		displaytext.scroll=15;
		/* check to see if its an outbound rule */
		if (strstr(s, "em0")) 
			displaytext.dir[displaytext.which] = 1;
		else
			displaytext.dir[displaytext.which] = 0;
		//printf("%s\n", displaytext.log[displaytext.which]);
			
	}
	displaytext.brightness[displaytext.which] = .75;
	
}

/*
 * where we display text all over the screen. yay!
 */
void drawdisplaytext() 
{
	
	int i;
	glColor3f(1,1,1);
	glRasterPos2i(displayx, displayy);
	renderstring(displaystring);
	if (displayrules)
	{
		for (i = 0; i < DISPLAYLINES; i++) 
		{
			if (displaytext.brightness[(i + displaytext.which) % DISPLAYLINES] > .01 || pausedisplay)
			{
				if (displaytext.dir[(i + displaytext.which) % DISPLAYLINES]) 
				{
					displaytext.brightness[(i + displaytext.which) % DISPLAYLINES] -= DISPLAYFADE * .5;
					float color = displaytext.brightness[(i+displaytext.which)%DISPLAYLINES];
					if (pausedisplay)
						color = .5;
					glColor3f(color*1.3,0,0);
				}
				else
				{
					displaytext.brightness[(i + displaytext.which) % DISPLAYLINES] -= DISPLAYFADE;
					float color = displaytext.brightness[(i+displaytext.which)%DISPLAYLINES];
					if (pausedisplay)
						color = .5;
					glColor3f(color,color,color);
				}
				/* figure out which direction to do this */
				if (displayy < 100)
					glRasterPos2i(0, (i+2)*15 - displaytext.scroll);
				else
					glRasterPos2i(0, (IMGHEIGHT - 30) - (i+1 ) * 15 + displaytext.scroll);
				//glRasterPos2i(0, IMGHEIGHT - 13 - i*13);
				renderstring(displaytext.log[(i+displaytext.which)%DISPLAYLINES]);
			}
		}
	}
	if (displaytext.scroll)
		displaytext.scroll-=5;
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
	unsigned int i, j;
	for (i = 0; i < MAPWIDTH; i++)
		for (j = 0; j < MAPHEIGHT; j++)
			datapointdrawpixel(&pointdata[i + j*MAPWIDTH], imgdata, i, j, IMGWIDTH);

#if HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT
	if (videostate)
		writeframe(imgdata, IMGWIDTH, IMGHEIGHT);
#endif
	glBindTexture(GL_TEXTURE_2D, texture);
#if (COLORDEPTH==4)
	glTexImage2D(GL_TEXTURE_2D, 0, 4, IMGWIDTH, IMGHEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, imgdata);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, 3, IMGWIDTH, IMGHEIGHT, 0, GL_BGR, GL_UNSIGNED_BYTE, imgdata);
#endif
	pthread_mutex_unlock(&imglock);
	glEnable(GL_TEXTURE_2D);
	 glBegin(GL_QUADS);
	  glTexCoord2d(0, 0); glVertex2d(0,0);
	  glTexCoord2d(1, 0); glVertex2d(IMGWIDTH, 0);
	  glTexCoord2d(1, 1); glVertex2d(IMGWIDTH, IMGHEIGHT);
	  glTexCoord2d(0, 1); glVertex2d(0, IMGHEIGHT);
	 glEnd();
	glDisable(GL_TEXTURE_2D);
	if (highlighting && highlightarea.x1 != highlightarea.x2 && highlightarea.y1 != highlightarea.y2)
	{
		glBegin(GL_QUADS);
			glColor4f(0,0,1,.3);
			glVertex2d(subnetarea.x1, subnetarea.y1);
			glVertex2d(subnetarea.x2+BLOCKWIDTH, subnetarea.y1);
			glVertex2d(subnetarea.x2+BLOCKWIDTH, subnetarea.y2+BLOCKHEIGHT);
			glVertex2d(subnetarea.x1, subnetarea.y2+BLOCKHEIGHT);
		glEnd();
	}

	int glerr;
	if ((glerr = glGetError())) {
		printf("errors drawing to the screen: %i\n", glerr);
	}
	/* draw the logged stuff */
	drawdisplaytext();
	SDL_GL_SwapBuffers();
	sched_yield();
}

/*
 * function:	update_image()
 * purpose:		to update the image according to a given flowpacket
 * recieves:	the flow packet
 */
void update_image(struct flowpacket* fp) 
{
	unsigned int i;
	unsigned int max = fp->count;
	if (max > MAXINDEX)
		max = MAXINDEX; /* watch those externally induced overflows */
	/* fp->base may not be the same as localip. if not, we need to offset it to match. */
	unsigned int diff = fp->base - localip;
		
	/* lock it up */
	pthread_mutex_lock(&imglock);
	for (i = 0; i < max; i++)
	{
		if (fp->data[i].incoming && !SHOW_IN)
			continue;
		if (!fp->data[i].incoming && !SHOW_OUT)
			continue;
		/*
		 * increment the local ip by the diff. overflow is ok
		 */
		fp->data[i].ip += diff;
		/* 
		 * need a more intelligent check of subnet. For now, just skip packets
		 * that are outside of our screen space
		 */
		
		if (((fp->data[i].ip & ~localmask)) != fp->data[i].ip)
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
int verifynulls(int which)
{
	int i;
	for (i = 0; i < MAPWIDTH * MAPHEIGHT; i++)
	{
		if (pointdata[i].msg)
		{
			printf("%010d: pointdata[%d].msg is not null\n", which, i);
			return 0;
		}
	}
	return 1;
}
// */

/*
 * function:	update_firewall()
 * purpose:		to update the image according to a given flowpacket
 * recieves:	the flow packet
 */
void update_firewall(struct fwflowpacket* fp) 
{
	unsigned int i;
	unsigned int max = fp->count;
	if (max > MAXINDEX)
		max = MAXINDEX; /* watch those externally induced overflows */
	/* lock it up */
	pthread_mutex_lock(&imglock);
	for (i = 0; i < max; i++)
	{
		if (((fp->data[i].ip & ~localmask)) != fp->data[i].ip)
			continue;
		int pixel = mappixel(fp->data[i].ip);

		datapointdrop(&pointdata[pixel], 192);
		if (pointdata[pixel].msg)
		{
			free((void*)pointdata[pixel].msg);
			pointdata[pixel].msg = 0;
		}
		if (rules[fp->data[i].rule])
			pointdata[pixel].msg = 0; //strdup(rules[fp->data[i].rule]);
		if (rules[fp->data[i].rule]) 
			printdisplay(rules[fp->data[i].rule], fp->data[i].ip);
	}
	pthread_mutex_unlock(&imglock);
	/* unlock it */

}

/*
 * function:	update_firewall_verbose()
 * purpose:		to update the image according to a given flowpacket
 * recieves:	the flow packet
 */
void update_firewall_verbose(struct verbosefirewall* fp) 
{
	unsigned int i;
	unsigned int max = fp->count;
	if (max > MAXINDEX)
		max = MAXINDEX; /* watch those externally induced overflows */
	/* lock it up */
	union iptobytes {
		unsigned char b[4];
		unsigned int i;
	} convert;
	pthread_mutex_lock(&imglock);
	for (i = 0; i < max; i++)
	{
		if (((fp->data[i].local & ~localmask)) != fp->data[i].local)
			continue;
		int pixel = mappixel(fp->data[i].local);

		datapointdrop(&pointdata[pixel], 192);
		char buffer[512];
		convert.i = fp->data[i].remote;
		const char * p = "";
		if (fp->data[i].packet == UDP)
			p = "  UDP";
		else if (fp->data[i].packet == TCP)
			p = "  TCP";
		if (fp->data[i].packet == OTHER)
			snprintf(buffer, 512, "OTHER %hhu.%hhu.%hhu.%hhu %s",
				convert.b[3], convert.b[2], convert.b[1], convert.b[0],
				fp->data[i].incoming? "-->" : "<--");
		else
			snprintf(buffer, 512, "%s %hhu.%hhu.%hhu.%hhu:%hu %s %hu:",
				p,
				convert.b[3], convert.b[2], convert.b[1], convert.b[0],
				fp->data[i].remoteport,
				fp->data[i].incoming? "-->" : "<--",				
				fp->data[i].localport);
		printdisplay(buffer, fp->data[i].local);
		if (pointdata[pixel].msg)
			free((void*)pointdata[pixel].msg);
		pointdata[pixel].msg = strdup(buffer);
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
		subnets[i].base = sp->subnets[i].base + sp->base;
		subnets[i].mask = sp->subnets[i].mask;
	}
	/* redraw the subnet stuff */
	initbuffer();
}

/*
 * this function will interpret and use a rule packet
 */
void update_rule(void *buffer) 
{
	struct fwrulepacket rp;
	readrulepacket(buffer, &rp);
	if (rules[rp.num])
		free((void*)rules[rp.num]);
	rules[rp.num] = rp.string;
	printf("Recieved rule %hu of %hu: %s\n", rp.num + 1, rp.max, rp.string);
}

/*
 * function:	collect_data()
 * purpose:		collects data to draw. this is a separate thread
 */
void* collect_data(void* p)
{
	p = 0; /* avoid compiler warnings, since we don't use this */
	dead = 0;
	/* start listening for data */
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverport);
	addr.sin_addr.s_addr = serverip;

	/* kill the thread if the socket couldnt be allocated */
	if (!sock)
	{
		dead = 1;
		return 0;
	}
	
	socklen_t addrlen = sizeof(addr);
	struct packetheader ph;
	while (!die)
	{
		recvfrom(sock, &ph, sizeof(ph), 0, (struct sockaddr*)&addr, &addrlen);
		switch (ph.packettype)
		{
		case PKT_FLOW:
			if (pausedisplay) continue;
			update_image((struct flowpacket*)ph.data);
			break;
		case PKT_SUBNET:
			updatesubnets((struct subnetpacket*)ph.data);
			break;
		case PKT_FIREWALL:
			if (pausedisplay) continue;
			update_firewall((struct fwflowpacket*)ph.data);
			break;
		case PKT_FWRULE:
			update_rule(ph.data);
			break;
		case PKT_VERBOSEFIREWALL:
			if (pausedisplay) continue;
			update_firewall_verbose((struct verbosefirewall*)ph.data);
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
	unsigned int a = num >> a_shift;
	unsigned int b = (num & b_mask) >> 8;
	unsigned int c = (num >> 4) & 0xf;
	unsigned int d = num & 0xf;
	
	unsigned short mapped = d + 16 * b + (c + 16 * a) * MAPWIDTH;

	return mapped;
}

unsigned short rev_netblock(unsigned short num)
{
	/*
	unsigned short x_inner, y_inner, x_outer, y_outer, mapped;
	x_inner = num % 16;
	y_inner = num / 16 % 16;
	x_outer = num / 256 % 16;
	y_outer = num / 4096 % 16;

	mapped = y_outer * 4096 + y_inner * 256 + x_outer * 16 + x_inner;
	// */
	//*
	unsigned int a = num >> a_shift;
	unsigned int b = (num & b_mask_rev) >> b_shift_rev;
	unsigned int c = (num & c_mask_rev) >> 4;
	unsigned int d = num & 0xf;

	unsigned int mapped = (a << a_shift) | (c << 8) | (b << 4) | d;
	// */
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
