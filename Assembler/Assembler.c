#include "Assembler.h"

FILE* logfile;

int iteration = FIRST;

char reg_name [][LENREG] = {
    "rax",
    "rbx",
    "rcx",
    "rdx"
};

char* From_File_to_buffer (size_t* ptr_buffsize, char* filename)
{
    assert (filename != NULL);
    logfile = fopen ("asm_logfile.txt", "w");
    assert (logfile != NULL);

    FILE* buffer = fopen (filename, "r");
    assert (buffer != NULL);

    fseek(buffer, 0, SEEK_SET);
    long start_of_File = ftell (buffer);

    fseek(buffer, 0, SEEK_END);
    long end_of_File = ftell (buffer);

    fseek(buffer, 0, SEEK_SET);
    long size_File = end_of_File - start_of_File;

    char* ptrBuff = (char*) calloc (size_File + 1, sizeof (char));
    assert (ptrBuff != NULL);

    *ptr_buffsize = fread (ptrBuff, sizeof (char), size_File, buffer);

    fclose (buffer);

    *(ptrBuff + *ptr_buffsize) = '\0';

    ptrBuff = (char*) realloc (ptrBuff, *ptr_buffsize + 1);
    assert (ptrBuff != NULL);

    return (char*)ptrBuff;
}

int Assm_Ctor (Assembler* ass, char* filename)
{
    assert (ass != NULL);
    size_t buffsize = 0;

    ass->funcs  = (function*) calloc (MAX_NUM_OF_FUNCS, sizeof (function));
    ass->lbls   = (label*)    calloc (MAX_NUM_OF_LBLS,  sizeof (label));
    ass->ptrbuf = From_File_to_buffer (&buffsize, filename);
    ass->code   = (char*)     calloc (10*buffsize, sizeof (char))  + sizeof (size_t);

    assert (ass->funcs  != NULL);
    assert (ass->lbls   != NULL);
    assert (ass->ptrbuf != NULL);
    assert (ass->code   != NULL);

    return NO_ERR;
}

int Assm_Dtor (Assembler* ass)
{
    assert (ass != NULL);
    assert (ass->code != NULL);
    assert (ass->funcs != NULL);
    assert (ass->lbls != NULL);
    assert (ass->ptrbuf != NULL);
    free (ass->funcs);
    free (ass->lbls);
    free (ass->ptrbuf);
    free (ass->code - sizeof (size_t));

    return NO_ERR;
}

