
#include "infer.h"

extern inline infer_state *infer_state_init(parser_state *const ps);

extern inline void infer_state_free(infer_state *state);

extern inline infer_status infer_error(infer_state *const state, infer_status status, ast_node *const node);

static bool get_type_from_node(const ast_node *const node, var_type *const type) {
    switch (node->type) {
        case AST_PFX(VAR):
            if (node->data.var->type == NULL) return false;
            var_type_copy(type, node->data.var->type);
            return true;
        case AST_PFX(INT):
            type->header = VAR_PFX(I64);
            return true;
        case AST_PFX(CHAR):
            type->header = VAR_PFX(CHAR);
            return true;
        default:
            if (is_op(node)) {
                if (node->data.op->return_type == NULL) return false;
                var_type_copy(type, node->data.op->return_type);
                return true;
            }
            return false;
    }
    return true;
}

static var_type *var_type_init_from_node(const ast_node *const node) {
    var_type src;
    if (get_type_from_node(node, &src) == false) return NULL;
    return var_type_init(src.header, src.body);
}

static bool node_equal_types(const ast_node *const left_node, const ast_node *const right_node) {
    // get the var type for each node
    var_type left, right;
    if (get_type_from_node(left_node, &left) == false) return false;
    if (get_type_from_node(right_node, &right) == false) return false;
    return var_type_equal(&left, &right);
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

static infer_status infer_node_with_equal_sides(infer_state *const state, ast_node *const node, bool (*type_check_fn)(var_type_header) ) {
    infer_status is;
    if (node->data.op->left == NULL) return infer_error(state, INFER_STATUS_PFX(INVALID_LEFT_SIDE), node);
    if ((is = infer_node(state, node->data.op->left)) != INFER_STATUS_PFX(OK))
        return infer_error(state, is, node);
    if (node->data.op->right == NULL) return infer_error(state, INFER_STATUS_PFX(INVALID_RIGHT_SIDE), node);
    if ((is = infer_node(state, node->data.op->right)) != INFER_STATUS_PFX(OK))
        return infer_error(state, is, node);
    // check types are equal
    if (node_equal_types(node->data.op->left, node->data.op->right) == false)
        return infer_error(state, INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL), node);
    // set the type of the node
    node->data.op->return_type = var_type_init_from_node(node->data.op->left);
    // check the allowed types for op
    if (type_check_fn(node->data.op->return_type->header) == false)
        return infer_error(state, INFER_STATUS_PFX(INVALID_TYPE_FOR_OP), node);
    return INFER_STATUS_PFX(OK);
}

static bool var_type_number_cmp(var_type_header header) {
    return var_type_is_number(header);
}

infer_status infer_node(infer_state *const state, ast_node *const node) {
    infer_status is;
    switch (node->type) {
        case AST_PFX(VAR):
            if (node->data.var->type == NULL) return infer_error(state, INFER_STATUS_PFX(VAR_TYPE_NOT_FOUND), node);
            return INFER_STATUS_PFX(OK);
        case AST_PFX(INT):
        case AST_PFX(CHAR):
            return INFER_STATUS_PFX(OK);
        case AST_PFX(VEC):
            // TODO
            break;
        case AST_PFX(FN):
            return infer_fn(state, node->data.fn);
        case AST_PFX(ASSIGN):
            // left must be a var or item in indexable
            if (node->data.op->left->type == AST_PFX(VAR)) {
                if ((is = infer_node(state, node->data.op->right)) != INFER_STATUS_PFX(OK))
                    return infer_error(state, is, node);
                if (node->data.op->left->data.var->type == NULL)
                    node->data.op->left->data.var->type = var_type_init_from_node(node->data.op->right); // copy type
                else if (node_equal_types(node->data.op->left, node->data.op->right) == false)
                    return infer_error(state, INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL), node); // types must be equal
            } else {
                return infer_error(state, INFER_STATUS_PFX(INVALID_ASSGIN_LEFT_SIDE), node);
            }
            return INFER_STATUS_PFX(OK);
        case AST_PFX(ADD):
        case AST_PFX(SUB):
            return infer_node_with_equal_sides(state, node, var_type_number_cmp);
        case AST_PFX(WRITE):
            // TODO
            break;
        case AST_PFX(EQUAL):
        case AST_PFX(LESSEQUAL):
            return infer_node_with_equal_sides(state, node, var_type_number_cmp);
        default:
            break;
    }
    return INFER_STATUS_PFX(INVALID_NODE);
}

infer_status infer(infer_state *const state) {
    return infer_fn(state, state->p->root_fn);
}
