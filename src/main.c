
#include "parser.h"
#include "print_json.h"
#include "infer.h"

int print_tokens(const char *const file) {
    int fd = file_open_r(file);
    if (fd == -1) errno_print_exit();
    string *str = file_read_to_string(fd);
    if (str == NULL) errno_print_exit();
    int clo = file_close(fd);
    if (clo == -1) errno_print_exit();
    // printf("%s\n", str->buffer);
    token *t = token_init();
    token *print = token_init();
    token_status ts;
    putchar('[');
    while ((ts = token_next(t, str)) == TOKEN_STATUS_PFX(SOME)) {
        if (print->type != TOKEN_PFX(UNKNOWN)) {
            token_print_json(print, str);
            putchar(',');
        }
        token_copy(print, t);
    }
    token_print_json(print, str);
    putchar(']');
    string_free(str);
    token_free(t);
    token_free(print);
    return 0;
}

int print_ast(const char *const file) {
    parser_state *state = parser_state_init();
    parser_status status = parse_module(state, file);
    if (status == PARSER_STATUS_PFX(DONE) || status == PARSER_STATUS_PFX(NONE))
        ast_fn_node_print_json(state->root_fn, state->s);
    else
        error_print_json(state->e, state->s);
    parser_state_free(state);
    return status;
}

int print_infer(const char *const file) {
    parser_state *pstate = parser_state_init();
    parser_status ps = parse_module(pstate, file);
    if (ps != PARSER_STATUS_PFX(DONE) && ps != PARSER_STATUS_PFX(NONE)) {
        error_print_json(pstate->e, pstate->s);
        parser_state_free(pstate);
        return ps;
    }
    infer_state *istate = infer_state_init(pstate);
    infer_status is = infer(istate);
    if (is != INFER_STATUS_PFX(OK)) error_print_json(istate->e, istate->p->s);
    infer_state_free(istate);
    return is;
}

int usage(const char *const basefile) {
    printf("Usage %s [-t(okens) -a(st) -i[nfer] file.sc\n", basefile);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 3) return usage(argv[0]);
    if (argv[1][0] == '-') {
        switch (argv[1][1]) {
            case 't':
                return print_tokens(argv[2]);
            case 'a':
                return print_ast(argv[2]);
                break;
            case 'i':
                return print_infer(argv[2]);
                break;
            default:
                break;
        }
    }
    // TODO run
    return usage(argv[0]);
}
