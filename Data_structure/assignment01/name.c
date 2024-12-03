#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_YEAR_DURATION	10	// 기간
#define LINEAR_SEARCH 0
#define BINARY_SEARCH 1

// 구조체 선언
typedef struct {
	char	name[20];		// 이름
	char	sex;			// 성별 'M' or 'F'
	int		freq[MAX_YEAR_DURATION]; // 연도별 빈도
} tName;

typedef struct {
	int		len;		// 배열에 저장된 이름의 수
	int		capacity;	// 배열의 용량 (배열에 저장 가능한 이름의 수)
	tName	*data;		// 이름 배열의 포인터
} tNames;

////////////////////////////////////////////////////////////////////////////////
// 함수 원형 선언(declaration)

// 연도별 입력 파일을 읽어 이름 정보(이름, 성별, 빈도)를 이름 구조체에 저장
// 이미 구조체에 존재하는(저장된) 이름은 해당 연도의 빈도만 저장
// 새로 등장한 이름은 구조체에 추가
// 주의사항: 동일 이름이 남/여 각각 사용될 수 있으므로, 이름과 성별을 구별해야 함
// names->capacity는 2배씩 증가
// 선형탐색(linear search) 버전
void load_names_lsearch( FILE *fp, int year_index, tNames *names);

// 이진탐색(binary search) 버전 (bsearch 함수 이용)
void load_names_bsearch( FILE *fp, int year_index, tNames *names);

// 구조체 배열을 화면에 출력
void print_names( tNames *names, int num_year);

// qsort, bsearch를 위한 비교 함수
// 정렬 기준 : 이름(1순위), 성별(2순위)
int compare( const void *n1, const void *n2);

////////////////////////////////////////////////////////////////////////////////
// 함수 정의 (definition)

// 이름 구조체를 초기화
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
	int option;
	
	FILE *fp;
	int num_year = 0;
	
	if (argc <= 2)
	{
		fprintf( stderr, "Usage: %s option FILE...\n\n", argv[0]);
		fprintf( stderr, "option\n\t-l\n\t\twith linear search\n\t-b\n\t\twith binary search\n");
		return 1;
	}
	
	if (strcmp( argv[1], "-l") == 0) option = LINEAR_SEARCH;
	else if (strcmp( argv[1], "-b") == 0) option = BINARY_SEARCH;
	else {
		fprintf( stderr, "unknown option : %s\n", argv[1]);
		return 1;
	}
	
	// 이름 구조체 초기화
	names = create_names();

	// 첫 연도 알아내기 "yob2009.txt" -> 2009
	int start_year = atoi( &argv[2][strlen(argv[2])-8]);
	
	for (int i = 2; i < argc; i++)
	{
		num_year++;
		fp = fopen( argv[i], "r");
		if( !fp) {
			fprintf( stderr, "cannot open file : %s\n", argv[i]);
			return 1;
		}

		int year = atoi( &argv[i][strlen(argv[i])-8]); // ex) "yob2009.txt" -> 2009
		
		fprintf( stderr, "Processing [%s]..\n", argv[i]);
		
		if (option == LINEAR_SEARCH)
		{
			// 연도별 입력 파일(이름 정보)을 구조체에 저장
			// 선형탐색 모드
			load_names_lsearch( fp, year-start_year, names);
		
		}
		else // (option == BINARY_SEARCH)
		{
			// 이진탐색 모드
			load_names_bsearch( fp, year-start_year, names);
			
			// 정렬 (이름순 (이름이 같은 경우 성별순))
			qsort( names->data, names->len, sizeof(tName), compare);
		}
		fclose( fp);

	}
	
	// 정렬 (이름순 (이름이 같은 경우 성별순))
	qsort( names->data, names->len, sizeof(tName), compare);
		
	// 이름 구조체를 화면에 출력
	print_names( names, num_year);

	// 이름 구조체 해제
	destroy_names( names);
	
	return 0;
}






