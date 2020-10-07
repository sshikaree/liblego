//
// strsplt() splits given string into tokens. 
//

#ifndef TOKENS_ARRAY_H
#define TOKENS_ARRAY_H

#include <stdlib.h>


typedef struct TokensArray {
	const char**	arr;
	size_t			len;
	size_t      	cap;
} TokensArray;


// TokensArray_new creates new TokensArray with zero length.
TokensArray* TokensArray_new(void);

// TokensArray_append appends token to the TokensArray and increases it's length.
void TokensArray_append(TokensArray* ta, const char* token);

// TokensArray_freeSelf frees only TokensArray, not including data.
void TokensArray_freeSelf(TokensArray* ta);

// TokensArray_freeFull frees all data and TokensArray itself.
void TokensArray_freeFull(TokensArray* ta);

// strsplt splits given string into tokens.
// Result must be freed with TokensArray_freeFull().
TokensArray* strsplt(const char* str, const char* delims);


#endif // TOKENS_ARRAY_H
