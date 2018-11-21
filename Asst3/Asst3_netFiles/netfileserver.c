#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>

#define BUCKETSIZE 100
#define MAX_BUFFER 7000
#define my_port "22225"

typedef struct filenode{ //node used for tracking files
	pthread_mutex_t writelock; //locks
	pthread_mutex_t nodelock;
	pthread_cond_t doneReading;
	pthread_cond_t doneWriting;
	int status;
	char* path;
	int reads;
	int writes;
	int unrestricted;
	int exclusive;
	int transaction;
	struct filenode* next;
} filenode;

typedef struct listnode{ //node used for tracking file descriptor
	int fd;
	char* path;
	char mode;
	int r;
	int w;
	struct listnode* next;
} listnode;

static filenode* Table[BUCKETSIZE]; //this hash table tracks every open file on the server
static listnode* List[BUCKETSIZE]; //this hash table tracks every file descriptor currently open
pthread_mutex_t tablelock; //locks so that the hash tables don't get messed up
pthread_mutex_t listlock;

/*Simple hash function for Table
  Hashes using the full path of the file*/
int hash(char* word){
	int l = strlen(word);
	unsigned int R = 0;                         
	int i;
	for(i = 0; i<l; i++){
		R+=((int) word[i]+31*R)%BUCKETSIZE;
	}
	return R % BUCKETSIZE;
}
/*Adds the given file descriptor to the List hash table*/
void AddtoList(char* path, int fd, int flags, char mode) {
	int bucket = fd % BUCKETSIZE;
	listnode *ptr, *prev = List[bucket];
	for(ptr = List[bucket]; ptr != NULL; ptr = ptr->next) { //navigate through bucket
		prev = ptr;
	}
	
	listnode* newNode = malloc(sizeof(listnode)); //add node to list
	newNode->fd = fd;
	newNode->path = (char*) malloc(strlen(path)+1);
	newNode->mode = mode;
	if((flags & O_ACCMODE) == O_WRONLY) {
		newNode->w = 1;
		newNode->r = 0;
	} else if((flags & O_ACCMODE) == O_RDONLY) {
		newNode->w = 0;
		newNode->r = 1;
	} else {
		newNode->w = 1;
		newNode->r = 1;
	}
	if(prev == NULL) {
		prev = newNode;
	} else {
		prev->next = newNode;
	}
}
/*This function serves two purposes:
  It determines whether the given path can be opened with it's current policies,
  as well as update Table if it can be opened*/
