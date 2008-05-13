/*
 * contains definitions and headers needed to  retrieve and store firewall rules
 */

struct fwrule 
{
	const char* rule;
	struct fwrule* next;
};

struct fwrule* getfwrules(const char* site, const char* webpage, const char* authentication, int* numrules);

