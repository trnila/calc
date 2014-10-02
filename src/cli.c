#include <stdio.h>
#include <string.h>
#include "calc.h"
#include "displayError.h"


#define MAX 100

int main() {
	char input[MAX + 1];


	char polish[255];
	Error error;
	double result;
	while(1) {
		printf("> ");
		fgets(input, MAX, stdin);

		// remove trailing newline
		char *trailing = strrchr(input, '\n');
		if(trailing) {
			*trailing = '\0';
		}


		if(strcmp(input, "exit") == 0) {
			printf("Ending....\n");
			break;
		}

		if(!convert(input, polish, &error) || !calculate(polish, &result, &error)) {
			printf(RED);
			displayError(&error, input);
			printf(CLR);
			continue;
		}

		printf("=%f\n", result);

	}

	return 0;
}