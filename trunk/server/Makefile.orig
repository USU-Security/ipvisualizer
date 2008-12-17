LDFLAGS=-lpcap `curl-config --libs`

flowserv: flowserv.o $(COMMON)/flowdata.o subnets.o fwrules.o sockutils.o globals.o $(COMMON)/config.o base64.o

subnets.o: base64.h

flowserv.o: flowserv.c mygettime.h structs.h $(COMMON)/flowdata.h globals.h $(COMMON)/config.h

$(COMMON)/flowdata.o:
	        make CFLAGS="$(CFLAGS)" -C $(COMMON) -f Makefile flowdata.o

base64test: base64.c
	gcc -g -DUNIT_TEST base64.c -o base64 && ./base64

clean:
	rm -f *.o

distclean: clean
	rm -f flowserv

.PHONY: clean distclean

