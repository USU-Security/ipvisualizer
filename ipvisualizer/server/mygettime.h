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
 * file:	mygettime.h
 * purpose:	i like my gettime function. in a .h for inlining
 * creator: 	rian shelley
 * created:	03-12-2007
 */

#ifndef MYGETTIME
#define MYGETTIME
/*
 * function:    gettime()
 * purpose: to return the time in milliseconds since the unix epoch
 * returns: didn't you read the purpose?
 */
unsigned long long gettime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}


#endif /*MYGETTIME_H*/
