
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

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
    VAR_PFX(TUPLE),
    VAR_PFX(VECTOR),
    VAR_PFX(STRUCT),
    VAR_PFX(HASH),
    VAR_PFX(FN),
    VAR_PFX(THREAD),
    VAR_PFX(FD)
} var_type;

typedef struct _var var;

typedef union {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    char ch[4];
    char *st;
    struct tm *date;
    struct timespec *ts;
    int fd;
} var_data;

typedef struct _var {
    var_type type;
    bool is_ref; // cannot be reassigned
    var_data data;
} var;
