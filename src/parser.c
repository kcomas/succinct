
#include "parser.h"

extern inline ast_node *ast_node_init(ast_type type, const token *const t, ast_data data);

void ast_fn_node_free(ast_fn_node *fn) {
    var_type_free(fn->type);
    if (fn->parent) ast_fn_node_free(fn->parent);
    free(fn);
}

extern inline ast_bop_node *ast_bop_node_init(void);

extern inline void ast_bop_node_free(ast_bop_node *bop);

extern inline ast_fn_node *ast_fn_node_init(ast_fn_node *parent);

void ast_node_free(ast_node *node) {
    if (node == NULL) return;
    switch (node->type) {
        case  AST_PFX(FN):
            ast_fn_node_free(node->data.fn);
            break;
        case AST_PFX(ASSIGN):
            ast_bop_node_free(node->data.bop);
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

static ast_node_link *ast_node_link_init(ast_node *n) {
    ast_node_link *list_node = calloc(1, sizeof(ast_node_link));
    list_node->node = n;
    return list_node;
}

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder *cur_node) {
    token_status ts;
    symbol_table_bucket *b;
    ast_node *n;
    while ((ts = token_next(state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
        switch (state->next->type) {
            case TOKEN_PFX(NEWLINE):
                if (cur_node == NULL) continue;
                break;
            case TOKEN_PFX(VAR):
                // found var create bucket and node
                b = symbol_table_insert(&cur_fn->type->body.fn->symbols, SYMBOL_PFX(LOCAL), state->next, state->s);
                n = ast_node_init(AST_PFX(VAR), state->next, (ast_data) { .var = b });
                break;
            case TOKEN_PFX(ASSIGN):
                n = ast_node_init(AST_PFX(ASSIGN), state->next, (ast_data) { .bop = ast_bop_node_init() });
                break;
            default:
                break;
        }
        if (n == NULL) break;
        // connect node
        if (cur_node->node == NULL) {
            cur_node->node = n;
            if (cur_fn->list_head == NULL) {
                cur_fn->list_head = ast_node_link_init(n);
                cur_fn->list_tail = cur_fn->list_head;
            } else {
                cur_fn->list_tail->next = ast_node_link_init(n);
                cur_fn->list_tail = cur_fn->list_tail->next;
            }
        } else {
            // set the node based on cur type
        }
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
