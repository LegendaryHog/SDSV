DEF_CMD(PUSH, 1, 1,
    {
        long long llarg    = 0;
        double    darg     = 0;
        int       local_ip = 0;

        if ((proc->code[ip - 1] & RAM_BIT) != 0)
        {
            Delay (DELAY_TM);
            if ((proc->code[ip - 1] & REG_BIT) != 0)
            {
                llarg += (long long)proc->reg[proc->code[ip] - 1];
                local_ip += sizeof (char);
            }
            if ((proc->code[ip - 1] & CONST_BIT) != 0)
            {
                llarg += *(long long*)(proc->code + ip + local_ip);
                local_ip += SIZEOF_ARG;
            }

            Stack_Push (proc->stk, (char*)proc->RAM + FREE_MEM_ST + llarg * SIZEOF_ARG);
            ip += local_ip;
        }
        else
        {
            if ((proc->code[ip - 1] & REG_BIT) != 0)
            {
                darg += proc->reg[proc->code[ip] - 1];
                local_ip += sizeof (char);
            }
            if ((proc->code[ip - 1] & CONST_BIT) != 0)
            {
                darg += *(double*)(proc->code + ip + local_ip);
                local_ip += SIZEOF_ARG;
            }

            Stack_Push_d (proc->stk, darg);
            ip += local_ip;
        }
    })

DEF_CMD(POP,  2, 1,
    {
        long long llarg    = 0;
        int       local_ip = 0;

        if ((proc->code[ip - 1] & RAM_BIT) != 0)
        {
            Delay (DELAY_TM);
            if ((proc->code[ip - 1] & REG_BIT) != 0)
            {
                llarg    += (long long)proc->reg[proc->code[ip] - 1];
                local_ip += sizeof (char);
            }
            if ((proc->code[ip - 1] & CONST_BIT) != 0)
            {
                llarg    += *(long long*)(proc->code + ip + local_ip);
                local_ip += SIZEOF_ARG;
            }

            Stack_Pop (proc->stk, (char*)proc->RAM + FREE_MEM_ST + llarg * SIZEOF_ARG);
            ip += local_ip;
        }
        else if ((proc->code[ip - 1] & REG_BIT) != 0)
        {
            proc->reg[proc->code[ip] - 1] = Stack_Pop_d (proc->stk);
            local_ip += sizeof (char);
            ip += local_ip;
        }
        else
        {
            Stack_Pop (proc->stk, NULL);
        }
    })

#define DEF_CMD_JUMP(name, num, cmp_op)                                      \
DEF_CMD(name, num, 1,                                                        \
    {                                                                        \
        if (Stack_Pop_d (proc->stk) cmp_op Stack_Pop_d(proc->stk))           \
        {                                                                    \
            ip = *(long long*)(proc->code + ip);                             \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            ip += sizeof (long long);                                        \
        }                                                                    \
    })                                                                       \

DEF_CMD(JMP, 3, 1,
    {
        ip = *(long long*)(proc->code + ip);
    })

DEF_CMD_JUMP(JAE, 4, >=)

DEF_CMD_JUMP(JBE, 5, <=)

DEF_CMD (JNE, 6, 1,
    {
        if (fabs (Stack_Pop_d (proc->stk) - Stack_Pop_d (proc->stk)) >= EPSILON)
        {
            ip = *(long long*)(proc->code + ip);
        }
        else
        {
            ip += sizeof (long long);
        }
    })

DEF_CMD_JUMP(JA,  7, >)

DEF_CMD_JUMP(JB,  8, <)

DEF_CMD(JE, 9, 1,
    {
        if (fabs (Stack_Pop_d (proc->stk) - Stack_Pop_d (proc->stk)) < EPSILON)
        {
            ip = *(long long*)(proc->code + ip);
        }
        else
        {
            ip += sizeof (long long);
        }
    })
DEF_CMD(CALL, 10, 1,
    {
        long long ip_ret = ip + sizeof (long long);
        Stack_Push (proc->stk, &ip_ret);
        ip = *(long long*)(proc->code + ip);
    })



DEF_CMD(IN, 16, 0,
    {
        double input = 0;
        while (scanf ("%lf", &input) == 0 || getchar () != '\n')
        {
            printf ("Incorrect input, try again\n");
            Clear_buffer ();
        }
        Stack_Push_d (proc->stk, input);
    })

