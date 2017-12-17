CFLAGS := -Wall -g

ifeq ($(DEBUG), 1)
	CFLAGS += -O1 -fsanitize=address
else
	CFLAGS += -O2
endif

all: btree_test

btree_test: btree.o btree_test.o
	$(CC) $^ $(CFLAGS) -o $@

%.o: %.c
	$(CC) $? $(CFLAGS) -c -o $@

.PHONY: clean all tags

tags:
	ctags -R

clean:
	rm -f *.o
	rm -f btree_test
