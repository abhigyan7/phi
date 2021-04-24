#ifndef LVAL_H
#define LVAL_H

typedef struct lval lval;

enum 
{
	LVAL_NUM, 
	LVAL_ERR, 
	LVAL_SYM, 
	LVAL_SEXPR, 
	LVAL_QEXPR, 
	LVAL_FUN,
	LVAL_BOOL
};

typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);

#define TRUE 1
#define FALSE 0

struct lval
{
	int type;

	long num;

	char* err;

	char* sym;

	lbuiltin builtin;
	lenv* env;
	lval* formals;
	lval* body;
	
	// number of child lvals
	int count;
	/* 
	 * Pointer to a list of child lvals
	 * If a struct contains a ref to itself
	 * append a struct before the declaration
	 * to avoid circular dependency
	 * Also, struct  can only contain pointers
	 * to their own type, not instances of their
	 * own type
	 */
	struct lval** cell;

	char bool_state;

};

void lval_del(lval*);
lval* lval_num(long);
lval* lval_err(char*, ...);
lval* lval_sym(char*);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_bool(int);
lval* lval_fun(lbuiltin);
lval* lval_lambda(lval*, lval*);
lval* lval_copy(lval*);
void lval_expr_print(lval*, char, char);
void lval_print(lval*);
void lval_println(lval*);
char* ltype_name(int);

#endif
