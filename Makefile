CC=gcc
CFLAGS=-Wall -Wextra `pkg-config --cflags libpulse cairo x11`
LDLIBS=-lm `pkg-config --libs libpulse cairo x11`
LDFLAGS=

OBJS=thing.o audio.o indicator.o

debug : CFLAGS +=-g
debug : LDFLAGS +=-g
debug : all

all : $(OBJS)
	$(CC) $(LDLIBS) $(OBJS) -o thing $(LDFLAGS)

thing.o : audio.h indicator.h
audio.o : audio.h
indicator.o : indicator.h

.PHONY : clean
clean :
	rm -f thing $(OBJS)
