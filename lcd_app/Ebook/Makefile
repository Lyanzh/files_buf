CROSSCOMPILE := arm-linux-

#CFLAGS := -Wall -O2 -c
CFLAGS := -Wall -g -c
CFLAGS += -I $(PWD)/include
CFLAGS += -DMEMWATCH -DMEMWATCH_STDIO

LDFLAGS := -lm -lfreetype -lts

CC := $(CROSSCOMPILE)gcc
LD := $(CROSSCOMPILE)ld

OBJS := main.o \
		draw.o \
		display/disp_manager.o \
		display/fb_lcd.o \
		encoding/encoding_manager.o \
		encoding/ascii.o \
		encoding/utf-8.o \
		encoding/utf-16le.o \
		encoding/utf-16be.o \
		fonts/fonts_manager.o \
		fonts/ascii.o \
		fonts/gbk.o \
		fonts/freetype.o \
		input/input_manager.o \
		input/stdin.o \
		input/touchscreen.o \
		memwatch.o

all: $(OBJS)
	$(CC) $(LDFLAGS) -o show_file $^

clean:
	rm -f show_file
	rm -f $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<