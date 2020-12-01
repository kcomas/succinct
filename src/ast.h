
#pragma once

#include "string.h"
#include "error.h"
#include "type.h"
#include "token.h"

#define AST_PFX(NAME) AST_##NAME

typedef enum {
    // Data Types
    AST_PFX(_VALUE),
    AST_PFX(UNKNOWN),
    AST_PFX(VAR),
    AST_PFX(INT),
    AST_PFX(CHAR),
    AST_PFX(FN),
    AST_PFX(CALL),
    AST_PFX(IF),
    AST_PFX(_END_VALUE),
    // OP
    AST_PFX(_OP),
    AST_PFX(ASSIGN),
    AST_PFX(ADD),
    AST_PFX(SUB),
    AST_PFX(WRITE),
    AST_PFX(EQUAL),
    AST_PFX(LESSEQUAL),
    AST_PFX(_END_OP)
} ast_type;

typedef struct _ast_node ast_node;

typedef struct _ast_node_link {
    struct _ast_node_link *next; // TODO possible empty link at end with newlines
    ast_node *node;
} ast_node_link;

typedef struct {
    var_type *return_type;
    ast_node *left, *right;
} ast_op_node;

typedef struct _ast_fn_node {
    var_type *type;
    struct _ast_fn_node *parent; // if null we are at the module level
    ast_node_link *body_head, *body_tail;
} ast_fn_node;

#define AST_MAX_ARGS 4

typedef struct {
    size_t num_args;
    ast_node *func;
    ast_node *args[];
} ast_call_node;

typedef struct _ast_if_cond {
    struct _ast_if_cond *next;
    ast_node *cond;
    ast_node_link *body_head, *body_tail;
} ast_if_cond;

typedef struct {
    var_type *return_type; // all bodies must have same type if if is being assigned
    ast_if_cond *conds_head, *conds_tail;
    ast_node_link *else_head, *else_tail;
} ast_if_node;

typedef union {
    ast_op_node *op;
    ast_fn_node *fn;
    ast_if_node *ifn;
    ast_call_node *call;
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

inline ast_call_node *ast_call_node_init(ast_node *const func, size_t num_args, ast_node *const args[]) {
    ast_call_node *c = calloc(1, sizeof(ast_call_node) + sizeof(ast_node*) * num_args);
    c->num_args = num_args;
    c->func = func;
    for (size_t i = 0; i < num_args; i++) c->args[i] = args[i];
    return c;
}

inline void ast_call_node_free(ast_call_node *c) {
    ast_node_free(c->func);
    for (size_t i = 0; i < c->num_args; i++) ast_node_free(c->args[i]);
    free(c);
}

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

bool is_value(ast_node *const n);

bool is_op(ast_node *const n);
