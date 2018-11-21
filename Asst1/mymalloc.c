#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"
#include <time.h>
static char mem[MEMSIZE]; 
void print(){
	printf("I: %d\t MEM: %d\t BLOCKS: %d\t\n", I,memory_allocated, blocks_allocated);
	int counter = 0;
	int i;
	for(i = 9; counter < blocks_allocated; i = i+Int(i+1)+5){
		if(mem[i] == 0){
			printf("BLOCK: %d\t FULL\t size: %d\n", counter+1, Int(i+1));
		}
		if( mem[i] == 1){
			printf("BLOCK: %d\t FREE\t size: %d\n", counter+1, Int(i+1));
		}
		counter++;
	}
}
void merge(){ // merges freed blocks
	int i, j;  // i is leading block, j is lagging
	j = 9;
	int counter = 0;

	for(i = 9; counter<blocks_allocated; i+=Int(i+1)+5){
		if(mem[j] &&  mem[i] && (i!=j)){
			Int(j+1) = Int(i+1)+Int(j+1)+5;      // extra 5 bytes is from elemination of overhead
			blocks_allocated = blocks_allocated-1; // merges the blocks
			memory_allocated = memory_allocated-5; // gets rid of allocated 5 bytes from overhead
			
		}
		else{ j = i; }
		counter++;
	}
	if(blocks_allocated == 1 && mem[9] == 1){
		I = 0; 
		memory_allocated = 14; 
		blocks_allocated = 0;
	}
}
void* newblock(int x){
	
	int count = 0; int i;
	for(i = 9; count<blocks_allocated; i += Int(i+1)+5){
		count++;
	}
	++blocks_allocated;
	mem[i] = 0;
	Int(i+1) = x;
	//int store = i; 				// makes new block at end of list
	memory_allocated = memory_allocated + x+storage;        // returns pointer to memory of new block
	return (void*) (mem+i+storage); 
}

void* mymalloc(int size, int LINE, char* FILE){
	if(size+5 >= MEMSIZE-memory_allocated){
		printf("over saturated memory on line %d, file %s\n", LINE, FILE); return (void*) 0; // not enough room!
	}
	if(!I){
		I = 1;
		blocks_allocated = 1;
		memory_allocated = 14+size;		// intitialized first block
		mem[9] = 0;
		Int(10) = size;
		return (void*) (mem+14);
	}
	int i; int counter = 0;
	for(i = 9; counter<blocks_allocated; i += Int(i+1)+5 ){
		if((Int(i+1) == size || (Int(i+1)<size && size<Int(i+1)+10)) && mem[i]){
			memory_allocated += Int(i+1);
			mem[i] = 0; return (void*) (mem+i+5);                           // If block is close enough don't bother splitting it.
		}
		if(Int(i+1) > size+10 && mem[i]){
			mem[i+size+5] = 1;
			Int(i+size+6) = Int(i+1)-size-5;		// mallocs memory to freed block, splits it.
			Int(i+1) = size;
			mem[i]= 0;
			++blocks_allocated;
			memory_allocated = memory_allocated+size+5;
			return (void*) (mem+i+5);
		}
		counter++;
		
	}
	return  newblock(size); 
}
void myfree(void* p, int LINE, char* FILE ){
	int ptr; int counter = 0; int good = 0;
	for(ptr = 9; counter< blocks_allocated; ptr += Int(ptr+1)+5 ){
		if((void*) (mem+ptr+5) == p){
			if(mem[ptr]){
				printf("error double free on line %d file %s\n ", LINE, FILE); //pointer is already free
				return;
			}
			if(blocks_allocated == 1 && !mem[ptr]){ 
				I = 0; 
				memory_allocated = 14; 
				blocks_allocated = 0;
			} else {
				mem[ptr] = 1; 
				memory_allocated = memory_allocated - (Int(ptr+1));
			} // freeing pointer frees up block space.
			good = 1;							  // overhead is still there
		}
		counter++; // moves forward in blocks.
	}										
	if(!good){
		printf("Pointer not allocated on line %d, file %s\n", LINE, FILE); //we never found the pointer!
		return;
	}
	merge();
	merge();  // merges freed blocks, to free up more space of contigious memory
	return ;
}
