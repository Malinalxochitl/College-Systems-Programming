all: client

client: client.c
	gcc -g -Wall -Werror -fsanitize=address client.c -o client -lm

clean:
	rm -rf client
