#
#makefile for helloforSkyeye
#
#author: SU Hang
#Date:   2004-08-28

#begin
    CC=arm-linux-gcc
    LD=arm-linux-ld
    CFLAGS= -c -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -pipe -g -mapcs-32 -march=armv4 -mtune=arm7tdmi 
    LDFLAGS= -N -p -X -Thello.lds
    
    LIB=
    
    all: hello 

    hello: start.o hello.o
	$(LD) $(LDFLAGS)  start.o hello.o -o hello
	arm-linux-objdump -xS hello > hello.s
	arm-linux-readelf -a hello > hello.r
	arm-linux-nm hello > hello.n
	 
    start.o:start.S
	$(CC) $(CFLAGS) start.S	 
	
    hello.o:hello.c
	$(CC) $(CFLAGS) hello.c

    clean:
	rm -rf *.o *.elf *.gdb hello *.r *.n *.s


#end
