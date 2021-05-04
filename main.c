#include "mpc.h"
#include "lenv.h"
#include "lval.h"
#include "expressions.h"
#include "arithmetics.h"
#include "semantics.h"
#include "bool.h"

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
	mpc_parser_t* Phi = _parser_elements->Phi;

	// create a new environment for global lvals
	// and add the builtins into it
	lenv* env = lenv_new();
	lenv_add_builtins(env);

	// add the Phi parser to the env
	env->phi = Phi;

	if (argc >= 2)
	{
		for (int i = 1; i < argc; i++)
		{
			lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));

			lval* x = builtin_load(env, args);

			if (x->type == LVAL_ERR)
			{
				lval_println(x);
			}
			lval_del(x);
		}
	} else {

		/* Debug info */
		puts("Phi 0.1");
		puts("Press Ctrl+C to exit\n");

		/* REPL */
		while (1)
		{
	
			char* input = readline("lisp> ");
	
			add_history(input);
	
			mpc_result_t r;
			if (mpc_parse("<stdin>", input, Phi, &r))
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
	}
	// Phi = NULL;
	env->phi = NULL;
	lenv_del(env);
	free_parsers(_parser_elements);

	return 0;

}

