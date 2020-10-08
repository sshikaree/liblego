//
//    complex selector ─┐       ┌ simple selector
//                      │       │     │
//            ┌─────────┴────┐  ├┐┌───┴─┐
//            p.classname > h1, h2.class2
//            │           ↑     └───────┼──── compound selector
//            │     combinator          │
//            │                         │
//            └───────────┬─────────────┘
//                        └ selector group / selector list

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdlib.h>

// typedefs for libtidy
typedef unsigned long ulong;
typedef unsigned int uint;
#include <tidy/tidy.h>

#include "../../dynamic_string/dynamic_string.h"
#include "selector.h"

typedef unsigned char byte;

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

    ParserError_UNKNOWN_ERROR
} ParserError;

// Selector match function prototype.
//extern bool (*MatchFunc)(Selector sel, TidyNode node);

typedef struct Parser {
    char*           s;                              // source string
    size_t          s_len;                          // source string length
    char*           pos;                            // current position in the source string
//    SimpleSelector  selectors[MAX_SELECTORS_NUM];   // array of found selectors. Do we need it??
//    int             sel_count;                      // number of found selectors
} Parser;


const char* ParserError_toString(ParserError err);

// Parser_init inits Parser with given source string.
void Parser_init(Parser* p, char *source_string);

// appendSelector adds selector to the p->selectors.
ParserError appendSelector(Parser* p, SimpleSelector* sel);


// skipWhitespace consumes whitespace characters and comments.
// It returns true if there was actually anything to skip.
bool skipWhitespace(Parser* p);

// parseIdentifier parses an identifier
ParserError parseIdentifier(Parser* p, String* result);

ParserError parseQuoted(Parser* p, String* result);

//bool parseTypeSelector(Parser* p);

//bool parseIDSelector(Parser* p);

//bool parseClassSelector(Parser* p);

//bool parseAttributeSelector(Parser* p);

// parseSimpleSelectorSequence parses a selector sequence that applies to
// a single element.
// Returns count of found selectors.
//int Parser_parseSimpleSelectorSequence(Parser* p);

// parseSelector parses a selector that may include combinators.
ParserError parseSelector(Parser* p, CombinedSelector* comb_sel);

// parseSelectorGroup parses a group of selectors, separated by commas.
ParserError parseSelectorGroup(Parser *p, CombinedSelector sel_group[], size_t group_size);



#endif    // PARSER_H
