all: invertedIndex

invertedIndex: index.c index.h
	gcc -g -Wall -Werror -fsanitize=address index.c -o invertedIndex -lm

clean:
	rm -rf invertedIndex
