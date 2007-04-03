LDFLAGS=-lGL -lGLU -lglut -lpthread -lavcodec -lavformat

display: display.o subnets.o video.o $(COMMON)/flowdata.o 

display.o: display.c $(COMMON)/flowdata.h pixelmapping.h video.h subnets.h

subnets.o: subnets.c subnets.h

video.o: video.c $(COMMON)/flowdata.h video.h

$(COMMON)/flowdata.o:
	make CFLAGS="$(CFLAGS)" -C $(COMMON) -f Makefile flowdata.o

clean:
	rm -f *.o

distclean: clean
	rm -f display

.PHONY: clean distclean $(COMMON)/flowdata.o

