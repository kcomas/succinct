
#include "parser.h"

extern inline ast_node *ast_node_init(ast_type type, ast_data data);

extern inline ast_fn_node *ast_fn_node_init(ast_fn_node *parent);

void ast_node_free(ast_node *node) {
    switch (node->type) {
        case  AST_PFX(FN):
            ast_fn_node_free(node->data.fn);
            break;
        default:
            break;
    }
    free(node);
}

void ast_fn_node_free(ast_fn_node *fn) {
    var_type_free(fn->type);
    if (fn->parent) ast_fn_node_free(fn->parent);
    free(fn);
}

extern inline parser_state *parser_state_init(void);

extern inline void parser_state_free(parser_state *state);

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node **cur_node) {
    token_status ts;
    while ((ts = token_next(&state->next, state->s)) == TOKEN_STATUS_PFX(SOME)) {
        switch (state->next.type) {
            case TOKEN_PFX(NEWLINE):
                if (cur_node == NULL) continue;
                break;
            default:
                break;
        }
    }
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
    return parse_stmt(state, state->root_fn, NULL);
}
