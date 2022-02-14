#ifndef PROC_INCLUDED
#define PROC_INCLUDED

#include <stdio.h>
#include <sys/stat.h>
#include "stack.h"
#include <assert.h>
#include <math.h>
#include <time.h>

enum ERR {
    ERR,
    NO_ERR,
};

enum PICTURE {
    SPACE = 0,
    PLUS  = 1,
};

enum ARG_TYPE {
    NO_ARG,
    REGISTER,
    DOUBLE_ARG,
    MODE,
};

typedef struct
{
    char*          memory;
    unsigned char  width;
    unsigned char  high;
}
video_mem;


typedef struct
{
    char*      code;
    stack*     stk;
    void*      RAM;
    double*    reg;
    video_mem* VID;
}
Processor;

int       Proc_Ctor           (Processor* proc, char* filename);

int       Proc_Dtor           (Processor* proc);

void      fprint_double       (FILE* file, void* ptrdouble);

void      Stack_Push_d        (stack* stk, double push);

void      Clear_buffer        (void);

int       Delay               (float sec);

double    Stack_Pop_d         (stack* stk);

long long Stack_Pop_ll        (stack* stk);

char*     Read                (char* filename, void* RAM);

int       Epsilon_Null_Check  (double value);

int       Pentium             (const Processor* const proc);

#endif //PROC_INCLUDE
