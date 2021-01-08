
#include "infer.h"

extern inline infer_state *infer_state_init(parser_state *const ps);

extern inline void infer_state_free(infer_state *state);

const char *infer_status_string(infer_status status) {
    static const char *statuses[] = {
        "_START_INFER",
        "OK",
        "INVALID_NODE",
        "CANNOT_GET_TYPE_FROM_NODE",
        "VAR_TYPE_NOT_FOUND",
        "INVALID_VEC_ITEM",
        "INVALID_FN",
        "CANNOT_GET_CALL_TYPE",
        "CALL_NOT_ON_FN",
        "INVALID_NUM_OF_ARGS_IN_CALL",
        "INVALID_CALL_ARG",
        "CANNOT_GET_ARG_TYPE",
        "INVALID_ARG_TYPE",
        "INVALID_COND",
        "INVALID_IF_BODY",
        "INVALID_ASSIGN_LEFT_SIDE",
        "INVALID_ASSIGN_RIGHT_SIDE",
        "INVALID_CAST_LEFT_NODE",
        "INVALID_RAW_INT_FD",
        "INVALID_LEFT_SIDE",
        "INVALID_RIGHT_SIDE",
        "NODE_TYPES_NOT_EQUAL",
        "INVALID_TYPE_FOR_NODE",
        "_END_INFER"
    };
    return status > INFER_STATUS_PFX(_START_INFER) && status < INFER_STATUS_PFX(_END_INFER) ? statuses[status]: "INFER_STATUS_NOT_FOUND";
}

extern inline infer_status infer_error(infer_state *const state, infer_status status, ast_node *const node);

