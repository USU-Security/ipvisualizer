LDFLAGS=-lpcap

flowserv: flowserv.o $(COMMON)/flowdata.o

flowserv.o: flowserv.c mygettime.h structs.h $(COMMON)/flowdata.h

$(COMMON)/flowdata.o:
	        make -C $(COMMON) -f Makefile flowdata.o

clean:
	rm -f *.o

distclean: clean
	rm -f flowserv

.PHONY: clean distclean

