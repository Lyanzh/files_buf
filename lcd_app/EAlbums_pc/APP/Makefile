CROSSCOMPILE := 

#CFLAGS := -Wall -O2 -c
CFLAGS := -Wall -g -c
CFLAGS += -I $(PWD)/include
CFLAGS += -DMEMWATCH -DMEMWATCH_STDIO

# gcc 4.6.3 use -pthread instead of -lpthread

LDFLAGS := -lm -pthread -L /usr/local/lib -ljpeg

CC := $(CROSSCOMPILE)gcc
LD := $(CROSSCOMPILE)ld

OBJS := main.o \
		draw.o \
		file.o \
		display/disp_manager.o \
		display/fb_lcd.o \
		display/fb_pc.o \
		input/input_manager.o \
		input/stdin.o \
		input/button.o \
		format/format_manager.o \
		format/bmp.o \
		format/jpeg.o \
		page/page_manager.o \
		page/main_page.o \
		page/auto_page.o \
		page/browse_page.o \
		page/setting_page.o \
		page/timer_page.o \
		page/page_mem.o \
		memwatch.o

all: $(OBJS)
	$(CC) -o show_pic $^ $(LDFLAGS)

clean:
	rm -f show_file
	rm -f $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<