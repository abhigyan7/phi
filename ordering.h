#ifndef ORDERING_H
#define ORDERING_H
lval* builtin_ordering_op(lenv* e, lval*a, char* op);
lval* builtin_lt(lenv* e, lval* a);
lval* builtin_gt(lenv* e, lval* a);
lval* builtin_lte(lenv* e, lval* a);
lval* builtin_gte(lenv* e, lval* a);
lval* builtin_eq(lenv* e, lval* a);
#endif
