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


typedef struct
{
	int type;
	long num;
	int err;
} lval;

enum { LVAL_NUM, LVAL_ERR };

enum { LERR_DIV_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM };


lval lval_num(long x) 
{
	lval v;
	v.type = LVAL_NUM;
	v.num = x;
	return v;
}


lval lval_err(int x)
{
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	return v;
}


void lval_print(lval v)
{

	switch (v.type)
	{
		// if the value is a number, print it
		case LVAL_NUM: printf("%li", v.num); break;

		// if an error, print message relevant to the error
		case LVAL_ERR:
			if (v.err == LERR_DIV_BY_ZERO) 
			{
				printf("Error: Division Zero");
			}
			if (v.err == LERR_BAD_OP) 
			{
				printf("Error: Invalid operator");
			}
			
			if (v.err == LERR_BAD_NUM) 
			{
				printf("Error: Invalid number");
			}
	}
}


void lval_println(lval v)
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


lval eval_op(lval x, char* op, lval y)
{
	
	// if x or y is an error, propagate it upwards
	if (x.type == LVAL_ERR) { return x; }
	if (y.type == LVAL_ERR) { return y; }

	// math ops
	if (strcmp(op, "+")==0) { return lval_num(x.num + y.num); }
	if (strcmp(op, "-")==0) { return lval_num(x.num - y.num); }
	if (strcmp(op, "*")==0) { return lval_num(x.num * y.num); }
	if (strcmp(op, "/")==0) 
	{ 
		if (y.num == 0)
		{
			return lval_err(LERR_DIV_BY_ZERO);
		}
		return lval_num(x.num / y.num); 
	}
	return lval_err(LERR_BAD_OP);

}

lval eval(mpc_ast_t* t)
{

	if (strstr(t->tag, "number"))
	{
		// errno is magically set to the error code
		// raised by stdlib functions
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}

	char* op = t->children[1]->contents;

	lval x = eval(t->children[2]);
	
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
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lisp = mpc_new("lisp");

	mpca_lang(MPCA_LANG_DEFAULT,
		"							\
		number	: /-?[0-9]+/ ;					\
		operator: '+' | '-' | '*' | '/' ;			\
		expr	: <number> | '(' <operator> <expr>+ ')' ;	\
		lisp	: /^/ <operator> <expr>+ /$/ ;			\
		",
		Number, Operator, Expr, Lisp);

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

			lval result = eval(r.output);
			lval_println(result);

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

	mpc_cleanup(4, Number, Operator, Expr, Lisp);

	return 0;

}
