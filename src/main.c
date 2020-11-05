
#include "parser.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage %s file.sc\n", argv[0]);
        return 1;
    }

    int fd = file_open_r(argv[1]);
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
    parser_state *state = parser_state_init();
    parser_status status = parse_module(state, argv[1]);
    parser_state_free(state);
    return status;
}
