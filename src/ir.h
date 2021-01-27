
#pragma once

#include "ast.h"

#define IR_PFX(NAME) OP_TYPE_##NAME

typedef enum {
    IR_PFX(_START_IR),
    // DATA
    IR_PFX(PUSH),
    IR_PFX(ALLOCATE_LOCAL),
    IR_PFX(FREE_LOCAL),
    // OP
    IR_PFX(_END_IR)
} ir_op;

typedef struct _ir ir;

typedef union {

} ir_data;

typedef struct _ir {
    ir_op op;
    var_type_header type_header;
    ir_data data;
    ast_node *node;
} ir;
