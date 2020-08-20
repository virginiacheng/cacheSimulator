#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct block{
	int v;
	unsigned long int tag;
}Block;

typedef struct set{
	unsigned long int setIndex;
	struct set* next;
	int evictPtr;
	int arrSize;
	Block blocks[];
}Set;

int checkIfInputValid(int, int, char*);
int getAssociativity(int, int, char*);
Set* createCache(int, int);
void freeCache(Set*);
unsigned long int getSetIndex(unsigned long int, int, int);
unsigned long int getTagIndex(unsigned long int, int);
Set* findSet(Set*, unsigned long int);
void shiftLRU(int, Set*);
int checkInCache(unsigned long int, Set*, char*);
void evictFifo(Set*, unsigned long int);
void populate(unsigned long int, Set*, char*);
