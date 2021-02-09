#include <stdio.h>


/* Declare a buffer for user input of size 2048 chars */
static char input[2048];

int main(int argc, char** argv) 
{

	/* Debug info */
	puts("Lisp version 0.0.0.0.1");
	puts("Press Ctrl+C to exit\n");


	/* REPL */
	while (1)
	{

		fputs("lisp> ", stdout);

		fgets(input, 2048, stdin);

		printf("No you're a %s", input);

	}

	return 0;

}
