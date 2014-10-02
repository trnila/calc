#include <stdio.h>
#include <math.h>
#include <string.h>
#include "calc.h"
#include "displayError.h"

bool failedTests = 0;

void testError(int test, const char in[], int code, int position, int which) {
	char polish[255];
	Error error;
	int success;

	success = convert(in, polish, &error);
	if(which == 2) {
		if(!success) {
			printf(RED "[%2d] failed - received unexpected error %d in RPN conversion\n", test, code);
			displayError(&error, polish);
			failedTests++;
			return;
		}

		success = calculate(polish, NULL, &error);
	}

	if(!success) {
		if(error.code != code) {
			printf(RED "[%2d] failed - received error %d instead of %d\n", test, error.code, code);
			displayError(&error, in);
			failedTests++;
			return;
		}

		if(error.position != position) {
			printf(RED "[%2d] failed - received position %d instead of %d\n", test, error.position, position);
			displayError(&error, in);
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
		displayError(&error, in);
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

		displayError(&error, polish);


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