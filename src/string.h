
#pragma once

#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t size, len; // size is allways 1 greater then len, null terminated
    char buffer[];
} string;

inline string *string_init(size_t size) {
    string *s = calloc(1, sizeof(string) + size * sizeof(char) + sizeof(char));
    s->size = size + sizeof(char);
    return s;
}

inline string *string_copy(const string *const s) {
    string *copy = string_init(s->len);
    copy->len = s->len;
    memcpy(copy->buffer, s->buffer, copy->len);
    return copy;
}

inline void string_free(string *s) {
    free(s);
}
