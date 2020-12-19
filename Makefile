CC=gcc
CFLAGS=`pkg-config --cflags libpulse cairo x11 xrandr`
LDLIBS=-lm `pkg-config --libs libpulse cairo x11 xrandr`
LDFLAGS=

OBJS=thing.o audio.o indicator.o

all : $(OBJS)
	$(CC) $(LDLIBS) $(OBJS) -o thing $(LDFLAGS)

debug : CFLAGS +=-g -Wall -Wextra -DDEBUG
debug : all

thing.o : audio.h indicator.h
audio.o : audio.h
indicator.o : indicator.h

.PHONY : clean
clean :
	rm -f thing $(OBJS)
