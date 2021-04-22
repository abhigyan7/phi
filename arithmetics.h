#ifndef ARITHMETICS_H
#define ARITHMETICS_H

lval* builtin_op(lenv*, lval*, char*);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);

#endif