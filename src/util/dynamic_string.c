#include "dynamic_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CHUNK_SZ 8  // default chunk size for (re)allocating memory for string

String* string_new_sized(size_t sz) {
    String* s = (String*)malloc(sizeof(String));
    if (!s) {
        return NULL;
    }
    s->str = (char*)malloc(sizeof(char) * sz);
    if (!s->str) {
        free(s);
        return NULL;
    }
    s->str[0] = '\0';
    s->len = 0;
    s->allocated_len = sz;

    return s;
}

String* string_new(const char* init) {
    if (init == NULL || *init == '\0') {
        return string_new_sized(CHUNK_SZ);
    }
    String* s = (String*)malloc(sizeof (String));
    if (!s) {
        return NULL;
    }
    s->len = strlen(init);
    s->allocated_len = (s->len/CHUNK_SZ + 1)*CHUNK_SZ;
    s->str = (char*)malloc(sizeof (char) * s->allocated_len);
    if (!s->str) {
        free(s);
        return NULL;
    }
    strcpy(s->str, init);
    return s;
}

// Adds a null-terminated C-string onto the end of a String.
String* string_append(String* s, const char* val) {
    size_t val_len = strlen(val);
    if (s->allocated_len - s->len < val_len + 1) {
        size_t new_alloc_len = ((s->len + val_len) / CHUNK_SZ + 1)*CHUNK_SZ;
		char* tmp = (char*)realloc(s->str, new_alloc_len);
		if (!tmp) {
			fprintf(stderr, "realloc() failed.\n");
			// should we return NULL or previous value?
			string_free(s);
            return NULL;
        }
		s->str = tmp;
        s->allocated_len = new_alloc_len;
    }
    strcat (s->str, val);
    s->len += val_len;

    return s;
}


//Appends len bytes of val to String.
//@val may contain embedded nuls and need not be nul-terminated.
//It is the caller's responsibility to ensure that @val has at least len addressable bytes.
String* string_append_len(String *s, const char *buf, size_t len){
    if (s->allocated_len - s->len < len + 1) {
        size_t new_alloc_len = ((s->len + len) / CHUNK_SZ + 1)*CHUNK_SZ;
        s->str = (char*)realloc(s->str, new_alloc_len);
        if (!s->str) {
            return NULL;
        }
        s->allocated_len = new_alloc_len;
    }
    memcpy(&s->str[s->len], buf, len);
    s->len += len;
    s->str[s->len] = '\0';
    return s;
}

// Adds a byte onto the end of a String.
String* string_append_c(String* s, char c) {
    if (s->allocated_len - s->len < 2) {
		s->allocated_len += CHUNK_SZ;
		char* tmp = (char*)realloc(s->str, s->allocated_len);
		if (!tmp) {
			string_free(s);
			return NULL;
		}
		s->str = tmp;
	}
    s->str[s->len++] = c;
    s->str[s->len] = '\0';
	
	return s;
	
}
	

void string_free(String* s) {
    if (!s) {
        return;
    }
    if (s->str) {
        free(s->str);
        s->str = NULL;
    }
    free(s);
}

