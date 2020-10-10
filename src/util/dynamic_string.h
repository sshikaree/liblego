#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include <stdlib.h>

typedef struct String {
    char*   str;
    size_t  len;
    size_t  allocated_len;
} String;

String* string_new_sized(size_t sz);

String* string_new(const char* init);

String* string_append(String* s, const char* val);

String* string_append_len(String* s, const char* buf, size_t len);

String* string_append_c(String* s, char c);

void string_free(String* s);



#endif // DYNAMIC_STRING_H
