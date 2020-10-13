
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "string.h"

#define TOKEN_PFX(NAME) TOKEN_##NAME

typedef enum {
    TOKEN_PFX(UNKNOWN),
    // Values
    TOKEN_PFX(VAR),
    TOKEN_PFX(INT),
    TOKEN_PFX(CHAR),
    TOKEN_PFX(STRING),
    // Types
    TOKEN_PFX(U64),
    // Parens
    TOKEN_PFX(LBRACE), // {
    TOKEN_PFX(RBRACE), // }
    TOKEN_PFX(LBRACKET), // [
    TOKEN_PFX(RBRACKET), // ]
    TOKEN_PFX(LPARENS), // (
    TOKEN_PFX(RPARENS), // )
    // Operators
    TOKEN_PFX(ASSIGN), // :
    TOKEN_PFX(DEFINE), // ::
    TOKEN_PFX(ADD), // +
    TOKEN_PFX(SUB), // -
    TOKEN_PFX(MUL), // *
    TOKEN_PFX(WRITE), // &>
    // Control Flow
    TOKEN_PFX(NEWLINE), // \n
    TOKEN_PFX(SEPRATOR), // ;
    TOKEN_PFX(COND), // ?
    TOKEN_PFX(EQUAL), // =
    TOKEN_PFX(LESS), // <
    TOKEN_PFX(LESSEQUAL), // <=
    TOKEN_PFX(AND), // &
    TOKEN_PFX(_MAX_TOKENS)
} token_type;

typedef struct {
    token_type type;
    size_t char_no, line_no;
    size_t start_idx, end_idx;
} token;

inline token *token_init(void) {
    token *t = calloc(1, sizeof(token));
    t->type = TOKEN_PFX(UNKNOWN);
    t->char_no = 1;
    t->line_no = 1;
    t->start_idx = 0;
    t->end_idx = 0;
    return t;
}

inline void token_free(token *t) {
    free(t);
}

inline size_t token_len(const token *const t) {
    return t->end_idx - t->start_idx + 1;
}

const char *token_type_string(token_type type);

inline void token_print(const token *const t, const string *const s) {
    printf("Line: %lu, Char %lu, Len: %lu, %s, ", t->line_no, t->char_no, token_len(t), token_type_string(t->type));
    for(size_t i = t->start_idx; i <= t->end_idx; i++) putchar(s->buffer[i]);
    putchar('\n');
}

inline void token_copy(token *const dest, const token *const src) {
    memcpy(dest, src, sizeof(token));
}

#define TOKEN_STATUS_PFX(NAME) TOKEN_STATUS_##NAME

typedef enum {
    TOKEN_STATUS_PFX(SOME),
    TOKEN_STATUS_PFX(NONE),
    TOKEN_STATUS_PFX(FILE_MUST_START_NEWLINE),
    TOKEN_STATUS_PFX(EXCEDED_MAX_STRING_LEN)
} token_status;

token_status token_next(token *const t, const string *const s);
