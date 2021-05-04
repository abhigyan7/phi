#ifndef LENV_H
#define LENV_H

typedef struct lval lval;
typedef struct lenv lenv;

typedef struct mpc_parser_t mpc_parser_t;

struct lenv
{
	lenv* par;
	int count;
	char** syms;
	lval** vals;
	mpc_parser_t* phi;
};

lenv* lenv_new(void);
void lenv_del(lenv*);
lenv* lenv_copy(lenv*);
lval* lenv_get(lenv*, lval*);
void lenv_def(lenv*, lval*, lval*);
void lenv_put(lenv*, lval*, lval*);

#endif
