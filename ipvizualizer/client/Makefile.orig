LDFLAGS=-lGL -lGLU -lSDL -lpthread -lavcodec -lavformat

display: display.o subnets.o video.o datapoint.o $(COMMON)/flowdata.o constants.o $(COMMON)/config.o

display.o: display.c $(COMMON)/flowdata.h pixelmapping.h video.h subnets.h text.c $(COMMON)/config.h

subnets.o: subnets.c subnets.h

video.o: video.c $(COMMON)/flowdata.h video.h

$(COMMON)/flowdata.o:
	make CFLAGS="$(CFLAGS)" -C $(COMMON) -f Makefile flowdata.o

$(COMMON)/config.o:
	make CFLAGS="$(CFLAGS)" -C $(COMMON) -f Makefile config.o

clean:
	rm -f *.o

distclean: clean
	rm -f display

.PHONY: clean distclean $(COMMON)/flowdata.o

