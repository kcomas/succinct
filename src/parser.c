
#include "parser.h"

extern inline ast_node *ast_node_init(ast_type type, const token *const t, ast_data data);

void ast_fn_node_free(ast_fn_node *fn) {
    var_type_free(fn->type);
    // TODO free links
    if (fn->parent) ast_fn_node_free(fn->parent);
    free(fn);
}

extern inline ast_op_node *ast_op_node_init(void);

extern inline void ast_op_node_free(ast_op_node *op);

extern inline ast_fn_node *ast_fn_node_init(ast_fn_node *parent);

void ast_node_free(ast_node *node) {
    if (node == NULL) return;
    switch (node->type) {
        case  AST_PFX(FN):
            ast_fn_node_free(node->data.fn);
            break;
        case AST_PFX(ASSIGN):
            ast_op_node_free(node->data.op);
            break;
        default:
            break;
    }
    free(node);
}

extern inline ast_node_holder *ast_node_holder_init(void);

extern inline void ast_node_holder_free(ast_node_holder *holder);

extern inline parser_state *parser_state_init(void);

extern inline void parser_state_free(parser_state *state);

static ast_node_link *ast_node_link_init(ast_fn_node *const cur_fn) {
    ast_node_link *link = calloc(1, sizeof(ast_node_link));
    if (cur_fn->list_head == NULL) {
        cur_fn->list_head = link;
        cur_fn->list_tail = cur_fn->list_head;
    } else {
        cur_fn->list_tail->next = link;
        cur_fn->list_tail = cur_fn->list_tail->next;
    }
    return link;
}

static bool is_value(ast_node *const n) {
    return n->type > AST_PFX(_VALUE) && n->type < AST_PFX(_END_VALUE);
}

static bool is_op(ast_node *const n) {
    return n->type > AST_PFX(_OP) && n->type < AST_PFX(_END_OP);
}

static bool wire_nodes(ast_node_holder *const cur_node, ast_node_link *const link, ast_node *const n) {
    if (link->node == NULL && is_op(n)) link->node = n;
    if (cur_node->node == NULL) {
        cur_node->node = n;
    } else if (is_op(n)) {
        n->data.op->left = cur_node->node;
        cur_node->node = n;
    } else if (is_op(cur_node->node)) {
        cur_node->node->data.op->right = n;
        cur_node->node = n;
    } else if (is_value(cur_node->node) && is_value(n)) {
        return false;
    }
    return true;
}

static var_type *parse_var_type(parser_state* const state) {
    printf("Arg Type\n");
    return NULL;
}

static ast_fn_node *parse_fn(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder *const cur_node) {
    // we are at the thing after first (
    token_status ts;
    token arg_name;
    var_type *arg_type;
    // parse args
    while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
        // find arg name
        if (state->next->type != TOKEN_PFX(VAR)) {
            // TODO set error
            return NULL;
        }
        token_copy(&arg_name, state->next);
        if ((ts = token_next(state->next, state->s)) != TOKEN_STATUS_PFX(SOME)) {
            // TODO set error
            return NULL;
        }
        if (state->next->type != TOKEN_PFX(DEFINE)) {
            // TODO set error
            return NULL;
        }
        arg_type = parse_var_type(state);
    }
    // parse return
    // parse body
}

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder *const cur_node) {
    token_status ts;
    symbol_table_bucket *b;
    ast_node *n;
    ast_fn_node *fn;
    // init the fn node list
    ast_node_link *link = ast_node_link_init(cur_fn);
    while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
        switch (state->next->type) {
            case TOKEN_PFX(NEWLINE):
                if (cur_node == NULL) continue;
                break;
            case TOKEN_PFX(VAR):
                // TODO check if fn call
                // found var create bucket and node
                // TODO check parent fns for scope
                b = symbol_table_findsert(&cur_fn->type->body.fn->symbols, SYMBOL_PFX(LOCAL), state->next, state->s);
                n = ast_node_init(AST_PFX(VAR), state->next, (ast_data) { .var = b });
                break;
            case TOKEN_PFX(LBRACE):
                token_copy(state->peek, state->next);
                if ((ts = token_next(state->peek, state->s)) != TOKEN_STATUS_PFX(SOME)) {
                    // TODO error
                }
                if (state->peek->type == TOKEN_PFX(LPARENS)) {
                    // fn def
                    token_copy(state->next, state->peek);
                    if ((fn = parse_fn(state, ast_fn_node_init(cur_fn), ast_node_holder_init())) == NULL) {
                        // TODO error
                    }
                    n = ast_node_init(AST_PFX(FN), state->next, (ast_data) { .fn = fn });
                }
                break;
            case TOKEN_PFX(ASSIGN):
                n = ast_node_init(AST_PFX(ASSIGN), state->next, (ast_data) { .op = ast_op_node_init() });
                break;
            default:
                break;
        }
        if (n == NULL) break;
        if (!wire_nodes(cur_node, link, n)) return PARSER_STATUS_PFX(INVALID_WIRE_NODE);
    }
    ast_node_holder_free(cur_node);
    return PARSER_STATUS_PFX(NONE);
}

parser_status parse_module(parser_state *const state, const char *const filename) {
    int fd;
    if ((fd = file_open_r(filename)) == -1) {
        error_errno(state->e);
        return PARSER_STATUS_PFX(CANNOT_OPEN_FILE);
    }
    if ((state->s = file_read_to_string(fd)) == NULL) {
        error_errno(state->e);
        file_close(fd);
        return PARSER_STATUS_PFX(CANNOT_READ_FILE);
    }
    if (file_close(fd) == -1) {
        error_errno(state->e);
        string_free(state->s);
        return PARSER_STATUS_PFX(CANNOT_CLOSE_FILE);
    }
    return parse_stmt(state, state->root_fn, ast_node_holder_init());
}
