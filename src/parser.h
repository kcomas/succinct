
#pragma once

#include <stdlib.h>

#define AST_PREFIX(NAME) AST_PFX_##NAME

typedef enum {
    AST_PREFIX(UOP),
    AST_PREFIX(BOP)
} ast_type;
