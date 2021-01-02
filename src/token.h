
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "string.h"

#define TOKEN_PFX(NAME) TOKEN_##NAME

typedef enum {
    TOKEN_PFX(_START_TOKENS),
    TOKEN_PFX(UNKNOWN),
    TOKEN_PFX(COMMENT),
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
    TOKEN_PFX(DIV), // /
    TOKEN_PFX(WRITE), // <&
    TOKEN_PFX(AT), // @
    // Control Flow
    TOKEN_PFX(NEWLINE), // \n
    TOKEN_PFX(SEPRATOR), // ;
    TOKEN_PFX(COND), // ?
    TOKEN_PFX(EQUAL), // =
    TOKEN_PFX(LESS), // <
    TOKEN_PFX(LESSEQUAL), // <=
    TOKEN_PFX(AND), // &
    TOKEN_PFX(_END_TOKENS)
} token_type;

const char *token_type_string(token_type type);

typedef struct _token {
    token_type type;
    size_t char_no, line_no;
    size_t start_idx, end_idx;
} token;

inline token *token_init(void) {
    token *t = calloc(1, sizeof(token));
    t->type = TOKEN_PFX(UNKNOWN);
    t->char_no = 1;
    t->line_no = 1;
    return t;
}

inline void token_free(token *t) {
    free(t);
}

inline size_t token_len(const token *const t) {
    return t->end_idx - t->start_idx + 1;
}

inline token *token_copy(token *const dest, const token *const src) {
    return memcpy(dest, src, sizeof(token));
}

inline token *token_init_copy(const token *const src) {
    token *t = token_init();
    token_copy(t, src);
    return t;
}

#define TOKEN_STATUS_PFX(NAME) TOKEN_STATUS_##NAME

typedef enum {
    TOKEN_STATUS_PFX(SOME),
    TOKEN_STATUS_PFX(PEEK_SOME), // for parser
    TOKEN_STATUS_PFX(NONE),
    TOKEN_STATUS_PFX(FILE_MUST_START_NEWLINE),
    TOKEN_STATUS_PFX(EXCEDED_MAX_STRING_LEN),
    TOKEN_STATUS_PFX(INVALID_MATCH), // for parser
} token_status;

token_status token_next(token *const t, const string *const s);