DEF_CMD(OUT,  17, 0,
    {
        printf ("%lf\n", Stack_Pop_d (proc->stk));
    })

DEF_CMD(ADD,  18, 0,
    {
        Stack_Push_d (proc->stk, Stack_Pop_d (proc->stk) + Stack_Pop_d (proc->stk));
    })

DEF_CMD(SUB,  19, 0,
    {
        Stack_Push_d (proc->stk, Stack_Pop_d (proc->stk) - Stack_Pop_d (proc->stk));
    })

DEF_CMD(MUL,  20, 0,
    {
        Stack_Push_d (proc->stk, Stack_Pop_d (proc->stk) * Stack_Pop_d (proc->stk));
    })

DEF_CMD(DIV,  21, 0,
    {
        double denominator = Stack_Pop_d (proc->stk);
        double numerator   = Stack_Pop_d (proc->stk);
        Stack_Push_d (proc->stk, numerator / denominator);
    })

DEF_CMD(SQRT, 22, 0,
    {
        double value = Stack_Pop_d (proc->stk);
        if (value < 0)
        {
            fprintf (logfile, "Negative number as argument of sqrt\n");
            fclose (logfile);
            return ERR;
        }
        else
        {
            Stack_Push_d (proc->stk, sqrt(value));
        }
    })

DEF_CMD(RET, 23, 0,
    {
        ip = Stack_Pop_ll (proc->stk);
    })
DEF_CMD(DUMP, 24, 0, 
    {
        Stack_Dump (proc->stk);
    })
DEF_CMD(FABS, 25, 0,
    {
        Stack_Push_d (proc->stk, fabs(Stack_Pop_d (proc->stk)));
    })
DEF_CMD(ISFIN, 26, 0,
    {
        if (isfinite (Stack_Pop_d (proc->stk)) == 0)
        {
            Stack_Push_d (proc->stk, 0);
        }
        else
        {
            Stack_Push_d (proc->stk, 1);
        }
    })

DEF_CMD(PIVO, 27, 0,
    {
        printf ("Go drink pivo!\n");
    })

DEF_CMD(PIC, 28, 0,
    {
        for (int i = 0; i < proc->VID->high; i++)
        {
            for (int j = 0; j < proc->VID->width; j++)
            {
                if (*(proc->VID->memory + i * proc->VID->width + j) == SPACE)
                {
                    putchar (' ');
                }
                else
                {
                    putchar ('+');
                }
            }
            putchar ('\n');
        }
    })

DEF_CMD(DRAW, 29, 0,
    {
        double y = Stack_Pop_d (proc->stk);
        double x = Stack_Pop_d (proc->stk);

        if (x > proc->VID->width)
        {
            fprintf (logfile, "x bigger than window width, x = %lf width = %d\n", x, proc->VID->width);
            fclose (logfile);
            return ERR;
        }
        else if (y > proc->VID->high)
        {
            fprintf (logfile, "y bigger then window high, y = %lf high = %d\n", y, proc->VID->high);
            fclose (logfile);
            return ERR;
        }
        else if (x < 0)
        {
            fprintf (logfile, "x less than null, x = %lf\n", x);
            fclose (logfile);
            return ERR;
        }
        else if (y < 0)
        {
            fprintf (logfile, "y less than null, y = %lf\n", y);
            fclose (logfile);
            return ERR;
        }
        else
        {
            *(proc->VID->memory + (proc->VID->high - (unsigned char) y) * proc->VID->width + (unsigned char) x) = PLUS;
        }
    })

DEF_CMD(SET, 30, 0,
    {
        double high  = Stack_Pop_d (proc->stk);
        double width = Stack_Pop_d (proc->stk);

        if (width > MAX_WIN_WIDTH || high > MAX_WIN_HIGH)
        {
            fprintf (logfile, "Too big window, max size window is 255x255\n:\nwidth = %lf\nhigh = %lf\n", width, high);
            fclose (logfile);
            return ERR;
        }
        else if (width < 0 || high < 0)
        {
            fprintf (logfile, "One of window parametr less null:\nwidth = %lf\nhigh = %lf\n", width, high);
            fclose (logfile);
            return ERR;
        }
        else
        {
            proc->VID->width = (unsigned char) width;
            proc->VID->high  = (unsigned char) high;

        }
    })

DEF_CMD(HLT,  31, 0,
    {
        fclose (logfile);
        return NO_ERR;
    })


