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

long eval_op(long x, char* op, long y)
{
    if(strcmp(op, "+")==0 || strcmp(op, "add")==0) {return x + y;}
    if(strcmp(op, "-")==0 || strcmp(op, "sub")==0) {return x - y;}
    if(strcmp(op, "*")==0 || strcmp(op, "mul")==0) {return x * y;}
    if(strcmp(op, "/")==0 || strcmp(op, "div")==0) {return x / y;}
    if(strcmp(op, "%")==0 || strcmp(op, "mod")==0) {return x % y;}
    if(strcmp(op, "^")==0 || strcmp(op, "pow")==0) {return pow(x,y);}
    return 0;
}

long eval(mpc_ast_t* t)
{
    if(strstr(t->tag, "number"))
    {
        return atof(t->contents);
    }
    char* op = t->children[1]->contents;
    long x=eval(t->children[2]);
    int i=3;
    while(strstr(t->children[i]->tag, "expr"))
    {
        x=eval_op(x, op, eval(t->children[i]));
        i++;
    }
    return x;
}

int main(int argc, char** argv)
{
    
    //Language & Grammar
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
        number   : /-?[0-9]*\\.[0-9]+/ | /-?[0-9]+/;        \
        operator : '+' | '-' | '*' | '/' | '%' | '^' |\ 
                   /add/ | /sub/ | /mul/ | /div/ | /mod/ | /pow/; \
        expr     : <number> | '(' <operator> <expr>+')'; \
        lispy    : /^/ <operator> <expr>+ /$/;            \
    ",
    Number, Operator, Expr, Lispy);
    /*for non polish notation: 
    lispy    : /^/ <expr>+ <operator> <expr>+ /$/;\*/
    puts("Nisp pre-alpha\nctrl+c to exit\n");
    while(TRUE)
    {
        char * input=readline("> ");
        add_history(input); //Add to history buffer

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r))
        {
            long result=eval(r.output);
            printf("%li\n",result);
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
