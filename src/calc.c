#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "calc.h"

typedef enum {NOTHING, OPERATOR, OPERAND} Token;

typedef struct node {
	double num;
	struct node *prev;
} List;

void write(List** list, double num) {
	List *dst = (List*) malloc(sizeof(List));
	dst->num = num;
	dst->prev = *list;

	*list = dst;
}

bool read(List** list, double *result) {
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
		printf("%f ", l->num);
		l = l->prev;
	}
	printf("\n");
}

void destroy(List **list) {
	while(read(list, NULL)){}
}

bool isEmpty(List **list) {
	return *list == NULL;
}

double strToInt(const char* str, size_t len) {
	double result = 0;
	bool afterPoint = false;

	// find dotPosition or last character
	int dotPosition;
	for(dotPosition = 0; dotPosition < len && str[dotPosition] != '.'; dotPosition++) {}

	for(size_t i = str[0] == '-'; i < len; i++) {
		int number = str[i] - '0';
		if(number >= 0 && number <= 9) {
			if(i < dotPosition) {
				result += number * pow(10, dotPosition - i - 1);
			} else {
				result += number * pow(10, (int) -(i - dotPosition));
			}
		}
	}

	// if first character is minus, negate number
	if(str[0] == '-') {
		result *= -1;
	}

	return result;
}

bool isNumber(char in) {
	return (in >= '0' && in <= '9') || in == '.';
}

bool calculate(const char *in, double *result, Error* error) {
	List* nums = NULL;

	int start = -1;

	// on strlen + 1 is '\0' which can take care of input like "5"
	for(size_t i = 0, len = strlen(in) + 1; i < len; i++) {
		// is number or minus following a letter
		if(isNumber(in[i]) || (in[i] == '-' && (i + 1) < len && isNumber(in[i+1]))) {
			if(start == -1) {
				start = i;
			}
		} else if(start >= 0) {
				double num = strToInt(in + start, i - start);
				start = -1;

				write(&nums, num);
		} else if(in[i] == '+' || in[i] == '-' || in[i] == '*' || in[i] == '/') {
			double a, b, result;
			if(!read(&nums, &b) || !read(&nums, &a)) {
				if(error) {
					error->code = MISSING_NUMBER;
					error->position = i-1;
				}

				return false;
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

	*result = 0;
	read(&nums, result);

	if(!isEmpty(&nums)) {
		printf("Stack not empty!\n");
		print(&nums);
		return false;
	}

	return true;
}



bool convert(const char in[], char *result, Error *error) {
	List *ops = NULL;
	int r = 0;
	Token last = NOTHING;

	for(size_t i = 0, size = strlen(in); i < size; i++) {
		// if is number, push it to output
		if(isNumber(in[i]) || in[i] == '.' || (last == NOTHING && in[i] == '-')) {
			result[r++] = in[i];
			last = OPERAND;
		} else {
			switch(in[i]) {
				// if its operator, push it to stack
				case '+':
				case '-':
				case '*':
				case '/': {
					// pop operators from stack when they have higher priority then actual operator
					while(ops && ((ops->num == '*' || ops->num == '/') || ((in[i] == '+' || in[i] == '-') && (ops->num == '+' || ops->num == '-')))) {
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

					last = NOTHING;

					break;
				}

				// pop until '(' is found in stack
				case ')': {
					double op;

					while(read(&ops, &op)) {
						if(op == '(') {
							break;
						}

						result[r++] = ' ';
						result[r++] = op;
					}

					break;
				}

				// space character is allowed - just do nothing
				case ' ': {

					break;
				}

				default: {
					error->code = UNEXPECTED_CHAR;
					error->position = i;
					return false;
				}
			}
		}
	}

	// pop all remaining operators from stack and put them to output
	double op;
	while(read(&ops, &op)) {
		if(op == '(') {
			error->code = MISSING_END_BRACKET;
			error->position = strstr(in, "(") - in;

			return false;
		}

		result[r++] = ' ';
		result[r++] = op;
	}

	result[r] = '\0';
}
