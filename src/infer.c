
#include "infer.h"

extern inline infer_state *infer_state_init(parser_state *const ps);

extern inline void infer_state_free(infer_state *state);

extern inline infer_status infer_error(infer_state *const state, infer_status status, ast_node *const node);

static infer_status infer_node(infer_state *const state, ast_node *const node) {
    switch (node->type) {
        case AST_PFX(ASSIGN):
            // left must be a var
            if (node->data.op->left->type != AST_PFX(VAR)) {
                // TODO error
            }
            break;
        default:
            break;
    }
    return INFER_STATUS_PFX(INVALID_NODE);
}

static infer_status infer_fn(infer_state *const state, ast_fn_node *const fn) {
    ast_node_link* head = fn->body_head;
    while (head != NULL) {
        if (head->node != NULL) {
           infer_status is = infer_node(state, head->node);
           if (is != INFER_STATUS_PFX(OK)) {
                // TODO error
                return is;
           }
        }
        head = head->next;
    }
    return INFER_STATUS_PFX(OK);
}

infer_status infer(infer_state *const state) {
    return infer_fn(state, state->p->root_fn);
}
