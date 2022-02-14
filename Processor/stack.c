#include "stack.h"



const int STK_L_CAN_DIE     = 1<<0;
const int STK_R_CAN_DIE     = 1<<1;
const int DATA_L_CAN_DIE    = 1<<2;
const int DATA_R_CAN_DIE    = 1<<3;
const int STK_UFLOW         = 1<<4;
const int STK_OFLOW         = 1<<5;
const int SIZE_LESS_NULL    = 1<<6;
const int CAP_LESS_STRT_CAP = 1<<7;
const int DATA_P_NULL       = 1<<8;
const int LOG_P_NULL        = 1<<9;
const int HASH_ERR          = 1<<10;
const int DOUBLE_DTOR       = 1<<11;
const int ERR_NUM           = 12;
const int CAPACITY_0        = 8;

const long long POISON = 1000 - 7;

enum{
    UP,
    DOWN,
};

char errors_name [][VLG] = {
    {"stack left canary corruption"},  //(0)
    {"stack right canary corruption"}, //(1)
    {"data left canary corruption"},   //(2)
    {"data right canary corruption"},  //(3)
    {"stack underflow"},               //(4)
    {"stack overflow"},                //(5)
    {"size less null"},                //(6)
    {"capacity less start capacity"},  //(7)
    {"!!! DATA POINTER IS NULL !!!"},  //(8)
    {"stk->logfile pointer is null"},  //(9)
    {"hash check error"},              //(10)
    {"!!! DOUBLE DTOR !!!"},           //(11)
    {"Stack pointer is null"}          //(12)
};

//----------------------------------------------------------------------------------------------------------------------------------------------------
//Hash
static long long Hash_Calc (stack* const stk)
{
    NULL_PTR_EXIT
    Stack_Check (stk);

    long long hash = 0;

    for (size_t i = 0; i < sizeof (stack); i++)
    {
        if (!(i >= (char*)&(stk->hash) - (char*)stk && i < (char*)&(stk->canary2) - (char*)stk))
        {
            hash += (i + 1) * (*((char*)stk + i));
        }
    }

    for (size_t i = 0; i < stk->capacity * stk->type_s + 2 * sizeof (canary_t); i++)
    {
        hash += (i + 1) * (*((char*)stk->data - sizeof (canary_t) + i));
    }

    Stack_Check (stk);
    return hash;
}

static int Hash_Check (stack* const stk)
{
    NULL_PTR_EXIT
    Stack_Check (stk);

    long long hash = Hash_Calc (stk);

    if (hash != stk->hash)
    {
        stk->error |= HASH_ERR;
        for (int i = 0; i < GOD_LOVE_TRIPLE; i++)
        {
            fprintf (stk->logfile, "!!! HASH CHECK ERROR !!! stk->hash = %lld hash = %lld\n", stk->hash, hash);
        }
        Stack_Check (stk);
        return 0;
    }
    else
    {
        Stack_Check (stk);
        return 1;
    }
}

