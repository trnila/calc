#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

typedef enum {false, true} bool;
typedef enum {NOTHING, OPERATOR, OPERAND} Token;
typedef struct {
	enum {UNEXPECTED_CHAR, MISSING_NUMBER, MISSING_END_BRACKET} code;
	size_t position;
} Error;

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
	return in >= '0' && in < '9' || in == '.';
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

#define GREEN "\033[22;32m"
#define RED "\033[22;31m"
#define CLR "\033[0m"

bool failedTests = 0;


void e(Error *error, const char* str) {
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

void testError(int test, const char in[], int code, int position, int which) {
	char polish[255];
	Error error;
	int success;

	success = convert(in, polish, &error);
	if(which == 2) {
		if(!success) {
			printf(RED "[%2d] failed - received unexpected error %d in RPN conversion\n", test, code);
			e(&error, polish);
			failedTests++;
			return;
		}

		success = calculate(polish, NULL, &error);
	}

	if(!success) {
		if(error.code != code) {
			printf(RED "[%2d] failed - received error %d instead of %d\n", test, error.code, code);
			e(&error, in);
			failedTests++;
			return;
		}

		if(error.position != position) {
			printf(RED "[%2d] failed - received position %d instead of %d\n", test, error.position, position);
			e(&error, in);
			failedTests++;
			return;
		}
		printf(GREEN "[%2d] Passed - received error\n", test);
	} else {
		printf(RED "[%2d] failed - error %d not received\n", test, code);
		failedTests++;
		return;
	}
}

void test(int test, const char in[], const char polishTemplate[], double resultTemplate) {
	char polish[255];
	Error error;

	// check parse error
	if(!convert(in, polish, &error)) {
		printf(RED "[%2d] converting to RPN returned error\n", test);
		printf("Input:\t\t%s\n", in);
		printf("Expected:\t%s\n", polishTemplate);
		printf("Received:\t%s\n", polish);
		e(&error, in);
		printf("\n" CLR);
		failedTests++;
		return;
	}

	// check if in required format
	if(strcmp(polish, polishTemplate) != 0) {
		printf(RED "[%2d] converted is not same as expected RPN\n", test);
		printf("\tInput:\t\t%s\n", in);
		printf("\tExpected:\t%s\n", polishTemplate);
		printf("\tReceived:\t%s\n", polish);

		printf("\n" CLR);
		failedTests++;
		return;
	}

	double result;

	// check if error
	if(!calculate(polish, &result, &error)) {
		printf(RED "[%2d] calculating returned error\n", test);
		printf("\tInput:\t\t%s\n", in);
		printf("\tConverted:\t%s\n", polish);

		e(&error, polish);


		printf("\n" CLR);

		failedTests++;
		return;
	}

	if(fabs(result - resultTemplate) > 0.000001) {
		printf(RED "[%2d]\n", test);
		printf("\tInput:\t\t%s\n", in);
		printf("\tConverted:\t%s\n", polish);
		printf("\tExpected:\t%f\n", resultTemplate);
		printf("\tReceived:\t%f\n", result);
		printf("\n" CLR);

		failedTests++;
		return;
	}

	printf(GREEN "[%2d] Passed '%s' = '%s' = %f\n" CLR, test, in, polish, resultTemplate);
}

int main() {
	test(1, "3 + 4", "3 4 +", 7);
	test(2, "3 + 4 + 5", "3 4 + 5 +", 12);
	test(3, "3 * 4 + 5", "3 4 * 5 +", 17);
	test(4, "3 + 4 * 5", "3 4 5 * +", 23);
	test(5, "3 * 4 * 5", "3 4 * 5 *", 60);
	test(6, "3 * 4 - 5", "3 4 * 5 -", 7);
	test(7, "3 - 4 * 5", "3 4 5 * -", -17);
	test(8, "40 / 4 + 5", "40 4 / 5 +", 15);
	test(9, "(3 - 4) * 5", "3 4 - 5 *", -5);
	test(10, "(4 * 6 + 2) + ((45 + 2) / 21)", "4 6 * 2 + 45 2 + 21 / +", 28.238095);
	test(11, "(4*6+2)+((45+2)/21)", "4 6 * 2 + 45 2 + 21 / +", 28.238095);

	// a(b+c) notation
	test(12, "2 + 3(12+4)", "2 3 12 4 + * +", 50);
	test(13, "2 + 3 + (12+4)", "2 3 + 12 4 + +", 21);

	// negative numbers
	test(14, "-2 + 3", "-2 3 +", 1);
	test(15, "-2*(-2)", "-2 -2 *", 4);
	test(16, "-2 + 3(-4+8)", "-2 3 -4 8 + * +", 10);
	test(17, "-2 - 3(-4-8)", "-2 3 -4 8 - * -", 34);
	test(18, "-2 - 3(-4-8) - 5(-2 - 3)", "-2 3 -4 8 - * - 5 -2 3 - * -", 59);

	// should be valid
	test(19, "", "", 0);
	test(19, "0", "0", 0);
	test(20, "5", "5", 5);
	test(21, "-5", "-5", -5);
	test(22, "123", "123", 123);

	// some invalid inputs
	testError(23, "abc", UNEXPECTED_CHAR, 0, 1);
	testError(24, "3+", MISSING_NUMBER, 2, 2);
	testError(25, "+", MISSING_NUMBER, 1, 2);
	testError(26, "4(7", MISSING_END_BRACKET, 1, 1);
	testError(27, "4(7+", MISSING_END_BRACKET, 1, 1);
	testError(28, "4 + 2 - 4(7+", MISSING_END_BRACKET, 9, 1);

	// test numbers with floating point
	test(29, "1.5 * 2.0", "1.5 2.0 *", 3);
	test(30, "1.5 * 2", "1.5 2 *", 3);
	test(31, "1 / 2", "1 2 /", 0.5);
	test(32, "1 / 3", "1 3 /", 0.333333);

	printf("\n");
	if(failedTests > 0) {
		printf(RED "%d test%s failed\n" CLR, failedTests, failedTests > 1 ? "s" : "");
	} else {
		printf(GREEN "All tests passed!\n" CLR);
	}

	return failedTests > 0;
}

