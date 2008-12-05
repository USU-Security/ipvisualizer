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

dist: distclean
	mkdir .tmp
	mkdir .tmp/ipvisualizer
	cp -r * .tmp/ipvisualizer/
	rm -f .tmp/ipvisualizer/ipvisualizer.conf
	echo "#the visualization server. Client only" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "server = localhost" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#the listening port" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "port = 17843" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#the local network " >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "localnet = 10.1.0.0/16" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#the path to the binary subnet specification." >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#Server only" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#subnetpath = " >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#the authorization for the subnet specification (user:pass). Server only" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#auth = user:password" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#which interface to sniff on. defaults to eth0. Server only" >> .tmp/ipvisualizer/ipvisualizer.conf
	echo "#interface = eth0" >> .tmp/ipvisualizer/ipvisualizer.conf
	find .tmp/ipvisualizer/ -iname ".*" | xargs rm -rf
	tar -cvzf ipvisualizer.tar.gz  -C .tmp ipvisualizer
	rm -rf .tmp
.PHONY: clean distclean display flowserv all


