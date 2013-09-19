all: fwsrv


fwsrv: fwsrv.c structs.h mygettime.h ../shared/flowdata.h ../shared/flowdata.c
	gcc -O2 fwsrv.c ../shared/flowdata.c -lpcap -o fwsrv

clean:
	rm -f *.o *~
