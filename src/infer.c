
#include "infer.h"

extern inline infer_state *infer_state_init(parser_state *const ps);

extern inline void infer_state_free(infer_state *state);

extern inline infer_status infer_error(infer_state *const state, infer_status status, ast_node *const node);

static var_type *get_type_from_node(const ast_node *const node) {
    // complex types are passed by ref
    switch (node->type) {
        case AST_PFX(INT):
            return var_type_init(VAR_PFX(I64), (var_type_body) {});
        case AST_PFX(CHAR):
            return var_type_init(VAR_PFX(CHAR), (var_type_body) {});
        case VAR_PFX(FN): return node->data.fn->type;
        default:
            break;
    }
    return NULL;
}

static bool node_equal_types(const ast_node *const left, const ast_node *const right) {
    // TODO get the var type header for each side
    var_type_header left_side, right_side;
    return true;
}

static infer_status infer_fn(infer_state *const state, ast_fn_node *const fn) {
    ast_node_link* head = fn->body_head;
    while (head != NULL) {
        if (head->node != NULL) {
            infer_status is;
            if ((is = infer_node(state, head->node)) != INFER_STATUS_PFX(OK)) return infer_error(state, is, head->node);
        }
        head = head->next;
    }
    return INFER_STATUS_PFX(OK);
}

infer_status infer_node(infer_state *const state, ast_node *const node) {
    infer_status is;
    switch (node->type) {
        case AST_PFX(FN):
            return infer_fn(state, node->data.fn);
        case AST_PFX(ASSIGN):
            // left must be a var or item in indexable
            if (node->data.op->left->type != AST_PFX(VAR))
                return infer_error(state, INFER_STATUS_PFX(INVALID_ASSGIN_LEFT_SIDE), node);
            // infer right side
            if ((is = infer_node(state, node->data.op->right)) != INFER_STATUS_PFX(OK))
                return infer_error(state, is, node);
            if (node->data.op->left->data.var->type == NULL) {
                // copy type
                node->data.op->left->data.var->type = get_type_from_node(node->data.op->right);
            } else {
                // TODO check type of new assign must be same
            }
            return INFER_STATUS_PFX(OK);
        case AST_PFX(ADD):
        case AST_PFX(SUB):
            if (node->data.op->right == NULL) return infer_error(state, INFER_STATUS_PFX(INVALID_RIGHT_SIDE), node);
            if ((is = infer_node(state, node->data.op->right)) != INFER_STATUS_PFX(OK))
                return infer_error(state, is, node);
            if (node->data.op->left != NULL && (is = infer_node(state, node->data.op->left)) != INFER_STATUS_PFX(OK))
                return infer_error(state, is, node);
            // check the allowed types for op
            // check types are equal
            if (node->data.op->left != NULL && node_equal_types(node->data.op->right, node->data.op->left) == false)
                return infer_error(state, INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL), node);
            return INFER_STATUS_PFX(OK);
        default:
            break;
    }
    return INFER_STATUS_PFX(INVALID_NODE);
}

infer_status infer(infer_state *const state) {
    return infer_fn(state, state->p->root_fn);
}
