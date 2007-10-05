#
CC = gcc -c 
LD = gcc
RM = rm -f
CFLAGS = -g -Iinclude -I/usr/src/linux/include

all: ulogserv

#distrib:

#ulog_test: ulog_test.c libipulog.a
#	$(CC) $(CFLAGS) ulog_test.c libipulog.a -o ulog_test
ulogserv: ulogserv2.c libipulog.o ../shared/flowdata.o ../shared/flowdata.h
	$(CC) $(CFLAGS) ulogserv2.c 
	$(LD) ulogserv2.o libipulog.o ../shared/flowdata.o -o ulogserv


libipulog.o: libipulog.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o 

distclean: clean
	$(RM) ulogserv

