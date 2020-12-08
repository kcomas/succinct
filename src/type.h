
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "def.h"
#include "token.h"

#define VAR_PFX(NAME) VAR_##NAME

typedef enum {
    VAR_PFX(_VAR_TYPE_HEADER),
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
    VAR_PFX(REGEX),
    VAR_PFX(_END_VAR_TYPE_HEADER)
} var_type_header;

const char *var_type_header_string(var_type_header header);

typedef struct _var_type var_type;

#define SYMBOL_PFX(NAME) SYMBOL_##NAME

typedef enum {
    SYMBOL_PFX(_SYMBOL_TYPE),
    SYMBOL_PFX(LOCAL),
    SYMBOL_PFX(ARG),
    SYMBOL_PFX(KEY),
    SYMBOL_PFX(_END_SYMBOL_TYPE)
} symbol_table_type;

const char *symbol_table_type_string(symbol_table_type type);

typedef struct _symbol_table_bucket {
    symbol_table_type table_type;
    size_t symbol_idx, size_len; // 1 + length for null term
    union {
        size_t stack, key; // absolute index of the stack, if hash with fixed keys get index of key
    } idx;
    struct _symbol_table_bucket *next;
    var_type *type;
    char symbol[];
} symbol_table_bucket;


typedef struct {
    size_t size, symbol_counter; // counter is used
    symbol_table_bucket *buckets[]; // list on collision
} symbol_table;

inline symbol_table *symbol_table_init(size_t size) {
    symbol_table *s = calloc(1, sizeof(symbol_table) + sizeof(symbol_table_bucket) * size);
    s->size = size;
    return s;
}

void symbol_table_free(symbol_table *s);

symbol_table_bucket *symbol_table_find(symbol_table *table, const token *const t, const string *const s);

symbol_table_bucket *_symbol_table_findsert(symbol_table **table, symbol_table_type type, const token *const t, const string *const s, bool insert_only);

inline symbol_table_bucket *symbol_table_insert(symbol_table **table, symbol_table_type type, const token *const t, const string *const s) {
    return _symbol_table_findsert(table, type, t, s, true);
}

inline symbol_table_bucket *symbol_table_findsert(symbol_table **table, symbol_table_type type, const token *const t, const string *const s) {
    return _symbol_table_findsert(table, type, t, s, false);
}

typedef struct {
    size_t num_args, num_locals;
    var_type *return_type;
    symbol_table* symbols;
} var_type_fn; // module has void return type and symbol table

typedef union {
    struct {
        size_t len; // 0 for dynamic
        var_type *dynamic, *items[]; // all items have dynamic type
    } *vec;
    struct {
        size_t len; // 0 for dynamic
        var_type *dynamic; // all keys have this type
        symbol_table *keys;
    } *hash;
    var_type_fn *fn;
} var_type_body;

typedef struct _var_type {
    var_type_header header;
    var_type_body body; // empty for all except for defined by union
} var_type;

inline var_type *var_type_init(var_type_header header, var_type_body body) {
    var_type *t = calloc(1, sizeof(var_type));
    t->header = header;
    t->body = body;
    return t;
}

void var_type_free(var_type *t);

inline var_type *var_type_fn_init(size_t symbol_table_size) {
    var_type_fn *fn = calloc(1, sizeof(var_type_fn));
    fn->return_type = var_type_init(VAR_PFX(UNKNOWN), (var_type_body) {});
    fn->symbols = symbol_table_init(symbol_table_size);
    return var_type_init(VAR_PFX(FN), (var_type_body) { .fn = fn });
}

inline void var_type_fn_free(var_type_fn *fn) {
    var_type_free(fn->return_type);
    symbol_table_free(fn->symbols);
    free(fn);
}
