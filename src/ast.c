
#include "ast.h"

const char *ast_type_string(ast_type type) {
    static const char *types[] = {
        "_VALUE",
        "TYPE",
        "VAR",
        "INT",
        "CHAR",
        "VEC",
        "FN",
        "CALL",
        "IF",
        "_END_VALUE",
        "_OP",
        "ASSIGN",
        "CAST",
        "ADD",
        "SUB",
        "WRITE",
        "EQUAL",
        "LESSEQUAL",
        "_END_OP"
    };
    return type > AST_PFX(_VALUE) && type < AST_PFX(_END_OP) ? types[type] : "AST_TYPE_NOT_FOUND";
}

extern inline ast_node *ast_node_init(ast_type type, ast_data data, const token *const t);

void ast_node_free(ast_node *node) {
    switch (node->type) {
        case AST_PFX(VAR):
            // var is freed in the symbol table
            break;
        case AST_PFX(FN):
            ast_fn_node_free(node->data.fn);
            break;
        case AST_PFX(CALL):
            ast_call_node_free(node->data.call);
            break;
        case AST_PFX(IF):
            ast_if_node_free(node->data.ifn);
            break;
        case AST_PFX(VEC):
            ast_vec_node_free(node->data.vec);
            break;
        default:
            if (is_op(node) == true) ast_op_node_free(node->data.op);
            break;
    }
    token_free(node->t);
    free(node);
}

extern inline ast_node_link *ast_node_link_init(void);

extern inline void ast_node_link_free(ast_node_link *head);

extern inline ast_vec_node *ast_vec_node_init(void);

extern inline void ast_vec_node_free(ast_vec_node *v);

extern inline ast_op_node *ast_op_node_init(void);

extern inline void ast_op_node_free(ast_op_node *op);

extern inline ast_fn_node *ast_fn_node_init(ast_fn_node *parent);

extern inline void ast_fn_node_free(ast_fn_node *fn);

extern inline ast_call_node *ast_call_node_init(ast_node *const func, size_t num_args, ast_node *const args[]);

extern inline void ast_call_node_free(ast_call_node *c);

extern inline ast_if_node *ast_if_node_init(void);

void ast_if_node_free(ast_if_node *if_node) {
    var_type_free(if_node->return_type);
    ast_if_cond *head = if_node->conds_head;
    while (head != NULL) {
        ast_node_free(head->cond);
        ast_node_link_free(head->body_head);
        ast_if_cond *tmp = head;
        head = head->next;
        free(tmp);
    }
    ast_node_link_free(if_node->else_head);
    free(if_node);
}

extern inline ast_if_cond *ast_if_cond_init(void);

extern inline ast_node_holder *ast_node_holder_init(void);

extern inline void ast_node_holder_free(ast_node_holder *holder);

bool is_value(const ast_node *const n) {
    return n->type > AST_PFX(_VALUE) && n->type < AST_PFX(_END_VALUE);
}

bool is_op(const ast_node *const n) {
    return n->type > AST_PFX(_OP) && n->type < AST_PFX(_END_OP);
}
