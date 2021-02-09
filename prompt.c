#include <stdio.h>
#include <stdlib.h>

#include <readline/history.h>
#include <readline/readline.h>


int main(int argc, char** argv) 
{

	/* Debug info */
	puts("Lisp version 0.0.0.0.1");
	puts("Press Ctrl+C to exit\n");


	/* REPL */
	while (1)
	{

		char* input = readline("lispy> ");

		add_history(input);

		printf("No you're a %s\n", input);

		free(input);

	}

	return 0;

}
