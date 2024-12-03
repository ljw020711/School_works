#include <stdio.h>
#include <stdlib.h> // malloc, realloc

#include "adt_heap.h"

/* Reestablishes heap by moving data in child up to correct location heap array
*/
static void _reheapUp( HEAP *heap, int index);


/* Reestablishes heap by moving data in root down to its correct location in the heap
*/
static void _reheapDown( HEAP *heap, int index);


HEAP *heap_Create( int capacity, int (*compare) (void *arg1, void *arg2))
{
	HEAP *pHeap = (HEAP *)malloc( sizeof(HEAP));
	
	if( pHeap)
	{
		pHeap->heapArr = (void *)malloc( capacity * sizeof(void *)); 
		pHeap->last = -1;
		pHeap->capacity = capacity;
		pHeap->compare = compare;
	}
	
	return pHeap;
}


void heap_Destroy( HEAP *heap)
{
	fprintf( stdout, "Inserting d: ");
	for( int i = 0; i < heap->last; i++)
	{
		free(heap->heapArr[i]);
	}
	
	free(heap->heapArr);
	free(heap);
}


int heap_Insert( HEAP *heap, void *dataPtr)
{
	if( (heap->last)+1 == heap->capacity)
	{
		heap->capacity = heap->capacity * 2;
		heap->heapArr = realloc( heap->heapArr, heap->capacity * sizeof( void*));
	}
	
	heap->last++;
	heap->heapArr[heap->last] = dataPtr;

	_reheapUp( heap, heap->last);
	
	return 1;
}


int heap_Delete( HEAP *heap, void **dataOutPtr)
{
	if( heap_Empty( heap))
	{
		return 0;
	}
	
	*dataOutPtr = heap->heapArr[0];
	heap->heapArr[0] = heap->heapArr[heap->last];
	heap->last--;
	
	_reheapDown( heap, 0);
	
	return 1;
}


int heap_Empty(  HEAP *heap)
{
	if( heap->last == -1)
	{
		return 1;
	}
	
	return 0;
}


void heap_Print( HEAP *heap, void (*print_func) (void *data))
{
	for( int i = 0; i <= heap->last; i++)
	{
		print_func(heap->heapArr[i]);
	}
	printf("\n");
}


static void _reheapUp( HEAP *heap, int index)
{
	int parent_index = ( index - 1 )/2;
	void *temp_swap;
	
	if( index != 0)
	{
		if( heap->compare( heap->heapArr[index], heap->heapArr[parent_index]) > 0)
		{
			temp_swap = heap->heapArr[index];
			heap->heapArr[index] = heap->heapArr[parent_index];
			heap->heapArr[parent_index] = temp_swap;
		
			_reheapUp( heap, parent_index);
		}
	}
}


static void _reheapDown( HEAP *heap, int index)
{
	int child_l = 2 * index + 1;
	int child_r = 2 * index + 2;
	int child_largest;
	void *temp_swap;
	
	
	if( index < heap->last)
	{
		if( ( (child_l <= heap->last) && heap->heapArr[child_l]))
		{
			if( (child_l <= heap->last) && (heap->heapArr[child_r]))
			{
				if( heap->compare( heap->heapArr[child_l], heap->heapArr[child_r]) > 0)
				{
					child_largest = child_l;
				}
				else
				{
					child_largest = child_r;
				}
			}
			else
			{
				child_largest = child_l;
			}
			
			if( heap->compare( heap->heapArr[index], heap->heapArr[child_largest]) < 0)
			{
				temp_swap = heap->heapArr[index];
				heap->heapArr[index] = heap->heapArr[child_largest];
				heap->heapArr[child_largest] = temp_swap;
		
				_reheapDown( heap, child_largest);
			}
		}
	}
}