//선형탐색
void load_names_lsearch( FILE *fp, int year_index, tNames *names)
{
	char buffer[100];
	int index = 0;
	
	char n[20];
	char s;
	int f;
	
	if( year_index == 0)
	{
		while( fgets(buffer, 100, fp) != NULL)
		{
			char* commaPtr = strstr(buffer, ",");
			strncpy(commaPtr, "\t", 1);
			commaPtr = strstr(buffer, ",");
			strncpy(commaPtr, "\t", 1);
			
			if( names->len >= names->capacity)
			{
				names->capacity *= 2;
				names->data = (tName*)realloc(names->data, names->capacity * sizeof(tName));
			}
			memset(names->data[index].freq, 0, sizeof(int) * MAX_YEAR_DURATION);
			sscanf(buffer, "%s\t%c\t%d", names->data[index].name, &names->data[index].sex, &names->data[index].freq[year_index]);
			index++;
			names->len++;
		}
	}
	
	else
	{
		while( fgets(buffer, 100, fp) != NULL)
		{
			int pass = 0;
			
			char* commaPtr = strstr(buffer, ",");
			strncpy(commaPtr, "\t", 1);
			commaPtr = strstr(buffer, ",");
			strncpy(commaPtr, "\t", 1);
			
			sscanf(buffer, "%s\t%c\t%d", n, &s, &f);
			
			
			for( int i = 0; i < names->len; i++)
			{
				if( (strcmp(n, names->data[i].name) == 0) && (s == names->data[i].sex))
				{
					sscanf(buffer, "%s\t%c\t%d", n, &s, &names->data[i].freq[year_index]);
					pass = 1;
				}
			}
			
			if(pass == 0)
			{
				if( names->len >= names->capacity)
				{
					names->capacity *= 2;
					names->data = (tName*)realloc(names->data, names->capacity * sizeof(tName));
				}
				int l = names->len;
				memset(names->data[l].freq, 0, sizeof(int) * MAX_YEAR_DURATION);
				sscanf(buffer, "%s\t%c\t%d", names->data[l].name, &names->data[l].sex, &names->data[l].freq[year_index]);
				index++;
				names->len++;
			}
		}
	}
}


//이진탐색
void load_names_bsearch( FILE *fp, int year_index, tNames *names)
{
	char buffer[100];
	int index = 0;
	
	char n[20];
	char s;
	int f;
	int tmplen = 0;
	tName *tempAdr;
	
	if( year_index == 0)
	{
		while( fgets(buffer, 100, fp) != NULL)
		{
			char* commaPtr = strstr(buffer, ",");
			strncpy(commaPtr, "\t", 1);
			commaPtr = strstr(buffer, ",");
			strncpy(commaPtr, "\t", 1);
			
			if( names->len >= names->capacity)
			{
				names->capacity *= 2;
				names->data = (tName*)realloc(names->data, names->capacity * sizeof(tName));
			}
			
			memset(names->data[index].freq, 0, sizeof(int) * MAX_YEAR_DURATION);
			sscanf(buffer, "%s\t%c\t%d", names->data[index].name, &names->data[index].sex, &names->data[index].freq[year_index]);
			
			index++;
			names->len++;
		}
	}
	else
	{
		while( fgets(buffer, 100, fp) != NULL)
		{
			
			char* commaPtr = strstr(buffer, ",");
			strncpy(commaPtr, "\t", 1);
			commaPtr = strstr(buffer, ",");
			strncpy(commaPtr, "\t", 1);
			sscanf(buffer, "%s\t%c\t%d", n, &s, &f);
			
			tName *tmp = (tName *)malloc( sizeof(tName));
			strcpy(tmp->name, n);
			tmp->sex = s;
			tmp->freq[0] = f;
			
			if( (tempAdr = bsearch( tmp, names->data, names->len, sizeof(tName), compare)) != NULL)
			{
				tempAdr->freq[year_index] = f;
			}
			else
			{
				if( names->len >= names->capacity)
				{
					names->capacity *= 2;
					names->data = (tName*)realloc(names->data, names->capacity * sizeof(tName));
				}
				int l = (names->len)+tmplen;
				memset(names->data[l].freq, 0, sizeof(int) * MAX_YEAR_DURATION);
				sscanf(buffer, "%s\t%c\t%d", names->data[l].name, &names->data[l].sex, &names->data[l].freq[year_index]);
				index++;
				tmplen++;
			}
			free(tmp);
		}
		names->len += tmplen;
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


//compare함수
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