
#pragma once

#include <stdio.h>
#include <errno.h>
#include "string.h"

#define ERROR_PFX(NAME) ERROR_##NAME

typedef enum {
    ERROR_PFX(OK),
    ERROR_PFX(ERRNO),
    ERROR_PFX(MESSAGE)
} error_type;

typedef struct {
    error_type type;
    int no;
    string *msg;
} error;

inline error *error_init(void) {
    return calloc(1, sizeof(error));
}

inline void error_free(error *e) {
    free(e);
}

inline void error_errno(error *const e) {
    e->type = ERROR_PFX(ERRNO);
    e->no = errno;
}

inline void errno_print_exit(void) {
    int err = errno;
    printf("%s\n", strerror(err));
    exit(err);
}
