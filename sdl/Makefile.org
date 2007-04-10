all: server client

INCLUDES = flowdata.h mygettime.h structs.h
CC = gcc -c -g
LINK = gcc

%.o: %.c $(INCLUDES)
	$(CC) $<

server: flowserv.o flowdata.h flowdata.o
	$(LINK) -lpcap flowserv.o flowdata.o -o flowserv

client: display.o video.o flowdata.o  subnets.o flowdata.h video.h subnets.h
	$(LINK) display.o video.o flowdata.o subnets.o -lGL -lGLU -lglut -lpthread -lavcodec -lavformat -o display

clean: 
	rm -f *.o

squeakyclean: clean
	rm -f flowserv display *.avi
