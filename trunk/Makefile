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
	cd .tmp
	mkdir ipvisualizer
	cp -r ../* ./ipvisualizer/
	rm -f ipvisualizer/ipvisualization.conf
	echo "#the visualization server. Client only" >> ipvisualizer/ipvisualization.conf
	echo "server = localhost" >> ipvisualizer/ipvisualization.conf
	echo "" >> ipvisualization.conf
	echo "#the listening port" >> ipvisualizer/ipvisualization.conf
	echo "port = 17843" >> ipvisualizer/ipvisualization.conf
	echo "" >> ipvisualizer/ipvisualization.conf
	echo "#the local network " >> ipvisualizer/ipvisualization.conf
	echo "localnet = 10.1.0.0/16" >> ipvisualizer/ipvisualization.conf
	echo "" >> ipvisualizer/ipvisualization.conf
	echo "#the path to the binary subnet specification." >> ipvisualizer/ipvisualization.conf
	echo "#Server only" >> ipvisualizer/ipvisualization.conf
	echo "subnetpath = https://thingy.security.usu.edu/cgi-bin/subnetlist.pl" >> ipvisualizer/ipvisualization.conf
	echo "" >> ipvisualizer/ipvisualization.conf
	echo "#the authorization (user:pass). Server only" >> ipvisualizer/ipvisualization.conf
	echo "auth = bndmgr:deck+screw" >> ipvisualizer/ipvisualization.conf
	echo "" >> ipvisualizer/ipvisualization.conf
	echo "#which interface to sniff on. defaults to eth0. Server only" >> ipvisualizer/ipvisualization.conf
	echo "#interface = eth0" >> ipvisualizer/ipvisualization.conf
	tar -cvzf ../ipvisualizer.tar.gz ipvisualizer
	cd ..
	rm -rf .tmp
.PHONY: clean distclean display flowserv all


