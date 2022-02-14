#ifndef ASS
#define ASS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>

#define NO_CMD 0
#define _BREAK_IF_ if (*ptr == '\0'){breaking = 1; break;}
#define MAX_LEN_CMD 5
#define MAX_NUM_OF_LBLS 512
#define MAX_NUM_OF_FUNCS 512
#define MAX_LEN_LABEL 15
#define MAX_LEN_FUNC 15
#define FIVE_MAGICS 5
#define LENREG 4
#define NOT_FIND_YET -1
#define IS_LABEL_OR_FUNC -69
#define RAM_BIT   (1<<4)
#define REG_BIT   (1<<5)
#define CONST_BIT (1<<6)

#define CASE_CMD_ALL_JMP \
case CMD_JMP:            \
case CMD_JAE:            \
case CMD_JBE:            \
case CMD_JNE:            \
case CMD_JA:             \
case CMD_JB:             \
case CMD_JE:             \

typedef struct
{ 
    long long jmp_ptr;
    char      lbl_name[MAX_LEN_LABEL];
}
label;

typedef struct
{
    long long jmp_ptr;
    char      func_name[MAX_LEN_FUNC];
}
function;

typedef struct
{
    function* funcs;
    label*    lbls;
    char*     ptrbuf;
    char*     code;
}
Assembler;

enum ERR {
    ERR,
    NO_ERR,
};

enum IT {
    FIRST  = 1,
    SECOND = 2,
};

#define DEF_CMD(name, num, ...) CMD_##name = num,

enum CMD
{
    #include "../commands.h"
};

#undef DEF_CMD

char*         From_File_to_buffer  (size_t* ptr_buffsize, char* filename);

int           Assm_Ctor            (Assembler* ass, char* filename);

int           Assm_Dtor            (Assembler* ass);

int           Buffer_to_Code       (Assembler* ass);

double        d_pow_10             (int power);

long long     ll_pow_10            (int power);

int           Arg_Scan             (char** ptr_ptrarg, Assembler* ass, size_t ip, size_t* ptr_ip_lbls, size_t* ptr_ip_func, size_t num_of_line, int cmd, unsigned char* ptr_typemask);

int           Func_Scan            (function* func,size_t* ptr_ip_funcs, long long ip, char* ptrfunc, size_t num_of_line, int* finder);

int           Label_Scan           (label* lbls, size_t* ptr_ip_lbls, long long ip, char* ptrlbl, size_t num_of_line);

int           Jump_Arg_Scan        (label* lbls, size_t* ptr_ip_lbls, char** ptr_ptrarg, char* code, size_t num_of_line);

int           Call_Arg_Scan        (function* func, size_t* ptr_ip_func, char** ptr_ptrarg, char* code, size_t num_of_line);

unsigned char Reg_Scan             (char* ptrarg);

int           Double_Arg_Scan      (char* ptrarg, double* darg, size_t num_of_line);

int           LL_Arg_Scan          (char* ptrarg, long long* ptr_llarg, size_t num_of_line);

int           Func_Check           (function* func, char* ptrarg, size_t num_of_line);

size_t        Enters_n_spaces_skip (char** ptr_on_ptr);

size_t        Skip_Spaces          (char** ptr_on_ptr);

size_t        Skip_Symbols         (char** ptr_on_ptr);

int           Funcs_n_lbls_Check   (function* func, label* lbls);

int           Mask_Check           (unsigned char typemask, char cmd, size_t num_of_line);

char          my_tolower           (char symb);

int           strncmp_tolower      (char* str1, char* str2, size_t num);

char*         Take_Word            (const char* ptrtext);

size_t        Max                  (const size_t num1, const size_t num2);

#endif

