#include "mpc.h"

#include "lenv.h"
#include "lval.h"
#include "expressions.h"
#include "arithmetics.h"
#include "semantics.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "common.h"
#include "builtins.h"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char** argv) 
{

	// construct the parsers for the atoms and get a ref
	// to the main language parser
	parser_elements* _parser_elements = get_parser();
	mpc_parser_t* Lisp = _parser_elements->Lisp;

	/* Debug info */
	puts("Lisp 0.1");
	puts("Press Ctrl+C to exit\n");

	// create a new environment for global lvals
	// and add the builtins into it
	lenv* env = lenv_new();
	lenv_add_builtins(env);


	/* REPL */
	while (1)
	{

		char* input = readline("lisp> ");

		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lisp, &r))
		{
			// Parse successful
			lval* x = lval_read(r.output);
			lval* result = lval_eval(env, x);
			lval_println(result);
			lval_del(result);

			mpc_ast_delete(r.output);

		} else {
			// Parse unsuccesful, print the error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);

	}

	lenv_del(env);
	
	free_parsers(_parser_elements);

	return 0;

}

