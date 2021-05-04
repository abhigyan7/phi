#ifndef BUILTINS_H
#define BUILTINS_H

lval* builtin_lambda(lenv*, lval*);
lval* builtin_var(lenv*, lval*, char*);
lval* builtin_def(lenv*, lval*);
void lenv_add_builtin(lenv*, char*, lbuiltin);
lval* lval_eval(lenv*, lval*);;
lval* builtin_eval(lenv*, lval*);
lval* builtin_load(lenv*, lval*);
lval* lval_call(lenv*, lval*, lval*);
lval* lval_eval(lenv*, lval*);
lval* lval_eval_sexpr(lenv*, lval*);
lval* lval_eval(lenv*, lval*);
void lenv_add_builtins(lenv*);

#endif

