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

#ifndef SUBNETS_H
#define SUBNETS_H

typedef struct subnet_t {
	unsigned int base;
	unsigned char mask;
} subnet;

#define SUBSTRUCT_SIZE 5

int getsubnets(subnet* subnets, int max, const char* webpage, const char* authentication);



#endif /* !SUBNETS_H */
