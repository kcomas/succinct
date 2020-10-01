
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "string.h"
#include "hash.h"

#define VAR_PFX(NAME) VAR_##NAME

typedef enum {
    VAR_PFX(VOID),
    VAR_PFX(U8),
    VAR_PFX(U16),
    VAR_PFX(U32),
    VAR_PFX(U64),
    VAR_PFX(I8),
    VAR_PFX(I16),
    VAR_PFX(I32),
    VAR_PFX(I64),
    VAR_PFX(F32),
    VAR_PFX(F64),
    VAR_PFX(CHAR),
    VAR_PFX(STRING),
    VAR_PFX(DATE),
    VAR_PFX(TIME),
    VAR_PFX(DATETIME),
    VAR_PFX(VEC),
    VAR_PFX(HASH),
    VAR_PFX(FN),
    VAR_PFX(THREAD),
    VAR_PFX(FD),
    VAR_PFX(REGEX)
} var_type;

typedef union {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    char chr[4];
    string *str;
    struct tm *date;
    struct timespec *ts;
    hash *h;
    int fd;
} var_data;

typedef struct _var {
    var_type type;
    bool is_ref; // cannot be reassigned
    var_data data;
} var;

inline var *var_init(var_type type, bool is_ref, var_data data) {
    var *v = calloc(1, sizeof(var));
    v->type = type;
    v->is_ref = is_ref;
    v->data = data;
    return v;
}

var *var_copy(const var *const v);