static void Hash_Up (stack* stk)
{
    stk->hash = Hash_Calc (stk);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

int Stack_Ctor (stack* stk, char* stk_name, size_t type_ass, void (*fprint_elem) (FILE* file, void* ptrelem), void* ptrmem)
{
    if (stk == NULL)
    {
        printf ("Pointer on stack is null\n");
        return 0;
    }
    if (stk_name == NULL)
    {
        printf ("Print me name of stack bitch or ORAORAORAORAORAORAORAORAORAORAORAORAORAORAORAORAORAOR\n");
        return 0;
    }

    stk->name = (char*) calloc (strlen (stk_name) + 1 , sizeof (char));

    for (size_t i = 0; i < strlen (stk_name); i++)
    {
        stk->name[i] = stk_name[i];
    }

    stk->type_s = type_ass;
    stk->fprint_elem = fprint_elem;

    stk->canary1  = 0xBE31AB;
    stk->capacity = CAPACITY_0;
    stk->size     = 0;
    stk->data     = (void*)((char*)ptrmem + sizeof (canary_t));
    DATACANARY1   = 0xD1CC0C;    //left canary of stack
    DATACANARY2   = 0xC0CA0;     //right canary of data
    char* file_name = (char*) calloc (strlen (stk->name) + 1, sizeof (char));
    for (size_t i = 0; i < strlen (stk->name); i++)
    {
        file_name[i] = stk_name[i];
    }
    
    file_name = strcat_r (file_name, (char*)"_logfile.txt");
    stk->logfile = fopen (file_name, "w"); 
    free (file_name);

    stk->error   = 0;
    stk->canary2 = 0xBADDED;

    stk->hash = Hash_Calc (stk);

    Hash_Check (stk);
    Stack_Check (stk);

    return 1;
}


int Stack_Dtor (stack* stk)
{
    Stack_Check (stk);

    NULL_PTR_EXIT
    if (stk->data == (void*)POISON)
    {
        stk->error |= DOUBLE_DTOR;
        Stack_Check (stk);
        return 0;
    }
    else if (stk->data == NULL)
    {
        Stack_Check (stk);
        return 0;
    }
    else
    {
        fclose (stk->logfile);
        free (stk->name);
        stk->data = (void*)POISON;

        return 1;
    }
}

int Stack_Push (stack* stk, void* ptrpush)
{
    NULL_PTR_EXIT

    Hash_Check (stk);
    Stack_Check (stk);
    Stack_Resize(stk, UP);

    for (size_t i = 0; i < stk->type_s; i++)
    {
        *((char*)stk->data + stk->size * stk->type_s + i) = *((char*)ptrpush + i);
    }

    stk->size++;
    Hash_Up (stk);
    Stack_Check (stk);
    return 1;
}

int Stack_Resize (stack* stk, const int  mode)
{
    Hash_Check (stk);
    Stack_Check (stk);

    NULL_PTR_EXIT

    long long start_capacity = stk->capacity;

    if ((stk->size == stk->capacity && mode == UP) || (stk->size == stk->capacity/2 - 5 && stk->capacity > CAPACITY_0 && mode == DOWN))
    {
        if (mode == UP)
        {
            stk->capacity *= 2;
        }
        else
        {
            stk->capacity /= 2;
        }

        if (stk->capacity > 1024)
        {
            Stack_Check (stk);
            Hash_Up (stk); 
            return 0;
        }
        else
        {
            DATACANARY2 = *((canary_t*)((char*)stk->data + start_capacity * stk->type_s));
            *((canary_t*)((char*)stk->data + start_capacity * stk->type_s)) = 0;
            Stack_Check (stk);
            Hash_Up (stk);
            return 1;
        }
    }
    else
    {
        Stack_Check (stk);
        return 0;
    }
}

int Stack_Pop (stack* stk, void* ptrpop)
{
    NULL_PTR_EXIT

    Hash_Check (stk);
    Stack_Check (stk);

    if (stk->size == 0)
    {
        stk->error |= STK_UFLOW;
        Stack_Check (stk);
        //---------
        //pop = 0
        //---------
        for (size_t i = 0; i < stk->type_s; i++)
        {
            *((char*)ptrpop + i) = 0;
        }
        return 0;
    }

    Stack_Resize (stk, DOWN);

    stk->size--;
    if (ptrpop != NULL)
    {
        //--------------
        //pop = *ptrpop
        //--------------
        for (size_t i = 0; i < stk->type_s; i++)
        {
            *((char*)ptrpop + i) = *((char*)stk->data + stk->size * stk->type_s + i);
        }
    }
    //-----------------
    //clean elem of stk
    //-----------------
    for (size_t i = 0; i < stk->type_s; i++)
    {
        *((char*)stk->data + stk->size * stk->type_s + i) = 0;
    }

    Hash_Up (stk);

    return 0;
}

int Stack_Dump (const stack* const stk)
{
    fprintf (stk->logfile, "Stack_%s[%p]", stk->name, stk);
    if (stk->error > 0)
    {
        fprintf (stk->logfile, "Stack is BAD:\n");

        for (int i = 0; i < ERR_NUM; i++)
        {
            if (stk->error & 1<<i)
            {
                fprintf (stk->logfile, "%s;\n", errors_name[i]);
            }
        }
    }
    else
    {
        fprintf (stk->logfile, "Stack is Ok.\n");
    }

    fprintf (stk->logfile, "{\n    canary1 = %0llX\n    hash = %lld\n    capacity = %lld\n    size = %lld\n    ", stk->canary1, stk->hash, stk->capacity, stk->size);
    fprintf (stk->logfile, "data [%p]\n    {\n        ", stk->data);

    if (stk->data > (void*)POISON)
    {
        fprintf (stk->logfile, "datacanary1 = %0llX\n        ", DATACANARY1);

        if (stk->fprint_elem != NULL)
        {
            if (stk->capacity <= LOW_CAPACITY)
            {
                for (long long i = 0; i < stk->capacity; i++)
                {
                    if (i < stk->size)
                    {
                        fprintf (stk->logfile, "*[%lld] = ", i);
                    }
                    else
                    {
                        fprintf (stk->logfile, " [%lld] = ", i);
                    }
                    stk->fprint_elem (stk->logfile, (char*)stk->data + i * stk->type_s);
                    fprintf (stk->logfile, "\n        ");
                }
            }
            else
            {
                for (long long i = 0; i < START_NUM; i++)
                {
                    if (i < stk->size)
                    {
                        fprintf (stk->logfile, "*[%lld] = ", i);
                    }
                    else
                    {
                        fprintf (stk->logfile, "*[%lld] = ", i);
                    }
                    stk->fprint_elem (stk->logfile, (char*)stk->data + i * stk->type_s);
                    fprintf (stk->logfile, "\n        ");
                }
                fprintf (stk->logfile, ".\n        .\n        .\n        ");

                for (long long i = END_NUM; i >= END_NUM && i < stk->capacity; i++)
                {
                    if (i < stk->size)
                    {
                        fprintf (stk->logfile, "*[%lld] = ", i);
                    }
                    else
                    {
                        fprintf (stk->logfile, " [%lld] = ", i);
                    }
                    stk->fprint_elem (stk->logfile, (char*)stk->data + i * stk->type_s);
                    fprintf (stk->logfile, "\n        ");
                }
            }
        }

        else
        {
            fprintf (stk->logfile, "\n        If you want to see elements of your stack,\n");
            fprintf (stk->logfile, "        you must write function that print element of your stack:\n\n         void (name) (FILE* file, void* ptrelem)\n\n");
            fprintf (stk->logfile, "        and give pointer on it in Stack_Ctor in third argument\n");
            fprintf (stk->logfile, "        do not be baka and write it.\n\n");
        }
        fprintf (stk->logfile, "datacanary2 = %0llX\n    }\n", DATACANARY2);
    }
    fprintf (stk->logfile, "    canary2 = %0llX\n}\n", stk->canary2);
    return 0;
}

int StaCkok (stack* const stk)
{
    NULL_PTR_EXIT

    stk->error |= STK_L_CAN_DIE     && (stk->canary1 != 0xBE31AB);
    stk->error |= STK_R_CAN_DIE     && (stk->canary2 != 0xBADDED);
    stk->error |= DATA_L_CAN_DIE    && (DATACANARY1 != 0xD1CC0C);
    stk->error |= DATA_R_CAN_DIE    && (DATACANARY2 != 0xC0CA0);
    stk->error |= STK_OFLOW         && ((stk->size > stk->capacity) || (stk->capacity > 1024));
    stk->error |= SIZE_LESS_NULL    && (stk->size < 0);
    stk->error |= CAP_LESS_STRT_CAP && (stk->capacity < CAPACITY_0);
    stk->error |= DATA_P_NULL       && (stk->data == NULL);
    stk->error |= LOG_P_NULL        && (stk->logfile == NULL);

    if (stk->error > 0)
    {
        return 0;
    }
    return 1;
}

int Stack_Check (stack* const stk)
{
    if (StaCkok (stk) == 0)
    {
        Stack_Dump (stk);
        return 0;
    }
    return 1;
}

char* strcat_r (char* str1, const char* str2)
{
    size_t len1 = strlen (str1);
    size_t len2 = strlen (str2);

    if (sizeof (str1) < len1 + len2 + 1)
    {
        str1 = (char*) realloc (str1, len1 + len2 + 3);
        for (size_t i = len1; i < len1 + len2 + 3; i++)
        {
            str1[i] = '\0';
        }
    }
    return my_strcat (str1, str2);
}

char* my_strcat (char* str1, const char* str2)
{
    char* ptr = str1;

    while (*str1 != '\0')
    {
        str1++;
    }

    while (*str2 != '\0')
    {
        *str1++ = *str2++;
    }
    *str1 = '\0';

    return ptr;
}


