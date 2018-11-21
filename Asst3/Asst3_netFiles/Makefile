all: netfileserver

netfileserver: netfileserver.c
	gcc -lpthread -g -Wall -Werror -fsanitize=address netfileserver.c -o netfileserver -lm

clean:
	rm -rf netfileserver
