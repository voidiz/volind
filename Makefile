CC		= gcc
CFLAGS	= -g
LIBS	= -lpulse

main: main.c
	$(CC) $(CFLAGS) $(LIBS) main.c -o thing

valgrind:
	valgrind --leak-check=full \
			 --show-leak-kinds=all \
			 --track-origins=yes \
			 --verbose \
			 --log-file=valgrind-out.txt \
			 ./thing
