
#include "infer.h"

extern inline infer_state *infer_state_init(parser_state *const ps);

extern inline void infer_state_free(infer_state *state);

const char *infer_status_string(infer_status status) {
    static const char *statuses[] = {
        "_START_INFER",
        "OK",
        "INVALID_NODE",
        "VAR_TYPE_NOT_FOUND",
        "INVALID_ASSIGN_LEFT_SIDE",
        "INVALID_LEFT_SIDE",
        "INVALID_RIGHT_SIDE",
        "NODE_TYPES_NOT_EQUAL",
        "INVALID_TYPE_FOR_OP",
        "CANNOT_GET_TYPE_FROM_NODE",
        "CANNOT_GET_CALL_TYPE",
        "CALL_NOT_ON_FN",
        "_END_INFER"
    };
    return status > INFER_STATUS_PFX(_START_INFER) && status < INFER_STATUS_PFX(_END_INFER) ? statuses[status]: "INFER_STATUS_NOT_FOUND";
}

extern inline infer_status infer_error(infer_state *const state, infer_status status, ast_node *const node);

static bool get_type_from_node(const ast_node *const node, var_type *const type) {
    var_type inner_type;
    if (node == NULL) return false;
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
        case AST_PFX(FN):
            var_type_copy(type, node->data.fn->type);
            return true;
        case AST_PFX(CALL):
            if (get_type_from_node(node->data.call->func, &inner_type) == false) return false;
            var_type_copy(type, &inner_type);
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
    // TODO check last stmt has the correct return type
    return INFER_STATUS_PFX(OK);
}

static infer_status infer_op_node_sides(infer_state *const state, ast_node *const node) {
    infer_status is;
    if (node->data.op->left == NULL) return INFER_STATUS_PFX(INVALID_LEFT_SIDE);
    if ((is = infer_node(state, node->data.op->left)) != INFER_STATUS_PFX(OK)) return is;
    if (node->data.op->right == NULL) return INFER_STATUS_PFX(INVALID_RIGHT_SIDE);
    if ((is = infer_node(state, node->data.op->right)) != INFER_STATUS_PFX(OK)) return is;
    return INFER_STATUS_PFX(OK);
}

static infer_status infer_node_with_equal_type_sides(infer_state *const state, ast_node *const node, bool (*type_check)(var_type_header)) {
    infer_status is;
    if ((is = infer_op_node_sides(state, node)) != INFER_STATUS_PFX(OK)) return infer_error(state, is, node);
    // check types are equal
    if (node_equal_types(node->data.op->left, node->data.op->right) == false)
        return infer_error(state, INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL), node);
    // set the type of the node
    node->data.op->return_type = var_type_init_from_node(node->data.op->left);
    // check the allowed types for op
    if (type_check(node->data.op->return_type->header) == false)
        return infer_error(state, INFER_STATUS_PFX(INVALID_TYPE_FOR_OP), node);
    return INFER_STATUS_PFX(OK);
}

static bool var_type_number_cmp(var_type_header header) {
    return var_type_is_number(header);
}

infer_status infer_node(infer_state *const state, ast_node *const node) {
    infer_status is;
    var_type type_check;
    switch (node->type) {
        case AST_PFX(VAR):
            if (node->data.var->type == NULL) return infer_error(state, INFER_STATUS_PFX(VAR_TYPE_NOT_FOUND), node);
            return INFER_STATUS_PFX(OK);
        case AST_PFX(INT):
        case AST_PFX(CHAR):
            return INFER_STATUS_PFX(OK);
        case AST_PFX(VEC):
            if (node->data.vec->type != NULL) return INFER_STATUS_PFX(OK);
            node->data.vec->type = var_type_vec_init(node->data.vec->num_items);
            if (node->data.vec->num_items > 0) {
                ast_node_link *head = node->data.vec->items_head;
                while (head != NULL) {
                    if (head->node != NULL) {
                        infer_status is;
                        if ((is = infer_node(state, head->node)) != INFER_STATUS_PFX(OK))
                            return infer_error(state, is, head->node);
                        var_type *item_type = var_type_init_from_node(head->node);
                        if (item_type == NULL)
                            return infer_error(state, INFER_STATUS_PFX(CANNOT_GET_TYPE_FROM_NODE), head->node);
                        node->data.vec->type->body.vec->items[node->data.vec->type->body.vec->len++] = item_type;
                    }
                    head = head->next;
                }
            } else {
                // TODO
            }
            return INFER_STATUS_PFX(OK);
        case AST_PFX(FN):
            return infer_fn(state, node->data.fn);
        case AST_PFX(CALL):
            // check for fn type and the correct num of args
            if (get_type_from_node(node->data.call->func, &type_check) == false)
                return infer_error(state, INFER_STATUS_PFX(CANNOT_GET_CALL_TYPE), node);
            if (type_check.header != VAR_PFX(FN)) return infer_error(state, INFER_STATUS_PFX(CALL_NOT_ON_FN), node);
            break;
        case AST_PFX(ASSIGN):
            // left must be a var
            if (node->data.op->left->type != AST_PFX(VAR))
                return infer_error(state, INFER_STATUS_PFX(INVALID_ASSIGN_LEFT_SIDE), node);
            if ((is = infer_node(state, node->data.op->right)) != INFER_STATUS_PFX(OK))
                return infer_error(state, is, node);
            if (node->data.op->left->data.var->type == NULL)
                node->data.op->left->data.var->type = var_type_init_from_node(node->data.op->right); // copy type
            else if (node_equal_types(node->data.op->left, node->data.op->right) == false)
                return infer_error(state, INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL), node); // types must be equal
            node->data.op->return_type = var_type_init(VAR_PFX(VOID), (var_type_body) {});
            return INFER_STATUS_PFX(OK);
        case AST_PFX(ADD):
        case AST_PFX(SUB):
            return infer_node_with_equal_type_sides(state, node, var_type_number_cmp);
        case AST_PFX(WRITE):
            // TODO
            break;
        case AST_PFX(EQUAL):
        case AST_PFX(LESSEQUAL):
            return infer_node_with_equal_type_sides(state, node, var_type_number_cmp);
        default:
            break;
    }
    return INFER_STATUS_PFX(INVALID_NODE);
}

infer_status infer(infer_state *const state) {
    return infer_fn(state, state->p->root_fn);
}
