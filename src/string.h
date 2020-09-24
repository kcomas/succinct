
#pragma once

#include <stdlib.h>

typedef struct {
    size_t size, len; // size is allways 1 greater then len, null terminated
    char buffer[];
} string;

inline string *string_init(size_t size) {
    string *s = calloc(1, sizeof(string) + size * sizeof(char) + sizeof(char));
    s->size = size;
    return s;
}

inline void string_free(string *s) {
    free(s);
}
