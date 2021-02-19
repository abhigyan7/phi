#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

/* Stubbing out readline and history for windows
 * Apparently windows handles history and 
 * navigation all well by default. */

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt)
{
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy)-1] = "\0";
	return cpy;
}

void add_history(char* unused) {}


#else
#include <readline/history.h>
#include <readline/readline.h>
#endif


typedef struct lval
{
	int type;
	long num;

	char* err;
	char* sym;

	// number of child lvals
	int count;
	// Pointer to a list of lvals
	// If a struct contains a ref to itself
	// append a struct before the declaration
	// to avoid circular dependency
	// Also, structs can only contain pointers
	// to their own type, not instances of their
	// own type
	struct lval** cell;
} lval;

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

enum { LERR_DIV_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval* lval_num(long x) 
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}


lval* lval_err(char* m)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}


lval* lval_sym(char* s)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s)+1);
	strcpy(v->sym, s);
	return v;
}


lval* lval_sexpr(void)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}


void lval_del(lval* v)
{
	switch(v->type)
	{

	
		case LVAL_NUM: break;

		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;

		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++)
			{
				lval_del(v->cell[i]);
			}

			free(v->cell);
		break;

	}
	free(v);
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close)
{
	putchar(open);
	for (int i = 0; i < v->count; i++)
	{
		lval_print(v->cell[i]);

		if (i != (v->count-1)) 
		{
			putchar(' ');
		}
	}
	putchar(close);
}


void lval_print(lval* v)
{
	
	switch (v->type)
	{
		// if the value is a number, print it
		case LVAL_NUM: printf("%li", v->num); break;

		// if an error, print message relevant to the error
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')');  break;

	}
}


void lval_println(lval* v)
{
	lval_print(v);
	putchar('\n');
}


int number_of_nodes(mpc_ast_t* t)
{
	if (t->children_num == 0)
	{
		return 1;
	}
	if (t->children_num >= 1)
	{
		int total = 1;
		for (int i = 0; i < t->children_num; i++)
		{
			total = total + number_of_nodes(t->children[i]);
		}
		return total;
	}
	return 0;
}


lval* lval_read_num(mpc_ast_t* t)
{
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ?
		lval_num(x) : lval_err("Invalid Number");
}


lval* lval_add(lval* v, lval* x) 
{
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count-1] = x;
	return v;
}


lval* lval_read(mpc_ast_t* t)
{

	if (strstr(t->tag, "number")) { return lval_read_num(t); }
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
	
	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
	if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }

	for (int i = 0; i < t->children_num; i++)
	{
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	
	printf("This is X: %d, ", x);
	printf("This is the tag: %s\n", t->tag);

	return x;

}


lval* eval_op(lval* x, char* op, lval* y)
{
	
	// if x or y is an error, propagate it upwards
	if (x->type == LVAL_ERR) { return x; }
	if (y->type == LVAL_ERR) { return y; }

	// math ops
	if (strcmp(op, "+")==0) { return lval_num(x->num + y->num); }
	if (strcmp(op, "-")==0) { return lval_num(x->num - y->num); }
	if (strcmp(op, "*")==0) { return lval_num(x->num * y->num); }
	if (strcmp(op, "/")==0) 
	{ 
		if (y->num == 0)
		{
			return lval_err("Division by zero!");
		}
		return lval_num(x->num / y->num); 
	}
	return lval_err("Bad operation!");

}

lval* eval(mpc_ast_t* t)
{

	if (strstr(t->tag, "number"))
	{
		// errno is magically set to the error code
		// raised by stdlib functions
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err("Bad Number");
	}

	char* op = t->children[1]->contents;

	lval* x = eval(t->children[2]);
	
	int i = 3;
	while (strstr(t->children[i]->tag, "expr"))
	{
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}

	return x;
}



int main(int argc, char** argv) 
{

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lisp = mpc_new("lisp");

	mpca_lang(MPCA_LANG_DEFAULT,
		"						\
		number	: /-?[0-9]+/ ;				\
		symbol : '+' | '-' | '*' | '/' ;		\
		sexpr	: '(' <expr>* ')' ;			\
		expr	: <number> | <symbol> | <sexpr> ;	\
		lisp	: /^/ <expr>* /$/ ;			\
		",
		Number, Symbol, Sexpr, Expr, Lisp);

	/* Debug info */
	puts("Lisp version 0.0.0.0.1");
	puts("Press Ctrl+C to exit\n");


	/* REPL */
	while (1)
	{

		char* input = readline("lisp> ");

		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lisp, &r))
		{
			/*
			 * Parse successful
			 */

			mpc_ast_print(r.output);

			lval* x = lval_read(r.output);
			printf("Read the ast\n");
			lval_println(x);
			//lval_del(x);
			//lval result = eval(r.output);
			//lval_println(result);

			mpc_ast_delete(r.output);
		} else {
			/*
			 * Print the error
			 */
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);

	}

	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lisp);

	return 0;

}
