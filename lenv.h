#ifndef LENV_H
#define LENV_H

typedef struct lval lval;
typedef struct lenv lenv;


struct lenv
{
	lenv* par;
	int count;
	char** syms;
	lval** vals;
};

lenv* lenv_new(void);
void lenv_del(lenv*);
lenv* lenv_copy(lenv*);
lval* lenv_get(lenv*, lval*);
void lenv_def(lenv*, lval*, lval*);
void lenv_put(lenv*, lval*, lval*);

#endif