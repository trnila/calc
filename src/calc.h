#ifndef CALC_H
	#define CALC_H

	typedef enum {false, true} bool;

	typedef struct {
		enum {UNEXPECTED_CHAR, MISSING_NUMBER, MISSING_END_BRACKET} code;
		size_t position;
	} Error;

	bool calculate(const char *in, double *result, Error* error);
	bool convert(const char in[], char *result, Error *error);

#endif