
#include "parser.h"

extern inline ast_node *ast_node_init(ast_type type, ast_data data);

extern inline ast_fn_node *ast_fn_node_init(ast_node *parent);

extern inline parser_state *parser_state_init(void);

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node **cur_node) {

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
