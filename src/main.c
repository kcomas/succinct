
#include "parser.h"

int print_tokens(const char *const file) {
    int fd = file_open_r(file);
    if (fd == -1) errno_print_exit();
    string *str = file_read_to_string(fd);
    if (str == NULL) errno_print_exit();
    int clo = file_close(fd);
    if (clo == -1) errno_print_exit();
    // printf("%s\n", str->buffer);
    token *t = token_init();
    token_status ts;
    while ((ts = token_next(t, str)) == TOKEN_STATUS_PFX(SOME)) {
        token_print_json(t, str);
        putchar(',');
    }
    string_free(str);
    token_free(t);
    return 0;
}

int print_ast(const char *const file) {
    parser_state *state = parser_state_init();
    parser_status status = parse_module(state, file);
    parser_state_free(state);
    return status;
}

int usage(const char *const basefile) {
    printf("Usage %s [-t(okens) -a(st)] file.sc\n", basefile);
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
        }
    }
    // TODO run
    return usage(argv[0]);
}
