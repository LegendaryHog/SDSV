#ifndef STACK_INCLUDED
#define STACK_INCLUDED

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define LOW_CAPACITY 48
#define START_NUM 24
#define END_NUM stk->capacity - 16
#define NULL_PTR_EXIT if (stk == NULL){return 0;}
#define GOD_LOVE_TRIPLE 3
#define VLG 34


#define DATACANARY1 *((canary_t*)((char*)stk->data - sizeof (canary_t)))
#define DATACANARY2 *((canary_t*)((char*)stk->data + stk->capacity * stk->type_s))

typedef long long canary_t;

typedef struct {
    long long canary1;
    char*      name;
    long long size;
    long long capacity;
    void*     data;
    long long hash;
    size_t    type_s;
    void      (*fprint_elem) (FILE* file, void* ptrelem);
    FILE*     logfile;
    unsigned  error;
    long long canary2;
} stack;

//--------------------------------------------
//Hash

/*static long long Hash_Calc (stack* const stk);

static void Hash_Up (stack* stk);

static int Hash_Check (const stack* const stk);*/

//--------------------------------------------

int  Stack_Ctor    (stack* stk, char* stk_name, size_t type_ass, void (*fprint) (FILE* file, void* ptrelem), void* ptrmem);

int  Stack_Dtor    (stack* stk);

int  Stack_Push    (stack* stk, void* ptrpush);

int  Stack_Resize  (stack* stk, const int mode);

int  Stack_Pop     (stack* stk, void* ptrpop);

int  Stack_Dump     (const stack* const stk);

int  StaCkok        (stack* const stk);

int  Stack_Check    (stack* const stk);

int  Stack_Push_ptr (stack* stk, void* ptr);

char* strcat_r      (char* str1, const char* str2);

char* my_strcat     (char* str1, const char* str2);


#endif