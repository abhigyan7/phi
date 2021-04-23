#include "lval.h"
#include "lenv.h"
#include "expressions.h"

#include "arithmetics.h"

#include <string.h>

lval* builtin_op(__attribute((unused)) lenv* e, lval* a, char* op)
{
	for (int i = 0; i < a->count; i++)
	{
		if (a->cell[i]->type != LVAL_NUM)
		{
			lval_del(a);
			return lval_err("Cannot operate on non-number!");
		}
	}

	lval* x = lval_pop(a, 0);

	if ((strcmp(op, "-") == 0) && a->count == 0)
	{
		x->num = -x->num;
	}

	while(a->count > 0)
	{
		lval* y = lval_pop(a, 0);

		if (strcmp(op, "+") == 0) { x->num += y->num; }
		if (strcmp(op, "-") == 0) { x->num -= y->num; }
		if (strcmp(op, "*") == 0) { x->num *= y->num; }
		if (strcmp(op, "/") == 0) { 
			if (y->num == 0)
			{
				lval_del(x); 
				lval_del(y);
				x = lval_err("Division by zero!");
				break;
			}
			x->num /= y->num;
		}
	lval_del(y);
	}
	lval_del(a);
	return x;
}


lval* builtin_add(lenv* e, lval*a)
{
	return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval*a)
{
	return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval*a)
{
	return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval*a)
{
	return builtin_op(e, a, "/");
}

