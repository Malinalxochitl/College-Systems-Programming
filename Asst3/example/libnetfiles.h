#define my_port 22222

extern int netopen(const char* pathname, int flags);
extern ssize_t netread(int fildes, void *buf, size_t nbyte);
extern ssize_t netwrite(int fildes, const void *buf, size_t nbyte);
extern int netclose(int fd);
extern int netserverinit(char* hostname, int filemode);