
#pragma once

#include <stdlib.h>
#include "token.h"
#include "type.h"

#define AST_PFX(NAME) AST_##NAME

typedef enum {
    AST_PFX(VAR),
    AST_PFX(INT),
    AST_PFX(CHAR),
    AST_PFX(FN),
} ast_type;

#define SYMBOL_PFX(NAME) SYMBOL_##NAME

typedef enum {
    SYMBOL_PFX(LOCAL),
    SYMBOL_PFX(ARG)
} symbol_table_type;

typedef struct _symbol_table_node {
    symbol_table_type type;
    size_t symbol_num, symbol_len;
    struct _symbol_table_node *next;
    char symbol[];
} symbol_table_bucket;

typedef struct {
    size_t size, used;
    symbol_table_bucket *buckets[];
} symbol_table;

inline symbol_table *symbol_table_init(size_t size) {
    symbol_table *s = calloc(1, sizeof(symbol_table) + sizeof(symbol_table_bucket) * size);
    s->size = size;
    return s;
}

void symbol_table_free(symbol_table *s);

typedef struct _ast_node ast_node;

typedef struct _ast_node_link {
    struct _ast_node_link *next;
    ast_node *node;
} ast_node_link;

typedef struct {
    ast_type ast_type;
    var_type *return_type;
    ast_node *left, *right;
} ast_bop_node;

typedef struct {
    size_t num_args, num_locals;
    symbol_table *symbols;
    ast_node_link *node_list;
} ast_fn_node;

#define PARSER_STATUS_PFX(NAME) PARSER_STATUS_##NAME

typedef enum {
    PARSER_STATUS_PFX(OK)
} parser_status;
