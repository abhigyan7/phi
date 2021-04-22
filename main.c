#include "mpc.h"

#include "lenv.h"
#include "lval.h"
#include "expressions.h"
#include "arithmetics.h"
#include "semantics.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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

lval* builtin_lambda(lenv* e, lval* a)
{
	LASSERT_NUM("\\", a, 2);
	LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
	LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

	for (int i = 0; i < a->cell[0]->count; i++)
	{
		LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
			"Cannot define non-symbol. Got %s, expected %s.",
			ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
	}

	lval* formals = lval_pop(a, 0);
	lval* body = lval_pop(a, 0);
	lval_del(a);

	return lval_lambda(formals, body);

}


lval* builtin_var(lenv* e, lval* a, char* func)
{
	LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

	lval* syms = a->cell[0];
	for (int i = 0; i < syms->count; i++)
	{
		LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
			"Function %s cannot define non-symbol. "
			"Got %s, expected %s.", func,
			ltype_name(syms->cell[i]->type),
			ltype_name(LVAL_SYM));

		LASSERT(a, (syms->count == a->count-1),
			"Function %s passed too many arguments for symbols. "
			"Got %i, expected %i", func, syms->count, a->count-1);
		
		for (int i = 0; i < syms->count; i++)
		{
			if (strcmp(func, "def") == 0)
			{
				lenv_def(e, syms->cell[i], a->cell[i+1]);
			}

			if (strcmp(func, "=") == 0)
			{
				lenv_put(e, syms->cell[i], a->cell[i+1]);
			}
			
		}
	}
	lval_del(a);
	return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* a)
{
	return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a)
{
	return builtin_var(e, a, "=");
}


void lenv_add_builtin(lenv* env, char* name, lbuiltin func)
{
	lval* k = lval_sym(name);
	lval* v = lval_fun(func);
	lenv_put(env, k, v);
	lval_del(k); lval_del(v);
}


lval* lval_eval(lenv* env, lval* x);

lval* builtin_eval(lenv* e, lval* a)
{
	LASSERT(a, a->count == 1,
		"Function 'eval' expected 1 argument, got %i!", a->count);
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, 
		"Function 'eval' passed incorrect type for argument 0. "
		"Got %s, Expected %s.",
		ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));


	LASSERT(a, a->count==1, "Function 'eval' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type!");

	lval* x = lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(e, x);
}


void lenv_add_builtins(lenv* e)
{
	lenv_add_builtin(e, "list", builtin_list);
	lenv_add_builtin(e, "head", builtin_head);
	lenv_add_builtin(e, "tail", builtin_tail);
	lenv_add_builtin(e, "join", builtin_join);
	lenv_add_builtin(e, "eval", builtin_eval);

	lenv_add_builtin(e, "+", builtin_add);
	lenv_add_builtin(e, "-", builtin_sub);
	lenv_add_builtin(e, "*", builtin_mul);
	lenv_add_builtin(e, "/", builtin_div);
	lenv_add_builtin(e, "\\", builtin_lambda);
	lenv_add_builtin(e, "def", builtin_def);
	lenv_add_builtin(e, "=", builtin_put);

}


lval* lval_call(lenv* e, lval* f, lval* a)
{
	if (f->builtin)
	{ return f->builtin(e, a);}

	int given = a->count;
	int total = f->formals->count;

	while (a->count)
	{
		if (f->formals->count == 0)
		{
			lval_del(a);
			return lval_err(
				"Function passed too many arguemnts. "
				"Got %i, expected %i.", given, total);
		}

		lval* sym = lval_pop(f->formals, 0);

		if (strcmp(sym->sym, "&") == 0)
		{
			if (f->formals->count != 1)
			{
				lval_del(a);
				return lval_err("Function format invalid. "
					"Symbol '&' not followed by single symbol.");
			}

			lval* nsym = lval_pop(f->formals, 0);
			lenv_put(f->env, nsym, builtin_list(e, a));
			lval_del(sym); lval_del(nsym);
			break;
		}

		lval* val = lval_pop(a, 0);

		lenv_put(f->env, sym, val);

		lval_del(sym); lval_del(val);
	}

	lval_del(a);

	if (f->formals->count > 0 && strcmp(f->formals->cell[0]->sym, "&") == 0)
	{
		if (f->formals->count != 2)
		{
			return lval_err("Function format invalid. "
				"Symbol '&' not followed by single symbol.");
		}

		lval_del(lval_pop(f->formals, 0));

		lval* sym = lval_pop(f->formals, 0);
		lval* val = lval_qexpr();

		lenv_put(f->env, sym, val);
		lval_del(sym); lval_del(val);
	}

	if (f->formals->count == 0)
	{
		f->env->par = e;

		return builtin_eval(
			f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
	} else {
		return lval_copy(f);
	}
}

lval* lval_eval(lenv*, lval*);

lval* lval_eval_sexpr(lenv* env, lval* v)
{

	for (int i = 0; i < v->count; i++)
	{
		v->cell[i] = lval_eval(env, v->cell[i]);
	}

	for (int i = 0; i < v->count; i++)
	{
		if (v->cell[i]->type == LVAL_ERR)
		{
			return lval_take(v, i);
		}
	}

	if (v->count == 0)
	{
		return v;
	}

	if (v->count == 1)
	{
		return lval_take(v, 0);
	}

	lval* f = lval_pop(v, 0);
	if (f->type != LVAL_FUN)
	{
		lval* err = lval_err(
			"S-expression starts with incorrect type. "
			"Got %s, expected %s.",
			ltype_name(f->type), ltype_name(LVAL_FUN)
		);
		lval_del(f);
		lval_del(v);
		return err;
	}

	lval* result = lval_call(env, f, v);
	lval_del(f);
	return result;
}


lval* lval_eval(lenv* env, lval* v)
{
	if (v->type == LVAL_SYM)
	{
		lval* x = lenv_get(env, v);
		lval_del(v);
		return x;
	}
	if (v->type == LVAL_SEXPR)
	{
		return lval_eval_sexpr(env, v);
	}
	return v;
}


int main(int argc, char** argv) 
{

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lisp = mpc_new("lisp");

	mpca_lang(MPCA_LANG_DEFAULT,
		"						\
		number	: /-?[0-9]+/ ;				\
		symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;	\
		sexpr	: '(' <expr>* ')' ;			\
		qexpr	: '{' <expr>* '}' ;			\
		expr	: <number> | <symbol> | <sexpr> | <qexpr> ;	\
		lisp	: /^/ <expr>* /$/ ;			\
		",
		Number, Symbol, Sexpr, Qexpr, Expr, Lisp);

	/* Debug info */
	puts("Lisp version 0.0.0.0.1");
	puts("Press Ctrl+C to exit\n");

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
			/*
			 * Parse successful
			 */

			lval* x = lval_read(r.output);
			lval* result = lval_eval(env, x);
			lval_println(result);
			lval_del(result);

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

	lenv_del(env);

	mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lisp);

	return 0;

}

