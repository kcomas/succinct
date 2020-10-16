
#pragma once

#include "token.h"
#include "type.h"
#include "error.h"
#include "file.h"

#define AST_PFX(NAME) AST_##NAME

typedef enum {
    // Data Types
    AST_PFX(UNKNOWN),
    AST_PFX(VAR),
    AST_PFX(INT),
    AST_PFX(CHAR),
    AST_PFX(FN),
    // UOP
    // BOP
    AST_PFX(ASSIGN)
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

typedef struct _ast_fn_node {
    var_type *type;
    struct _ast_fn_node *parent; // if null we are at the module level
    ast_node_link *list_head, *list_tail;
} ast_fn_node;

typedef union {
    ast_uop_node *uop;
    ast_bop_node *bop;
    ast_fn_node *fn;
    symbol_table_bucket *var;
} ast_data;

typedef struct _ast_node {
    ast_type type;
    ast_data data;
    token *t;
} ast_node;

typedef struct {
    ast_node *node;
} ast_node_holder;

inline ast_node *ast_node_init(ast_type type, const token *const t, ast_data data) {
    ast_node *node = calloc(1, sizeof(ast_node));
    node->type = type;
    node->t = token_copy(token_init(), t);
    node->data = data;
    return node;
}

void ast_node_free(ast_node *node);

inline ast_node_holder *ast_node_holder_init(void) {
    return calloc(1, sizeof(ast_node_holder));
}

inline void ast_node_holder_free(ast_node_holder *holder) {
    free(holder);
}

inline ast_fn_node *ast_fn_node_init(ast_fn_node *parent) {
    ast_fn_node *fn = calloc(1, sizeof(ast_fn_node));
    fn->type = var_type_fn_init(DEFAULT_SYMBOL_TABLE_SIZE);
    fn->parent = parent;
    return fn;
}

void ast_fn_node_free(ast_fn_node *fn);

#define PARSER_STATUS_PFX(NAME) PARSER_STATUS_##NAME

typedef enum {
    PARSER_STATUS_PFX(SOME),
    PARSER_STATUS_PFX(NONE),
    // System Error
    PARSER_STATUS_PFX(CANNOT_OPEN_FILE),
    PARSER_STATUS_PFX(CANNOT_READ_FILE),
    PARSER_STATUS_PFX(CANNOT_CLOSE_FILE)
    // Parser Error
} parser_status;

typedef struct {
    token *next, *peek;
    string *s;
    ast_fn_node *root_fn;
    error *e;
} parser_state;

inline parser_state *parser_state_init(void) {
    parser_state *state = calloc(1, sizeof(parser_state));
    state->next = token_init();
    state->peek = token_init();
    state->root_fn = ast_fn_node_init(NULL);
    state->e = error_init();
    // string is added on parse
    return state;
}

inline void parser_state_free(parser_state *state) {
    token_free(state->next);
    token_free(state->peek);
    if (state->s) string_free(state->s);
    ast_fn_node_free(state->root_fn);
    error_free(state->e);
    free(state);
}

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder *cur_node);

parser_status parse_module(parser_state *const state, const char *const filename);
