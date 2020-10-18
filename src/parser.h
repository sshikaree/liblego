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

#include "parser_error.h"

#include <stdbool.h>
#include <stdlib.h>

// typedefs for libtidy
typedef unsigned long ulong;
typedef unsigned int uint;
#include <tidy/tidy.h>

#include "util/dynamic_string.h"
#include "util/slist.h"
#include "selector.h"

typedef unsigned char byte;


typedef struct lego_Parser {
    char*           s;                              // source string
    size_t          s_len;                          // source string length
    char*           pos;                            // current position in the source string
	SelectorGroup	sel_group;						// null-terminated array of found selectors. Do we need it??
//    size_t		 sel_group_count;				// number of found selectors
} lego_Parser;



// skipWhitespace consumes whitespace characters and comments.
// It returns true if there was actually anything to skip.
bool skipWhitespace(lego_Parser* p);

// parseIdentifier parses an identifier
lego_ParserError parseIdentifier(lego_Parser* p, String* result);

lego_ParserError parseQuoted(lego_Parser* p, String* result);



#endif    // PARSER_H
