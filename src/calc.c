#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "calc.h"

#define M_PI 3.14159265358979323846264338327

typedef enum {NOTHING, OPERATOR, OPERAND} Token;
typedef enum {OP_PLUS = '+', OP_MINUS = '-', OP_MULTIPLY = '*', OP_DIVISION = '/', OP_POW = '^', OP_SQRT = 0, OP_SIN = 1, OP_COS = 2, OP_TG = 3, OP_COTG = 4} Operator;

typedef struct node {
	double num;
	struct node *prev;
} List;

void writeList(List** list, double num) {
	List *dst = (List*) malloc(sizeof(List));
	dst->num = num;
	dst->prev = *list;

	*list = dst;
}

bool readList(List** list, double *result) {
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

void printList(List **list) {
	List *l = *list;
	while(l) {
		printf("%f ", l->num);
		l = l->prev;
	}
	printf("\n");
}

void destroyList(List **list) {
	while(readList(list, NULL)){}
}

bool isEmptyList(List **list) {
	return *list == NULL;
}

double strToInt(const char* str, size_t len) {
	double result = 0;

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

#define isMathFunction(type) strncmp(type, in + i, strlen(type)) == 0

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

				writeList(&nums, num);
		} else if(in[i] == '+' || in[i] == '-' || in[i] == '*' || in[i] == '/' || in[i] == '^') {
			double a, b, result;
			if(!readList(&nums, &b) || !readList(&nums, &a)) {
				if(error) {
					error->code = MISSING_NUMBER;
					error->position = i-1;
				}

				destroyList(&nums);
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
				case '^':
					result = pow(a, b);
					break;
			}

			writeList(&nums, result);
		} else {
			if(isMathFunction("sqrt") || isMathFunction("sin") || isMathFunction("cos") || isMathFunction("tg") || isMathFunction("cotg")) {
				double a, result;
				if(!readList(&nums, &a)) {
					if(error) {
						error->code = MISSING_NUMBER;
						error->position = i-1;
					}

					destroyList(&nums);
					return false;
				}

				if(isMathFunction("sqrt")) {
					result = sqrt(a);
				} else if(isMathFunction("sin")) {
					result = sin(a * M_PI / 180);
				} else if(isMathFunction("cos")) {
					result = cos(a * M_PI / 180);
				} else if(isMathFunction("tg")) {
					result = tan(a * M_PI / 180);
				} else if(isMathFunction("cotg")) {
					result = 1 / tan(a * M_PI / 180);
				}

				writeList(&nums, result);

			}
		}
	}

	if(result) {
		*result = 0;
		readList(&nums, result);
	}

	if(!isEmptyList(&nums)) {
		printf("Stack not empty!\n");
		printList(&nums);

		destroyList(&nums);
		return false;
	}

	return true;
}

bool hasBiggerPriority(char op1, char op2) {
	if(op1 == OP_POW) {
		return true;
	}

	if(op1 == OP_DIVISION || op1 == OP_MULTIPLY) {
		return true;
	}

	if((op1 == OP_PLUS || op1 == OP_MINUS) && (op2 == OP_PLUS || op2 == OP_MINUS)) {
		return true;
	}

	return false;
}

void writeOperator(Operator op, char* result, int *r) {
	switch(op) {
		case OP_PLUS:
			*result = '+';
			(*r)++;
			break;
		case OP_MINUS:
			*result = '-';
			(*r)++;
			break;
		case OP_MULTIPLY:
			*result = '*';
			(*r)++;
			break;
		case OP_DIVISION:
			*result = '/';
			(*r)++;
			break;
		case OP_POW:
			*result = '^';
			(*r)++;
			break;
		case OP_SQRT:
			strcpy(result, "sqrt");
			*r += strlen("sqrt");
			break;
		case OP_SIN:
			strcpy(result, "sin");
			*r += strlen("sin");
			break;
		case OP_COS:
			strcpy(result, "cos");
			*r += strlen("cos");
			break;
		case OP_TG:
			strcpy(result, "tg");
			*r += strlen("tg");
			break;
		case OP_COTG:
			strcpy(result, "cotg");
			*r += strlen("cotg");
			break;
		default:
			strcpy(result, "UNKNOWN");
			*r += strlen("UNKNOWN");
	}
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
				case '^':
				case '/': {
					// pop operators from stack when they have higher priority then actual operator
					while(ops && hasBiggerPriority(ops->num, in[i])) {
					//while(ops && ((ops->num == '*' || ops->num == '/') || ((in[i] == '+' || in[i] == '-') && (ops->num == '+' || ops->num == '-')))) {
						if(r > 1 && result[r - 1] != ' ') {
							result[r++] = ' ';
						}

						writeOperator(ops->num, result + r, &r);


						readList(&ops, NULL); // pop from queue
					}

					result[r++] = ' ';
					writeList(&ops, in[i]);

					last = OPERATOR;
					break;
				}

				// push starting bracket to stack
				case '(': {
					// support for a(a+b) = a*(a+b) notation
					if(last == OPERAND) {
						writeList(&ops, '*');
						result[r++] = ' ';
					}

					writeList(&ops, in[i]);

					last = NOTHING;

					break;
				}

				// pop until '(' is found in stack
				case ')': {
					double op;

					while(readList(&ops, &op)) {
						if(op == '(') {
							break;
						}

						result[r++] = ' ';
						writeOperator(op, result + r, &r);
					}

					break;
				}

				// space character is allowed - just do nothing
				case ' ': {

					break;
				}

				default: {
					if(strncmp("sqrt", in + i, strlen("sqrt")) == 0) {
						i += strlen("sqrt");

						writeList(&ops, OP_SQRT);
					} else if(strncmp("sin", in + i, strlen("sin")) == 0) {
						i += strlen("sin");
						writeList(&ops, OP_SIN);
					} else if(strncmp("cos", in + i, strlen("cos")) == 0) {
						i += strlen("cos");
						writeList(&ops, OP_COS);
					} else if(strncmp("tg", in + i, strlen("tg")) == 0) {
						i += strlen("tg");
						writeList(&ops, OP_TG);
					} else if(strncmp("cotg", in + i, strlen("cotg")) == 0) {
						i += strlen("cotg");
						writeList(&ops, OP_COTG);
					} else {
						error->code = UNEXPECTED_CHAR;
						error->position = i;

						destroyList(&ops);
						return false;
					}
				}
			}
		}
	}

	// pop all remaining operators from stack and put them to output
	double op;
	while(readList(&ops, &op)) {
		if(op == '(') {
			error->code = MISSING_END_BRACKET;
			error->position = strstr(in, "(") - in;

			destroyList(&ops);

			return false;
		}

		result[r++] = ' ';

		writeOperator(op, result + r, &r);
	}

	result[r] = '\0';

	return true;
}