int Buffer_to_Code (Assembler* ass)
{
    int       cmd         = 0;
    size_t    num_of_line = 0;
    int       breaking    = 0;
    int       args_number = 0;
    size_t    num_of_cmd  = 0;
    size_t    ip_lbls     = 0;
    size_t    ip_funcs    = 0;
    long long ip          = 0;
    int       finder      = 0; 

    char* ptr = ass->ptrbuf;

    while (breaking == 0)
    {
        _BREAK_IF_

        num_of_line += Enters_n_spaces_skip (&ptr);

        _BREAK_IF_

        char* word = Take_Word (ptr);

        #define DEF_CMD(name, num, args_num, ...)                              \
        if (strncmp_tolower ((char*)#name, word,  Max (strlen ((char*)#name), strlen (word))) == 0)   \
        {                                                                      \
            cmd = num;                                                         \
            args_number = args_num;                                            \
        }                                                                      \
        else

        #include "../commands.h"

        //else
        {
            if (Func_Scan (ass->funcs, &ip_funcs, ip, ptr, num_of_line, &finder) == ERR)
            {
                return ERR;
            }
            if (Label_Scan (ass->lbls, &ip_lbls, ip, ptr, num_of_line) == ERR && finder == 0)
            {
                return ERR;                
            }
            Skip_Symbols (&ptr);
            cmd = IS_LABEL_OR_FUNC;
        }

        free (word);
        #undef DEF_CMD

        #define DEF_CMD(name, num, args_num, ...)                                                                                      \
        case CMD_##name:                                                                                                               \
        {                                                                                                                              \
            num_of_cmd++;                                                                                                              \
            ptr += strlen (#name);                                                                                                     \
            unsigned char typemask = 0;                                                                                                \
                                                                                                                                       \
            while (args_number > 0)                                                                                                    \
            {                                                                                                                          \
                if (*ptr != ' ' && *ptr != '\t' && cmd != CMD_POP)                                                                     \
                {                                                                                                                      \
                    fprintf (logfile, "No space between command/argument and argument of %s in line %zd\n", #name, num_of_line + 1);   \
                    fprintf (logfile, "Letter is: (%c) (%d)\n", *ptr, *ptr);                                                           \
                    fclose (logfile);                                                                                                  \
                    return ERR;                                                                                                        \
                }                                                                                                                      \
                _BREAK_IF_                                                                                                             \
                Skip_Spaces (&ptr);                                                                                                    \
                                                                                                                                       \
                if (Arg_Scan (&ptr, ass, ip, &ip_lbls, &ip_funcs, num_of_line, cmd, &typemask) == ERR)                                 \
                {                                                                                                                      \
                    return ERR;                                                                                                        \
                }                                                                                                                      \
                args_number--;                                                                                                         \
            }                                                                                                                          \
            if ((cmd & (1<<4)) != 0)                                                                                                   \
            {                                                                                                                          \
                cmd &= ~(1<<4);                                                                                                        \
                cmd |= (1<<7);                                                                                                         \
            }                                                                                                                          \
            ass->code[ip] = cmd | typemask;                                                                                            \
            ip++;                                                                                                                      \
            if ((typemask & REG_BIT) != 0)                                                                                             \
            {                                                                                                                          \
                ip += sizeof (char);                                                                                                   \
            }                                                                                                                          \
            if ((typemask & CONST_BIT) != 0)                                                                                           \
            {                                                                                                                          \
                ip += sizeof (double);                                                                                                 \
            }                                                                                                                          \
            Skip_Spaces (&ptr);                                                                                                        \
                                                                                                                                       \
            _BREAK_IF_                                                                                                                 \
            if (*ptr != '\n' && *ptr != '\0' && *ptr != '\r')                                                                          \
            {                                                                                                                          \
                fprintf (logfile, "Symbols after %s in line %zd\n", #name, num_of_line + 1);                                           \
                fprintf (logfile, "incorrect letter = %c (%d)\n", *ptr, *ptr);                                                         \
                fclose (logfile);                                                                                                      \
                return ERR;                                                                                                            \
            }                                                                                                                          \
        }                                                                                                                              \
        break;                                                                                                                         \

        switch (cmd) {
            #include "../commands.h"
            case IS_LABEL_OR_FUNC:
                Skip_Spaces (&ptr);
                if (*ptr != '\0' && *ptr != '\n' && *ptr != '\r')
                {
                    fprintf (logfile, "incorrect letter = %c (%d)\n", *ptr, *ptr);
                    fprintf (logfile, "Incorrect symbols after label or function in line %zd\n", num_of_line + 1);
                    fclose (logfile);
                    return ERR;
                }
                break;
            default:
                fprintf (logfile, "Unexpected error in switch\n");
                fclose (logfile);
                return ERR;
                break;
        }

        #undef DEF_CMD

    }
    
    if (iteration == SECOND)
    {
        fclose (logfile);
    }

    *((size_t*)(ass->code - sizeof (size_t))) = num_of_cmd;

    iteration = SECOND;

    return NO_ERR;
}

size_t Enters_n_spaces_skip (char** ptr_on_ptr)
{
    size_t find_enter_counter = 0;
    while (**ptr_on_ptr == ' ' || **ptr_on_ptr == '\t' || **ptr_on_ptr == '\n' || **ptr_on_ptr == '\r')
    {
        if (**ptr_on_ptr == '\n')
        {
            find_enter_counter++;
        }
        (*ptr_on_ptr)++;
    }

    return find_enter_counter;
}

size_t Skip_Spaces (char** ptr_on_ptr)
{
   size_t counter = 0;

   while (**ptr_on_ptr == ' ' || **ptr_on_ptr == '\t')
   {
       (*ptr_on_ptr)++;
       counter++;
   }

   return counter;
}

size_t Skip_Symbols (char** ptr_on_ptr)
{
    size_t counter = 0;

    while (**ptr_on_ptr != ' ' && **ptr_on_ptr != '\t' && **ptr_on_ptr != '\n' && **ptr_on_ptr != '\0' && **ptr_on_ptr != '\r')
    {
        (*ptr_on_ptr)++;
        counter++;
    }

    return counter;
}

double d_pow_10 (int power)
{
    double res = 1;

    if (power > 0)
    {
        for (int i = 0; i < power; i++)
        {
            res *= 10;
        }
    }
    else if (power < 0)
    {
        for (int i = 0; i > power; i--)
        {
            res /= 10;
        }
    }

    return res;
}

long long ll_pow_10 (int power)
{
    long long res = 1;

    for (int i = 0; i < power; i++)
    {
        res *= 10;
    }

    return res;
}

int Arg_Scan (char** ptr_ptrarg, Assembler* ass, size_t ip, size_t* ptr_ip_lbls, size_t* ptr_ip_func, size_t num_of_line, int cmd, unsigned char* ptr_typemask)
{
    int local_ip = 1;
    switch (cmd)
    {
        case CMD_PUSH:
        case CMD_POP:
        {
            unsigned char reg   = 0;
            double        darg  = 0;
            long long     llarg = 0;

            if (**ptr_ptrarg == '[')
            {
                *ptr_typemask |= RAM_BIT;
                (*ptr_ptrarg)++;
                Skip_Spaces (ptr_ptrarg);
            }
            if (**ptr_ptrarg == 'r')
            {
                *ptr_typemask |= REG_BIT;
                reg = Reg_Scan (*ptr_ptrarg);
                if (reg == 0)
                {
                    fprintf (logfile, "Incorrect register in line %zd\n", num_of_line + 1);
                    fclose (logfile);
                    return ERR;
                }
                (*ptr_ptrarg) += 3;
                Skip_Spaces (ptr_ptrarg);
            }

            if (**ptr_ptrarg == '+' || **ptr_ptrarg == '-' || (**ptr_ptrarg >= '0' && **ptr_ptrarg <= '9'))
            {
                *ptr_typemask |= CONST_BIT;
                Skip_Spaces (ptr_ptrarg);
                if ((*ptr_typemask & RAM_BIT) == 0)
                {
                    if (Double_Arg_Scan (*ptr_ptrarg, &darg, num_of_line) == ERR)
                    {
                        return ERR;
                    }
                    //printf ("symb = %c (%d)\n", **ptr_ptrarg, **ptr_ptrarg);
                    while (**ptr_ptrarg == '+' || **ptr_ptrarg == '-' || **ptr_ptrarg == '.' || **ptr_ptrarg == ' ' || (**ptr_ptrarg >= '0' && **ptr_ptrarg <= '9'))
                    {
                        (*ptr_ptrarg)++;
                    }
                    if (**ptr_ptrarg != ' ' && **ptr_ptrarg != '\t' && **ptr_ptrarg != '\n' && **ptr_ptrarg != '\r' && **ptr_ptrarg != ']')
                    {
                        fprintf (logfile, "Incorrect input in line %zd\n", num_of_line + 1);
                        fclose (logfile);
                        return ERR;
                    }
                }
                else
                {
                    if (LL_Arg_Scan (*ptr_ptrarg, &llarg, num_of_line) == ERR)
                    {
                        return ERR;
                    }
                    while (**ptr_ptrarg == '+' || **ptr_ptrarg == '-' || (**ptr_ptrarg >= '0' && **ptr_ptrarg <= '9'))
                    {
                        (*ptr_ptrarg)++;
                    }
                    if (**ptr_ptrarg != ' ' && **ptr_ptrarg != '\t' && **ptr_ptrarg != '\n' && **ptr_ptrarg != '\r' && **ptr_ptrarg != ']')
                    {
                        fprintf (logfile, "Incorrect input in line %zd\n", num_of_line + 1);
                        fclose (logfile);
                        return ERR;
                    }
                }
            }
            Skip_Spaces (ptr_ptrarg);

            if (**ptr_ptrarg != ']' && ((*ptr_typemask  & RAM_BIT) != 0))
            {
                fprintf (logfile, "Incorrect input in line %zd\n", num_of_line + 1);
                fclose (logfile);
                return ERR;
            }
            
            if ((*ptr_typemask  & RAM_BIT) != 0)
            {
                (*ptr_ptrarg)++;
            }

            if (Mask_Check (*ptr_typemask, cmd, num_of_line) == ERR)
            {
                return ERR;
            }

            if ((*ptr_typemask & REG_BIT) != 0)
            {
                *(char*)(ass->code + ip + local_ip) = reg;
                local_ip++;
            }

            if ((*ptr_typemask & RAM_BIT) == 0)
            {
                *(double*)(ass->code + ip + local_ip) = darg;
            }
            else
            {
                *(long long*)(ass->code + ip + local_ip) = llarg;
            }
            return NO_ERR;
        }
        break;
        CASE_CMD_ALL_JMP
            *ptr_typemask |= CONST_BIT;
            return Jump_Arg_Scan (ass->lbls, ptr_ip_lbls, ptr_ptrarg, ass->code + ip + 1, num_of_line);
            break;
        case CMD_CALL:
            *ptr_typemask |= CONST_BIT;
            return Call_Arg_Scan (ass->funcs, ptr_ip_func, ptr_ptrarg, ass->code + ip + 1, num_of_line);
            break;
        default:
            fprintf (logfile, "Command without argument, but has argument in commands.h \n");
            fclose (logfile);
            return ERR;
            break;
    }
    fprintf (logfile, "Not switch enter in line %zd\n", num_of_line + 1);
    return ERR;
}

int Call_Arg_Scan (function* func, size_t* ptr_ip_func, char** ptr_ptrarg, char* code, size_t num_of_line)
{
    size_t i         = 0;
    size_t len_func  = 0;
    int    find_func = 0;
    int    counter   = 0;

    while ((*ptr_ptrarg)[len_func] != ':' && (*ptr_ptrarg)[len_func] != ' ' && (*ptr_ptrarg)[len_func] != '\t' && (*ptr_ptrarg)[len_func] != '\0')
    {
        len_func++;
    }

    while ((*ptr_ptrarg)[i] != '\n' && (*ptr_ptrarg)[i] != '\0' && (*ptr_ptrarg)[i] != ':' && (*ptr_ptrarg)[i] != '\r')
    {
        i++;
    }

    if ((*ptr_ptrarg)[i] != ':')
    {
        fprintf (logfile, "Incorrect input in line %zd\n", num_of_line + 1);
        return ERR;
    }
    else if (len_func >= MAX_LEN_FUNC)
    {
        fprintf (logfile, "Too long name of function in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }
    else
    {
        for (int j = 0; j < MAX_NUM_OF_FUNCS; j++)
        {
            if (strncmp (func[j].func_name, *ptr_ptrarg, len_func) == 0)
            {
                counter++;
                find_func = j;
            }
        }
        if (counter == 0)
        {
            strncpy (func[*ptr_ip_func].func_name, *ptr_ptrarg, len_func);
            func[*ptr_ip_func].jmp_ptr = NOT_FIND_YET;
            (*ptr_ip_func)++;
            (*ptr_ptrarg) += len_func + 1;
            return NO_ERR;
        }
        else if (counter > 1)
        {
            fprintf (logfile, "Two or more functions with one name in line %zd\n", num_of_line + 1);
            fclose (logfile);
            return ERR;
        }
        else
        {
            *(long long*)code = func[find_func].jmp_ptr;
            (*ptr_ptrarg) += len_func + 1;
            return NO_ERR;
        }
    }
}

int Jump_Arg_Scan (label* lbls, size_t* ptr_ip_lbls, char** ptr_ptrarg, char* code, size_t num_of_line)
{
    size_t i        = 0;
    size_t len_lbl  = 0;
    int    counter  = 0;
    int    find_lbl = 0;

    while ((*ptr_ptrarg)[len_lbl] != ':' && (*ptr_ptrarg)[len_lbl] != ' ' && (*ptr_ptrarg)[len_lbl] != '\t' && (*ptr_ptrarg)[len_lbl] != '\0')
    {
        len_lbl++;
    }

    while ((*ptr_ptrarg)[i] != '\n' && (*ptr_ptrarg)[i] != '\0' && (*ptr_ptrarg)[i] != ':' && (*ptr_ptrarg)[i] != '\r')
    {
        i++;
    }

    if ((*ptr_ptrarg)[i] != ':')
    {
        fprintf (logfile, "Incorrect input in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }

    else if (len_lbl >= MAX_LEN_LABEL)
    {
        fprintf (logfile, "Too long name of label in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }
    else
    {
        for (int j = 0; j < MAX_NUM_OF_LBLS; j++)
        {
            if (strncmp (lbls[j].lbl_name, *ptr_ptrarg, len_lbl) == 0)
            {
                find_lbl = j;
                counter++;
            }
        }
        if (counter == 0)
        {
            strncpy (lbls[*ptr_ip_lbls].lbl_name, *ptr_ptrarg, len_lbl);
            lbls[*ptr_ip_lbls].jmp_ptr = NOT_FIND_YET;
            (*ptr_ip_lbls)++;
            (*ptr_ptrarg) += len_lbl + 1;
            return NO_ERR;
        }
        else if (counter > 1 && iteration != SECOND)
        {
            fprintf (logfile, "Two or more labels with one name, one is in line %zd\n", num_of_line + 1);
            fclose (logfile);
            return ERR;
        }
        else
        {
            *(long long*)code = lbls[find_lbl].jmp_ptr;
            (*ptr_ptrarg) += len_lbl + 1;
            return NO_ERR;
        }
    } 
}

int Func_Scan (function* func, size_t* ptr_ip_funcs, long long ip, char* ptrfunc, size_t num_of_line, int* finder)
{
    size_t len_func  = 0;
    size_t i         = 0;
    int    counter   = 0;
    int    find_func = 0;

    while (ptrfunc[len_func] != ':' && ptrfunc[len_func] != ' ' && ptrfunc[len_func] != '\t' && ptrfunc[len_func] != '\0')
    {
        len_func++;
    }

    while (ptrfunc[i] != ':' && ptrfunc[i] != '\n' && ptrfunc[i] != '\0' && ptrfunc[i] != '\r')
    {
        i++;
    }

    if (ptrfunc[i] != ':')
    {
        fprintf (logfile, "Incorrect command or label in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }
    else if (len_func== 0 || len_func > MAX_LEN_LABEL)
    {
        fprintf (logfile, "Too long label in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }
    else
    {
        for (int j = 0; j < MAX_NUM_OF_FUNCS; j++)
        {
            if (strncmp (ptrfunc, func[j].func_name, len_func) == 0)
            {
                find_func = j;
                counter++;
                *finder = 1;
            }
        }
        if (counter == 0)
        {
            strncpy (func[*ptr_ip_funcs].func_name, ptrfunc, len_func);
            func[*ptr_ip_funcs].jmp_ptr = ip;
            (*ptr_ip_funcs)++;   
            return NO_ERR;
        }
        else if (counter == 1 && func[find_func].jmp_ptr == NOT_FIND_YET)
        {
            func[find_func].jmp_ptr = ip;
            return NO_ERR;
        }
        else if (iteration == SECOND)
        {
            return NO_ERR;
        }
        else
        {
            fprintf (logfile, "Two label with one name, second in line %zd\n", num_of_line + 1);
            fclose (logfile);
            return ERR;
        }
    }
}

int Label_Scan (label* lbls, size_t* ptr_ip_lbls, long long ip, char* ptrlbl, size_t num_of_line)
{
    size_t len_lbl  = 0;
    size_t i        = 0;
    int    counter  = 0;
    int    find_lbl = 0;

    while (ptrlbl[len_lbl] != ':' && ptrlbl[len_lbl] != ' ' && ptrlbl[len_lbl] != '\t' && ptrlbl[len_lbl] != '\0')
    {
        len_lbl++;
    }

    while (ptrlbl[i] != ':' && ptrlbl[i] != '\n' && ptrlbl[i] != '\0' && ptrlbl[i] != '\r')
    {
        i++;
    }

    if (ptrlbl[i] != ':')
    {
        fprintf (logfile, "Incorrect command or label in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }
    else if (len_lbl == 0 || len_lbl > MAX_LEN_LABEL)
    {
        fprintf (logfile, "Too long label in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }
    else
    {
        for (int j = 0; j < MAX_NUM_OF_LBLS; j++)
        {
            if (strncmp (ptrlbl, lbls[j].lbl_name, len_lbl) == 0)
            {
                find_lbl = j;
                counter++;
            }
        }
        if (counter == 0)
        {
            strncpy (lbls[*ptr_ip_lbls].lbl_name, ptrlbl, len_lbl);
            lbls[*ptr_ip_lbls].jmp_ptr = ip;
            (*ptr_ip_lbls)++;
            return NO_ERR;
        }
        else if (counter == 1 && lbls[find_lbl].jmp_ptr == NOT_FIND_YET)
        {
            lbls[find_lbl].jmp_ptr = ip;
            return NO_ERR;
        }
        else if (iteration == SECOND)
        {
            return NO_ERR;
        }
        else
        {
            fprintf (logfile, "Two label with one name, second in line %zd\n", num_of_line + 1);
            fclose (logfile);
            return ERR;
        }
    }
}

unsigned char Reg_Scan (char* ptrarg)
{
    unsigned char reg = 0;

    for (int i = 0; i < 4; i++)
    {
        if (strncmp (ptrarg, reg_name[i], 3) == 0)
        {
            reg = i + 1;
        }
    }

    return reg;
}

int Double_Arg_Scan (char* ptrarg, double* ptr_darg, size_t num_of_line)
{
    int dot_pos = 0;
    double darg = 0;
    size_t dot_counter = 0;
    int sign = 0;

    if (*ptrarg == '+')
    {
        ptrarg++;
        Skip_Spaces (&ptrarg);
    }

    if (*ptrarg == '-')
    {
        sign = -1;
        ptrarg++;
    }
    else
    {
        sign = 1;
    }
    Skip_Spaces (&ptrarg);

    while (ptrarg[dot_pos] >= '0' && ptrarg[dot_pos] <= '9')
    {
        dot_pos++;
    }

    if (ptrarg[dot_pos] != '.' && ptrarg[dot_pos] && ptrarg[dot_pos] != ' ' && ptrarg[dot_pos] != '\t' && ptrarg[dot_pos] != '\n' && ptrarg[dot_pos] != '\r' && ptrarg[dot_pos] != '\0')
    {
        fprintf (logfile, "Fucking symbols in argument in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }
    else
    {
        while (*ptrarg != ' ' && *ptrarg != '\t' && *ptrarg != '\0' && *ptrarg != '\n' && *ptrarg != '\r')
        {
            if (*ptrarg != '.')
            {
                darg += (*ptrarg - '0') * d_pow_10 (dot_pos - 1);
                dot_pos--;
            }
            else
            {
                dot_counter++;
            }
            ptrarg++;
        }
        if (dot_counter > 1)
        {
            fprintf (logfile, "Two dots in argument in line %zd\n", num_of_line + 1);
            fclose (logfile);
            return ERR;
        }   
    }
    
    *ptr_darg = sign * darg;
    return NO_ERR;
}

int LL_Arg_Scan (char* ptrarg, long long* ptr_llarg, size_t num_of_line)
{
    int len = 0;
    int sign = 0;
    long long llarg = 0;

    if (*ptrarg == '+')
    {
        ptrarg++;
        Skip_Spaces (&ptrarg);
    }

    if (*ptrarg == '-')
    {
        sign = -1;
        ptrarg++;
    }
    else
    {
        sign = 1;
    }
    Skip_Spaces (&ptrarg);

    while (ptrarg[len]>= '0' && ptrarg[len]<= '9')
    {
        len++;
    }

    if (len == 0)
    {
        fprintf (logfile, "Incorrect input in long long argument in line %zd\n", num_of_line + 1);
        fclose (logfile);
        return ERR;
    }
    while (len != 0)
    {
        llarg += (*ptrarg - '0') * ll_pow_10 (len - 1);
        ptrarg++;
        len--;
    }

    *ptr_llarg = sign * llarg;

    return NO_ERR;
}

int Funcs_n_lbls_Check (function* func, label* lbls)
{
    for (int i = 0; i < MAX_NUM_OF_FUNCS; i++)
    {
        if (func[i].jmp_ptr == NOT_FIND_YET)
        {
            fprintf (logfile, "Not full functions massive\n func: %s\n", func[i].func_name);
            fclose (logfile);
            return ERR;
        }
    }

    for (int i = 0; i < MAX_NUM_OF_LBLS; i++)
    {
        if (lbls[i].jmp_ptr == NOT_FIND_YET)
        {
            fprintf (logfile, "Not full labels massive\n");
            fclose (logfile);
            return ERR;
        }
    }
    return NO_ERR;
}

int Mask_Check (unsigned char typemask, char cmd, size_t num_of_line)
{
    switch (cmd)
    {
        case CMD_PUSH:
            if ((typemask & REG_BIT) == 0 && (typemask & CONST_BIT) == 0)
            {
                fprintf (logfile, "Have not const or register in argument in line %zd\n", num_of_line + 1);
                fclose (logfile);
                return ERR;
            }
            break;
        case CMD_POP:
            if ((typemask & RAM_BIT) == 0 && (typemask & REG_BIT) != 0 && (typemask & CONST_BIT) != 0)
            {
                fprintf (logfile, "Arg type is \"register + const\" of pop in line %zd\n", num_of_line + 1);
                fclose (logfile);
                return ERR;
            }
            if ((typemask & CONST_BIT) != 0 && (typemask & RAM_BIT) == 0)
            {
                fprintf (logfile, "Arg type is has const, but has not RAM in pop in line %zd\n", num_of_line + 1);
                fclose (logfile);
                return ERR;
            }
            break;
        default:
            return NO_ERR;
    }
    return NO_ERR;
}

char my_tolower (char symb)
{
    if (symb >= 'A' && symb <= 'Z')
    {
        return symb + ('a' - 'A');
    }
    else
    {
        return symb;
    }
}

int strncmp_tolower (char* str1, char* str2, size_t num)
{
    size_t i = 0;

    while (my_tolower (str1[i]) == my_tolower (str2[i]) && i < num - 1)
    {
        i++;
    }

    return my_tolower(str1[i]) - my_tolower(str2[i]);
}

char* Take_Word (const char* ptrtext)
{
    size_t len = 0;
    size_t i   = 0;
    
    while (ptrtext[i] != ' ' && ptrtext[i] != '\r' && ptrtext[i] != '\n' && ptrtext[i] != '\0')
    {
        i++;
        len++;
    }

    char* ptrword = (char*) calloc (len + 1, sizeof(char));

    return strncpy (ptrword, ptrtext, len);
}

size_t Max (const size_t num1, const size_t num2)
{
    if (num1 >= num2)
    {
        return num1;
    }
    else
    {
        return num2;
    }
}