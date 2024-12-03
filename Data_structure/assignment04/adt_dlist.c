#include <stdlib.h> // malloc

#include "adt_dlist.h"

/* internal insert function
	inserts data into a new node
	return	1 if successful
			0 if memory overflow
*/
static int _insert( LIST *pList, NODE *pPre, void *dataInPtr);

/* internal delete function
	deletes data from a list and saves the (deleted) data to dataOut
*/
static void _delete( LIST *pList, NODE *pPre, NODE *pLoc, void **dataOutPtr);

/* internal search function
	searches list and passes back address of node
	containing target and its logical predecessor
	return	1 found
			0 not found
*/
static int _search( LIST *pList, NODE **pPre, NODE **pLoc, void *pArgu);


LIST *createList( int (*compare)(const void *, const void *))
{
	LIST *plist = (LIST *)malloc( sizeof(LIST));
	
	if( plist)
	{
		plist->count = 0;
		plist->head = NULL;
		plist->rear = NULL;
		plist->compare = compare;
	}
	
	return plist;
}

void destroyList( LIST *pList, void (*callback)(void *))
{
	NODE* destroy_node;
	
	while( pList->count > 0)
	{
		destroy_node = pList->head;
		pList->head = destroy_node->rlink;
		pList->count--;
		
		(*callback)( destroy_node->dataPtr);
		free(destroy_node);
	}
	free(pList);
}

int addNode( LIST *pList, void *dataInPtr, void (*callback)(const void *, const void *))
{
	NODE* pPre = NULL;
	NODE* pLoc = pList->head;
	
	int result = _search( pList, &pPre, &pLoc, dataInPtr);
	
	if( result == 0)
	{
		pList->count++;
		return _insert( pList, pPre, dataInPtr);
	}
	else
	{
		(*callback)( pLoc->dataPtr, dataInPtr);
	}
	return 2;
}

int removeNode( LIST *pList, void *keyPtr, void **dataOutPtr)
{
	NODE* pPre = NULL;
	NODE* pLoc = pList->head;
	
	int result = _search( pList, &pPre, &pLoc, keyPtr);
	
	if( result)
	{
		pList->count--;
		_delete( pList, pPre, pLoc, dataOutPtr);
	}
	
	return result;
}

int searchList( LIST *pList, void *pArgu, void **dataOutPtr)
{
	NODE* pPre = NULL;
	NODE* pLoc = pList->head;
	
	if( _search( pList, &pPre, &pLoc, pArgu))
	{
		*dataOutPtr = pLoc->dataPtr;
		return 1;
	}
	
	return 0;
}

int countList( LIST *pList)
{
	return pList->count;
}

int emptyList( LIST *pList)
{
	if( pList->count == 0)
	{
		return 1;
	}
	return 0;
}

void traverseList( LIST *pList, void (*callback)(const void *))
{
	NODE* pLoc = pList->head;
	
	while( pLoc != NULL)
	{
		(*callback)(pLoc->dataPtr);
		pLoc = pLoc->rlink;
	}
}

void traverseListR( LIST *pList, void (*callback)(const void *))
{
	NODE* pLoc = pList->rear;
	
	while( pLoc != NULL)
	{
		(*callback)(pLoc->dataPtr);
		pLoc = pLoc->llink;
	}
}


static int _insert( LIST *pList, NODE *pPre, void *dataInPtr)
{
	NODE *pNew = (NODE *)malloc( sizeof(NODE));
	int return_val = 0;
	
	if( pNew)
	{
		pNew->dataPtr = dataInPtr;
		pNew->llink = NULL;
		pNew->rlink = NULL;
		
		return_val = 1;
		
		if( !pPre)
		{
			if( pList->head)
			{
				pNew->rlink = pList->head;
				pList->head->llink = pNew;
				pList->head = pNew;
			}
			else
			{
				pList->head = pNew;
				pList->rear = pNew;
			}
		}
		else
		{
			if( pPre->rlink)
			{
				pNew->rlink = pPre->rlink;
				pNew->llink = pPre;
				pPre->rlink->llink = pNew;
				pPre->rlink = pNew;
			}
			else
			{
				pPre->rlink = pNew;
				pNew->llink = pPre;
				pList->rear = pNew;
			}
		}
	}
	
	return return_val;
}

static void _delete( LIST *pList, NODE *pPre, NODE *pLoc, void **dataOutPtr)
{
	*dataOutPtr = pLoc->dataPtr;
	
	if( !pPre)
	{
		pList->head = pLoc->rlink;
	}
	else
	{
		pPre->rlink = pLoc->rlink;
	}
	if( pLoc->rlink != NULL)
	{
		pLoc->rlink->llink = pPre;
	}
	else
	{
		pList->rear = pPre;
	}
	
	free(pLoc);
}

static int _search( LIST *pList, NODE **pPre, NODE **pLoc, void *pArgu)
{
	while( ( (*pLoc) != NULL)&&(pList->compare((*pLoc)->dataPtr, pArgu) < 0))
	{
		*pPre = *pLoc;
		*pLoc = (*pPre)->rlink;
	}
	if( *pLoc == NULL)
	{
		return 0;
	}
	if( pList->compare((*pLoc)->dataPtr, pArgu) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}