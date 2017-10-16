all: ps

ps: ps.o
	$(CC) $? -g -o $@

%.o: %.c
	$(CC) $? -c -g -o $@

clean:
	rm -f *.o
	rm -f ./ps
