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
void setnet(unsigned int base, unsigned int mask);
