CFLAGS=-g -Wall -Wextra 

COMMON=../shared

all: display flowserv

display:
	make COMMON=$(COMMON) CFLAGS="$(CFLAGS)" -C client -f Makefile display
	[ -f client/display ] && cp -f client/display .

flowserv:
	make COMMON=$(COMMON) CFLAGS="$(CFLAGS)" -C server -f Makefile
	[ -f server/flowserv ] && cp -f server/flowserv .

ulogserv:
	make COMMON=$(COMMON) CFLAGS="$(CFLAGS)" -C iptulog -f Makefile
	[ -f iptulog/ulogserv ] && cp -f iptulog/ulogserv .

clean:
	make -C client -f Makefile clean
	make -C server -f Makefile clean
	make -C shared -f Makefile clean
	make -C iptulog -f Makefile clean

distclean: clean
	make -C client -f Makefile distclean
	make -C server -f Makefile distclean
	make -C shared -f Makefile distclean
	make -C iptulog -f Makefile distclean
	rm -f display flowserv ulogserv

.PHONY: clean distclean display flowserv all


