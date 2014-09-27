#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

typedef enum {false, true} bool;
typedef enum {NOTHING, OPERATOR, OPERAND} Token;

typedef struct node {
	int num;
	struct node *prev;
} List;

void write(List** list, int num) {
	List *dst = (List*) malloc(sizeof(List));
	dst->num = num;
	dst->prev = *list;

	*list = dst;
}

bool read(List** list, int *result) {
	if(*list) {
		List *dst = *list;

		// save result if provided
		if(result) {
			*result = dst->num;
		}

		*list = dst->prev;

		free(dst);
		return 1;
	}

	return 0;
}

void print(List **list) {
	List *l = *list;
	while(l) {
		printf("%d ", l->num);
		l = l->prev;
	}
	printf("\n");
}

void destroy(List **list) {
	while(read(list, NULL)){}
}

bool isEmpty(List **list) {
	return list != NULL;
}

int strToInt(const char* str, size_t len) {
	int result = 0;
	for(size_t i = 0; i < len; i++) {
		result += (str[i] - '0') * pow(10, len - i - 1);
	}

	return result;
}

int calculate(const char *in) {
	List* nums = NULL;

	int start = -1;
	for(size_t i = 0, len = strlen(in); i < len; i++) {
		if(in[i] >= '0' && in[i] <= '9') {
			if(start == -1) {
				start = i;
			}
		} else {
			if(start >= 0) {
				int num = strToInt(in + start, i - start);
				start = -1;

				write(&nums, num);
			}
		}

		if(in[i] == '+' || in[i] == '-' || in[i] == '*' || in[i] == '/') {
			int a, b, result;
			if(!read(&nums, &b)) {
				printf("Number B missing!");
				return 0;
			}

			if(!read(&nums, &a)) {
				printf("Number A missing!");
				return 0;
			}

			switch(in[i]) {
				case '+':
					result = a + b;
					break;
				case '-':
					result = a - b;
					break;
				case '*':
					result = a*b;
					break;
				case '/':
					result = a/b;
					break;
			}

			write(&nums, result);
		}
	}

	int result;
	read(&nums, &result);

	if(!isEmpty(&nums)) {
		printf("Stack not empty!");
	}

	return result;
}



void convert(const char in[], char *result) {
	List *ops = NULL;
	int r = 0;
	Token last = NOTHING;

	for(size_t i = 0, size = strlen(in); i < size; i++) {
		// if is number, push it to output
		if(in[i] >= '0' && in[i] <= '9') {
			result[r++] = in[i];
			last = OPERAND;
		} else {
			switch(in[i]) {
				// if its operator, push it to stack
				case '+':
				case '-':
				case '*':
				case '/': {
					// if previous operator is * or / - pop him from stack, because it has high priority
					if(ops && (ops->num == '*' || ops->num == '/')) {
						if(r > 1 && result[r - 1] != ' ') {
							result[r++] = ' ';
						}
						result[r++] = ops->num;

						read(&ops, NULL); // pop from queue
					}

					result[r++] = ' ';
					write(&ops, in[i]);

					last = OPERATOR;
					break;
				}

				// push starting bracket to stack
				case '(': {
					// support for a(a+b) = a*(a+b) notation
					if(last == OPERAND) {
						write(&ops, '*');
						result[r++] = ' ';
					}

					write(&ops, in[i]);

					break;
				}

				// pop until '(' is found in stack
				case ')': {
					int op;
					bool openBracket = false;

					while(read(&ops, &op)) {
						if(op == '(') {
							openBracket = true;
							break;
						}

						result[r++] = ' ';
						result[r++] = op;
					}

					if(!openBracket) {
						printf("No open bracket found!");
						return;
					}

					break;
				}
			}
		}
	}

	// pop all remaining operators from stack and put them to output
	int op;
	while(read(&ops, &op)) {
		result[r++] = ' ';
		result[r++] = op;
	}

	result[r] = '\0';
}

#define GREEN "\033[22;32m"
#define RED "\033[22;31m"
#define CLR "\033[0m"

bool status = 0;

void test(int test, const char in[], const char polishTemplate[], int resultTemplate) {
	char polish[255];
	convert(in, polish);

	if(strcmp(polish, polishTemplate) != 0) {
		printf(RED "[%d] '%s' != '%s'\n" CLR, test, polish, polishTemplate);
		status = 1;
		return;
	}

	int result = calculate(polish);
	if(result != resultTemplate) {
		printf(RED "[%d] '%s' != '%s'; %d != %d\n" CLR, test, in, polishTemplate, resultTemplate, result);
		status = 1;
		return;
	}


	printf(GREEN "[%d] Passed '%s' = '%s' = %d\n" CLR, test, in, polish, resultTemplate);


}


int main() {
	test(1, "3 + 4", "3 4 +", 7);
	test(2, "3 + 4 + 5", "3 4 5 + +", 12);
	test(3, "3 * 4 + 5", "3 4 * 5 +", 17);
	test(4, "3 + 4 * 5", "3 4 5 * +", 23);
	test(5, "3 * 4 * 5", "3 4 * 5 *", 60);
	test(6, "3 * 4 - 5", "3 4 * 5 -", 7);
	test(7, "3 - 4 * 5", "3 4 5 * -", -17);
	test(8, "40 / 4 + 5", "40 4 / 5 +", 15);
	test(9, "(3 - 4) * 5", "3 4 - 5 *", -5);
	test(10, "(4 * 6 + 2) + ((45 + 2) / 21)", "4 6 * 2 + 45 2 + 21 / +", 28);
	test(11, "(4*6+2)+((45+2)/21)", "4 6 * 2 + 45 2 + 21 / +", 28);

	// a(b+c) notation
	test(12, "2 + 3(12+4)", "2 3 12 4 + * +", 50);
	test(13, "2 + 3 + (12+4)", "2 3 12 4 + + +", 21);

	return status;
}

