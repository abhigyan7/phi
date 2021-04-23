all: executable

executable:
	gcc -Wall -std=c99 semantics.c main.c lval.c lenv.c expressions.c arithmetics.c mpc.c -ledit -o bin/lisp -g

