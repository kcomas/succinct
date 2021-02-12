
#pragma once

#include "infer.h"

#define IR_PFX(NAME) IR_##NAME

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

typedef struct {
    size_t size, len;
    ir *ops[];
} ir_fn;

typedef union {
    int64_t intv;
    utf8 cv;
    ir_fn *fn;
} ir_data;

typedef struct _ir {
    ir_op op;
    var_type_header type_header;
    ir_data data;
    ast_node *node; // reference to node
} ir;

#define IR_STATUS_PFX(NAME) IR_STATUS_##NAME

typedef enum {
    IR_STATUS_PFX(_START_IR_STATUS),
    IR_STATUS_PFX(OK),
    IR_STATUS_PFX(_END_IR_STATUS)
} ir_status;

typedef struct {
    size_t local_stack_size, local_stack_len;
    var_type_header local_type[]; // for ever alloc there is a free of that type at end of fn
} ir_fn_local_stack;

typedef struct {
    size_t fn_stack_size, fn_stack_len;
    infer_state *ins;
    ir_fn *root_ir;
    ir_fn_local_stack fn_stack[];
} ir_state;
