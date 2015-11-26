#include <stdio.h>
#include <stdlib.h>
#include "mpc/mpc.h"

#define BUF_SIZE 2048
#define TRUE 1
#define FALSE 0

#ifdef _WIN32
#include <string.h>

static char buffer[BUF_SIZE]; //String Buffer for Windows readline

char * readline(char * prompt)
{
    fputs(prompt, stdout);
    fgets(buffer, BUF_SIZE, stdin);
    char * str_copy = malloc(strlen(buffer)+1);
    strcpy(str_copy, buffer);
    str_copy[strlen(str_copy)-1]='\0';
    return str_copy;
}

void add_history(char * unused){} //Empty add_history function since windows
                                  //supports history natively

#elif __linux__

#include <editline/readline.h>
#include <editline/readline.h>

#else

#error "OS not supported"

#endif

int main(int argc, char** argv)
{
    
    //Language & Grammar
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
        number   : /-?[0-9]+/;              \
        operator : '+' | '-' | '*' | '/' | '%' | /add/ | /sub/ | /mul/ | /div/ | /mod/; \
        expr     : <number> | '(' <operator> <expr>+ ')'; \
        lispy    : /^/ <operator> <expr>+ /$/;            \
    ",
    Number, Operator, Expr, Lispy);
    //| '/-?[0-9].[0-9]+/'
    puts("Nisp pre-alpha\nctrl+c to exit\n");
    while(TRUE)
    {
        char * input=readline("> ");
        add_history(input); //Add to history buffer
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r))
        {
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        }
        else
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input); //de-allocate
    }
    mpc_cleanup(4,Number,Operator,Expr,Lispy);
    return 0;
}
