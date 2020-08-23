#include "first.h"

//if inputs are valid, return 1 otherwise return 0
int checkIfInputValid(int cacheSize, int blockSize, char* policy){
	//check if cacheSize and blockSize are powers of 2
	if(cacheSize <= 0 || blockSize <= 0){
		//printf("negative\n");
		return 0;
	}
	
	if(ceil(log2(cacheSize)) != floor(log2(cacheSize))){
		printf("Invalid cache size input\n");
		return 0;
	}

	if(ceil(log2(blockSize)) != floor(log2(blockSize))){
		printf("Invalid block size input\n");
		return 0;
	}

	//check if policy input is valid: fifo or lru
	if(strcmp(policy, "fifo") == 0){
		return 1;
	}
	else if(strcmp(policy, "lru") == 0){
		return 1;
	}
	else{
		printf("Invalid cache policy input\n");
		return 0;
	}
}

//returns n in n-way cache
//return -1 if input format is incorrect
int getAssociativity(int cacheSize, int blockSize, char* assoc){
	//direct mapped cache
	if(strcmp("direct", assoc) == 0){
		return 1;
	}
	
	//fully associative cache
	if(strcmp("assoc", assoc) == 0){
		return cacheSize/blockSize;
	}

	//n-way associative cache
	char* token = strtok(assoc, ":");
	
	//check if input is correct
	if(strcmp(token, "assoc") != 0){
		return -1;
	}
	
	token = strtok(NULL, ":");
	int assocN = atoi(token);

	//check if assocN is a power of 2
	if(ceil(log2(assocN)) != floor(log2(assocN))){
		return -1;
	}

	return assocN;

}

//returns pointer to the head of the newly created cache
Set* createCache(int n, int setNum){
	int i;
	int j;
	Set* head;
	Set* temp;
	Set* ptr = NULL;
	//printf("Cache Structure\n");
	for(i = 0; i < setNum; i++){
		//allocates Set and the array of blocks within set
		temp = (Set*)malloc(sizeof(Set) + sizeof(Block) * n);
		temp->setIndex = (unsigned long int)i;
		temp->next = NULL;
		temp->arrSize = n;
		if(ptr == NULL){
			head = temp;
		}
		else{
			ptr->next = temp;
		}
		//printf("Set %d: ", i);
		
		for(j = 0; j < n; j++){	
			Block newBlock;
			newBlock.v = 0;
			temp->blocks[j] = newBlock;
			//printf("Block %d: %d | ", j, v);

		}

		//printf("\n");
		ptr = temp;
	}

	return head;
}

//free the allocated memory
void freeCache(Set* cache){
	Set* ptr = cache;
	Set* ptrNext;
	while(ptr != NULL){
		ptrNext = ptr->next;
		free(ptr);
		ptr = ptrNext;	
	}
}

//returns set value within the address
unsigned long int getSetIndex(unsigned long int address, int offset, int setBits){
	return (address>>offset)&((1<<setBits)-1);
}

//returns tag value within the address
unsigned long int getTagIndex(unsigned long int address, int sum){
	return address>>sum;
}

//returns ptr to set address should be in
Set* findSet(Set* cache, unsigned long int setIndex){
	Set* ptr;
	ptr = cache;
	while(ptr->setIndex != setIndex){
		ptr = ptr->next;
	}
	return ptr;
}

//brings the data block to the front and shift everything else to the right
void shiftLRU(int index, Set* ptr){
	int i;
	unsigned long int temp;
	temp = ptr->blocks[index].tag;
	for(i = index; i > 0; i--){
		ptr->blocks[i].tag = ptr->blocks[i-1].tag;
	}
	ptr->blocks[0].tag = temp;

}

//checks if the address is already in the cache
//return 1 if hit, 0 if miss 
int checkInCache(unsigned long int tagIndex, Set* ptr, char* policy){
	int i;
	int size;
	size = ptr->arrSize;
	for(i = 0; i < size; i++){
		if(ptr->blocks[i].v == 1){
			if(ptr->blocks[i].tag == tagIndex){
				if(strcmp(policy, "lru") == 0){
					shiftLRU(i, ptr);
				}
				return 1;
			}
		}
	}

	return 0;
}	

