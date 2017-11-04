NAME := vfat-root-ls

all: $(NAME)

$(NAME): vfat.o
	$(CC) $? -g -o $@

%.o: %.c
	$(CC) $? -c -g -o $@

clean:
	rm -f *.o
	rm -f $(NAME)
