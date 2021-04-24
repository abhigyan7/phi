#include "lval.h"
#include "lenv.h"
#include "mpc.h"
#include "semantics.h"
#include "expressions.h"

#include <stdio.h>
#include <stdlib.h>

lval* lval_read(mpc_ast_t* t)
{

	if (strstr(t->tag, "number")) { return lval_read_num(t); }
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
	
	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
	if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }
	if (strstr(t->tag, "qexpr")) { x = lval_qexpr(); }

	for (int i = 0; i < t->children_num; i++)
	{
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
		if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	
	return x;
}

lval* lval_read_num(mpc_ast_t* t)
{
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ?
		lval_num(x) : lval_err("Invalid Number");
}

int number_of_nodes(mpc_ast_t* t)
{
	if (t->children_num == 0)
	{
		return 1;
	}
	if (t->children_num >= 1)
	{
		int total = 1;
		for (int i = 0; i < t->children_num; i++)
		{
			total = total + number_of_nodes(t->children[i]);
		}
		return total;
	}
	return 0;
}

parser_elements* get_parser(void)
{
	parser_elements* ret = malloc(sizeof(parser_elements));

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lisp = mpc_new("lisp");

	mpca_lang(MPCA_LANG_DEFAULT,
		"						\
		number	: /-?[0-9]+/ ;				\
		symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;	\
		sexpr	: '(' <expr>* ')' ;			\
		qexpr	: '{' <expr>* '}' ;			\
		expr	: <number> | <symbol> | <sexpr> | <qexpr> ;	\
		lisp	: /^/ <expr>* /$/ ;			\
		",
		Number, Symbol, Sexpr, Qexpr, Expr, Lisp);

	ret->Number = Number;
	ret->Symbol = Symbol;
	ret->Sexpr = Sexpr;
	ret->Qexpr = Qexpr;
	ret->Expr= Expr;
	ret->Lisp = Lisp;

	return ret;

}

void free_parsers(parser_elements* _parser_elements)
{
	free(_parser_elements);
	mpc_cleanup(
			6, 
			_parser_elements->Number, 
			_parser_elements->Symbol, 
			_parser_elements->Sexpr, 
			_parser_elements->Qexpr, 
			_parser_elements->Expr, 
			_parser_elements->Lisp
	);
}

