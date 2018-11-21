#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MEMSIZE 5000
#define malloc( x )  mymalloc( x, __LINE__ , __FILE__)
#define free( x )  myfree( x, __LINE__, __FILE__)
#define blocks_allocated *((int*) (mem+5))
#define memory_allocated *((int*) (mem+1))
#define memptr ((int*) (mem+1))
#define blockptr ((int*) (mem+5))
#define I mem[0]
#define Int(x) *((int*) (mem+x))
#define storage 5

extern void* mymalloc(int x, int LINE, char* FILE);
extern void myfree(void* p, int LINE, char* FILE);