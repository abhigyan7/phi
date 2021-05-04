#include "lval.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "mpc.h"

lenv* lenv_new(void);
void lenv_del(lenv*);
lenv* lenv_copy(lenv*);

lval* lval_num(long x)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}

lval* lval_str(char* s)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_STR;
	v->str = malloc(strlen(s) + 1);
	strcpy(v->str, s);
	return v;
}

lval* lval_err(char* fmt, ...)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;


	va_list va;
	va_start(va, fmt);

	v->err = malloc(512);

	vsnprintf(v->err, 511, fmt, va);

	v->err = realloc(v->err, strlen(v->err)+1);

	va_end(va);

	return v;
}

lval* lval_sym(char* s)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s)+1);
	strcpy(v->sym, s);
	return v;
}

lval* lval_sexpr(void)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval* lval_qexpr(void)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval* lval_bool(int state)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_BOOL;
	if (state == 0){
		v->bool_state = FALSE;
	} else {
		v->bool_state = TRUE;
	}
	return v;
}

lval* lval_fun(lbuiltin func)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	v->builtin = func;
	return v;
}

lval* lval_lambda(lval* formals, lval* body)
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;

	v->builtin = NULL;

	v->env = lenv_new();

	v->formals = formals;
	v->body = body;

	return v;
}

void lval_del(lval* v)
{
	switch(v->type)
	{
	
		case LVAL_NUM: break;
		case LVAL_BOOL: break;

		case LVAL_STR: free(v->str); break;

		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;

		case LVAL_SEXPR:
		case LVAL_QEXPR:
			for (int i = 0; i < v->count; i++)
			{
				lval_del(v->cell[i]);
			}

			free(v->cell);
			break;
		case LVAL_FUN:
			if (!v->builtin)
			{
				lenv_del(v->env);
				lval_del(v->formals);
				lval_del(v->body);
			}
			break;

	}
	free(v);
}

lval* lval_copy(lval* v)
{
	lval* x = malloc(sizeof(lval));
	x->type = v->type;

	switch(v->type)
	{
		case LVAL_NUM:
			x->num = v->num; break;
		case LVAL_BOOL:
			x->bool_state = v->bool_state; break;
		case LVAL_STR:
			x->str = malloc(strlen(v->str) + 1);
			strcpy(x->str, v->str);
			break;
		case LVAL_FUN:
			if (v->builtin)
			{
				x->builtin= v->builtin; 
			} else {
				x->builtin = NULL;
				x->env = lenv_copy(v->env);
				x->formals = lval_copy(v->formals);
				x->body = lval_copy(v->body);
			}
			break;

		case LVAL_ERR:
			x->err = malloc(strlen(v->err) + 1);
			strcpy(x->err, v->err); break;

		case LVAL_SYM:
			x->sym = malloc(strlen(v->sym) + 1);
			strcpy(x->sym, v->sym); break;

		case LVAL_SEXPR:
		case LVAL_QEXPR:
			x->count = v->count;
			x->cell = malloc(v->count * sizeof(lval));
			for (int i = 0; i < v->count; i++)
			{
				x->cell[i] = lval_copy(v->cell[i]);
			}
		break;
	}
	return x;
}

int lval_eq(lval* x, lval* y)
{
	if (x->type != y->type)
	{
		return 0;
	}

	switch (x->type)
	{
		case LVAL_NUM: return (x->num == y->num);
		case LVAL_STR: return (strcmp(x->str, y->str) == 0);
		case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
		case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);

		case LVAL_FUN:
			if (x->builtin || y->builtin)
			{
				return x->builtin == y->builtin;
			} else {
				return lval_eq(x->formals, y->formals) && lval_eq(x->body, y->body);
			}
		case LVAL_QEXPR:
		case LVAL_SEXPR:
			if (x->count != y->count)
			{
				return 0;
			}
			for (int i = 0; i < y->count; i++)
			{
				if (!lval_eq(x->cell[i], y->cell[i]))
				{
					return 0;
				}
			}
			return 1;
		break;
	}
	return 0;
}

void lval_expr_print(lval* v, char open, char close)
{
	putchar(open);
	for (int i = 0; i < v->count; i++)
	{
		lval_print(v->cell[i]);

		if (i != (v->count-1)) 
		{
			putchar(' ');
		}
	}
	putchar(close);
} 

void lval_print_str(lval* v) {
  /* Make a Copy of the string */
  char* escaped = malloc(strlen(v->str)+1);
  strcpy(escaped, v->str);
  /* Pass it through the escape function */
  escaped = mpcf_escape(escaped);
  /* Print it between " characters */
  printf("\"%s\"", escaped);
  /* free the copied string */
  free(escaped);
}

void lval_print(lval* v)
{
	switch (v->type)
	{
		case LVAL_NUM: printf("%li", v->num); break;

		case LVAL_BOOL: 
			if (v->bool_state == TRUE)
			{
				printf("true");
			} else {
				printf("false");
			}
		break;

		// if an error, print message relevant to the error
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_QEXPR: lval_expr_print(v, '{', '}');  break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')');  break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_FUN: 
			if (v->builtin)
			{
				printf("<builtin>");
			} else {
				printf("(\\ "); lval_print(v->formals);
				putchar(' '); lval_print(v->body); putchar(')');
			}
		break;
		case LVAL_STR: lval_print_str(v); break;
	}
}

void lval_println(lval* v)
{
	lval_print(v);
	putchar('\n');
}

char* ltype_name(int t)
{
	switch(t)
	{
		case LVAL_NUM: return "Number"; break;
		case LVAL_STR: return "String"; break;
		case LVAL_FUN: return "Function"; break;
		case LVAL_ERR: return "Error"; break;
		case LVAL_SYM: return "Symbol"; break;
		case LVAL_SEXPR: return "S-expression"; break;
		case LVAL_QEXPR: return "Q-expression"; break;
		case LVAL_BOOL: return "Boolean"; break;
		default: return "Unknown";
	}
}

