typedef struct Node{
	char* val;
	struct Node* next;      // To make Linked List of words
} node;
node* createNode(int length){
	struct Node* A = (struct Node*)( malloc(sizeof(struct Node)));
	A->val = (char*) (malloc(sizeof(char)*length));  // To make Nodes holding a word of length "length"
	A->next = NULL;
	return A;
}
node* add(node* root, node* newNode){
	if(root == NULL){                     //newNode is last in the list alphabetically
		return newNode;
	}
	if(strcmp(newNode->val,root->val) < 0){ //Adds newNode before the current root,
		newNode->next = root; 					 //ending the traversal of the list
		return newNode;
	}
	root->next = add(root->next, newNode);
	return root;
}
void print(node* root){
	if(root == NULL){return;}
	printf("%s\n", root->val);
	print(root->next);          // prints linked list
}
void freeLinkedList(node* root){
	if(root == NULL){return;}
	freeLinkedList(root->next);  // frees linked list
	free(root->val);
	free(root);
}		 
