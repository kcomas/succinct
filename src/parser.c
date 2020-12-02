
#include "parser.h"

extern inline parser_state *parser_state_init(void);

extern inline void parser_state_free(parser_state *state);

extern inline bool parser_mode_push(parser_state *const state, parser_mode mode);

extern inline bool parser_mode_pop(parser_state *const state);

extern inline parser_mode parser_mode_get(parser_state *const state);

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

    return ts;
}

static parser_status wire_final_value(ast_node *const value_tmp, ast_node *const cur_node, parser_status ret_type) {
    if (is_op(cur_node) && value_tmp != NULL) {
        if (cur_node->data.op->right == NULL) {
            cur_node->data.op->right = value_tmp;
        } else {
            // TODO error
            return PARSER_STATUS_PFX(INVALID_FINAL_VALUE);
        }
    } else if (value_tmp != NULL) {
        // TODO error
        return PARSER_STATUS_PFX(INVALID_FINAL_VALUE);
    }
    return ret_type;
}

static ast_node* make_op(const parser_state *const state, ast_type type) {
    return ast_node_init(type, state->next, (ast_data) { .op = ast_op_node_init() });
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

static ast_node *parse_check_call(parser_state *const state, ast_fn_node *const cur_fn, ast_node *const func) {
    // checks for fn call returns the assumed func with no error if not found
    token_status ts = token_peek_check(state, TOKEN_PFX(LPARENS));
    if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
        ast_node *ast_args[AST_MAX_ARGS];
        if (parser_mode_push(state, PARSER_MODE_PFX(FN_ARGS)) == false) {
            // TODO error
            return NULL;
        }
        ast_node_holder *holder = ast_node_holder_init();
        size_t num_args = 0;
        // at first arg
        while (num_args < AST_MAX_ARGS) {
            if (num_args >= AST_MAX_ARGS) {
                // TODO error
                return NULL;
            }
            parser_status ps;
            if ((ps = parse_stmt(state, cur_fn, holder)) != PARSER_STATUS_PFX(SOME)) {
                // TODO error
                return NULL;
            }
            ast_args[num_args++] = holder->node;
            holder->node = NULL;
        }
        ast_node_holder_free(holder);
        if (parser_mode_pop(state) == false) {
            // TODO error
            return false;
        }
        return ast_node_init(AST_PFX(CALL), state->next, (ast_data) { .call = ast_call_node_init(func, num_args, ast_args) });
    } else if (ts != TOKEN_STATUS_PFX(SOME)) {
        // TODO error
        return NULL;
    }
    return func;
}

static ast_fn_node *parse_fn(parser_state *const state, ast_fn_node *const parent_fn) {
    // we are at the first maybe var after first (
    ast_fn_node *cur_fn = ast_fn_node_init(parent_fn);
    token_status ts;
    parser_status ps;
    // parse args
    while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
        // find arg name
        if (state->next->type != TOKEN_PFX(VAR)) {
            // TODO set error
            ast_fn_node_free(cur_fn, false);
            return NULL;
        }
        token arg_name;
        token_copy(&arg_name, state->next);
        if ((ts = token_next_check(state, TOKEN_PFX(DEFINE))) != TOKEN_STATUS_PFX(SOME)) {
            // TODO set error
            ast_fn_node_free(cur_fn, false);
            return NULL;
        }
        var_type *arg_type = parse_var_type(state);
        if (arg_type == NULL) {
            // TODO error
            ast_fn_node_free(cur_fn, false);
            return NULL;
        }
        symbol_table_bucket *b = symbol_table_findsert(&cur_fn->type->body.fn->symbols, SYMBOL_PFX(ARG), &arg_name, state->s);
        b->type = arg_type;
        // inc arg count
        cur_fn->type->body.fn->num_args++;
        // check if another arg or done
        ts = token_peek_check(state, TOKEN_PFX(SEPRATOR));
        if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
            continue; // another arg
        } else if (ts != TOKEN_STATUS_PFX(SOME)) {
            // TODO error
            ast_fn_node_free(cur_fn, false);
            return NULL;
        }
        ts = token_peek_check(state, TOKEN_PFX(RPARENS));
        if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
            break; // done with args
        } else if (ts != TOKEN_STATUS_PFX(SOME)) {
            // TODO error
            ast_fn_node_free(cur_fn, false);
            return NULL;
        }
    }
    // parse return
    if ((ts = token_next_check(state, TOKEN_PFX(LBRACKET))) != TOKEN_STATUS_PFX(SOME)) {
        // TODO set error
        ast_fn_node_free(cur_fn, false);
        return NULL;
    }
    cur_fn->type->body.fn->return_type = parse_var_type(state);
    if ((ts = token_next_check(state, TOKEN_PFX(RBRACKET))) != TOKEN_STATUS_PFX(SOME)) {
        // TODO set error
        ast_fn_node_free(cur_fn, false);
        return NULL;
    }
    // parse body
    if ((ps = parse_stmts(state, cur_fn, cur_fn->body_tail)) != PARSER_STATUS_PFX(DONE)) {
        // TODO error
        ast_fn_node_free(cur_fn, false);
        return NULL;
    }
    return cur_fn;
}

