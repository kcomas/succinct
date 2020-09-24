
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

inline void errno_print_exit(void) {
    int err = errno;
    printf("%s\n", strerror(err));
    exit(err);
}