//evict based on fifo and populate the evicted row with new data
void evictFifo(Set* ptr, unsigned long int newTag){
	int size;
	size = ptr->arrSize;
	int index;
	index = ptr->evictPtr;
	
	ptr->blocks[index].tag = newTag;
	index++;
	if(index == size){
		ptr->evictPtr = 0;
	}
	else{
		ptr->evictPtr = index;
	}
}

//evict based on lru and populate the first index of block array with new data
void evictLru(Set* ptr, unsigned long int newTag){
	int size;
	size = ptr->arrSize;
	//shifts all values over so that the block to evict is the first block
	shiftLRU(size-1, ptr);
	//replace the first block with new tag
	ptr->blocks[0].tag = newTag;
}

//add the new address into the cache
void populate(unsigned long int tag, Set* ptr, char* policy){
	//first check if any v is 0
	//if yes, then populate
	//if not, call evict
	int i;
	int size;
	size = ptr->arrSize;
	for(i = 0; i < size; i++){
		//populate in empty row
		if(ptr->blocks[i].v == 0){
			ptr->blocks[i].tag = tag;
			ptr->blocks[i].v = 1;
			
			//update values based on cache policy
			if(i == 0 && strcmp(policy, "fifo") == 0){
				ptr->evictPtr = 0;
			}
			return;
		}
	}

	//cache is full so needs to evict and replace
	if(strcmp(policy, "fifo") == 0){
		evictFifo(ptr, tag);
	}
	else if(strcmp(policy, "lru") == 0){
		evictLru(ptr, tag);
	}
}	



int main(int argc, char** argv){
	int cacheSize;
	int blockSize;
	char* policy;
	char* assoc;

	cacheSize = atoi(argv[1]);
	blockSize = atoi(argv[2]);
	policy = argv[3];
	assoc = argv[4];

	//first check if input is in correct format
	int isValid;
	isValid = checkIfInputValid(cacheSize, blockSize, policy);
	if(isValid == 0){
		return 1;
	}
	
	//assoc will be checked in a different method
	int n; //number of lines per set
	n = getAssociativity(cacheSize, blockSize, assoc);
	if(n == -1){
		printf("Invalid associativity input\n");
		return 1;
	}
	
	//create empty cache
	int blockNum; //TOTAL NUMBER OF BLOCKS/LINES
	int setNum, offset, setBits, tagBits, sum;
	blockNum = cacheSize/blockSize;
	//printf("Total Num of Blocks: %d\n", blockNum);
	setNum = blockNum/n;
	//printf("Total Num of Sets: %d\n", setNum);
	offset = log2(blockSize);
	//printf("Offset Bits: %d\n", offset);
	setBits = log2(setNum);
	//printf("Set bits: %d\n", setBits);
	tagBits = 48 - offset - setBits;
	//printf("Tag bits: %d\n", tagBits);
	sum = setBits + offset;
	Set* cache = createCache(n, setNum);
	
	int hit = 0;
	int miss = 0;
	int memRead = 0;
	int memWrite = 0;
	
	//access file
	FILE *infile = fopen(argv[5], "r");
	char command;
	unsigned long int address;
	unsigned long int setIndex;
	unsigned long int tagIndex;
	int isInCache;
	Set* setPtr;
	
	while(fscanf(infile, "%c %lx\n", &command, &address) == 2){
		setIndex = getSetIndex(address, offset, setBits);
		//printf("Set Index: %lx\n", setIndex);
		tagIndex = getTagIndex(address, sum);
		//printf("Tag Index: %lx\n", tagIndex);
		setPtr = findSet(cache, setIndex);
		isInCache = checkInCache(tagIndex, setPtr, policy);
		
		//if read command
		if(command == 'R'){
			//HIT
			if(isInCache == 1){
				hit++;
			}

			//MISS
			else{
				miss++;
				memRead++;
				populate(tagIndex, setPtr, policy);
			}
		}
		
		//if write command
		else if(command == 'W'){
			//HIT
			if(isInCache == 1){
				hit++;
				memWrite++;
			}

			//MISS
			else{
				miss++;
				memRead++;
				memWrite++;
				populate(tagIndex, setPtr, policy);
			}

		}

	}
	fclose(infile);
	
	printf("Memory reads: %d\n", memRead);
	printf("Memory writes: %d\n", memWrite);
	printf("Cache hits: %d\n", hit);
	printf("Cache misses: %d\n", miss);
	
	freeCache(cache);
	return 0;
}