static ast_if_node *parse_if(parser_state *const state, ast_fn_node *const cur_fn) {
    // we are at the fist cond newline or (
    ast_if_node *if_node = ast_if_node_init();
    token_status ts;
    parser_status ps;
    bool in_else = false;
    ast_if_cond *cond_node;
    ast_node_holder *cond_holder;
    for (;;) {
        // parse cond
        cond_node = NULL;
        while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME))
            if (state->next->type != TOKEN_PFX(NEWLINE)) break; // before cond remove newline
        if (state->next->type == TOKEN_PFX(LBRACE)) {
            // reached else stmt
            in_else = true;
        } else if (state->next->type != TOKEN_PFX(LPARENS)) {
            // TODO error
            ast_if_node_free(if_node);
            return NULL;
        } else {
            if (parser_mode_push(state, PARSER_MODE_PFX(IF_COND)) == false) {
                // TODO error
                return NULL;
            }
            cond_holder = ast_node_holder_init();
            if ((ps = parse_stmt(state, cur_fn, cond_holder)) != PARSER_STATUS_PFX(DONE)) {
                // TODO error
                ast_if_node_free(if_node);
                return NULL;
            }
            cond_node = ast_if_cond_init();
            cond_node->cond = cond_holder->node;
            cond_holder->node = NULL;
            if (parser_mode_pop(state) == false) {
                // TODO error
                return NULL;
            }
        }
        // TODO error
        // parse body
        while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME))
            if (in_else == true || state->next->type != TOKEN_PFX(NEWLINE)) break; // before body remove newline
        if (parser_mode_push(state, PARSER_MODE_PFX(IF_BODY)) == false) {
            // TODO error
            return NULL;
        }
        if (in_else == true) {
            if (if_node->conds_head == NULL) {
                // TODO error
                // cannot have if stmt with no conds
                ast_if_node_free(if_node);
                return NULL;
            }
            // create else link
            if_node->else_head = ast_node_link_init();
            if_node->else_tail = if_node->else_head;
            if ((ps = parse_stmts(state, cur_fn, if_node->else_tail)) != PARSER_STATUS_PFX(DONE)) {
                // TODO error
                ast_if_node_free(if_node);
                exit(ps);
                return NULL;
            }
            exit(52);
        } else {
            if (state->next->type != TOKEN_PFX(LBRACE)) {
                // TODO error
                ast_if_node_free(if_node);
                return NULL;
            }
            if ((ps = parse_stmts(state, cur_fn, cond_node->body_tail)) != PARSER_STATUS_PFX(DONE)) {
                // TODO error
                ast_if_node_free(if_node);
                return NULL;
            }
        }
        if (parser_mode_pop(state) == false) {
            // TODO error
            return NULL;
        }
        // TODO error
        // attach cond node to if node
        if (if_node->conds_head == NULL) {
            if_node->conds_head = cond_node;
            if_node->conds_tail = if_node->conds_head;
        } else {
            if_node->conds_tail->next = cond_node;
            if_node->conds_tail = if_node->conds_tail->next;
        }
        // outside body possible next cond or end of if
    }
    return if_node;
}

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder *const head) {
    token_status ts;
    symbol_table_bucket *b = NULL;
    ast_node *n;
    ast_node *value_tmp = NULL;
    ast_node *cur_node = NULL;
    ast_fn_node *fn, *parent;
    ast_if_node *if_node;
    // init the fn node list
    while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
        switch (state->next->type) {
            case TOKEN_PFX(NEWLINE):
                if (cur_node == NULL) continue;
                return wire_final_value(value_tmp, cur_node, PARSER_STATUS_PFX(SOME));
            case TOKEN_PFX(SEPRATOR):
                break;
            case TOKEN_PFX(VAR):
                // found var create bucket and node
                // check if exists in current scope
                parent = cur_fn;
                while (parent != NULL) {
                    b = symbol_table_find(parent->type->body.fn->symbols, state->next, state->s);
                    if (b != NULL) break;
                    parent = parent->parent;
                }
                // not in parent add to cur
                if (b == NULL) {
                    b = symbol_table_findsert(&cur_fn->type->body.fn->symbols, SYMBOL_PFX(LOCAL), state->next, state->s);
                    // inc local count
                    cur_fn->type->body.fn->num_locals++;
                    n = ast_node_init(AST_PFX(VAR), state->next, (ast_data) { .var = b });
                } else {
                    // check if fn call
                    n = parse_check_call(state, cur_fn, ast_node_init(AST_PFX(VAR), state->next, (ast_data) { .var = b }));
                }
                b = NULL;
                break;
            case TOKEN_PFX(INT):
                n = ast_node_init(AST_PFX(INT), state->next, (ast_data) {});
                break;
            case TOKEN_PFX(LBRACE):
                ts = token_peek_check(state, TOKEN_PFX(LPARENS));
                if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
                    if ((fn = parse_fn(state, cur_fn)) == NULL) {
                        // TODO error
                    }
                    n = ast_node_init(AST_PFX(FN), state->next, (ast_data) { .fn = fn });
                } else if (ts != TOKEN_STATUS_PFX(SOME)) {
                    // TODO error
                }
                break;
            case TOKEN_PFX(RBRACE):
                if (parser_mode_get(state) == PARSER_MODE_PFX(IF_BODY) || parser_mode_get(state) == PARSER_MODE_PFX(FN))
                    return wire_final_value(value_tmp, cur_node, PARSER_STATUS_PFX(DONE));
                break;
            case TOKEN_PFX(RPARENS):
                if (parser_mode_get(state) == PARSER_MODE_PFX(IF_COND) || parser_mode_get(state) == PARSER_MODE_PFX(FN_ARGS))
                    return wire_final_value(value_tmp, cur_node, PARSER_STATUS_PFX(DONE));
                break;
            case TOKEN_PFX(ASSIGN):
                // TODO fn call
                n = make_op(state, AST_PFX(ASSIGN));
                break;
            case TOKEN_PFX(ADD):
                // TODO fn call
                n = make_op(state, AST_PFX(ADD));
                break;
            case TOKEN_PFX(SUB):
                // TODO fn call
                n = make_op(state, AST_PFX(SUB));
                break;
            case TOKEN_PFX(WRITE):
                // TODO fn call
                n = make_op(state, AST_PFX(WRITE));
                break;
            case TOKEN_PFX(EQUAL):
                // TODO fn call
                n = make_op(state, AST_PFX(EQUAL));
                break;
            case TOKEN_PFX(LESSEQUAL):
                // TODO fn call
                n = make_op(state, AST_PFX(LESSEQUAL));
                break;
            case TOKEN_PFX(COND):
                ts = token_peek_check(state, TOKEN_PFX(LBRACE));
                if (ts == TOKEN_STATUS_PFX(PEEK_SOME)) {
                    // if stmt
                    if ((if_node = parse_if(state, cur_fn)) == NULL) {
                        // TODO error
                    }
                } else if (ts != TOKEN_STATUS_PFX(SOME)) {
                    // TODO error
                }
                break;
            default:
                break;
        }
        if (n == NULL) break;
        if (is_value(n) && cur_node == NULL) {
            head->node = n;
            cur_node = n;
        } else if (is_op(n) && (head->node == NULL || is_value(head->node))) {
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
    parser_status ps;
    while ((ps = parse_stmt(state, cur_fn, holder)) == PARSER_STATUS_PFX(SOME)) {
        tail->node = holder->node;
        holder->node = NULL;
        tail->next = ast_node_link_init();
        tail = tail->next;
    }
    tail->node = holder->node;
    ast_node_holder_free(holder);
    if (ps != PARSER_STATUS_PFX(DONE)) {
        // TODO error
        return ps;
    }
    return ps;
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
    if (parser_mode_push(state, PARSER_MODE_PFX(FN)) == false) {
        // TODO error
        return PARSER_STATUS_PFX(MODE_PUSH_FAIL);
    }
    parser_status ps = parse_stmts(state, state->root_fn, state->root_fn->body_tail);
    if (parser_mode_pop(state) == false) {
        // TODO error
        return PARSER_STATUS_PFX(MODE_POP_FAIL);
    }
    return ps;
}
