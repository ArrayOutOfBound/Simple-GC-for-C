
#ifndef SIMPLE_GC_H
#define SIMPLE_GC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <math.h>
#include <time.h>

typedef struct BlockHead
{
	struct BlockHead *prior, *next, *link;
	
	size_t mark, len, nothing;
}
blockHead; //sizeof(blockHead) must be multiple of 8

extern size_t *StackStart;

#define GcInit(...) StackStart = (size_t*)(&argc)

void *GcAlloc( size_t size );

void GC( void );
 
#endif
