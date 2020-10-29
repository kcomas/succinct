
#include "parser.h"

extern inline ast_node *ast_node_init(ast_type type, const token *const t, ast_data data);

void ast_fn_node_free(ast_fn_node *fn) {
    var_type_free(fn->type);
    // TODO free links
    if (fn->parent) ast_fn_node_free(fn->parent);
    free(fn);
}

extern inline ast_node_link *ast_node_link_init(void);

extern inline void ast_node_link_free(ast_node_link *link);

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

static bool is_value(ast_node *const n) {
    return n->type > AST_PFX(_VALUE) && n->type < AST_PFX(_END_VALUE);
}

static bool is_op(ast_node *const n) {
    return n->type > AST_PFX(_OP) && n->type < AST_PFX(_END_OP);
}

static token_status token_next_check(parser_state *const state, token_type type) {
    token_status ts;
    if ((ts = token_next(state->next, state->s)) != TOKEN_STATUS_PFX(SOME)) return ts;
    if (state->next->type != type) return TOKEN_STATUS_PFX(INVALID_MATCH);
    return ts;
}

static token_status token_peek_check(parser_state *const state, token_type type) {
    token_status ts;
    token_copy(state->peek, state->next);
    if ((ts = token_next(state->peek, state->s)) != TOKEN_STATUS_PFX(SOME)) return ts;
    if (state->peek->type == type) {
        token_copy(state->next, state->peek);
        return TOKEN_STATUS_PFX(PEEK_SOME);
    }
    return TOKEN_STATUS_PFX(NONE);
}

static var_type *parse_var_type(parser_state* const state) {
    token_status ts;
    if ((ts = token_next(state->next, state->s)) != TOKEN_STATUS_PFX(SOME)) {
        // TODO error
        return NULL;
    }
    switch (state->next->type) {
        case TOKEN_PFX(U64): return var_type_init(VAR_PFX(U64), (var_type_body) {});
        default:
            break;
    }
    return NULL;
}

static ast_fn_node *parse_fn(parser_state *const state, ast_fn_node *const cur_fn) {
    // we are at the first maybe var after first (
    token_status ts;
    // parse args
    while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
        // find arg name
        if (state->next->type != TOKEN_PFX(VAR)) {
            // TODO set error
            return NULL;
        }
        token arg_name;
        token_copy(&arg_name, state->next);
        if ((ts = token_next_check(state, TOKEN_PFX(DEFINE))) != TOKEN_STATUS_PFX(SOME)) {
            // TODO set error
            return NULL;
        }
        var_type *arg_type = parse_var_type(state);
        if (arg_type == NULL) {
            // TODO error
            return NULL;
        }
        symbol_table_bucket *b = symbol_table_findsert(&cur_fn->type->body.fn->symbols, SYMBOL_PFX(ARG), &arg_name, state->s);
        b->type = arg_type;
        // TODO inc arg count
        // check if another arg or done
        ts = token_peek_check(state, TOKEN_PFX(SEPRATOR));
        if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
            continue; // another arg
        } else if (ts != TOKEN_STATUS_PFX(SOME)) {
            // TODO error
            return NULL;
        }
        ts = token_peek_check(state, TOKEN_PFX(RPARENS));
        if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
            break; // done with args
        } else if (ts != TOKEN_STATUS_PFX(SOME)) {
            // TODO error
            return NULL;
        }
    }
    // parse return
    if ((ts = token_next_check(state, TOKEN_PFX(LBRACKET))) != TOKEN_STATUS_PFX(SOME)) {
        // TODO set error
        return NULL;
    }
    cur_fn->type->body.fn->return_type = parse_var_type(state);
    if ((ts = token_next_check(state, TOKEN_PFX(RBRACKET))) != TOKEN_STATUS_PFX(SOME)) {
        // TODO set error
        return NULL;
    }
    // parse body
    parser_status status = parse_stmts(state, cur_fn, cur_fn->list_tail);
    if (status != PARSER_STATUS_PFX(SOME)) {
        // TODO error
        return NULL;
    }
    return cur_fn;
}

static parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder* const head) {
    token_status ts;
    symbol_table_bucket *b;
    ast_node *n;
    ast_node *cur_node = NULL;
    ast_node *value_tmp = NULL;
    ast_fn_node *fn, *parent;
    // init the fn node list
    while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
        switch (state->next->type) {
            case TOKEN_PFX(NEWLINE):
                if (value_tmp != NULL && is_op(cur_node)) {
                    cur_node->data.op->right = value_tmp;
                } else if (value_tmp != NULL) {
                    // TODO error
                }
                return PARSER_STATUS_PFX(SOME);
            case TOKEN_PFX(VAR):
                // TODO check if fn call
                // found var create bucket and node
                // check parent fns for scope
                parent = cur_fn->parent;
                while (parent != NULL) {
                    b = symbol_table_find(parent->type->body.fn->symbols, state->next, state->s);
                    if (b != NULL) break;
                    parent = parent->parent;
                }
                // not in parent add to cur
                if (parent == NULL)
                    b = symbol_table_findsert(&cur_fn->type->body.fn->symbols, SYMBOL_PFX(LOCAL), state->next, state->s);
                    // TODO inc local count
                n = ast_node_init(AST_PFX(VAR), state->next, (ast_data) { .var = b });
                break;
            case TOKEN_PFX(INT):
                n = ast_node_init(AST_PFX(INT), state->next, (ast_data) {});
                break;
            case TOKEN_PFX(LBRACE):
                ts = token_peek_check(state, TOKEN_PFX(LPARENS));
                if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
                    if ((fn = parse_fn(state, ast_fn_node_init(cur_fn))) == NULL) {
                        // TODO error
                    }
                    n = ast_node_init(AST_PFX(FN), state->next, (ast_data) { .fn = fn });
                } else if (ts != TOKEN_STATUS_PFX(SOME)) {
                    // TODO error
                }
                break;
            case TOKEN_PFX(ASSIGN):
                n = ast_node_init(AST_PFX(ASSIGN), state->next, (ast_data) { .op = ast_op_node_init() });
                break;
            case TOKEN_PFX(ADD):
                n = ast_node_init(AST_PFX(ADD), state->next, (ast_data) { .op = ast_op_node_init() });
                break;
            case TOKEN_PFX(WRITE):
                n = ast_node_init(AST_PFX(WRITE), state->next, (ast_data) { .op = ast_op_node_init() });
                break;
            case TOKEN_PFX(COND):
                ts = token_peek_check(state, TOKEN_PFX(LBRACE));
                if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
                    // if stmt
                } else if (ts != TOKEN_STATUS_PFX(SOME)) {
                    // TODO error
                }
                break;
            default:
                break;
        }
        if (n == NULL) break;
        if (is_value(n) && cur_node == NULL) {
            cur_node = n;
        } else if (is_op(n) && head->node == NULL) {
            n->data.op->left = cur_node;
            head->node = n;
            cur_node = n;
        } else if (is_value(n) && value_tmp == NULL) {
            value_tmp = n;
        } else if (is_op(cur_node) && is_op(n)) {
            n->data.op->left = value_tmp;
            value_tmp = NULL;
            cur_node->data.op->right = n;
            cur_node = n;
        } else {
            return PARSER_STATUS_PFX(INVALID_TOKEN_SEQUENCE);
        }
    }
    // newline or ; denote success
    return PARSER_STATUS_PFX(NONE);
}

parser_status parse_stmts(parser_state *const state, ast_fn_node *const cur_fn, ast_node_link *tail) {
    ast_node_holder *holder = ast_node_holder_init();
    while (parse_stmt(state, cur_fn, holder) == PARSER_STATUS_PFX(SOME)) {
        tail->node = holder->node;
        holder->node = NULL;
        tail->next = ast_node_link_init();
        tail = tail->next;
    }
    ast_node_holder_free(holder);
    return PARSER_STATUS_PFX(DONE);
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
    return parse_stmts(state, state->root_fn, state->root_fn->list_tail);
}
