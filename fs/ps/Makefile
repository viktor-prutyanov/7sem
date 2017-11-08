all: ps

ps: ps.o
	$(CC) $? -Wall -g -o $@

%.o: %.c
	$(CC) $? -Wall -c -g -o $@

clean:
	rm -f *.o
	rm -f ./ps
