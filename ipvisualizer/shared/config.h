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
 * file: 	config.h
 * created:	2008-11-21
 * purpose:	Allows us to load a configuration from a file, and possibly
 * 			override them from the command line
 * creator:	rian shelley
 */



#ifndef CONFIG_H
#define CONFIG_H

enum opt{CONFIG_SERVER, CONFIG_PORT, CONFIG_LOCALNET, CONFIG_SUBNET, CONFIG_AUTH, CONFIG_INTERFACE, CONFIG_PCAPFILE, CONFIG_CONFIGFILE, CONFIG_LEFTCLICK, CONFIG_RIGHTCLICK, CONFIG_RIGHTNET, MAXOPTS};

int config_loadfile(const char* file);
int config_loadargs(int argc, char** argv);

int config_int(enum opt o);
const char* config_string(enum opt o);
int config_bool(enum opt o);
unsigned int config_ip(enum opt o);
int config_net(enum opt o, unsigned int* ip, unsigned int* mask);

int masktocidr(unsigned int m);

#endif//CONFIG_H

