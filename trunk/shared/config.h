/*
 * file: 	config.h
 * created:	2008-11-21
 * purpose:	Allows us to load a configuration from a file, and possibly
 * 			override them from the command line
 * creator:	rian shelley
 */



#ifndef CONFIG_H
#define CONFIG_H

enum opt{CONFIG_SERVER, CONFIG_PORT, CONFIG_LOCALNET, CONFIG_SUBNET, CONFIG_AUTH, CONFIG_INTERFACE, MAXOPTS};

int config_loadfile(const char* file);
int config_loadargs(int argc, char** argv);

int config_int(enum opt o);
const char* config_string(enum opt o);
int config_bool(enum opt o);
unsigned int config_ip(enum opt o);
int config_net(enum opt o, unsigned int* ip, unsigned int* mask);

int masktocidr(unsigned int m);

#endif//CONFIG_H

