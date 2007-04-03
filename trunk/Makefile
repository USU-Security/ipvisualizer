CFLAGS=-g -Wall -Wextra -pedantic

COMMON=../shared

all: display flowserv

display:
	make COMMON=$(COMMON) CFLAGS="$(CFLAGS)" -C client -f Makefile display
	[ -f client/display ] && cp -f client/display .

flowserv:
	make COMMON=$(COMMON) CFLAGS="$(CFLAGS)" -C server -f Makefile
	[ -f server/flowserv ] && cp -f server/flowserv .

clean:
	make -C client -f Makefile clean
	make -C server -f Makefile clean
	make -C shared -f Makefile clean

distclean: clean
	make -C client -f Makefile distclean
	make -C server -f Makefile distclean
	make -C shared -f Makefile distclean
	rm -f display flowserv

.PHONY: clean distclean display flowserv all


