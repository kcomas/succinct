
#pragma once

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint8_t c[4];
} utf8;

inline utf8 utf8_from_c_char(char c) {
    utf8 u = (utf8) { .c = { [0 ... 3] = '\0' } };
    u.c[0] = c;
    return u;
}