static bool get_type_from_node(const ast_node *const node, var_type *const type) {
    var_type inner_type;
    if (node == NULL) return false;
    switch (node->type) {
        case AST_PFX(TYPE):
            var_type_copy(type, node->data.type);
            return true;
        case AST_PFX(VAR):
            if (node->data.var->type == NULL || node->data.var->type->header == VAR_PFX(UNKNOWN)) return false;
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
            // get return type of the fn
            if (get_type_from_node(node->data.call->func, &inner_type) == false) return false;
            if (inner_type.header != VAR_PFX(FN)) return false;
            if (inner_type.body.fn == NULL) return false;
            var_type_copy(type, inner_type.body.fn->return_type);
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

static infer_status get_type_and_check(const ast_node *const node, var_type *const type, bool (*check_fn)(var_type_header header)) {
    if (get_type_from_node(node, type) == false)
        return INFER_STATUS_PFX(CANNOT_GET_TYPE_FROM_NODE);
    if (check_fn(type->header) == false)
        return INFER_STATUS_PFX(INVALID_TYPE_FOR_NODE);
    return INFER_STATUS_PFX(OK);
}

static bool node_equal_types(const ast_node *const left_node, const ast_node *const right_node) {
    // get the var type for each node
    var_type left, right;
    if (get_type_from_node(left_node, &left) == false) return false;
    if (get_type_from_node(right_node, &right) == false) return false;
    return var_type_equal(&left, &right);
}

static infer_status infer_node_list(infer_state *const state, ast_node_link *head, ast_node_link **const found_tail) {
    infer_status is;
    while (head != NULL) {
        if (head->node != NULL) {
            if ((is = infer_node(state, head->node)) != INFER_STATUS_PFX(OK)) return infer_error(state, is, head->node);
            *found_tail = head;
        } else {
            break;
        }
        head = head->next;
    }
    return INFER_STATUS_PFX(OK);
}

static infer_status infer_fn(infer_state *const state, ast_fn_node *const fn) {
    ast_node_link *found_tail = NULL, *head = fn->body_head;
    if (infer_node_list(state, head, &found_tail) != INFER_STATUS_PFX(OK)) return INFER_STATUS_PFX(INVALID_FN);
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
        return infer_error(state, INFER_STATUS_PFX(INVALID_TYPE_FOR_NODE), node);
    return INFER_STATUS_PFX(OK);
}

static bool var_type_number_cmp(var_type_header header) {
    return var_type_is_unsgined(header) || var_type_is_signed(header) || var_type_is_float(header);
}

static bool var_type_not_void(var_type_header header) {
    return header != VAR_PFX(VOID);
}

static bool var_type_write_left(var_type_header header) {
    return header == VAR_PFX(FD) || header == VAR_PFX(I64);
}

static bool var_type_cast_not_collection(var_type_header header) {
    return !var_type_is_collection(header);
}

infer_status infer_node(infer_state *const state, ast_node *const node) {
    infer_status is;
    var_type type_a;
    ast_if_cond *conds_head = NULL;
    ast_node_link *found_tail = NULL;
    switch (node->type) {
        case AST_PFX(TYPE):
            return INFER_STATUS_PFX(OK);
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
                        if (infer_node(state, head->node) != INFER_STATUS_PFX(OK))
                            return infer_error(state, INFER_STATUS_PFX(INVALID_VEC_ITEM), head->node);
                        var_type *item_type = var_type_init_from_node(head->node);
                        if (item_type == NULL)
                            return infer_error(state, INFER_STATUS_PFX(CANNOT_GET_TYPE_FROM_NODE), head->node);
                        node->data.vec->type->body.vec->items[node->data.vec->type->body.vec->len++] = item_type;
                    }
                    head = head->next;
                }
            } else {
                // TODO dynamic size fixed type
            }
            return INFER_STATUS_PFX(OK);
        case AST_PFX(FN):
            if (infer_fn(state, node->data.fn) != INFER_STATUS_PFX(OK))
                return infer_error(state, INFER_STATUS_PFX(INVALID_FN), node);
            return INFER_STATUS_PFX(OK);
        case AST_PFX(CALL):
            // check for fn type and the correct num of args
            if (get_type_from_node(node->data.call->func, &type_a) == false)
                return infer_error(state, INFER_STATUS_PFX(CANNOT_GET_CALL_TYPE), node);
            if (type_a.header != VAR_PFX(FN)) return infer_error(state, INFER_STATUS_PFX(CALL_NOT_ON_FN), node);
            if (node->data.call->num_args != type_a.body.fn->num_args)
                return infer_error(state, INFER_STATUS_PFX(INVALID_NUM_OF_ARGS_IN_CALL), node);
            // assert each arg/node has same fn arg type
            for (size_t i = 0; i < node->data.call->num_args; i++) {
                if (infer_node(state, node->data.call->args[i]) != INFER_STATUS_PFX(OK))
                    return infer_error(state, INFER_STATUS_PFX(INVALID_CALL_ARG), node->data.call->args[i]);
                if (get_type_from_node(node->data.call->args[i], &type_a) == false)
                    return infer_error(state, INFER_STATUS_PFX(CANNOT_GET_ARG_TYPE), node->data.call->args[i]);
                if (var_type_equal(&type_a, type_a.body.fn->args[i]->type) == false)
                    return infer_error(state, INFER_STATUS_PFX(INVALID_ARG_TYPE), node->data.call->args[i]);
            }
            return INFER_STATUS_PFX(OK);
        case AST_PFX(IF):
            conds_head = node->data.ifn->conds_head;
            node->data.ifn->return_type = var_type_init(VAR_PFX(UNKNOWN), (var_type_body) {});
            while (conds_head != NULL) {
                // infer cond
                if (infer_node(state, conds_head->cond) != INFER_STATUS_PFX(OK))
                    return infer_error(state, INFER_STATUS_PFX(INVALID_COND), node);
                // infer body
                if (infer_node_list(state, conds_head->body_head, &found_tail) != INFER_STATUS_PFX(OK))
                    return infer_error(state, INFER_STATUS_PFX(INVALID_IF_BODY), node);
                if (get_type_from_node(found_tail->node, &type_a) == false)
                    return infer_error(state, INFER_STATUS_PFX(CANNOT_GET_TYPE_FROM_NODE), found_tail->node);
                if (node->data.ifn->return_type->header == VAR_PFX(UNKNOWN)) // set type
                    var_type_copy(node->data.ifn->return_type, &type_a);
                else if (var_type_equal(node->data.ifn->return_type, &type_a) == false) // check type
                    return infer_error(state, INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL), node);
                conds_head = conds_head->next;
            }
            // infer else
            if (node->data.ifn->else_head != NULL) {
                if (infer_node_list(state, node->data.ifn->else_head, &found_tail) != INFER_STATUS_PFX(OK))
                    return infer_error(state, INFER_STATUS_PFX(INVALID_COND), node);
                if (get_type_from_node(found_tail->node, &type_a) == false)
                    return infer_error(state, INFER_STATUS_PFX(CANNOT_GET_TYPE_FROM_NODE), found_tail->node);
                if (var_type_equal(node->data.ifn->return_type, &type_a) == false)
                    return infer_error(state, INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL), node);
            }
            return INFER_STATUS_PFX(OK);
        case AST_PFX(ASSIGN):
            // left must be a var
            if (node->data.op->left->type != AST_PFX(VAR))
                return infer_error(state, INFER_STATUS_PFX(INVALID_ASSIGN_LEFT_SIDE), node);
            if (infer_node(state, node->data.op->right) != INFER_STATUS_PFX(OK))
                return infer_error(state, INFER_STATUS_PFX(INVALID_ASSIGN_RIGHT_SIDE), node);
            if (node->data.op->left->data.var->type == NULL)
                node->data.op->left->data.var->type = var_type_init_from_node(node->data.op->right); // copy type
            else if (node_equal_types(node->data.op->left, node->data.op->right) == false)
                return infer_error(state, INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL), node); // types must be equal
            node->data.op->return_type = var_type_init(VAR_PFX(VOID), (var_type_body) {});
            return INFER_STATUS_PFX(OK);
        case AST_PFX(CAST):
            if ((is = infer_op_node_sides(state, node)) != INFER_STATUS_PFX(OK)) return infer_error(state, is, node);
            // left must be node type or var with type
            if (node->data.op->left->type != AST_PFX(TYPE) && node->data.op->left->type != AST_PFX(VAR))
                return infer_error(state, INFER_STATUS_PFX(INVALID_CAST_LEFT_NODE), node->data.op->left);
            // can only cast to non collections
            if ((is = get_type_and_check(node->data.op->left, &type_a, var_type_cast_not_collection)) != INFER_STATUS_PFX(OK))
                return infer_error(state, is, node->data.op->left);
            // return type is the cast
            node->data.op->return_type = var_type_init(type_a.header, (var_type_body) {});
            // right can't be void
            if ((is = get_type_and_check(node->data.op->right, &type_a, var_type_not_void)) != INFER_STATUS_PFX(OK))
                return infer_error(state, is, node->data.op->right);
            return INFER_STATUS_PFX(OK);
        case AST_PFX(ADD):
        case AST_PFX(SUB):
            return infer_node_with_equal_type_sides(state, node, var_type_number_cmp);
        case AST_PFX(WRITE):
            if ((is = infer_op_node_sides(state, node)) != INFER_STATUS_PFX(OK)) return infer_error(state, is, node);
            // left must be a fd or int
            if ((is = get_type_and_check(node->data.op->left, &type_a, var_type_write_left)) != INFER_STATUS_PFX(OK))
                return infer_error(state, is, node->data.op->left);
            if (type_a.header == VAR_PFX(I64)) {
                // must be to 1 stdout or 2 stderr
                if (node->data.op->left->data.intv != 1 && node->data.op->left->data.intv != 2)
                    return infer_error(state, INFER_STATUS_PFX(INVALID_RAW_INT_FD), node->data.op->left);
            }
            // right side can be anything except void
            if ((is = get_type_and_check(node->data.op->right, &type_a, var_type_not_void)) != INFER_STATUS_PFX(OK))
                return infer_error(state, is, node->data.op->right);
            node->data.op->return_type = var_type_init(VAR_PFX(VOID), (var_type_body) {});
            return INFER_STATUS_PFX(OK);
        case AST_PFX(EQUAL):
        case AST_PFX(LESSEQUAL):
            return infer_node_with_equal_type_sides(state, node, var_type_number_cmp);
        default:
            break;
    }
    return infer_error(state, INFER_STATUS_PFX(INVALID_NODE), node);
}

infer_status infer(infer_state *const state) {
    return infer_fn(state, state->p->root_fn);
}
