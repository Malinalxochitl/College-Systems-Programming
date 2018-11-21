#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include"stringsorter.h"		 
int main(int argc, char** argv){
	if(argc != 2){printf("You need one input! Try again.\n"); return 0;}
	const char s[2] = " ";
	node* root = NULL;
	int l = strlen(argv[1]);
	int i;
	for(i = 0; i<l; i++){
		if(!isalpha(argv[1][i])){
			argv[1][i] = 32;     // Turns all non-alphebetic characters into spaces
		}
	}
	char* p = strtok(argv[1], s);        // Gets first word
	while(p != NULL){
		int l = strlen(p)+1;
		node* NewNode = createNode(l);
		strcpy(NewNode->val,p);
		root = add(root, NewNode);    // Adds words to linked list
		p = strtok(NULL, s);
	}
	print(root);     // Prints linked list
	freeLinkedList(root);
	return 0;
}

















	
