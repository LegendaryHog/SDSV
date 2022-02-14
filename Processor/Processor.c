#include "Processor.h"

FILE* logfile = NULL;

const int RAM_BIT    = 1<<4;
const int REG_BIT    = 1<<5;
const int CONST_BIT  = 1<<6;
const int SIZEOF_ARG = 8;
const int ADIN       = 1;
const int REG_NUM    = 4;
const int RAM_VOL    = 1024*1024;
const int CODE_END   = 1024*8;
const int STACK_ST   = RAM_VOL - 1024;
const int MAX_WIN_WIDTH  = 255;
const int MAX_WIN_HIGH   = 255;
const int FREE_MEM_ST    = CODE_END + (((sizeof (video_mem) + MAX_WIN_WIDTH * MAX_WIN_HIGH)/SIZEOF_ARG) + 1) * SIZEOF_ARG;
const int FREE_MEM_END   = STACK_ST;

const double EPSILON  = 1E-6;
const float  DELAY_TM = 0.005;  


int Proc_Ctor (Processor* proc, char* filename)
{
    assert (proc != NULL);
    proc->RAM = (void*) calloc (RAM_VOL, sizeof(char));

    proc->code = Read (filename, proc->RAM) + sizeof (size_t);

    proc->VID          = (video_mem*)((char*)proc->RAM + CODE_END);
    proc->VID->width   = 0;
    proc->VID->high    = 0;
    proc->VID->memory  = (char*)((char*)proc->RAM + CODE_END + sizeof (video_mem));

    proc->stk  = (stack*)((char*)proc->RAM + STACK_ST - sizeof (stack));
    Stack_Ctor (proc->stk, (char*)"proc_stack", sizeof (double), fprint_double, (char*)proc->RAM + STACK_ST); 

    proc->reg = (double*) calloc (REG_NUM, sizeof (double));

    return NO_ERR;
}

int Proc_Dtor (Processor* proc)
{
    assert (proc != NULL);
    Stack_Dtor (proc->stk);
    free (proc->reg);
    free (proc->RAM);

    return NO_ERR;
}

void fprint_double (FILE* file, void* ptrdouble)
{
    fprintf (file, "%lf", *((double*)ptrdouble));
}

void Stack_Push_d (stack* stk, double push)
{
    Stack_Push (stk, &push);
}

void Clear_buffer (void)
{
    while (getchar () != '\n') {;}
}

double Stack_Pop_d (stack* stk)
{
    double pop = 0;
    Stack_Pop (stk, &pop);
    return pop;
}

long long Stack_Pop_ll (stack* stk)
{
    long long pop = 0;
    Stack_Pop (stk, &pop);
    return pop;
}

int Epsilon_Null_Check (double value)
{
    if (fabs (value) < EPSILON)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char* Read (char* filename, void* RAM)
{
    assert (filename != NULL);
    logfile = fopen ("proc_logfile.txt", "w");
    FILE* mach_code = fopen (filename, "rb");
    assert (mach_code != NULL);

    struct stat file_code = {};

    stat (filename, &file_code);

    char* ptrbuf = (char*) RAM;

    fread (ptrbuf, sizeof (char), file_code.st_size, mach_code);

    fclose (mach_code);

    return ptrbuf;
}

#define DEF_CMD(name, num, args_num, cmd_code)                                                      \
    case num:                                                                                       \
        ip++;                                                                                       \
        cmd_code                                                                                    \
        if (Stack_Check (proc->stk) == 0)                                                           \
        {                                                                                           \
            fflush (proc->stk->logfile);                                                            \
            fprintf (logfile, "Stack error in cmd: %s on ip: %lld\n", #name, ip);                   \
            fclose (logfile);                                                                       \
            return ERR;                                                                             \
        }                                                                                           \
        break;                                                                                      \

int Pentium (const Processor* const proc)
{
    long long ip  = 0;
    unsigned  cmd = 0;

    for (;;)
    {
        cmd = (proc->code[ip] & (0x0F)) | ((proc->code[ip] & (1<<7)) >> 3);
        
        switch (cmd)
        {
            #include "../commands.h"
            default:
                fprintf (logfile, "Unknown command with enum number %d, byte value %d on position in code %lld\n", cmd, proc->code[ip], ip);
                fclose (logfile);
                return ERR;
                break;
        }
    }
    fprintf (logfile, "not hehe\n");
    fclose (logfile);
    return ERR;
}

int Delay (float sec)
{
    clock_t start_time = clock ();

    while ((double)((clock() - start_time)/CLOCKS_PER_SEC) < sec) {;}

    return 0;
}

#undef DEF_CMD