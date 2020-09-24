
#pragma once

#include <stdlib.h>
#include <stdio.h>

#define TOKEN_PFX(NAME) TOKEN_##NAME

typedef enum {
    // Values
    TOKEN_PFX(VAR),
    TOKEN_PFX(INT),
    TOKEN_PFX(CHAR),
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
    TOKEN_PFX(SUB), // -
    TOKEN_PFX(MUL), // *
    TOKEN_PFX(WRITE), // &>
    // Control Flow
    TOKEN_PFX(NEWLINE), // \n
    TOKEN_PFX(SEPRATOR), // ;
    TOKEN_PFX(IF), // ?
    TOKEN_PFX(EQUAL), // =
    TOKEN_PFX(LESSEQUAL) // <=
} token_type;

typedef struct {
    token_type type;
    size_t char_no, line_no;
    size_t start_idx, end_idx;
} token;

#define TOKEN_STATUS_PFX(NAME) TOKEN_STATUS_##NAME

typedef enum {
    TOKEN_STATUS_PFX(OK)
} token_status;
