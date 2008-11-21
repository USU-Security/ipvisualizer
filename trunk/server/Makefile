LDFLAGS=-lpcap

flowserv: flowserv.o $(COMMON)/flowdata.o subnets.o fwrules.o sockutils.o

flowserv.o: flowserv.c mygettime.h structs.h $(COMMON)/flowdata.h

$(COMMON)/flowdata.o:
	        make CFLAGS="$(CFLAGS)" -C $(COMMON) -f Makefile flowdata.o

clean:
	rm -f *.o

distclean: clean
	rm -f flowserv

.PHONY: clean distclean

