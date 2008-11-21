
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
