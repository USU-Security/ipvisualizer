
flowdata.o: flowdata.c flowdata.h

config: config.c config.h

clean:
	rm -f *.o

distclean: clean

test: configtest


configtest: config.c
	gcc -g -DUNIT_TEST config.c -o config && ./config 


.PHONY: clean distclean

