ipvisualizer_flowservprgdir=$(prefix)/bin/
AM_LDFLAGS= -lpcap `curl-config --libs`
ipvisualizer_flowservprg_PROGRAMS=ipvisualizer_flowserv
ipvisualizer_flowserv_SOURCES=flowserv.c fwrules.c globals.c sockutils.c subnets.c ../shared/flowdata.c ../shared/config.c fwrules.h globals.h mygettime.h sockutils.h structs.h subnets.h ../shared/config.h ../shared/flowdata.h
