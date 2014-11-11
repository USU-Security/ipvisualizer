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

#include <stdio.h>
#include <unistd.h>

/*
 * sub:         readstring()
 * purpose:     to read a string, the slow way. reads till it finds a newline, discards it. 
 * recieves:    the file descriptor, a pointer to a character array, and its size
 * returns:     how many characters were read
 */
int readstring(int fd, char* buffer, int size)
{
	int len=0;

	/* read characters until read burps or we hit a newline */
	while (len < size-2)
	{
		if (read(fd, &buffer[len], 1) < 1)
			return -1;
		if (len > 0 && buffer[len-1] == '\x0d' && buffer[len] == '\x0a')
		{
			len--;
			break;
		}
		len++;
	} 	
	buffer[len] = 0;
	fprintf(stderr, "returning '%s'\n", buffer);
	return len;
}

/*
 * sub:         readstring()
 * purpose:     to read a string, the slow way. reads till it finds a newline, discards it. 
 * recieves:    the file descriptor, a pointer to a character array, and its size
 * returns:     how many characters were read
 */
int readunixstring(int fd, char* buffer, int size)
{
	int len=0;

	/* read characters until read burps or we hit a newline */
	while (len < size-2)
	{
		if (read(fd, &buffer[len], 1) < 1)
			return -1;
		if (buffer[len] == '\x0a')
		{
			break;
		}
		len++;
	} 	
	buffer[len] = 0;
	fprintf(stderr, "returning '%s'\n", buffer);
	return len;
}
