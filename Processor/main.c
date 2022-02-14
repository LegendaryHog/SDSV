#include "Processor.h"

int main (int argc, char* argv[])
{
    Processor proc = {};

    Proc_Ctor (&proc, argv[argc - 1]);

    if (Pentium (&proc) == ERR)
    {
        printf ("Processor error\n");
        Proc_Dtor (&proc);
        return -1;
    }

    Proc_Dtor (&proc);

    return 0;
}

