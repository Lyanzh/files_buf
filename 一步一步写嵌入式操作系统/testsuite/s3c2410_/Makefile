CC=arm-linux-gcc
LD=arm-linux-ld
CFLAGS= -c -g -march=armv4t -mtune=arm920t
LDFLAGS= -N -p -X -Tmyhello.lds

myhello: start.o myhello.o
	$(LD) $(LDFLAGS)  start.o myhello.o -o myhello
	arm-linux-objdump -xS myhello > myhello.s
	arm-linux-readelf -a myhello > myhello.r
	arm-linux-nm myhello > myhello.n 

start.o: start.S
	$(CC) $(CFLAGS) start.S

myhello.o: myhello.c
	$(CC) $(CFLAGS) myhello.c

clean:
	rm -rf *.o myhello *.r *.n *.s
