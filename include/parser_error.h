#ifndef PARSER_ERROR_H
#define PARSER_ERROR_H

typedef enum {
	ParserError_NO_ERROR,
	ParserError_UNEXPECTED_EOF,
	ParserError_UNEXPECTED_EOL,
	ParserError_UNEXPECTED_CHAR,
	ParserError_WRONG_IDENTIFIER,
	ParserError_WRONG_ID,
	ParserError_WRONG_CLASSNAME,
	ParserError_WRONG_ATTRIBUTE,
	ParserError_WRONG_ATTRIBUTE_OPERATOR,
	ParserError_WRONG_PSEUDOCLASS,
	ParserError_INVALID_ESCAPE_SEQUENCE,
	ParserError_SEL_BUF_OVERFLOW,
	ParserError_MEMORY_ERROR,

	ParserError_UNKNOWN_ERROR
} lego_ParserError;

const char* lego_ParserError_toString(lego_ParserError err);

#endif // PARSER_ERROR_H
