#include "lval.h"
#include "lenv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


lenv* lenv_new(void)
{
	lenv* env = malloc(sizeof(lenv));
	env->par = NULL;
	env->count = 0;
	env->syms = NULL;
	env->vals = NULL;
	return env;
}


void lenv_del(lenv* e)
{
	for (int i = 0; i < e->count; i++)
	{
		free(e->syms[i]);
		lval_del(e->vals[i]);
	}
	free(e->syms);
	free(e->vals);
	free(e);
}


lenv* lenv_copy(lenv* e)
{
	lenv* n = malloc(sizeof(lenv));
	n->par = e->par;
	n->count = e->count;
	n->syms = malloc(sizeof(char*) * n->count);
	n->vals = malloc(sizeof(lval*) * n->count);

	for (int i = 0; i < e->count; i++)
	{
		n->syms[i] = malloc(strlen(e->syms[i]) + 1);
		strcpy(n->syms[i], e->syms[i]);
		n->vals[i] = lval_copy(e->vals[i]);
	}
	return n;
}


void lenv_def(lenv* e, lval* k, lval* v)
{
	while (e->par) { e = e->par; }
	lenv_put(e, k, v);
}


lval* lenv_get(lenv* e, lval* k)
{
	for (int i = 0; i < e->count; i++)
	{
		if (strcmp(e->syms[i], k->sym) == 0)
		{
			return lval_copy(e->vals[i]);
		}
	}
	
	if (e->par)
	{
		return lenv_get(e->par, k);
	} else {
		return lval_err("Unbound symbol '%s'", k->sym);
	}
}


void lenv_put(lenv* e, lval* k, lval* v)
{
	for (int i = 0; i < e->count; i++)
	{
		if (strcmp(e->syms[i], k->sym) == 0)
		{
			lval_del(e->vals[i]);
			e->vals[i] = lval_copy(v);
			return;
		}
	}

	e->count++;
	e->vals = realloc(e->vals, sizeof(lval*) * e->count);
	e->syms = realloc(e->syms, sizeof(char*) * e->count);

	e->vals[e->count-1] = lval_copy(v);
	e->syms[e->count-1] = malloc(strlen(k->sym)+1);
	strcpy(e->syms[e->count-1], k->sym);
}

