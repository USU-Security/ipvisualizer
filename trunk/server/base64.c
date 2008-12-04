
/*
 * this code takes a given binary stream and encodes it as base64, as defined
 * by rfc3548
 */

const unsigned char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//not so easy
const unsigned char normal[256] = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,	
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,	
	0,0,0,0,0,0,0,0,
	0,0,0,
	62,0,0,0,63,	
	/* 0-9 */
	52,53,54,55,56,57,58,59,60,61,
	0,0,0,0,0,0,0,
	/* A */                                                       /* Z */
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,
	/* [\]^_` */
	0,0,0,0,0,0,
	/* a */                                                               /* z */
	26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51
};


struct b64conv 
{
	union
	{
		struct {
			unsigned int d:6;
			unsigned int c:6;
			unsigned int b:6;
			unsigned int a:6;
		} b64;
		struct {
			unsigned int c:8;
			unsigned int b:8;
			unsigned int a:8;
		} b256;
	} u;
};

int base64encode(const char* in, int inlen, char* out, int *outlen)
{
	/* make sure that outlen is 33% bigger than inlen */
	if ((*outlen >> 2) * 3 < inlen)
		return 0; /* not enough room */
	int i=0, o=0;
	while (i + 2 < inlen)
	{
		struct b64conv c;
		c.u.b256.a = in[i + 0];
		c.u.b256.b = in[i + 1];
		c.u.b256.c = in[i + 2];
		out[o + 0] = b64[c.u.b64.a];
		out[o + 1] = b64[c.u.b64.b];
		out[o + 2] = b64[c.u.b64.c];
		out[o + 3] = b64[c.u.b64.d];
		i += 3;
		o += 4;
	}
	if (i + 2 == inlen) /* two extra bytes */
	{
		struct b64conv c;
		c.u.b256.a = in[i + 0];
		c.u.b256.b = in[i + 1];
		c.u.b256.c = 0;
		out[o + 0] = b64[c.u.b64.a];
		out[o + 1] = b64[c.u.b64.b];
		out[o + 2] = b64[c.u.b64.c];
		out[o + 3] = '=';
		o+=4;
	}
	else if (i + 1 == inlen) /* one extra byte */
	{
		struct b64conv c;
		c.u.b256.a = in[i + 0];
		c.u.b256.b = 0;
		c.u.b256.c = 0;
		out[o + 0] = b64[c.u.b64.a];
		out[o + 1] = b64[c.u.b64.b];
		out[o + 2] = '=';
		out[o + 3] = '=';
		o+= 4;
	}
	out[o] = 0; /* null terminate it */
	*outlen = o;
	return 1;
}

#ifdef UNIT_TEST
int main()
{
	const char* test1 = "username:password";
	char output[512] = {0};
	int outsize = 512;
	if (!base64encode(test1, strlen(test1), output, &outsize))
		printf("Base64 encoding reported failure\n");
	if (outsize != 24)
		printf("Base64 endoced output is the wrong length: %i\n", outsize);
	if (strcmp(output, "dXNlcm5hbWU6cGFzc3dvcmQ="))
		printf("Base64 encoded output is wrong: '%s'\n", output);
	char test2[256];
	int i;
	for (i = 0; i < 256; i++)
		test2[i] = 255-i;
	outsize = 512;
	base64encode(test2, 256, output, &outsize);
	if (strcmp(output, "//79/Pv6+fj39vX08/Lx8O/u7ezr6uno5+bl5OPi4eDf3t3c29rZ2NfW1dTT0tHQz87NzMvKycjHxsXEw8LBwL++vby7urm4t7a1tLOysbCvrq2sq6qpqKempaSjoqGgn56dnJuamZiXlpWUk5KRkI+OjYyLiomIh4aFhIOCgYB/fn18e3p5eHd2dXRzcnFwb25tbGtqaWhnZmVkY2JhYF9eXVxbWllYV1ZVVFNSUVBPTk1MS0pJSEdGRURDQkFAPz49PDs6OTg3NjU0MzIxMC8uLSwrKikoJyYlJCMiISAfHh0cGxoZGBcWFRQTEhEQDw4NDAsKCQgHBgUEAwIBAA=="))
		printf("Base64 incorrectly encoded the binary set: '%s'\n", output);
	return 0;
}
#endif

