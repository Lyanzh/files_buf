CROSSCOMPILE := 

#CFLAGS := -Wall -O2 -c
CFLAGS := -Wall -g -c
CFLAGS += -I $(PWD)/include
CFLAGS += -DMEMWATCH -DMEMWATCH_STDIO

LDFLAGS := -lm -lpthread

CC := $(CROSSCOMPILE)gcc
LD := $(CROSSCOMPILE)ld

OBJS := main.o \
		draw.o \
		display/disp_manager.o \
		display/fb_lcd.o \
		display/fb_pc.o \
		format/format_manager.o \
		format/bmp.o \
		memwatch.o

all: $(OBJS)
	$(CC) $(LDFLAGS) -o show_pic $^

clean:
	rm -f show_file
	rm -f $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<