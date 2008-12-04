
#include "config.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "flowdata.h"

const char * validopts[MAXOPTS][2] = {
/* short, long, default */
	{"s", "server"},
	{"p", "port"},
	{"l", "localnet"},
	{"u", "subnetpath"},
	{"a", "auth"},
	{"i", "interface"}
};

const char* config_opts[] = {0, STRPORT, 0, 0, 0, "eth0"};
int optcount = 0;


/* find the option, if its not there, add it */
int findopt(const char* o)
{
	int i;
	for (i = 0; i < MAXOPTS; i++)
	{
		if (!strcmp(validopts[i][1], o)) //long opt
			return i;
		if (!strcmp(validopts[i][0], o)) //short opt
			return i;
	}
	return i;
}

int config_loadfile(const char* file)
{
	char k[512];
	char v[512];
	config_opts;
	char * pv;
	FILE* f = fopen(file, "r");
	clearerr(f);
	int ret = 0;
	while (1 == fscanf(f, "%511s", k)) 
	{
		if (k[0] == '#') //don't process comments
		{
			fscanf(f, "%511[^\n]", v);
			continue;
		}
		if (1 != fscanf(f, "%511[^\n]", v))
			break;
		pv = v;
		while (*pv && (isspace(*pv) || *pv == '='))
			pv++;

		int o = findopt(k);
		if (o == MAXOPTS)
			fprintf(stderr, "Invalid config option: %s", k);
		else
			config_opts[o] = strdup(pv);
		ret ++;
	}
	if (!feof(f))
		fprintf(stderr, "Invalid configuration file after parsing %i options\n", ret);
	return ret;
	
}

int config_loadargs(int argc, char** argv)
{
	int i;
	int ret =0;
	for (i = 1; i < argc-1; i+=2)
	{
		char* s = argv[i];
		while (*s && *s == '-')
			s++;
		int o = findopt(s);
		if (o == MAXOPTS)
			fprintf(stderr, "Invalid option: %s", s);
		else
			config_opts[o] = strdup(argv[i+1]);
		ret++;
	}
	return ret;
}

int config_int(enum opt o)
{
	return atoi(config_opts[(int)o]);
}
const char* config_string(enum opt o)
{
	return config_opts[(int)o];
}
int config_bool(enum opt o)
{
	if (0 == config_opts[(int)o][0] ||
		!strcasecmp(config_opts[(int)o], "no") ||
		!strcasecmp(config_opts[(int)o], "false") ||
		!strcasecmp(config_opts[(int)o], "") ||
		!strcasecmp(config_opts[(int)o], "0"))
		return 0;
	return 1;
}
inline const char* iptolong(const char *b, unsigned int *ip)
{
	const char* a[4] = {0};
	int i=0;
	a[i++] = b;
	while (*b == '.' || isdigit(*b))
	{
		if (*b == '.')
		{
			b++;
			if (b)
				a[i++] = b;
			if (i == 4)
				break;
		}
		b++;
	}
	if (i == 4)
		*ip =  (atoi(a[0]) << 24) + (atoi(a[1]) << 16) + (atoi(a[2]) << 8) + atoi(a[3]);
	else
		*ip = 0;
	while (*b && isdigit(*b))
		b++;
	return b;

}
int masktocidr(unsigned int m)
{
	unsigned int i = 1, j;
	int ret = 0;
	for (j = 0; j < 32; j++, i <<= 1)
	{
		if (i & m)
			ret++;
	}
	return ret;
}
unsigned int cidrtomask(int c)
{
	unsigned int ret = 0;
	int i;
	for (i = 31; i > (31-c); i--)
	{
		ret |= (1 << i);
	}

	return ret;
}
unsigned int config_ip(enum opt o)
{
	unsigned int ret;
	iptolong(config_opts[(int)o], &ret);
	return ret;
}
int config_net(enum opt o, unsigned int*ip, unsigned int*mask)
{
	const char* b = iptolong(config_opts[(int)o], ip);
	/* attempt to read a mask */
	if (*b == '/') 
	{
		b++;
		*mask = cidrtomask(atoi(b));
		if (*mask)
			return 1;
	}
	else if (*b)
	{
		b++;
		if (isdigit(*b))
		{
			/* specified it longhand? */
			b = iptolong(b, mask);
			if (*mask)
				return 1;
		}
	}
	return 0;
}




#ifdef UNIT_TEST
int main()
{
	const char* argv[] = {
		"stuff",
		"--server",
		"ipvisualizer",
		"-l",
		"192.168.10.128/25",
		"--subnetpath",
		"http://path/to/subnets.py",
		"--a",
		"10.1.20.0 255.255.255.0",
		"-none",
		"badarg"
		};
	int argc = 9;
	if (4 != config_loadargs(argc, (char*)argv))
		printf("Arg count is wrong\n");
	if (strcmp(config_string(CONFIG_SERVER), "ipvisualizer"))
		printf("server != ipvisualizer\n");
	if (strcmp(config_string(CONFIG_LOCALNET), "192.168.10.128/25"))
		printf("localnet != '192.168.10.128/25'\n");
	unsigned int ip=0, mask=0;
	config_net(CONFIG_LOCALNET, &ip, &mask);
	if (ip != 0xc0a80a80 || mask != 0xffffff80)
		printf("0x%08x 0x%08x != 0xc0a80880 0xffffff80\n", ip, mask);
	if (strcmp(config_string(CONFIG_SUBNET), "http://path/to/subnets.py"))
		printf("subnetpath != \"http://path/to/subnets.py\"\n");
	config_net(CONFIG_AUTH, &ip, &mask);
	if (ip != 0x0a011400 || mask != 0xffffff00)
		printf("0x%08x 0x%08x != 0x0a011400 0xffffff00\n", ip, mask);
	int p = config_int(CONFIG_PORT);
	if (p != 17843)
		printf("%d != 17843\n", p);

	FILE *f = fopen("/tmp/config_test.conf", "w");
	fprintf(f, "#This is a comment. Woot!\n");
	fprintf(f, "#\n\n");
	fprintf(f, "server = yes\n");
	fprintf(f, "port true\n");
	fprintf(f, "localnet no\n");
	fprintf(f, "#test\n");
	fprintf(f, "subnetpath = what a string, isn't this amazing?\n");
	fprintf(f, "auth 0\n\n\n\n\n");
	fclose(f);

	config_loadfile("/tmp/config_test.conf");
	if (!config_bool(CONFIG_SERVER))
		printf("server is not true\n");
	if (!config_bool(CONFIG_PORT))
		printf("port is not true\n");
	if (config_bool(CONFIG_LOCALNET))
		printf("localnet is true\n");
	if (strcmp(config_string(CONFIG_SUBNET), "what a string, isn't this amazing?"))
		printf("subnetpath is wrong\n");
	if (strcmp(config_string(CONFIG_AUTH), "0"))
		printf("auth is not 0\n");
	if (config_bool(CONFIG_AUTH))
		printf("auth is true\n");

	printf("Tests completed\n");

	return 0;
}
#endif
