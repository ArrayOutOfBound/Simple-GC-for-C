
#include "SimpleGC.h"

#define HASH_SIZE 10007 //Prime Number is better

#define PtrHash(key) ( ( key >> 4 ) % HASH_SIZE ) //Hash Function


blockHead *PtrHashTable[HASH_SIZE]; //Records all pointers allocated 

blockHead *BlockList = NULL; //List of all blocks allocated


//Record a pointer to HashTable
static void Record( blockHead *head )
{
	int index = PtrHash((size_t)(head+1));
	
	blockHead *p = PtrHashTable[index];
	
	PtrHashTable[index] = head; 
	
	if( p ) p->next = head;
	
	head->prior = p;
	head->next = NULL;
	head->link = BlockList;
	
	BlockList = head;
}


//Determine whether a pointer has been allocated
static int ValidPtr( blockHead *ptr )
{
	size_t val = (size_t) ptr;
	
	if( val & 7 || !val ) return 0;
	
	int index = PtrHash( val );
	
	blockHead *p = PtrHashTable[index];
	
	while( p )
	{
		if( p + 1 == ptr ) return 1;
		
		p = p->prior;
	}
	return 0;
} 


//Will not return until succeed
static void *Alloc( size_t size )
{
	void *p;
	
	while( !( p = malloc( size ) ) ) GC();
	
	return p;
}


static int GcThreshold = 10000000;


//Use this function to replace malloc()
void *GcAlloc( size_t size )
{
	static int GcTrigger = 0;
	
	GcTrigger += size;
	
	if( GcTrigger >= GcThreshold ) { GcTrigger = 0; GC(); }
	
	blockHead *head = Alloc( sizeof(blockHead) + size );
	
	Record( head );
	
	head->mark = 0;
	head->len = size / sizeof(size_t);
	
	return head + 1;
}


static void Free( blockHead *head )
{
	if( head->prior ) head->prior->next = head->next;
	if( head->next ) head->next->prior = head->prior;
	else
	{
		int index = PtrHash((size_t)(head+1));
		PtrHashTable[index] = head->prior;
	}
	free( head );
}


static int SafeNumber = 1;

static void MarkBlock( blockHead *head )
{
	if( head->mark != SafeNumber )
	{
		head->mark = SafeNumber;
		
		blockHead **p = (blockHead**)( head + 1 );
		
		for( int i=0; i<head->len; i++ )
		{
			if( ValidPtr( p[i] ) ) MarkBlock( p[i]-1 );
		}
	}
}

static void MarkSweep( size_t *start, size_t *end )
{
	SafeNumber = 3 - SafeNumber;
	
	int n = ( (size_t)start - (size_t)end ) / sizeof(size_t);
	
	blockHead **p = (blockHead**)end;
	
	for( int i=0; i<n; i++ )
	{
		if( ValidPtr( p[i] ) ) MarkBlock( p[i]-1 );
	}
	blockHead *iter = BlockList, *next, *temp = NULL;
	
	while( iter )
	{
		next = iter->link;
		
		if( iter->mark == SafeNumber )
		{
			iter->link = temp;
			temp = iter;
		}
		else Free( iter ); 
		
		iter = next;
	}
	BlockList = temp;
}


size_t *StackStart; //The starting point of stack scanning

void GC()
{
	volatile size_t *StackEnd = (size_t*)(&StackEnd); //The terminal point of stack scanning
	
	MarkSweep( StackStart, (size_t*)StackEnd );
}
