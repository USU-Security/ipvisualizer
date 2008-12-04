#include "../shared/config.h"

unsigned int localip;
unsigned int localmask;
unsigned int localport;

void initglobals()
{
	config_net(CONFIG_LOCALNET, &localip, &localmask);
	localport = config_int(CONFIG_PORT);
}


