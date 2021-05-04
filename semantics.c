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
	if (strstr(t->tag, "string")) { return lval_read_str(t); }
	
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
		if (strstr(t->children[i]->tag, "comment")) { continue; }
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

lval* lval_read_str(mpc_ast_t* t)
{
	// replace the end quotes with a null terminator
	t->contents[strlen(t->contents)-1] = '\0';

	// copy the string from 1st char after the beginning quotes
	char* unescaped = malloc(strlen(t->contents+1)+1);
	strcpy(unescaped, t->contents+1);

	unescaped = mpcf_unescape(unescaped);

	lval* str = lval_str(unescaped);

	free(unescaped);
	return str;
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
	mpc_parser_t* String = mpc_new("string");
	mpc_parser_t* Comment = mpc_new("comment");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Phi = mpc_new("phi");

	mpca_lang(MPCA_LANG_DEFAULT,
		"													\
		number	: /-?[0-9]+/ ;								\
		symbol	: /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;		\
		string	: /\"(\\\\.|[^\"])*\"/ ;					\
		comment	: /;[^\\r\\n]*/ ;							\
		sexpr	: '(' <expr>* ')' ;							\
		qexpr	: '{' <expr>* '}' ;							\
		expr	: <number> | <symbol> | <string> | <comment> | <sexpr> | <qexpr> ;	\
		phi : /^/ <expr>* /$/ ;							\
		",
		Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Phi);

	ret->Number = Number;
	ret->Symbol = Symbol;
	ret->String = String;
	ret->Comment = Comment;
	ret->Sexpr = Sexpr;
	ret->Qexpr = Qexpr;
	ret->Expr= Expr;
	ret->Phi = Phi;

	return ret;

}

void free_parsers(parser_elements* _parser_elements)
{
	mpc_cleanup(
			8, 
			_parser_elements->Number, 
			_parser_elements->Symbol, 
			_parser_elements->String, 
			_parser_elements->Comment, 
			_parser_elements->Sexpr, 
			_parser_elements->Qexpr, 
			_parser_elements->Expr, 
			_parser_elements->Phi
	);
	free(_parser_elements);
}

