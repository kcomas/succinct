
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

typedef struct _ast_node ast_node;

typedef struct _ast_node_link {
    struct _ast_node_link *next;
    ast_node *node;
} ast_node_link;

typedef struct {
    var_type *return_type;
    ast_node *right;
} ast_uop_node;

typedef struct {
    var_type *return_type;
    ast_node *left, *right;
} ast_bop_node;

typedef struct {
    var_type *type;
    ast_node *parent; // if null we are at the module level
    ast_node_link *node_list;
} ast_fn_node;

typedef struct _ast_node {
    ast_type type;
    union {
        ast_uop_node *uop;
        ast_bop_node *bop;
        ast_fn_node *fn;
        symbol_table_bucket *var_arg;
    } data;
} ast_node;

inline ast_fn_node *ast_fn_node_init(ast_node *parent) {
    ast_fn_node *fn = calloc(1, sizeof(ast_fn_node));
    fn->type = var_type_fn_init(DEFAULT_MODULE_SYMBOLE_TABLE_SIZE);
    fn->parent = parent;
    return fn;
}

#define PARSER_STATUS_PFX(NAME) PARSER_STATUS_##NAME

typedef enum {
    PARSER_STATUS_PFX(OK)
} parser_status;

typedef struct {
    token next, peek;
    ast_fn_node *root_fn;
} parser_state;

inline parser_state *parser_state_init(void) {
    parser_state *state = calloc(1, sizeof(parser_state));
    token_init(&state->next);
    token_init(&state->peek);
    state->root_fn = ast_fn_node_init(NULL);
    return state;
}

parser_status parse_string(parser_state *const state, const string *const s);
