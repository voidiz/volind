CC=gcc
CFLAGS=-Wall -Werror `pkg-config --cflags libpulse cairo x11`
LDLIBS=-lm `pkg-config --libs libpulse cairo x11`

OBJS=thing.o audio.o indicator.o

all : $(OBJS)
	$(CC) -o thing $(LDLIBS) $(OBJS)

thing.o : audio.h indicator.h
audio.o : audio.h
indicator.o : indicator.h

.PHONY : clean
clean :
	rm thing $(OBJS)
