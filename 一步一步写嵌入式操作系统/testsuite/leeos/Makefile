CC = arm-linux-gcc
LD = arm-linux-ld
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump

CFLAGS = -O2 -g
ASFLAGS = -O2 -g
LDFLAGS = -Tleeos.lds -Ttext 0x30000000

OBJS = init.o start.o boot.o abnormal.o

.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(CC) $(ASFLAGS) -c $<
	
leeos:$(OBJS)
	$(CC) -static -nostartfiles -nostdlib $(LDFLAGS) $? -o $@ -lgcc
	$(OBJCOPY) -O binary $@ leeos.bin
	$(OBJDUMP) -D -m arm leeos > leeos.dis
	
clean:
	rm *.o leeos leeos.bin -f