#define _XOPEN_SOURCE 700 
#include <stdlib.h>
#include <unistd.h>
#include <ftw.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "index.h"

/*Debugging function*/
/*void print(){
	node* ptr;
	for(ptr = List; ptr != NULL; ptr = ptr->next){
		printf("WORD: %s\n", ptr->word);
	}
	printf("END\n");
	int i;
	for(i = 0; i<BUCKETSIZE; i++){
		if(Table[i] != NULL){
			HashNode* p;
			for(p = Table[i]; p != NULL; p = p->next){
				printf("WORD: %s\n", p->word);
				fileNode* ptr;
				for(ptr = (p->fileList); ptr != NULL; ptr = ptr->next){
					printf("\tFILE: %s COUNT: %d\n", ptr->fileName, ptr->count[0]);
				}
			}
		}
	}
}*/

int main(int argc, char** argv){
	if(argc != 3){
		printf("This program requires two inputs\n");
		return 0;
	}
	if(file_exist(argv[1])){ //if the file exists, user is prompted to overwrite it
		char * answer = (char*) malloc(100);
		printf("The file we need to write to already exists. Would you like to overwrite it? yes or no\n");
		while(1){
			scanf("%s", answer);
			if(strcmp(answer ,"yes") == 0){
				printf("Alrighty then. Program continuing :)\n");
				break;
			}
			else if(strcmp(answer, "no") == 0){
				printf("Program terminating then :(\n");
				return 0;
			}
			else{
				printf("Please type is yes or no EXACTLY. Would you like to overwrite the file?\n");
			}
		}
	}
	int i;
	for(i = 0; i<BUCKETSIZE; i++){
		Table[i] = NULL;
	}
	List = NULL;
	int fd_limit = 15;
	int flags = FTW_DEPTH | FTW_MOUNT;
	int ret;
	char* startpath = argv[2];
	ret = nftw(startpath, f, fd_limit,  flags);
	if(ret == -1) {
		fprintf(stderr, "Error trying to index %s. %s. Terminating program.\n", startpath, strerror(errno));
		return 0;
	}
	fileprint(argv[1]);
	return 0;
}

/*Creates a new node from a given word
  Returns newly malloced node containing the word*/
node* createNode(char* word) {
	node* A = (node*) malloc(sizeof(node));
	A->word = (char*) malloc(strlen(word)+1);
	strcpy(A->word, word);
	A->next = NULL;
	return A;
}
/*Creates a fileNode, which will be used for tracking filenames
  Returns a fileNode for the given fileName*/
fileNode* CreateFileNode(const char* fileName){
	fileNode* returnValue = (fileNode*) malloc(sizeof(fileNode));    
	returnValue->count = (int*) malloc(sizeof(int));                
	returnValue->count[0] = 1;				       
	returnValue->next = NULL;
	returnValue->fileName = (char*) malloc(strlen(fileName)+1);
	strcpy(returnValue->fileName, fileName);
	return returnValue;
}

/*Creates a HashNode which will be used for sorting in the hash table
  Returns a HashNode with the given word and filename*/
HashNode* CreateHashNode(const char* word, const char* file){
	HashNode* returnValue = (HashNode*) (malloc(sizeof(HashNode)));
	returnValue->word = (char*) (malloc(strlen(word)+1));
	(returnValue-> fileList) = CreateFileNode(file);
	returnValue->next = NULL;
	strcpy(returnValue->word, word);
	return returnValue;
}

/*Simple hash function. Sums ASCII values(with a factor of 31 to make the # of collisions reasonable) to determine bucket placement
  Returns int signifying which bucket to be used*/
int hash(char* word){
	int l = strlen(word);
	unsigned int R = 0;                         
	int i;
	for(i = 0; i<l; i++){
		R+=((int) word[i]+31*R)%BUCKETSIZE;
	}
	return R % BUCKETSIZE;
}

/*Swaps the values of two int pointers*/
void swapI(int* a, int* b){
	int temp = *a;
	*a = *b;
	*b = temp;
}

/*Adds the given file into the HashNode's fileList
  The fileList is adjusted according to count with every file added to it*/
void AddtoFlist(HashNode* F, const char* file){
	fileNode* files = F->fileList;
	if(files->next == NULL){
		if(compare(files->fileName, file) == 0){
			files->count[0]++;
			return;
		} 
		fileNode* newNode = CreateFileNode(file);
		if(files->count[0] > 1){
			files->next = newNode;
			return;                                      // adds to the file list
		} else if(compare(files->fileName, file) < 0) {
			files->next = newNode;
			return;
		} else{
			newNode->next = files;
			F->fileList = newNode;
			return;
		}
	}
	fileNode* ptr, *end;
	end = files;
	for(ptr = files->next; ptr != NULL; ptr = ptr->next){
		if(compare(ptr->fileName, file) == 0){
			ptr->count[0]++;
			if((end->count[0] < ptr->count[0]) || ((end->count[0] == ptr->count[0]) && (compare(end->fileName, ptr->fileName) >0)))
			{
				int l1 = strlen(ptr->fileName);
				int l2 = strlen(end->fileName);
				char* temp = (char*) malloc(l1+1);
				strcpy(temp, ptr->fileName);                    //swaps the entries in the nodes
				ptr->fileName = (char*) malloc(l2+1);				//this brings the fileList to sorted order 
				strcpy(ptr->fileName, end->fileName);
				end->fileName = (char*) malloc(l1+1);
				strcpy(end->fileName, temp);
				swapI(end->count, ptr->count);
				free(temp);
			}
			return;
		}
		end = ptr;
	}
	end->next = CreateFileNode(file);
	 
}

