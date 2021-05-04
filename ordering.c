#include "lval.h"
#include "lenv.h"
#include "expressions.h"
#include <string.h>
#include "ordering.h"
#include <stdio.h>
#include "common.h"

lval* builtin_ordering_op(__attribute__((unused)) lenv* e, lval*a, char* op)
{
	
	// ensure every argument is a number
	for (int i = 0; i < a->count; i++)
	{
		if (a->cell[i]->type != LVAL_NUM)
		{
			lval_del(a);
			return lval_err("Cannot operate on non-number!");
		}
	}

	lval* x = lval_pop(a, 0);
	lval* ret = lval_bool(TRUE);

	while(a->count > 0)
	{
		lval* y = lval_pop(a, 0);

		if (strcmp(op, "<") == 0) { 
			ret->bool_state = (x->num < y->num) ? TRUE : FALSE;
		} else if (strcmp(op, ">") == 0) { 
			ret->bool_state = (x->num > y->num) ? TRUE : FALSE;
		} else if (strcmp(op, "<=") == 0) { 
			ret->bool_state = (x->num <= y->num) ? TRUE : FALSE;
		} else if (strcmp(op, ">=") == 0) { 
			ret->bool_state = (x->num >= y->num) ? TRUE : FALSE;
		} else if (strcmp(op, "==") == 0) { 
			ret->bool_state = lval_eq(x, y);
		}

		if ( ret->bool_state == FALSE) {
			lval_del(a);
			lval_del(y);
			return lval_bool(FALSE);
		}
		lval_del(x);
		x = y;
	}
	lval_del(a);
	return lval_bool(TRUE);
}

lval* builtin_lt(lenv* e, lval* a)
{
	return builtin_ordering_op(e, a, "<");
}
lval* builtin_gt(lenv* e, lval* a)
{
	return builtin_ordering_op(e, a, ">");
}
lval* builtin_lte(lenv* e, lval* a)
{
	return builtin_ordering_op(e, a, "<=");
}
lval* builtin_gte(lenv* e, lval* a)
{
	return builtin_ordering_op(e, a, ">=");
}
lval* builtin_eq(lenv* e, lval* a)
{
	return builtin_ordering_op(e, a, "==");
}
lval* builtin_neq(lenv* e, lval* a)
{
	LASSERT_NUM("!=", a, 2);
	lval* eq = builtin_eq(e, a);
	eq->bool_state = eq->bool_state? FALSE: TRUE;
	return eq;
}

