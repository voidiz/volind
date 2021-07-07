CC=gcc
CFLAGS=`pkg-config --cflags libpulse sdl2`
LDLIBS=-lm `pkg-config --libs libpulse sdl2`
LDFLAGS=

OBJS=main.o audio.o indicator.o

all : $(OBJS)
	$(CC) $(LDLIBS) $(OBJS) -o volind $(LDFLAGS)

debug : CFLAGS +=-g -Wall -Wextra -DDEBUG
debug : all

main.o : audio.h indicator.h
audio.o : audio.h
indicator.o : indicator.h

.PHONY : clean
clean :
	rm -f volind $(OBJS) vgcore.*