int AddtoTable(char* path, char mode, int flags) {
	int bucket = hash(path), fd;
	filenode *ptr, *prev = Table[bucket];
	
	for(ptr = Table[bucket]; ptr != NULL; ptr = ptr->next) { //navigates through bucket
		
		if(strcmp(ptr->path, path) == 0) { //matching node found
			if(mode == 'U') { //unrestricted open errorchecking
				if(ptr->transaction > 0) {
					errno = EACCES;
					return -1;
				} else if(ptr->exclusive > 0 && ((flags & O_ACCMODE) == O_WRONLY || (flags & O_ACCMODE) == O_RDWR)) {
					errno = EACCES;
					return -1;
				}
			} else if (mode == 'E') { //exclusive open errorchecking
				if(ptr->transaction > 0) {
					errno = EACCES;
					return -1;
				} else if(ptr->writes > 0) {
					errno = EACCES;
					return -1;
				}
			} else { //transaction fails immediately on already open file
				errno = EACCES; 
				return -1;		 
			}
			fd = open(path, flags);
			if(fd == -1) {
				return -1;
			}
			pthread_mutex_lock(&ptr->nodelock);
			if((flags & O_ACCMODE) == O_RDONLY) {
				ptr->reads = ptr->reads + 1;
			} else if ((flags & O_ACCMODE) == O_WRONLY) {  //adds read/write permissions to existing node
				ptr->writes = ptr->writes + 1;
			} else {
				ptr->reads = ptr->reads + 1;
				ptr->writes = ptr->writes + 1;
			}
			if(mode == 'U') {
				ptr->unrestricted = ptr->unrestricted + 1;
			} else {
				ptr->exclusive = ptr->exclusive + 1;
			}
			pthread_mutex_unlock(&ptr->nodelock);
			return fd;
			
		}
		prev = ptr;
	}
	fd = open(path, flags);
	if(fd == -1) {
		return -1;
	}
	
	pthread_mutex_lock(&tablelock); 										//node doesn't exist in Table, therefore add a new node
	filenode* newNode = (filenode*) (malloc(sizeof(filenode)));
	if(mode == 'U') {
		newNode->unrestricted = 1;
	} else if(mode == 'E') {
		newNode->exclusive = 1;
	} else {
		newNode->transaction = 1;
	}
	if((flags & O_ACCMODE) == O_RDONLY) {
		newNode->reads = 1;
	} else if ((flags & O_ACCMODE) == O_WRONLY) {
		newNode->writes = 1;
	} else {
		newNode->reads = 1;
		newNode->writes = 1;
	}
	pthread_mutex_init(&newNode->writelock, NULL);
	pthread_mutex_init(&newNode->nodelock, NULL);
	pthread_cond_init(&newNode->doneReading, NULL);
	pthread_cond_init(&newNode->doneWriting, NULL);
	newNode->path = (char*) (malloc(strlen(path)+1));
	strcpy(newNode->path, path);
	if(prev == NULL) {
		prev = newNode;
	} else {
		prev->next = newNode;
	}
	pthread_mutex_unlock(&tablelock);
	return fd;
}
/*Simple driver function for opening a file*/
int fileopen(char* path, char mode, int flags, int length) {
	int fd;
	fd = AddtoTable(path, mode, flags);
	if(fd != -1) {
		AddtoList(path, fd, flags, mode);
	}
	return fd;
}
/*This function closes a file descriptor, and updates both List and
  Table. If a file isn't open by any client at all, the respective
  nodes are removed.*/
int fileclose(int fd) {
	if(close(fd) == -1) { //closing the file descriptor
		return -1;
	}
	int bucket = fd % BUCKETSIZE;
	char* path;
	char filemode;
	int r, w;
	pthread_mutex_lock(&listlock);
	listnode *prev = List[bucket], *ptr = prev->next;
	if(prev->fd == fd) { //node is head of bucket
		path = (char*)malloc(sizeof(prev->path));
		strcpy(path, prev->path);
		r = prev->r;
		w = prev->w;
		filemode = prev->mode;
		free(prev->path);
		free(prev);
		List[bucket] = ptr;
	} else {
		while(ptr->fd != fd) { //navigates through List
			prev = ptr;
			ptr = ptr->next;
		}
		path = (char*)malloc(sizeof(ptr->path)); //removes node
		strcpy(path, ptr->path);
		r = ptr->r;
		w = ptr->w;
		filemode = ptr->mode;
		prev->next = ptr->next;
		free(ptr->path);
		free(ptr);
	}
	pthread_mutex_unlock(&listlock);
	
	bucket = hash(path);						//begin updating Table
	pthread_mutex_lock(&tablelock);
	filenode *previ = Table[bucket], *potr = previ->next;
	if(strcmp(path, previ->path) == 0) { //node is head of bucket
		previ->reads = previ->reads - r;
		previ->writes = previ->writes - w;
		if(filemode == 'U') {
			previ->unrestricted = previ->unrestricted - 1;
		} else if (filemode == 'E') {
			previ->exclusive = previ->exclusive - 1;
		} else {
			previ->transaction = previ->transaction - 1;
		}
		if(previ->reads == 0 && previ->writes == 0) { //file isn't open by any client, thus node is removed
			pthread_mutex_destroy(&previ->writelock);
			pthread_mutex_destroy(&previ->nodelock);
			pthread_cond_destroy(&previ->doneReading);
			pthread_cond_destroy(&previ->doneWriting);
			free(previ->path);
			free(previ);
			Table[bucket] = potr;
		}
	} else {
		while(strcmp(path, potr->path) != 0) { //navigating through Table
			previ = potr;
			potr = potr->next;
		}
		if(filemode == 'U') {
			potr->unrestricted = potr->unrestricted - 1;
		} else if (filemode == 'E') {
			potr->exclusive = potr->exclusive - 1;
		} else {
			potr->transaction = potr->transaction - 1;
		}
		potr->reads = potr->reads - r;
		potr->writes = potr->writes - w;
		if(potr->reads == 0 && potr->writes == 0) { //file isn't open by any client, thus node is removed
			pthread_mutex_destroy(&potr->writelock);
			pthread_mutex_destroy(&potr->nodelock);
			pthread_cond_destroy(&potr->doneReading);
			pthread_cond_destroy(&potr->doneWriting);
			free(potr->path);
			previ->next = potr->next;
			free(potr);
		}
	}
	pthread_mutex_unlock(&tablelock);
	return 0;
}

