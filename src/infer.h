
#pragma once

#include "parser.h"
#include "error.h"

typedef struct {
    parser_state *p;
    error *e;
} infer_state;

inline infer_state *infer_state_init(parser_state *const p) {
    infer_state *state = calloc(1, sizeof(parser_state));
    state->p = p;
    state->e = error_init();
    return state;
}

inline void infer_state_free(infer_state *state) {
    parser_state_free(state->p);
    error_free(state->e);
    free(state);
}

#define INFER_STATUS_PFX(NAME) INFER_STATUS_##NAME

typedef enum {
    INFER_STATUS_PFX(_START_INFER),
    INFER_STATUS_PFX(OK),
    INFER_STATUS_PFX(INVALID_NODE),
    INFER_STATUS_PFX(CANNOT_GET_TYPE_FROM_NODE),
    INFER_STATUS_PFX(VAR_TYPE_NOT_FOUND),
    INFER_STATUS_PFX(INVALID_VEC_ITEM),
    INFER_STATUS_PFX(INVALID_FN),
    INFER_STATUS_PFX(FN_INVALID_FINAL_STMT),
    INFER_STATUS_PFX(FN_CANNOT_GET_FINAL_TYPE),
    INFER_STATUS_PFX(FN_LAST_TYPE_NOT_EQUAL),
    INFER_STATUS_PFX(CALL_TARGET_NOT_VAR),
    INFER_STATUS_PFX(RECURSIVE_CALL_ON_MODULE_LEVEL),
    INFER_STATUS_PFX(CALL_DOES_NOT_EXIST_IN_PARENT),
    INFER_STATUS_PFX(CANNOT_GET_CALL_TYPE),
    INFER_STATUS_PFX(CALL_NOT_ON_FN),
    INFER_STATUS_PFX(INVALID_NUM_OF_ARGS_IN_CALL),
    INFER_STATUS_PFX(INVALID_CALL_TARGET),
    INFER_STATUS_PFX(INVALID_CALL_ARG),
    INFER_STATUS_PFX(CANNOT_GET_ARG_TYPE),
    INFER_STATUS_PFX(INVALID_ARG_TYPE),
    INFER_STATUS_PFX(INVALID_COND),
    INFER_STATUS_PFX(INVALID_IF_BODY),
    INFER_STATUS_PFX(INVALID_ASSIGN_LEFT_SIDE),
    INFER_STATUS_PFX(INVALID_ASSIGN_RIGHT_SIDE),
    INFER_STATUS_PFX(INVALID_CAST_LEFT_NODE),
    INFER_STATUS_PFX(INVALID_RAW_INT_FD),
    INFER_STATUS_PFX(INVALID_LEFT_SIDE),
    INFER_STATUS_PFX(INVALID_RIGHT_SIDE),
    INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL),
    INFER_STATUS_PFX(INVALID_TYPE_FOR_NODE),
    INFER_STATUS_PFX(_END_INFER)
} infer_status;

const char *infer_status_string(infer_status status);

inline infer_status infer_error(infer_state *const state, infer_status status, ast_node *const node) {
    error_infer(state->e, status, node);
    return status;
}

infer_status infer_node(infer_state *const state, ast_fn_node *const cur_fn, ast_node *const node);

infer_status infer(infer_state *const state);
