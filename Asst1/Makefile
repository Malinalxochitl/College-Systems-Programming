all: memgrind

memgrind: memgrind.o mymalloc.o
	gcc -g -Wall -Werror -fsanitize=address memgrind.o mymalloc.o -O0 -o memgrind

memgrind.o: memgrind.c mymalloc.h
	gcc -g -Wall -Werror -fsanitize=address -c -O0 memgrind.c

mymalloc.o: mymalloc.c mymalloc.h
	gcc -g -Wall -Werror -fsanitize=address -c -O0 mymalloc.c

clean:
	rm -rf memgrind
