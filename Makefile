SHELL := /bin/sh
CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra
EXEC = encode decode
OBJS = word.o trie.o io.o set.o

all: $(EXEC)

encode: $(OBJS) encode.o
	$(CC) -o encode $(OBJS) encode.o `pkg-config --libs gmp`

decode: $(OBJS) decode.o
	$(CC) -o decode $(OBJS) decode.o `pkg-config --libs gmp`

encode.o: encode.c
	$(CC) $(CFLAGS) -c encode.c

decode.o: decode.c
	$(CC) $(CFLAGS) -c decode.c

word.o: word.c
	$(CC) $(CFLAGS) -c word.c

trie.o: trie.c
	$(CC) $(CFLAGS) -c trie.c

io.o: io.c
	$(CC) $(CFLAGS) -c io.c

set.o: set.c
	$(CC) $(CFLAGS) -c set.c

clean:
	rm -f $(OBJS) $(EXEC) encode encode.o decode decode.o

format:
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build --use-cc=$(CC) make
