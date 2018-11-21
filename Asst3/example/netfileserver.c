#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#define MAX_BUFFER 1024
#define my_port "22225"

void *process (void* arg) {
	int n;
	int* sock = (int*) arg;
	char buffer[MAX_BUFFER];
	memset(&buffer, 0, MAX_BUFFER);
	n = read(sock[1], buffer, MAX_BUFFER-1);
	printf("recieved %d bytes:\n%s\n", n, buffer);
	write(sock[1], "Message recieved", 17);
	close(sock[1]);
	if (n == 2 && buffer[0] == 'X') {
		close(sock[0]);
		exit(1);
	}
	return NULL;
}

int main(int argc, char ** argv)
	{
		int err, sfd, connection_id;
		struct addrinfo hints, *res;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = PF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		
		getaddrinfo(NULL, my_port, &hints, &res);
		
		sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
		err = bind(sfd, res->ai_addr, res->ai_addrlen);
		freeaddrinfo(res);
		err = listen(sfd, 100);
		
		if(err) {
			printf("error");
		}
		
		struct sockaddr_storage client_addr;
		socklen_t client_addr_len;
		client_addr_len = sizeof(struct sockaddr_storage);
		int array[2];
		array[0] = sfd;
		while(1) {
			connection_id = accept(sfd, (struct sockaddr *) &client_addr, &client_addr_len);
			array[1] = connection_id;
			pthread_t temp;
			pthread_create(&temp, NULL, process, &array);
			pthread_detach(temp);
		}
		return 0;
	}