
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "def.h"
#include "token.h"

#define ERROR_PFX(NAME) ERROR_##NAME

typedef enum {
    ERROR_PFX(OK),
    ERROR_PFX(ERRNO),
    ERROR_PFX(PARSER),
    ERROR_PFX(_END_ERROR)
} error_type;

const char *error_type_string(error_type type);

typedef struct {
    uint8_t mode, status; // parser mode, parser status
    token *t; // copied
} parser_stack;

typedef struct {
    size_t stack_head;
    parser_stack stack[]; // stack is reversed
} error_parser_stack;

typedef union {
    int eno;
    error_parser_stack *parser;
} error_data;

typedef struct _error {
    error_type type;
    error_data data;
} error;

inline error *error_init(void) {
    return calloc(1, sizeof(error));
}

inline void error_free(error *e) {
    free(e);
}

inline void errno_print_exit(void) {
    int err = errno;
    printf("%s\n", strerror(err));
    exit(err);
}

void error_parser(error *const e, uint8_t mode, uint8_t status, const token *const t);
