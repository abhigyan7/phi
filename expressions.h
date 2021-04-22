#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* lval_join(lval* x, lval*y);
lval* builtin_join(lenv* e, lval* a);

#endif
