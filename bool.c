#include "lval.h"
#include "lenv.h"
#include "expressions.h"
#include "bool.h"

lval* builtin_and(__attribute__((unused)) lenv* e, lval* a)
{
	lval* x = lval_pop(a, 0);

	if (x->bool_state == FALSE)
	{
		lval_del(x);
		lval_del(a);
		return lval_bool(FALSE);
	}

	while (a->count > 0)
	{
		lval* y = lval_pop(a, 0);
		if (y->bool_state == FALSE)
		{
			lval_del(x);
			lval_del(y);
			lval_del(a);
			return lval_bool(FALSE);
		}
		lval_del(y);
	}
	lval_del(x);
	lval_del(a);

	return lval_bool(TRUE);
}

lval* builtin_or(__attribute__((unused)) lenv* e, lval* a)
{
	lval* x = lval_pop(a, 0);

	if (x->bool_state == TRUE)
	{
		lval_del(x);
		lval_del(a);
		return lval_bool(TRUE);
	}

	while (a->count > 0)
	{
		lval* y = lval_pop(a, 0);
		if (y->bool_state == TRUE)
		{
			lval_del(x);
			lval_del(y);
			lval_del(a);
			return lval_bool(TRUE);
		}
		lval_del(y);
	}
	lval_del(x);
	lval_del(a);

	return lval_bool(FALSE);
}

lval* builtin_not(__attribute__((unused)) lenv* e, lval* a)
{
	lval* x = lval_pop(a, 0);
	if (a->count == 0)
	{
		if (x->bool_state == FALSE)
		{
			lval_del(x);
			lval_del(a);
			return lval_bool(TRUE);
		}
		lval_del(x);
		lval_del(a);
		return lval_bool(FALSE);
	}
	return lval_err("not expected a single bool argument, got many.");
}

