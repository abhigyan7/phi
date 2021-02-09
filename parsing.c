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



long eval_op(long x, char* op, long y)
{

	if (strcmp(op, "+")==0) { return x + y; }
	if (strcmp(op, "-")==0) { return x - y; }
	if (strcmp(op, "*")==0) { return x * y; }
	if (strcmp(op, "/")==0) { return x / y; }
	return 0;
}

long eval(mpc_ast_t* t)
{

	if (strstr(t->tag, "number"))
	{
		return atoi(t->contents);
	}

	char* op = t->children[1]->contents;

	long x = eval(t->children[2]);
	
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

			printf("%ld\n", eval(a));

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
