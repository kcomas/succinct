
#pragma once

#include <stdlib.h>
#include <stdbool.h>

#define VAR_PFX(NAME) VAR_##NAME

typedef enum {
    VAR_PFX(UNKNOWN),
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
    VAR_PFX(DATETIME),
    VAR_PFX(VEC),
    VAR_PFX(HASH),
    VAR_PFX(FN),
    VAR_PFX(THREAD),
    VAR_PFX(FD),
    VAR_PFX(REGEX)
} var_type_header;

typedef struct _var_type var_type;

#define SYMBOL_PFX(NAME) SYMBOL_##NAME

typedef enum {
    SYMBOL_PFX(LOCAL),
    SYMBOL_PFX(ARG),
    SYMBOL_PFX(KEY)
} symbol_table_type;

typedef struct _symbol_table_bucket {
    symbol_table_type table_type;
    size_t symbol_num, symbol_size_len; // 1 + len for null term
    union {
        ssize_t stack, key; // -1 for unused, absolute idx of the stack, if hash with fixed keys get exact idx of key
    } idx;
    struct _symbol_table_bucket *next;
    var_type *type;
    char symbol[];
} symbol_table_bucket;

typedef struct {
    size_t size, used;
    symbol_table_bucket *buckets[]; // list on collision
} symbol_table;

inline symbol_table *symbol_table_init(size_t size) {
    symbol_table *s = calloc(1, sizeof(symbol_table) + sizeof(symbol_table_bucket) * size);
    s->size = size;
    return s;
}

void symbol_table_free(symbol_table *s);

typedef union {
    struct {
        size_t len; // 0 for dynamic
        var_type *dynamic, *items[]; // all items have dynamic type
    } vec;
    struct {
        size_t len; // 0 for dynamic
        var_type *dynamic; // all keys have this type
        symbol_table *keys;
    } hash;
    struct {
        size_t num_args, num_locals;
        var_type *return_type;
        symbol_table* symbols;
    } fn;
} var_type_body;

typedef struct _var_type {
    var_type_header header;
    bool is_ref;
    var_type_body *body; // NULL for all except for defined by union
} var_type;
