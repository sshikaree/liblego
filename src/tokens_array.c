#include "tokens_array.h"

// #include <stdio.h>
#include <string.h>

#define  REALLOC_CAP 5 // default appended capacity when reallocated


// Creates new TokensArray with zero length
TokensArray* TokensArray_new(void) {
	TokensArray* ta = (TokensArray*)malloc(sizeof(TokensArray));
	ta->arr = (const char**)malloc(sizeof(char*) * REALLOC_CAP);
    ta->cap = REALLOC_CAP;
	ta->len = 0;

	return ta;
}

// Appends token to the TokensArray and increases it's length
void TokensArray_append(TokensArray* ta, const char* token) {
	ta->arr[ta->len] = token;
	ta->len += 1;
    if ((ta->cap-ta->len) <= 1) {
        ta->arr = (const char**)realloc(ta->arr, sizeof(char*)*(ta->cap + REALLOC_CAP));    
    }
}

// Frees only TokensArray, not including data
extern inline void TokensArray_freeSelf(TokensArray* ta) {
	free(ta->arr);
	free(ta);
}

// Frees all data and TokensArray itself
extern inline void TokensArray_freeFull(TokensArray* ta) {
	//for (; ta->len; --(ta->len)) {
    for (; ta->len; --(ta->len)) {
		// printf("Freeing array->array[%lu]\n", ta->len-1);
        free((void*)ta->arr[ta->len-1]);
	}
	free(ta->arr);
	free(ta);
}

// Result must be freed with TokensArray_freeFull()
TokensArray* strsplt(const char* str, const char* delims) {
	if (!str || !delims) {
		return NULL;
	}
	TokensArray* ta = TokensArray_new();

	const char* tok_start = str;
	const char* c_ptr = str;
	for (; *c_ptr; ++c_ptr) {
		if (strchr(delims, *c_ptr)) {
			// printf("Found '%c' at pos [%lu]\n", *c_ptr, c_ptr-str);
			if (tok_start == c_ptr) {
				++tok_start;
				continue;
			}
			char* token = NULL;
			if (tok_start == str) {
                token = malloc(sizeof(char) * (size_t)(c_ptr - tok_start + 1));
                strncpy(token, tok_start, (size_t)(c_ptr-tok_start));
				token[c_ptr-tok_start] = '\0';
			} else {
                token = malloc(sizeof(char) * (size_t)(c_ptr - tok_start));
                strncpy(token, tok_start + 1, (size_t)(c_ptr-tok_start - 1));
				token[c_ptr-tok_start - 1] = '\0';
			}
			
			TokensArray_append(ta, token);
			tok_start = c_ptr;
		}
	}
	// when *c_ptr == '\0' at the end of the given string
	if (tok_start != c_ptr) {
		char* token = NULL;
		if (tok_start == str) {
            token = malloc(sizeof(char) * (size_t)(c_ptr - tok_start + 1));
            strncpy(token, tok_start, (size_t)(c_ptr-tok_start));
			token[c_ptr-tok_start] = '\0';
		} else {
            token = malloc(sizeof(char) * (size_t)(c_ptr - tok_start));
            strncpy(token, tok_start + 1, (size_t)(c_ptr-tok_start - 1));
			token[c_ptr-tok_start - 1] = '\0';
		}
		TokensArray_append(ta, token);
	}

	return ta;
}
