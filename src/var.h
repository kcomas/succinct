
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "type.h"
#include "string.h"
#include "hash.h"

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
    bool is_ref; // cannot be reassigned
    var_type *type;
    var_data data;
} var;
