#include "Assembler.h"

int main (int argc, char* argv[])
{
    Assembler ass = {};

    Assm_Ctor (&ass, argv[argc - 2]);

    if (Buffer_to_Code (&ass) == ERR)
    {
        printf ("Assembler error\n");
        return - 1;
    }

    if (Funcs_n_lbls_Check (ass.funcs, ass.lbls) == ERR)
    {
        printf ("Funcs and labels error\n");
        return -1;
    }

    Buffer_to_Code (&ass);
    assert (ass.code != NULL);

    FILE* Code = fopen (argv[argc - 1], "wb");
    size_t num_of_cmd = *(size_t*)(ass.code - sizeof (size_t));

    fwrite (ass.code - sizeof (size_t), sizeof (char), 10 * num_of_cmd, Code);

    fclose (Code);
    Assm_Dtor (&ass);

    return 0;
}
