
#include <stdio.h>
#include "file.h"
#include "error.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage %s file.sc\n", argv[0]);
        return 1;
    }
    int source_fd = file_open_r(argv[1]);
    if (source_fd == -1) errno_print_exit();
    string *source_string = file_read_to_string(source_fd);
    if (source_string == NULL) errno_print_exit();
    file_close(source_fd);
    printf("%s\n", source_string->buffer);
    string_free(source_string);
    return 0;
}