/*Adds given word and file to hash table
  Returns 1 if a HashNode exists in the bucket, 0 if a new HashNode is created*/
int AddtoHash(char* word, const char* file){                   // add alphabetical sorting, real hash function, ordered adding(by occurence)
	int place = hash(word);
	if(Table[place] == NULL){
		Table[place] = CreateHashNode(word, file);
		return 0;
	}
	HashNode* ptr, *ptr2;
	ptr2 = Table[place];
	for(ptr = Table[place]; ptr != NULL; ptr = ptr->next){
		if(strcmp(ptr->word, word) == 0){
			AddtoFlist(ptr, file);
			return 1;
		}
		ptr2 = ptr;
	}
	HashNode* newNode = CreateHashNode(word, file);
	ptr2->next = newNode;
	return 0;
}
/*Adds a word to the word list in alphabetical order
  If the word already exists in the list, nothing is changed*/
void AddtoList(char* word){
	if(List == NULL){
		List = createNode(word);
	}
	node* ptr; node* newNode = createNode(word);
	if(compare(List->word, word) > 0){
		newNode->next = List;
		List = newNode;
		return;
	}
	if(compare(List->word, word) == 0){
		return;
	}
	node* ptr2 = List;
	for(ptr = List->next; ptr!= NULL; ptr = ptr->next){
		if(compare(ptr->word,word) > 0){
			ptr2->next = newNode;
			newNode->next = ptr;
			return;
		}
		if(compare(ptr->word, word) == 0){
			return;
		}
		ptr2 = ptr;
	}
	ptr2->next = newNode;
}

/*Prints the index of words and files into an xml file*/
void fileprint(char* file){
	FILE* fp = fopen(file, "w");
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fp, "<fileIndex>\n");
	node* ptr;
	for(ptr = List; ptr != NULL; ptr = ptr->next){
		fprintf(fp, "\t<word text=\"%s\"\n", ptr->word);
		HashNode* Q, *P;
		for(Q = Table[hash(ptr->word)]; Q!=NULL; Q = Q->next){
			P = Q;
			if(strcmp(Q->word, ptr->word) == 0){
				break;                                  // prints indexer into file
			}
		}
		fileNode* p;
		for(p = P->fileList; p!=NULL; p = p->next){
			fprintf(fp, "\t\t<file name=\"%s\">%d</file>\n", p->fileName, p->count[0]);
		}
		fprintf(fp, "\t</word>\n");
	}
	fprintf(fp, "</fileIndex>");
	fclose(fp);
	                                                                                            
}

/*
  This function is called by nftw on every file and directory in the path specified at runtime
  It goes through the specified file, tokenizes words, and adds them to List
  
  The used arguments are as follows:
  const char* fullpath is the path name of the file to be processed
  int filetype specifies if fullpath is a directory or a file
  
  Arguments temp1 and temp2 are passed by nftw but are unused
*/
int f(const char* fullpath, const struct stat* temp1, int filetype, struct FTW* temp2){
	if(filetype != FTW_F){ //directories are not processed
		return 0;
	}
	int i, L = 0;
	for(i = strlen(fullpath)-1; i>=0 ; i--){
		if(fullpath[i] == '/'){
			break;
		}
		L++;
	}
	char* filename = (char*) malloc(L+1);
	for(i = 0; i<L; i++){
			filename[i] = tolower(fullpath[strlen(fullpath)-L+i]);
	}
	filename[L] = '\0';
	FILE* fp = fopen(fullpath, "r");
	while(!feof(fp)){
		char c[5000];
		if(fscanf(fp, "%s", c) != 1){
			break;
		}
		int l = strlen(c); 
		int start = 0;
		for(i = 0; i<l; i++){
			int isalpha = isalpha(c[i]);
			int isalnum = isalnum(c[i]);
			if(isalpha){
				start = 1;															//All non-alphanumeric characters are
			} else if(!isalnum || (!isalpha && !start)){ 				//replaced with a whitespace
				c[i] = 32;
				start = 0;
			}
		}
		char* p = strtok(c, " ");
		while(p != NULL){
			for(i = 0; i<strlen(p); i++){                            //takes a token and hashes it
				p[i] = tolower(p[i]);
			}
			if(!AddtoHash(p, filename)){
				AddtoList(p);
			}
			p = strtok(NULL, " ");
		}
	}
	fclose(fp);
	return 0;
}
/*Returns the alphanumeric value of a given character.
  Unlike ASCII values, numbers have a greater value than letters*/
int val(char c){
	if(c == 46){
		return 0;
	}
	if(c > 47 && c<=57){
		return 10-((int) c)%48;
	}
	if(c >=65 && c<= 90){
		return 36-((int) c) % 65;
	}
	if(c >= 97 && c<= 122){
		return 62 - ((int) c) % 97;
	} 
	return -1;
}
/*Compares the value of two char* alphanumerically.
  Returns 1 if char* B comes first, -1 if char* A comes first, and 0 if they are equal*/
int compare(const char* A, const char* B){
	int i,R,l,la,lb;
	la = strlen(A);
	lb = strlen(B);
	if(la>lb){
		l = lb;
		R = 1;
	} else if(la<lb){
		l = la;
		R = -1;
	} else{
		l = la;
		R = 0;
	}
	for(i = 0; i<l; i++){
		if(val(A[i])-val(B[i]) > 0){
			return -1;
		}
		if(val(A[i]) - val(B[i]) < 0){
			return 1;
		}
	}
	return R;
}
