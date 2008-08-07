/*
 * contains definitions and headers needed to  retrieve and store firewall rules
 */

struct fwrule 
{
	const char* rule;
	struct fwrule* next;
};


