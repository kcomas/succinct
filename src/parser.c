
#include "parser.h"

static bool is_value(ast_node *const n) {
    return n->type > AST_PFX(_VALUE) && n->type < AST_PFX(_END_VALUE);
}

static bool is_op(ast_node *const n) {
    return n->type > AST_PFX(_OP) && n->type < AST_PFX(_END_OP);
}

extern inline ast_node *ast_node_init(ast_type type, const token *const t, ast_data data);

void ast_node_free(ast_node *node) {
    if (node == NULL) return;
    if (is_op(node)) {
        ast_op_node_free(node->data.op);
    } else {
        switch (node->type) {
            case  AST_PFX(FN):
                ast_fn_node_free(node->data.fn, true);
                break;
            default:
                break;
        }
    }
    token_free(node->t);
    free(node);
}

const char *ast_type_string(ast_type type) {
    static const char *types[] = {
        "_VALUE",
        "UNKNOWN",
        "VAR",
        "INT",
        "CHAR",
        "FN",
        "_END_VALUE",
        "_OP",
        "ASSIGN",
        "ADD",
        "WRITE",
        "LESSEQUAL",
        "_END_OP"
    };
    return type > AST_PFX(_VALUE) && type < AST_PFX(_END_OP) ? types[type] : "AST_TYPE_NOT_FOUND";
}

static void ast_node_link_print_json(ast_node_link *head, const string *const s) {
    // print all links
    putchar('[');
    while (head != NULL) {
        // TODO tail link has unused link
        if (head->node != NULL) ast_node_print_json(head->node, s);
        if (head->next != NULL && head->next->node) putchar(',');
        head = head->next;
    }
    putchar(']');
}

void ast_fn_node_print_json(const ast_fn_node *const fn, const string *const s) {
    printf("{\"type\":");
    var_type_print_json(fn->type);
    printf(",\"parent\":");
    if (fn->parent != NULL) ast_fn_node_print_json(fn->parent, s);
    else printf("{}");
    printf(",\"body\":");
    ast_node_link_print_json(fn->body_head, s);
    putchar('}');
}

void ast_node_print_json(const ast_node *const node, const string *const s) {
    printf("{\"type\":\"%s\",\"data\":", ast_type_string(node->type));
    switch (node->type) {
        case AST_PFX(VAR):
            symbol_table_bucket_print_json(node->data.var);
            break;
        case AST_PFX(INT):
            printf("{}");
            break;
        case AST_PFX(FN):
            ast_fn_node_print_json(node->data.fn, s);
            break;
        default:
            // op node
            printf("{\"return_type\":");
            var_type_print_json(node->data.op->return_type);
            printf(",\"left\":");
            ast_node_print_json(node->data.op->left, s);
            printf(",\"right\":");
            ast_node_print_json(node->data.op->right, s);
            putchar('}');
            break;
    }
    putchar(',');
    printf("\"token\":");
    token_print_json(node->t, s);
    putchar('}');
}

extern inline ast_node_link *ast_node_link_init(void);

extern inline void ast_node_link_free(ast_node_link *head);

extern inline ast_op_node *ast_op_node_init(void);

extern inline void ast_op_node_free(ast_op_node *op);

extern inline ast_fn_node *ast_fn_node_init(ast_fn_node *parent);

extern inline void ast_fn_node_free(ast_fn_node *fn, bool free_parent);

extern inline ast_if_node *ast_if_node_init(void);

void ast_if_node_free(ast_if_node *if_node) {
    // TODO
    free(if_node);
}

extern inline ast_node_holder *ast_node_holder_init(void);

extern inline void ast_node_holder_free(ast_node_holder *holder);

extern inline parser_state *parser_state_init(void);

extern inline void parser_state_free(parser_state *state);

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

static parser_status wire_final_value(ast_node *const value_tmp, ast_node *const cur_node) {
    if (value_tmp != NULL && is_op(cur_node)) {
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
    return PARSER_STATUS_PFX(SOME);
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

static ast_fn_node *parse_fn(parser_state *const state, ast_fn_node *const parent_fn) {
    // we are at the first maybe var after first (
    ast_fn_node *cur_fn = ast_fn_node_init(parent_fn);
    token_status ts;
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
    parser_status status = parse_stmts(state, cur_fn, cur_fn->body_tail);
    if (status != PARSER_STATUS_PFX(SOME)) {
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
    bool parse = true;
    ast_if_cond *cond_node = NULL;
    ast_node_holder *cond_holder;
    while (parse == true) {
        // parse cond
        while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
            // before cond remove newline
            if (state->next->type == TOKEN_PFX(NEWLINE)) continue;
            if (state->next->type == TOKEN_PFX(LBRACE)) {
                // reached else stmt
                in_else = true;
                break;
            }
            if (state->next->type != TOKEN_PFX(LPARENS)) {
                // TODO error
                ast_if_node_free(if_node);
                return NULL;
            }
            state->mode = PARSER_MODE_PFX(IF_COND);
            cond_holder = ast_node_holder_init();
            if ((ps = parse_stmt(state, cur_fn, cond_holder)) != PARSER_STATUS_PFX(SOME)) {
                // TODO error
                return NULL;
            }
        }
        // TODO error
        // parse body
        while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
            // before body remove newline
            if (in_else == false && state->next->type == TOKEN_PFX(NEWLINE)) continue;
            if (in_else == true) {
                if (cond_node->body_tail->node == NULL) {
                    // TODO error
                    // cannot have if stmt with no conds
                    return NULL;
                }
            } else {

            }
        }
        // TODO error
        // attach cond node to if node
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
                return wire_final_value(value_tmp, cur_node);
            case TOKEN_PFX(VAR):
                // TODO check if fn call
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
                }
                n = ast_node_init(AST_PFX(VAR), state->next, (ast_data) { .var = b });
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
            case TOKEN_PFX(RPARENS):
                if (state->mode == PARSER_MODE_PFX(IF_COND)) return wire_final_value(value_tmp, cur_node);
                break;
            case TOKEN_PFX(ASSIGN):
                // TODO fn check
                n = make_op(state, AST_PFX(ASSIGN));
                break;
            case TOKEN_PFX(ADD):
                // TODO fn check
                n = make_op(state, AST_PFX(ADD));
                break;
            case TOKEN_PFX(WRITE):
                // TODO fn check
                n = make_op(state, AST_PFX(WRITE));
                break;
            case TOKEN_PFX(LESSEQUAL):
                // TODO fn check
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
    return parse_stmts(state, state->root_fn, state->root_fn->body_tail);
}
