#ifndef SEMANTICS_H
#define SEMANTICS_H

lval* lval_read(mpc_ast_t* t);
lval* lval_read_num(mpc_ast_t* t);
int number_of_nodes(mpc_ast_t* t);

#endif
