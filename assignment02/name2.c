#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_YEAR_DURATION	10	// 기간

// 구조체 선언
typedef struct {
	char	name[20];		// 이름
	char	sex;			// 성별 M or F
	int		freq[MAX_YEAR_DURATION]; // 연도별 빈도
} tName;

typedef struct {
	int		len;		// 배열에 저장된 이름의 수
	int		capacity;	// 배열의 용량 (배열에 저장 가능한 이름의 수)
	tName	*data;		// 이름 배열의 포인터
} tNames;

// 함수 원형 선언

// 연도별 입력 파일을 읽어 이름 정보(연도, 이름, 성별, 빈도)를 이름 구조체에 저장
// 이미 구조체에 존재하는(저장된) 이름은 해당 연도의 빈도만 저장
// 새로 등장한 이름은 구조체에 추가
// 주의사항: 동일 이름이 남/여 각각 사용될 수 있으므로, 이름과 성별을 구별해야 함
// 주의사항: 정렬 리스트(ordered list)를 유지해야 함 (qsort 함수 사용하지 않음)
// 이미 등장한 이름인지 검사하기 위해 bsearch 함수를 사용
// 새로운 이름을 저장할 메모리 공간을 확보하기 위해 memmove 함수를 이용하여 메모리에 저장된 내용을 복사
// names->capacity는 2배씩 증가
void load_names( FILE *fp, int start_year, tNames *names);

// 구조체 배열을 화면에 출력
void print_names( tNames *names, int num_year);

// bsearch를 위한 비교 함수
int compare( const void *n1, const void *n2);

// 이진탐색 함수
// return value: key가 발견되는 경우, 배열의 인덱스
//				key가 발견되지 않는 경우, key가 삽입되어야 할 배열의 인덱스
int binary_search( const void *key, const void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));

// 함수 정의

// 이름 구조체 초기화
// len를 0으로, capacity를 1로 초기화
// return : 구조체 포인터
tNames *create_names(void)
{
	tNames *pnames = (tNames *)malloc( sizeof(tNames));
	
	pnames->len = 0;
	pnames->capacity = 1;
	pnames->data = (tName *)malloc(pnames->capacity * sizeof(tName));

	return pnames;
}

// 이름 구조체에 할당된 메모리를 해제
void destroy_names(tNames *pnames)
{
	free(pnames->data);
	pnames->len = 0;
	pnames->capacity = 0;

	free(pnames);
}
	
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	tNames *names;
	FILE *fp;
	
	if (argc != 2)
	{
		fprintf( stderr, "Usage: %s FILE\n\n", argv[0]);
		return 1;
	}

	// 이름 구조체 초기화
	names = create_names();
	
	fp = fopen( argv[1], "r");
	if (!fp)
	{
		fprintf( stderr, "cannot open file : %s\n", argv[1]);
		return 1;
	}

	fprintf( stderr, "Processing [%s]..\n", argv[1]);
		
	// 연도별 입력 파일(이름 정보)을 구조체에 저장
	load_names( fp, 2009, names);
	
	fclose( fp);
	
	// 이름 구조체를 화면에 출력
	print_names( names, MAX_YEAR_DURATION);

	// 이름 구조체 해제
	destroy_names( names);
	
	return 0;
}


//배열 입력
void load_names( FILE *fp, int start_year, tNames *names)
{
	int y;
	char n[20];
	char s;
	int f;
	
	int index = 0;
	char buffer[100];
	
	int value_exist = 0;
	
	while( fgets(buffer, 100, fp) != NULL)
	{	
		sscanf(buffer, "%d\t%s\t%c\t%d", &y, n, &s, &f);
		tName *tmp = (tName*)malloc( sizeof(tName));
		strcpy( tmp->name, n);
		tmp->sex = s;
		tmp->freq[0] = f;
		
		
		index = binary_search( tmp, names->data, names->len, sizeof(tName), compare);
		
		if( index >= names->len)
		{
			strcpy( names->data[index].name, "");
			names->data[index].sex = '\0';
		}
		
		if((strcmp(names->data[index].name, tmp->name) == 0) && (names->data[index].sex - tmp->sex == 0))
		{
			names->data[index].freq[y-start_year] = f;
		}
		
		else
		{
			if( names->len >= names->capacity || index+1 >= names->capacity)
			{
				names->capacity *= 2;
				names->data = (tName*)realloc(names->data, names->capacity * sizeof(tName));
			}	
			memmove(names->data+index+1, names->data+index, (names->len-index)*sizeof(tName));
			memset(names->data[index].freq, 0, sizeof(int) * MAX_YEAR_DURATION);
			
			strcpy(names->data[index].name, n);
			names->data[index].sex = s;
			names->data[index].freq[y-start_year] = f;
			
			names->len++;

		}

		free( tmp);
		
	}
}


//구조체 출력
void print_names( tNames *names, int num_year)
{
	for( int i = 0; i < names->len; i++)
	{
		printf("%s\t%c", names->data[i].name, names->data[i].sex);
		for( int j = 0; j < num_year; j++)
		{
			printf("\t%d", names->data[i].freq[j]);
		}
		printf("\n");
	}
}


//Compare 함수
int compare( const void *n1, const void *n2)
{
	tName *p1 = (tName*) n1;
	tName *p2 = (tName*) n2;
	
	
	if(strcmp(p1->name, p2->name) == 0)
	{
		return (int)(p1->sex - p2->sex);
	}
	return strcmp(p1->name, p2->name);
}


//이진 탐색
int binary_search( const void *key, const void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *))
{
	tName *k = (tName*) key;
	tName *b = (tName*) base;

	tName* addr = bsearch( k, b, nmemb, size, compare);

	
	int address;	
	int l, r, m;
	
	if( addr == NULL)
	{
		l = 0;
		r = nmemb - 1;
		m = (l + r) / 2;
		
		while( l <= r)
		{
			if( strcmp(b[m].name, k->name) != 0)
			{
				if( strcmp(b[m].name, k->name) > 0)
				{
					r = m - 1;
					m = (l + r) / 2; 
				}
				else
				{
					l = m + 1;
					m = (l + r) / 2;
				}
			}
			else
			{
				if( b[m].sex > k->sex)
				{
					r = m - 1;
					m = (l + r) / 2; 
				}
				else
				{
					l = m + 1;
					m = (l + r) / 2;
				}
			}
		}
		return l;
	}	
	
	address = addr - b;
	
	return address;
}