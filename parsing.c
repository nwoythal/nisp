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

//Lisp error types
enum { LERR_DIVIDE_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

//Possible Lisp types
enum { LVAL_NUM, LVAL_ERR };

//Lisp value
typedef struct
{
    int type;
    double num;
    int err;
} lval;

//Number type creation
lval lval_num(double x)
{
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

//Error type creation
lval lval_err(int x)
{
    lval v;
    v.type=LVAL_ERR;
    v.err=x;
    return v;
}

//Lisp value print
void lval_print(lval v)
{
    //Depending on the type of the lisp value, we'll print...
    switch(v.type)
    {
        case LVAL_NUM: //... the value or...
            printf("%.3f\n", v.num);
            break;
        case LVAL_ERR://... an error message.
            if(v.err == LERR_DIVIDE_BY_ZERO)
            {
                printf("Divide by zero error!\n");
            }
            if(v.err == LERR_BAD_OP)
            {
                printf("Invalid operation!\n");
            }
            if(v.err == LERR_BAD_NUM)
            {
                printf("Number not recognized!\n");
            }
            break;
    }
}

lval eval_op(lval x, char* op, lval y)
{
    //Error Checking
    if( x.type==LVAL_ERR ) { return x; }
    if( y.type==LVAL_ERR ) { return y; }
    
    //Operation carryout
    if(strcmp(op, "+")==0 || strcmp(op, "add")==0) {return lval_num(x.num + y.num);}
    if(strcmp(op, "-")==0 || strcmp(op, "sub")==0) {return lval_num(x.num - y.num);}
    if(strcmp(op, "*")==0 || strcmp(op, "mul")==0) {return lval_num(x.num * y.num);}
    if(strcmp(op, "/")==0 || strcmp(op, "div")==0) 
    {
        return y.num == 0 //check for div by zero
            ? lval_err(LERR_DIVIDE_BY_ZERO)
            : lval_num(x.num / y.num);
    }
    if(strcmp(op, "%")==0 || strcmp(op, "mod")==0) 
    {
        return y.num == 0 //check for div by zero
            ? lval_err(LERR_DIVIDE_BY_ZERO)
            : lval_num( (int) x.num % (int) y.num );
    }
    if(strcmp(op, "^")==0 || strcmp(op, "pow")==0) {return lval_num(pow(x.num,y.num));}
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t)
{
    //If it is tagged as a number, return the number or an error
    if(strstr(t->tag, "number"))
    {
        errno = 0;
        double x=strtod(t->contents, NULL);
        return errno != ERANGE
            ? lval_num(x)
            : lval_err(LERR_BAD_NUM);
    }
    
    //The operator is the first value, and we recursively call eval on the second child
    char* op = t->children[1]->contents;
    lval x=eval(t->children[2]);

    //check if the third child is an expression, if it is, we recursively call evalon it, and perform the operation on the result
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
        number   : /-?[0-9]+/ | /-?[0-9]*\\.[0-9]+/;         \
        operator : '+' | '-' | '*' | '/' | '%' | '^' |\ 
                   /add/ | /sub/ | /mul/ | /div/ | /mod/ | /pow/; \
        expr     : <number> | '(' <operator> <expr>+ ')'; \
        lispy    : /^/ <operator> <expr>+ /$/;            \
    ",
    Number, Operator, Expr, Lispy);
    /*for non polish notation: 
    /^/ <expr>+ <operator> <expr>+ /$/;\*/
    puts("Nisp alpha\nctrl+c to exit\n");
    while(TRUE)
    {
        char * input=readline("> ");
        add_history(input); //Add to history buffer

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Expr, &r) || mpc_parse("<stdin>", input, Lispy, &r)) //If the input matches the grammars provided, we will evaluate
        {
            lval result=eval(r.output);
            lval_print(result);
            mpc_ast_delete(r.output);
        }
        else //otherwise, we throw an error
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input); //de-allocate
    }
    mpc_cleanup(4,Number,Operator,Expr,Lispy);
    return 0;
}
