
#pragma once

#include <stdlib.h>
#include "type.h"

#define AST_PFX(NAME) AST_##NAME

typedef enum {
    AST_PFX(VAR),
    AST_PFX(INT),
    AST_PFX(CHAR),
    AST_PFX(FN),
    AST_PFX(VEC)
} ast_type;

typedef struct _ast_node ast_node;

typedef struct {
    ast_type ast_type;
    var_type *return_type;
    ast_node *left, *right;
} ast_bop_node;
