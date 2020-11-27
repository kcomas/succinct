
#pragma once

#include "token.h"
#include "type.h"
#include "error.h"
#include "file.h"

#define AST_PFX(NAME) AST_##NAME

typedef enum {
    // Data Types
    AST_PFX(_VALUE),
    AST_PFX(UNKNOWN),
    AST_PFX(VAR),
    AST_PFX(INT),
    AST_PFX(CHAR),
    AST_PFX(FN),
    AST_PFX(_END_VALUE),
    // OP
    AST_PFX(_OP),
    AST_PFX(ASSIGN),
    AST_PFX(ADD),
    AST_PFX(WRITE),
    AST_PFX(EQUAL),
    AST_PFX(LESSEQUAL),
    AST_PFX(_END_OP)
} ast_type;

typedef struct _ast_node ast_node;

typedef struct _ast_node_link {
    struct _ast_node_link *next;
    ast_node *node;
} ast_node_link;

typedef struct {
    var_type *return_type;
    ast_node *left, *right;
} ast_op_node;

typedef struct _ast_fn_node {
    var_type *type;
    struct _ast_fn_node *parent; // if null we are at the module level
    ast_node_link *body_head, *body_tail; // TODO empty link at end
} ast_fn_node;

typedef struct _ast_if_cond {
    struct _ast_if_cond *next;
    ast_node *cond;
    ast_node_link *body_head, *body_tail;
} ast_if_cond;

typedef struct {
    ast_if_cond *conds_head, *conds_tail;
    var_type *return_type; // all bodies must have same type if if is being assigned
    ast_node_link *else_head, *else_tail;
} ast_if_node;

typedef union {
    ast_op_node *op;
    ast_fn_node *fn;
    ast_if_node *cond;
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

const char *ast_type_string(ast_type type);

void ast_node_print_json(const ast_node *const node, const string *const s);

inline ast_node_link *ast_node_link_init(void) {
    return calloc(1, sizeof(ast_node_link));
}

inline void ast_node_link_free(ast_node_link *head) {
    while (head != NULL) {
        ast_node_free(head->node);
        ast_node_link *tmp = head;
        head = head->next;
        free(tmp);
    }
}

inline ast_node_holder *ast_node_holder_init(void) {
    return calloc(1, sizeof(ast_node_holder));
}

inline void ast_node_holder_free(ast_node_holder *holder) {
    free(holder);
}

inline ast_op_node *ast_op_node_init(void) {
    ast_op_node *op = calloc(1, sizeof(ast_op_node));
    op->return_type = var_type_init(VAR_PFX(UNKNOWN), (var_type_body) {});
    return op;
}

inline void ast_op_node_free(ast_op_node *op) {
    var_type_free(op->return_type);
    ast_node_free(op->left);
    ast_node_free(op->right);
    free(op);
}

inline ast_fn_node *ast_fn_node_init(ast_fn_node *parent) {
    ast_fn_node *fn = calloc(1, sizeof(ast_fn_node));
    fn->type = var_type_fn_init(DEFAULT_SYMBOL_TABLE_SIZE);
    fn->parent = parent;
    fn->body_head = ast_node_link_init();
    fn->body_tail = fn->body_head;
    return fn;
}

inline void ast_fn_node_free(ast_fn_node *fn, bool free_parent) {
    var_type_free(fn->type);
    // free links
    ast_node_link_free(fn->body_head);
    if (free_parent && fn->parent) ast_fn_node_free(fn->parent, free_parent);
    free(fn);

}

void ast_fn_node_print_json(const ast_fn_node *const fn, const string *const s);

inline ast_if_node *ast_if_node_init(void) {
    ast_if_node *if_node = calloc(1, sizeof(ast_if_node));
    if_node->return_type = var_type_init(VAR_PFX(UNKNOWN), (var_type_body) {});
    return if_node;
}

void ast_if_node_free(ast_if_node *if_node);

inline ast_if_cond *ast_if_cond_init(void) {
    ast_if_cond *cond = calloc(1, sizeof(ast_if_cond));
    cond->body_head = ast_node_link_init();
    cond->body_tail = cond->body_head;
    return cond;
}

#define PARSER_STATUS_PFX(NAME) PARSER_STATUS_##NAME

typedef enum {
    PARSER_STATUS_PFX(SOME),
    PARSER_STATUS_PFX(NONE), // none found
    PARSER_STATUS_PFX(DONE), // no more statements
    // System Error
    PARSER_STATUS_PFX(CANNOT_OPEN_FILE),
    PARSER_STATUS_PFX(CANNOT_READ_FILE),
    PARSER_STATUS_PFX(CANNOT_CLOSE_FILE),
    // Parser Error
    PARSER_STATUS_PFX(INVALID_TOKEN_SEQUENCE),
    PARSER_STATUS_PFX(INVALID_FINAL_VALUE)
} parser_status;

#define PARSER_MODE_PFX(NAME) PARSER_MODE_##NAME

typedef enum {
    PARSER_MODE_PFX(FN),
    PARSER_MODE_PFX(IF_COND),
    PARSER_MODE_PFX(IF_BODY)
} parser_mode;

typedef struct {
    parser_mode mode;
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
    ast_fn_node_free(state->root_fn, true);
    error_free(state->e);
    free(state);
}

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder *const head);

parser_status parse_stmts(parser_state *const state, ast_fn_node *const cur_fn, ast_node_link *tail);

parser_status parse_module(parser_state *const state, const char *const filename);
