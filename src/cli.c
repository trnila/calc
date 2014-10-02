#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include "calc.h"
#include "displayError.h"

#define HISTORY_FILE "/.calc_history"

int main() {
	char *input;
	char polish[255];
	Error error;
	double result;

	char *home = getenv("HOME");
	char *historyFile = (char*) malloc(strlen(home) + strlen(HISTORY_FILE));
	sprintf(historyFile, "%s%s", home, HISTORY_FILE);

	// read history from file
	read_history(historyFile);

	while(1) {
		input = readline("> ");

		if(!input || strcmp(input, "exit") == 0) {
			printf("Ending....\n");
			break;
		}

		// add history
		add_history(input);

		//save history to file
		if(append_history(1, historyFile) == ENOENT) {
			if(write_history(historyFile) != 0) {
				fprintf(stderr, "Could not save to history file %s: %s\n", historyFile, strerror(errno));
			}
		}


		// compute result
		if(!convert(input, polish, &error) || !calculate(polish, &result, &error)) {
			printf(RED);
			displayError(&error, input);
			printf(CLR);
			continue;
		}

		printf("=%f\n", result);

		free(input);

	}

	free(historyFile);

	return 0;
}