/*This is the function is what each thread uses to service client requests. Every message recieved by the client
  is sent here. If nothing is recieved over 20 seconds, the thread exits. Protocols are unpacked here and sent
  to their respective functions.*/
void *process (void* arg) { //contents are a placeholder right now
	while(1) {
		int rv, n;
		int* sock = (int*) arg;
		char buffer[MAX_BUFFER];
		memset(&buffer, 0, MAX_BUFFER);
		fd_set set;
		struct timeval timeout;
		FD_ZERO(&set);
		FD_SET(sock[1], &set);
		
		timeout.tv_sec = 0;
		timeout.tv_usec = 20000000; //20 seconds
		
		rv = select(sock[1]+1, &set, NULL, NULL, &timeout);
		if(rv == -1) {
			perror("select");
		} else if (rv == 0) { //if no packet is recieved after 20 seconds the connection is terminated
			close(sock[1]);
			break;
		} else {
			n = read(sock[1], buffer, MAX_BUFFER); //reading from socket
		}
		
		if (n == 2 && buffer[0] == 'X') { //killswitch for server
			close(sock[1]);					 //if server is not killed via this method, port becomes unusable for some time
			close(sock[0]);
			exit(0);
		}
		
		if(buffer[0] == 'O') { 												//open
		
			int flags, length, fd;
			char mode = buffer[1];
			memcpy(&flags, buffer+2, sizeof(int));
			memcpy(&length, buffer+2+sizeof(int), sizeof(int));
			char path[length+1];
			memcpy(path, buffer+2+(sizeof(int)*2), length+1);
			
			fd = fileopen(path, mode, flags, length);
			char message[1+sizeof(int)];
			if(fd == -1) { //error opening file
				message[0] = 'e';
				memcpy(message+1, &errno, sizeof(int));
			} else { //file opened successfully
				message[0] = 'f';
				memcpy(message+1, &fd, sizeof(int));
			}
			write(sock[1], message, 1+sizeof(int));
			
		} else if(buffer[0] == 'R') { 									//read
		
			int fd;
			size_t bytes;
			memcpy(&fd, buffer+1, sizeof(int));
			int bucket = fd % BUCKETSIZE;
			listnode* ptr1 = List[bucket];
			while(ptr1 != NULL && ptr1->fd != fd){ //navigating through List
				ptr1 = ptr1->next;
			}
			if(ptr1 == NULL) { //file descriptor node not found
				char newmessage[sizeof(int)+1];
				newmessage[0] = 'e';
				memcpy(newmessage+1, &errno, sizeof(int));
				write(sock[1], newmessage, sizeof(int)+1);
			} else {
				bucket = hash(ptr1->path);
				filenode *ptr2 = Table[bucket];
				while(ptr2 != NULL && ptr2->path != ptr1->path) { //navigating through Table
					ptr2 = ptr2->next;
				}
				pthread_mutex_lock(&ptr2->writelock);
				while(ptr2->status == 2) {
					pthread_cond_wait(&ptr2->doneWriting, &ptr2->writelock); //other thread is writing
				}
				ptr2->status = 1;
				memcpy(&bytes, buffer+1+sizeof(int), sizeof(size_t));
				char message[bytes+1];
				message[0] = 'r';
				ssize_t i = read(fd, message+1, bytes);
				if(i == -1) {
					char newmessage[sizeof(int)+1];
					newmessage[0] = 'e';
					memcpy(newmessage+1, &errno, sizeof(int));
					write(sock[1], newmessage, sizeof(int)+1);
				} else {
					write(sock[1], message, i);
				}
				ptr2->status = 0;
				pthread_mutex_unlock(&ptr2->writelock);
				pthread_cond_signal(&ptr2->doneReading);
			}
		} else if(buffer[0] == 'W') { 											//write
		
			int fd;
			size_t bytes;
			memcpy(&fd, buffer+1, sizeof(int));
			memcpy(&bytes, buffer+sizeof(int), sizeof(size_t));
			char data[bytes];
			memcpy(data, buffer+sizeof(int)+sizeof(size_t), bytes);
			int bucket = fd % BUCKETSIZE;
			listnode *ptr1 = List[bucket];
			while(ptr1 != NULL && ptr1->fd != fd){ //navigate through List
				ptr1 = ptr1->next;
			}
			if(ptr1 == NULL) { //file descriptor node not found
				char newmessage[sizeof(int)+1];
				newmessage[0] = 'e';
				memcpy(newmessage+1, &errno, sizeof(int));
				write(sock[1], newmessage, sizeof(int)+1);
			} else {
				bucket = hash(ptr1->path);
				filenode *ptr2 = Table[bucket];
				while(ptr2 != NULL && ptr2->path != ptr1->path) { //navigate through Table
					ptr2 = ptr2->next;
				}
				pthread_mutex_lock(&ptr2->writelock);
				while(ptr2->status == 1) {
					pthread_cond_wait(&ptr2->doneReading, &ptr2->writelock); //other thread is reading
				}
				while(ptr2->status == 2) {
					pthread_cond_wait(&ptr2->doneWriting, &ptr2->writelock); //other thread is writing
				}
				ptr2->status = 2;
				ssize_t ret = write(fd, data, bytes);
				if(ret == -1) {
					char message[sizeof(int)+1];
					message[0] = 'e';
					memcpy(message+1, &errno, sizeof(int));
					write(sock[1], message, sizeof(int)+1);
				} else {
					char message[sizeof(ssize_t)+1];
					message[0] = 'w';
					memcpy(message+1, &ret, sizeof(ssize_t));
					write(sock[1], message, sizeof(ssize_t)+1);
				}
				ptr2->status = 0;
				pthread_mutex_unlock(&ptr2->writelock);
				pthread_cond_signal(&ptr2->doneWriting);
			}
		} else if(buffer[0] == 'C') { 												//close
		
			int fd;
			memcpy(&fd, buffer+2, sizeof(int));
			
			if(fileclose(fd) == -1) { //file failed to close
				char message[sizeof(int)+1];
				message[0] = 'e';
				memcpy(message+1, &errno, sizeof(int));
				write(sock[1], message, 1+sizeof(int));
			} else {
				write(sock[1], "yes", 4);
			}
		}
	}
	return NULL;
}
/*Main function initializes the server and spawns new threads with each new connection*/
int main(int argc, char ** argv)
	{
		pthread_mutex_init(&tablelock, NULL);
		pthread_mutex_init(&listlock, NULL);
		int err, sfd, connection_id;
		struct addrinfo hints, *res;
		memset(&hints, 0, sizeof(struct addrinfo)); //initializations
		hints.ai_family = PF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		
		getaddrinfo(NULL, my_port, &hints, &res); //setting up the connection
		
		sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //socket setup
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
		while(1) { //new client connections are sent to their own thread
			connection_id = accept(sfd, (struct sockaddr *) &client_addr, &client_addr_len);
			array[1] = connection_id;
			pthread_t temp;
			pthread_create(&temp, NULL, process, &array);
			pthread_detach(temp);
		}
		return 0;
	}