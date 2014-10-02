#define GREEN "\033[22;32m"
#define RED "\033[22;31m"
#define CLR "\033[0m"

void displayError(Error *error, const char* str) {
	printf("'%s'\n", str);
	for(int i = 0; i <= error->position; i++) {
		printf(" ");
	}
	printf("└────── ");
	if(error->code == MISSING_NUMBER) {
		printf("Missing number");
	} else if(error->code == MISSING_END_BRACKET) {
		printf("Missing ending bracket");
	} else if(error->code == UNEXPECTED_CHAR) {
		printf("Unexpected character: '%c' (%d)", str[error->position], str[error->position]);
	} else {
		printf("Unknown error");
	}
	printf("\n");
}