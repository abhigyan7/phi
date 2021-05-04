#include <stdio.h>
#include "lval.h"
#include "lenv.h"
#include "expressions.h"
#include "arithmetics.h"
#include "common.h"
#include "bool.h"
#include "ordering.h"

#include "builtins.h"

lval* builtin_lambda(__attribute__((unused)) lenv* e, lval* a)
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

lval* builtin_if(__attribute__((unused)) lenv* e, lval* a)
{

	lval* condition = lval_pop(a, 0);
	lval* true_codepath = lval_pop(a, 0);

	lval* false_codepath;
	if (a->count == 1)
	{
		// the codepath to run when false 
		false_codepath = lval_pop(a, 0);
	} else if (a->count == 0) {
		// empty codepath to run if no false_codepath given
		false_codepath = lval_qexpr();
	} else {
		lval_del(condition);
		lval_del(true_codepath);
		return lval_err("if expected two qexprs, got more");
	}

	lval_del(a);

	if (condition->type == LVAL_QEXPR)
	{
		condition->type = LVAL_SEXPR;
		lval* condition_evaled = lval_eval(e, condition);
		if (!(condition_evaled->type == LVAL_BOOL))
		{
			lval_del(condition_evaled);
			return lval_err("Expected if condition to evaluate to a bool, it didn't");
		}
		condition = condition_evaled;
	}

	if (condition->bool_state == TRUE)
	{
		lval_del(condition);
		if (true_codepath->type == LVAL_QEXPR) true_codepath->type = LVAL_SEXPR;
		return lval_eval(e, true_codepath);
	}

	lval_del(condition);
	if (true_codepath->type == LVAL_QEXPR) false_codepath->type = LVAL_SEXPR;
	return lval_eval(e, false_codepath);
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

	// variable arguments similar to *args in python
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
		// all formals have been substituted, eval the function and return
		f->env->par = e;

		return builtin_eval(
			f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
	} else {
		// not all formals have been substituted, return a partial
		return lval_copy(f);
	}

}

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

void lenv_add_builtin_fun(lenv* env, char* name, lbuiltin func)
{
	lval* k = lval_sym(name);
	lval* v = lval_fun(func);
	lenv_put(env, k, v);
	lval_del(k); lval_del(v);
}

void lenv_add_builtin_val(lenv* env, char* name, lval* v)
{
	lval* k = lval_sym(name);
	lenv_put(env, k, v);
	lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e)
{
	lenv_add_builtin_fun(e, "list", builtin_list);
	lenv_add_builtin_fun(e, "head", builtin_head);
	lenv_add_builtin_fun(e, "tail", builtin_tail);
	lenv_add_builtin_fun(e, "join", builtin_join);
	lenv_add_builtin_fun(e, "eval", builtin_eval);

	lenv_add_builtin_fun(e, "+", builtin_add);
	lenv_add_builtin_fun(e, "-", builtin_sub);
	lenv_add_builtin_fun(e, "*", builtin_mul);
	lenv_add_builtin_fun(e, "/", builtin_div);
	lenv_add_builtin_fun(e, "\\", builtin_lambda);
	lenv_add_builtin_fun(e, "def", builtin_def);
	lenv_add_builtin_fun(e, "=", builtin_put);
	
	lenv_add_builtin_val(e, "true", lval_bool(TRUE));
	lenv_add_builtin_val(e, "false", lval_bool(FALSE));

	lenv_add_builtin_fun(e, "and", builtin_and);
	lenv_add_builtin_fun(e, "or", builtin_or);
	lenv_add_builtin_fun(e, "not", builtin_not);

	lenv_add_builtin_fun(e, "<", builtin_lt);
	lenv_add_builtin_fun(e, ">", builtin_gt);
	lenv_add_builtin_fun(e, "<=", builtin_lte);
	lenv_add_builtin_fun(e, ">=", builtin_gte);
	lenv_add_builtin_fun(e, "==", builtin_eq);
	lenv_add_builtin_fun(e, "!=", builtin_neq);

	lenv_add_builtin_fun(e, "if", builtin_if);
}

