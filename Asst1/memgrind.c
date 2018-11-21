#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include "mymalloc.h"

//allocates and immediately frees 1 byte count times
void testA(int count) {
	char* ptr;
	int i;
	for (i = 0; i < count; i++) {
		ptr = (char*) malloc(1);
		free(ptr);	
	}
}

//allocates 1 byte count times and then frees count times
void testB(int count){
  char* ptr2[count];
  int i;
  for(i = 0; i<count; i++){
   ptr2[i] = (char*) malloc(1);
  }
//print();
  for(i = 0; i<count; i++){
    free(ptr2[i]);
  }
	//print();
}
void testC(int count) {
	int ranNum;
	int* array[count];				
	int mallCount = 0;		//These variables keep track of the number
	int freeCount = 0;		//of frees, mallocs, and total memory
	int totalMem = 5000;
	while(mallCount < count) {
		if(mallCount == freeCount) { //nothing to free, thus a malloc must occur
			ranNum = 0;
		} else if (totalMem <= 66) { //limited memory, thus a free must occur
			ranNum = 1;
		} else {
			ranNum = rand() % 2;
		}
		if (ranNum == 0) { //malloc
			ranNum = 1;
			array[mallCount] = (int*) malloc(ranNum);   // same as work load D, but without mallTrack tracking. Code reused.
			totalMem -= (ranNum + 5);
			mallCount++;
		} else { //free
			free(array[freeCount]);
			totalMem += (6);
			freeCount++; 
		}
	}
	for(;freeCount < mallCount; freeCount++) { //frees all remaining memory 
		free(array[freeCount]);
	}
}
//randomly allocates 1 to 64 bytes or frees pointers
void testD(int count) {
	int ranNum;
	int* array[count];				
	int mallCount = 0;		//These variables keep track of the number
	int freeCount = 0;		//of frees, mallocs, and total memory
	int mallTrack[count];
	int totalMem = 5000;
	while(mallCount < count) {
		if(mallCount == freeCount) { //nothing to free, thus a malloc must occur
			ranNum = 0;
		} else if (totalMem <= 66) { //limited memory, thus a free must occur
			ranNum = 1;
		} else {
			ranNum = rand() % 2;
		}
		if (ranNum == 0) { //malloc
			ranNum = (rand() % 64) + 1; //memory size is chosen
			array[mallCount] = (int*) malloc(ranNum);
			totalMem -= (ranNum + 5);
			mallTrack[mallCount] = ranNum;
			mallCount++;
		} else { //free
			free(array[freeCount]);
			totalMem += (mallTrack[freeCount] + 5);
			freeCount++; 
		}
	}
	for(;freeCount < mallCount; freeCount++) { //frees all remaining memory 
		free(array[freeCount]);
	}
}

//alternate allocating 8 and 32 bytes, free the 32 byte pointers
//and allocate 12 byte pointers
void testE(int count) {
	int i;
	int* arrayA[count];
	int* arrayB[count];
	for(i = 0; i < count; i++) {
		arrayA[i] = (int*) malloc(8);
		arrayB[i] = (int*) malloc(32);
	}
	for(i = 0; i < count; i++) {
		free(arrayB[i]);
		arrayB[i] = NULL;
	}
	for(i = 0; i < count; i++) {
		arrayB[i] = (int*) malloc(12);
	}
	for(i = 0; i < count; i++) {
		free(arrayA[i]);
		free(arrayB[i]);
	}
}
// mallocs until null pointer is returned with byte size N, frees all pointers, 
	// for 1<=N<=count
void testF(int count){
	int N = 1;  char* ptr;
	while(3000/N>1){
		 ptr = (char*) malloc(3000/N);
		
		free(ptr);
		N++;
	}
}
			
int main(int argc, char** argv) {
	struct timeval t1, t2;
	double time;
	int i;
	
	gettimeofday(&t1, NULL);			//workload A
	for (i = 0; i < 100; i++) {
		testA(150);
	}
	gettimeofday(&t2, NULL);
	time = (t2.tv_sec - t1.tv_sec) * 1000.0;
	time += (t2.tv_usec - t1.tv_usec) / 1000.0;
	printf("Workload A took on average %f miliseconds\n", time/100);
	
	gettimeofday(&t1, NULL);			//workload B
	for (i = 0; i < 100; i++) {
		testB(150);
	}
	gettimeofday(&t2, NULL);
	time = (t2.tv_sec - t1.tv_sec) * 1000.0;
	time += (t2.tv_usec - t1.tv_usec) / 1000.0;
	printf("Workload B took on average %f miliseconds\n", time/100);
	
	gettimeofday(&t1, NULL);			//workload C
	for (i = 0; i < 100; i++) {
		testC(150);
	}
	gettimeofday(&t2, NULL);
	time = (t2.tv_sec - t1.tv_sec) * 1000.0;
	time += (t2.tv_usec - t1.tv_usec) / 1000.0;
	printf("Workload C took on average %f miliseconds\n", time/100);

	gettimeofday(&t1, NULL);			//workload D
	for (i = 0; i < 100; i++) {
		testD(150);
	}
	gettimeofday(&t2, NULL);
	time = (t2.tv_sec - t1.tv_sec) * 1000.0;
	time += (t2.tv_usec - t1.tv_usec) / 1000.0;
	printf("Workload D took on average %f miliseconds\n", time/100);
	
	gettimeofday(&t1, NULL);			//workload E
	for (i = 0; i < 100; i++) {
		testE(60);				// 150 overloads memory
	}
	gettimeofday(&t2, NULL);
	time = (t2.tv_sec - t1.tv_sec) * 1000.0;
	time += (t2.tv_usec - t1.tv_usec) / 1000.0;
	printf("Workload E took on average %f miliseconds\n", time/100);
	
	gettimeofday(&t1, NULL);			//workload F
	for (i = 0; i < 100; i++) {
		testF(150);
	}
	gettimeofday(&t2, NULL);
	time = (t2.tv_sec - t1.tv_sec) * 1000.0;
	time += (t2.tv_usec - t1.tv_usec) / 1000.0;
	printf("Workload E took on average %f miliseconds\n", time/100);
	return 0;
}
