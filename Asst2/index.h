#define BUCKETSIZE 5000
typedef struct FileNode {
	struct FileNode* next;
	int* count;                 // for linked list of file names and occurences
	char* fileName;
} fileNode;

typedef struct HashNode {
	struct HashNode* next;
	char* word;
	fileNode* fileList;           // for chaining in the Hash table.
} HashNode;

typedef struct node {
	struct node* next;      // for storing a complete list of alphabetical words
	char* word;		// we need it since the hash table is not alphabetical
} node;

/*Checks if the given filename exists
  Returns 1 if the file exists, 0 if it does not*/
int file_exist (char *filename) {
  struct stat buffer;   
  return (stat (filename, &buffer) == 0);
}
static node* List;             //Contains all the words in all the files sorted alphabetically
static HashNode* Table[BUCKETSIZE];

node* createNode(char* word);
int val(char A);
int compare(const char* A, const char* B);
fileNode* CreateFileNode(const char* fileName);
HashNode* CreateHashNode(const char* word, const char* file);
int hash(char* word);
void swapI(int* a, int* b);
void AddtoFlist(HashNode* F, const char* file);
int AddtoHash(char* word, const char* file);
void AddtoList(char* word);
void fileprint(char* file);
int f(const char* F, const struct stat* B, int C, struct FTW* D);
