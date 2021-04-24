#include "lval.h"
#include "lenv.h"
#include <stdio.h>
#include <stdlib.h>
#include "expressions.h"
#include <string.h>
#include "common.h"

lval* lval_add(lval* v, lval* x) 
{
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count-1] = x;
	return v;
}

lval* lval_pop(lval* v, int i)
{

	lval* x = v->cell[i];

	memmove(&v->cell[i], &v->cell[i+1],
		sizeof(lval*) * (v->count-i-1));

	v->count--;

	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;
}

lval* lval_take(lval* v, int i)
{
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;

}

lval* builtin_head(__attribute__((unused)) lenv* e, lval* a)
{

	LASSERT(a, a->count == 1,
		"Function 'head' passed too many arguments."
		"Got %i, Expected %i",
		a->count, 1);

	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, 
		"Function 'head' passed incorrect type for argument 0. "
		"Got %s, Expected %s.",
		ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

	LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed an empty q-expression!");
	

	lval* v = lval_take(a, 0);

	while(v->count > 1)
	{
		lval_del(lval_pop(v, 1));
	}

	return v;
}

lval* builtin_tail(__attribute__((unused)) lenv* e, lval* a)
{
	LASSERT(a, a->count == 1,
		"Function 'tail' expected 1 argument, got %i!", a->count);
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, 
		"Function 'tail' passed incorrect type for argument 0. "
		"Got %s, Expected %s.",
		ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

	LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed an empty q-expression!");

	lval* v = lval_take(a, 0);

	lval_del(lval_pop(v, 0));
	return v;

}

lval* builtin_list(__attribute__((unused)) lenv* e, lval* a)
{
	a->type = LVAL_QEXPR;
	return a;
}

lval* lval_join(lval* x, lval*y)
{
	while(y->count)
	{
		x = lval_add(x, lval_pop(y, 0));
	}

	lval_del(y);
	return x;
}

lval* builtin_join(__attribute__((unused)) lenv* e, lval* a)
{

	for (int i = 0; i < a->count; i++)
	{
		LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
		"Function 'join' passed incorrect type.");
	}

	lval* x = lval_pop(a, 0);

	while (a->count)
	{
		x = lval_join(x, lval_pop(a, 0));
	}

	lval_del(a);
	return x;
}

