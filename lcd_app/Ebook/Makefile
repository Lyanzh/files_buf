CROSSCOMPILE := arm-linux-

CFLAGS := -Wall -O2 -c
CFLAGS += -I $(PWD)/include

LDFLAGS := -lm -lfreetype

CC := $(CROSSCOMPILE)gcc
LD := $(CROSSCOMPILE)ld

OBJS := main.o \
		draw.o \
		display/disp_manager.o \
		display/fb_lcd.o \
		encoding/encoding_manager.o \
		encoding/ascii.o \
		fonts/fonts_manager.o \
		fonts/freetype.o

all: $(OBJS)
	$(CC) $(LDFLAGS) -o show_file $^

clean:
	rm -f show_file
	rm -f $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<