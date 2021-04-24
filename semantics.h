#ifndef SEMANTICS_H
#define SEMANTICS_H

typedef struct parser_elements
{
	mpc_parser_t* Number;
	mpc_parser_t* Symbol;
	mpc_parser_t* Sexpr;
	mpc_parser_t* Qexpr;
	mpc_parser_t* Expr;
	mpc_parser_t* Lisp;
} parser_elements;

lval* lval_read(mpc_ast_t* t);
lval* lval_read_num(mpc_ast_t* t);
int number_of_nodes(mpc_ast_t* t);

parser_elements* get_parser(void);
void free_parsers(parser_elements*);

#endif

