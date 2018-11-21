#define PORTNO "22222"
#define INVALID_FILE_MODE 5
#define herror(INVALID_FILE_MODE) printf(stderr, "Invalid File Mode")
#define hstrerror(INVALID_FILE_MODE) "Invalid File Mode"
#define MAX_BUFFER 7000

extern int netopen(const char* pathname, int flags);
extern ssize_t netread(int fildes, void *buf, size_t nbyte);
extern ssize_t netwrite(int fildes, const void *buf, size_t nbyte);
extern int netclose(int netfd);
extern int netserverinit(char* hostname, int filemode);

static int sockfd = -1;
static char mode;