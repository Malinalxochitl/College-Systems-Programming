#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define HOSTNAME "grep.cs.rutgers.edu"
#define PORTNO "22225"

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, n;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(HOSTNAME, PORTNO, &hints, &res);
    
    if (res == NULL) {
    	printf("error");
    	exit(0);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) 
        error("ERROR opening socket");
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) 
        error("ERROR connecting");
    char buffer[256];
    printf("Please enter the message: ");
    memset(buffer,'0',256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    return 0;
}
