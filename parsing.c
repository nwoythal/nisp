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

//Possible Lisp types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

//Lisp value
typedef struct lval
{
    int type;
    double num;
    char* err;
    char* sym;
    int count;
    struct lval** cell;
} lval;

//Prototypes
void lval_print(lval* v);
lval* lval_eval(lval* v);

//Number type creation
lval* lval_num(double x)
{
    lval* v=malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

//Error type creation
lval* lval_err(char* m)
{
    lval* v=malloc(sizeof(lval));
    v->type=LVAL_ERR;
    v->err=malloc(strlen(m)+1);
    strcpy(v->err, m);
    return v;
}

//Symbol type creation
lval* lval_sym(char* s)
{
    lval* v=malloc(sizeof(lval));
    v->type=LVAL_SYM;
    v->sym=malloc(strlen(s)+1);
    strcpy(v->sym,s);
    return v;
}

//S-Expression creation
lval* lval_sexpr(void)
{
    lval* v=malloc(sizeof(lval));
    v->type=LVAL_SEXPR;
    v->count=0;
    v->cell=NULL;
    return v;
}

//delete lval
void lval_del(lval* v)
{
    switch(v->type)
    {
        case LVAL_NUM:
            break;
        case LVAL_ERR:
            free(v->err); //free allocated string
            break;
        case LVAL_SYM:
            free(v->sym); //free allocated string
            break;
        //recurse over expression to free allocated memory
        case LVAL_SEXPR:
            for(int i=0; i<v->count; i++)
            {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }
    free(v);
}

//Add element to list
lval* lval_add(lval* v, lval* x)
{
    v->count++;
    v->cell=realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1]=x;
    return v;
}

//Pop lval at location from list
lval* lval_pop(lval* v, int i)
{
    //Grab item at index i
    lval* x=v->cell[i];
    
    //Move list head, change item count, & reallocate
    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count-i-1));
    v->count--;
    v->cell=realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_take(lval* v, int i)
{
    lval* x=lval_pop(v,i);
    lval_del(v);
    return x;
}

void lval_expr_print(lval* v, char open, char close)
{
    putchar(open);
    for(int i=0; i<v->count; i++)
    {
        lval_print(v->cell[i]);
        if(i!=(v->count-1))
        {
            putchar(' ');
        }
    }
    putchar(close);
}

//Lisp value print
void lval_print(lval* v)
{
    switch(v->type)
    {
        case LVAL_NUM:
            printf("%.3f",v->num);
            break;
        case LVAL_ERR:
            printf("Error: %s",v->err);
            break;
        case LVAL_SYM:
            printf("%s",v->sym);
            break;
        case LVAL_SEXPR:
            lval_expr_print(v, '(', ')');
            break;
    }
}

void lval_println(lval* v)
{
    lval_print(v);
    putchar('\n');
}

lval* builtin_op(lval* a, char* op)
{
    //Input validation
    for(int i=0;i<a->count;i++)
    {
        if(a->cell[i]->type != LVAL_NUM)
        {
            lval_del(a);
            return lval_err("Input is non-numerical!");
        }
    }
    
    lval* x=lval_pop(a,0);

    if((strcmp(op,"-")==0 && a->count==0))
    {
        x->num = -x->num;
    }
    
    while(a->count > 0)
    {
        //get second value
        lval* y=lval_pop(a,0);

        //Operation carryout
        if(strcmp(op, "+")==0 || strcmp(op, "add")==0) {x->num += y->num;}
        if(strcmp(op, "-")==0 || strcmp(op, "sub")==0) {x->num -= y->num;}
        if(strcmp(op, "*")==0 || strcmp(op, "mul")==0) {x->num *= y->num;}
        if(strcmp(op, "^")==0 || strcmp(op, "pow")==0) {x->num=pow(x->num,y->num);}
        if(strcmp(op, "/")==0 || strcmp(op, "div")==0) 
        {
            if(y->num==0)
            {
                lval_del(x);
                lval_del(y);
                x=lval_err("Divide by zero error!");
                break;
            }
            x->num/=y->num;
        }
        if(strcmp(op, "%")==0 || strcmp(op, "mod")==0) 
        { 
            if(y->num==0)
            {
                lval_del(x);
                lval_del(y);
                x=lval_err("Divide by zero error!");
                break;
            }
            x->num=(double) ((int) x->num % (int) y->num);
        }
        lval_del(y);
    }
    lval_del(a);
    return x;
}

