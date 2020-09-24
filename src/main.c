
#include <stdio.h>
#include "file.h"
#include "error.h"
#include "token.h"

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
    printf("%s\n", str->buffer);
    token t;
    token_init(&t);
    token_next(&t, str);
    string_free(str);
    return 0;
}
