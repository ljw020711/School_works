#include <stdio.h>
#include <stdlib.h>	// malloc
#include <string.h>	// strdup
#include <ctype.h>	// isupper, tolower

#define MAX_DEGREE	27 // 'a' ~ 'z' and EOW
#define EOW			'$'	// end of word

// used in the following functions: trieInsert, trieSearch, triePrefixList
#define getIndex(x)		(((x) == EOW) ? MAX_DEGREE-1 : ((x) - 'a'))

// TRIE type definition
typedef struct trieNode {
	int 			index; // -1 (non-word), 0, 1, 2, ...
	struct trieNode	*subtrees[MAX_DEGREE];
} TRIE;

////////////////////////////////////////////////////////////////////////////////
// Prototype declarations

/* Allocates dynamic memory for a trie node and returns its address to caller
	return	node pointer
			NULL if overflow
*/
TRIE *trieCreateNode(void);

/* Deletes all data in trie and recycles memory
*/
void trieDestroy( TRIE *root);

/* Inserts new entry into the trie
	return	1 success
			0 failure
*/
// 주의! 엔트리를 중복 삽입하지 않도록 체크해야 함
// 대소문자를 소문자로 통일하여 삽입
// 영문자와 EOW 외 문자를 포함하는 문자열은 삽입하지 않음
int trieInsert( TRIE *root, char *str, int dic_index);

/* Retrieve trie for the requested key
	return	index in dictionary (trie) if key found
			-1 key not found
*/
int trieSearch( TRIE *root, char *str);

/* prints all entries in trie using preorder traversal
*/
void trieList( TRIE *root, char *dic[]);

/* prints all entries starting with str (as prefix) in trie
	ex) "abb" -> "abbas", "abbasid", "abbess", ...
	this function uses trieList function
*/
void triePrefixList( TRIE *root, char *str, char *dic[]);

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	TRIE *trie;
	char *dic[100000];

	int ret;
	char str[100];
	FILE *fp;
	int index = 0;
	
	if (argc != 2)
	{
		fprintf( stderr, "Usage: %s FILE\n", argv[0]);
		return 1;
	}
	
	fp = fopen( argv[1], "rt");
	if (fp == NULL)
	{
		fprintf( stderr, "File open error: %s\n", argv[1]);
		return 1;
	}
	
	trie = trieCreateNode();
	
	while (fscanf( fp, "%s", str) != EOF)
	{
		ret = trieInsert( trie, str, index);

		if (ret) 
			dic[index++] = strdup( str);
	}
	
	fclose( fp);
	
	printf( "\nQuery: ");
	while (fscanf( stdin, "%s", str) != EOF)
	{
		// wildcard search
		if (str[strlen(str)-1] == '*')
		{
			str[strlen(str)-1] = 0;
			triePrefixList( trie, str, dic);
		}
		// keyword search
		else
		{
			ret = trieSearch( trie, str);
			if (ret == -1) printf( "[%s] not found!\n", str);
			else printf( "[%s] found!\n", dic[ret]);
		}
		
		printf( "\nQuery: ");
	}
	
	for (int i = 0; i < index; i++)
		free( dic[i]);
	
	trieDestroy( trie);
	
	return 0;
}


TRIE *trieCreateNode(void)
{
	TRIE *pTrie = (TRIE *)malloc( sizeof(TRIE));
	
	if( pTrie)
	{
		pTrie->index = -1;
		
		for( int i = 0; i < MAX_DEGREE; i++)
		{
			pTrie->subtrees[i] = NULL;
		}
	}
	
	return pTrie; 
}


void trieDestroy( TRIE *root)
{
	if( root)
	{
		for( int i = 0; i < MAX_DEGREE; i++)
		{
			trieDestroy( root->subtrees[i]);
		}
		
		free(root);
	}
}


int trieInsert( TRIE *root, char *str, int dic_index)
{ 	
	int len = strlen( str);
	int letter_index = 0;
	
	for( int i = 0; i < len; i++)
	{
		letter_index = getIndex(str[i]);
		
		if( letter_index >= MAX_DEGREE && letter_index < 0)
		{
			return 0;
		}
		
		if( isupper( str[i]))
		{
			str[i] = tolower( str[i]);
		}
		
		
		if( !(root->subtrees[letter_index]))
		{
			TRIE *pNew = (TRIE *)malloc( sizeof(TRIE));
			pNew->index = -1;
		
			for( int i = 0; i < MAX_DEGREE; i++)
			{
				pNew->subtrees[i] = NULL;
			}
			
			root->subtrees[letter_index] = pNew;
		}
		
		root = root->subtrees[letter_index];
	}
	
	if( root->index != -1)
	{
		return 0;
	}
	else
	{
		root->index = dic_index;
	}
	
	return 1;
}


int trieSearch( TRIE *root, char *str)
{	
	int len = strlen( str);
	int letter_index = 0;
	
	for( int i = 0; i < len; i++)
	{	
		letter_index = getIndex(str[i]);
		root = root->subtrees[letter_index];
		
		if( !root)
		{
			return -1;
		}
	}
	
	return root->index;
}


void trieList( TRIE *root, char *dic[])
{
	if( root)
	{
		if(root->index != -1)
		{
			printf("%s\n", dic[root->index]);
		}
		
		for( int i = 0; i < MAX_DEGREE; i++)
		{
			trieList( root->subtrees[i], dic);
		}
	}
}


void triePrefixList( TRIE *root, char *str, char *dic[])
{
	int len =  strlen( str);
	int letter_index = 0;
	
	for( int i = 0; i < len; i++)
	{	
		letter_index = getIndex(str[i]);
		root = root->subtrees[letter_index];
		
		if( !root)
		{
			break;
		}
	}
	
	trieList( root, dic);
}