lval* lval_eval_sexpr(lval* v)
{
    //Recurse down S-Expression
    for(int i=0;i<v->count;i++)
    {
        v->cell[i]=lval_eval(v->cell[i]);
    }

    for(int i=0;i<v->count;i++)
    {
        if(v->cell[i]->type==LVAL_ERR)
        {
            return lval_take(v,i);
        }
    }

    //Empty SExpr
    if(v->count==0)
    {
        return v;
    }

    //Single SExpr
    if(v->count==1)
    {
        return lval_take(v,0);
    }

    lval* f=lval_pop(v,0);
    if(f->type != LVAL_SYM)
    {
        lval_del(f);
        lval_del(v);
        return lval_err("S-Expression has incorrect syntax.");
    }

    lval* result=builtin_op(v, f->sym);
    lval_del(f);
    return result;
}

lval* lval_eval(lval* v)
{
    if(v->type==LVAL_SEXPR)
    {
        return lval_eval_sexpr(v);
    }
    return v;
}

lval* lval_read_num(mpc_ast_t* t)
{
    errno=0;
    double x=strtod(t->contents, NULL);
    return errno!=ERANGE
        ? lval_num(x)
        : lval_err("Invalid number.");
}

lval* lval_read(mpc_ast_t* t)
{
    if(strstr(t->tag, "number"))
    {
        return lval_read_num(t);
    }
    if(strstr(t->tag, "symbol"))
    {
        return lval_sym(t->contents);
    }
    
    lval* x=NULL;
    if(strcmp(t->tag, ">")==0)
    {
        x=lval_sexpr();
    }
    if(strstr(t->tag, "sexpr"))
    {
        x=lval_sexpr();
    }
    for(int i=0; i<t->children_num; i++)
    {
        if(strcmp(t->children[i]->contents, "(")==0) {continue;}
        if(strcmp(t->children[i]->contents, ")")==0) {continue;}
        if(strcmp(t->children[i]->contents, "{")==0) {continue;}
        if(strcmp(t->children[i]->contents, "}")==0) {continue;}
        if(strcmp(t->children[i]->contents, "[")==0) {continue;}
        if(strcmp(t->children[i]->contents, "]")==0) {continue;}
        if(strcmp(t->children[i]->tag,  "regex")==0) {continue;}
        x=lval_add(x, lval_read(t->children[i]));
    }
    return x;
}

int main(int argc, char** argv)
{ 
    //Language & Grammar
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                            \
        number : /-?[0-9]*\\.[0-9]+/ | /-?[0-9]+/;               \
        symbol : '+' | '-' | '*' | '/' | '%' | '^' |             \ 
                   /add/ | /sub/ | /mul/ | /div/ | /mod/ | /pow/;\
        sexpr  : '(' <expr>* ')';                                \
        expr   : <number> | <symbol> | <sexpr>;                  \
        lispy  : /^/ <expr>* /$/;                                \
    ",
    Number, Symbol, Sexpr, Expr, Lispy);
    puts("Nisp alpha\nctrl+c to exit\n");
    while(TRUE)
    {
        char * input=readline(">");
        add_history(input); //Add to history buffer

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) //If the input matches the grammars provided, we will evaluate
        {
            lval* x=lval_eval(lval_read(r.output));
            lval_println(x);
            lval_del(x);
            mpc_ast_delete(r.output);
        }
        else //otherwise, we throw an error
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input); //de-allocate
    }
    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
    return 0;
}
