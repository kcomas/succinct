
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
    INFER_STATUS_PFX(OK),
    INFER_STATUS_PFX(INVALID_NODE),
    INFER_STATUS_PFX(VAR_TYPE_NOT_FOUND),
    INFER_STATUS_PFX(INVALID_ASSGIN_LEFT_SIDE),
    INFER_STATUS_PFX(INVALID_LEFT_SIDE),
    INFER_STATUS_PFX(INVALID_RIGHT_SIDE),
    INFER_STATUS_PFX(NODE_TYPES_NOT_EQUAL),
    INFER_STATUS_PFX(INVALID_TYPE_FOR_OP)
} infer_status;

inline infer_status infer_error(infer_state *const state, infer_status status, ast_node *const node) {
    error_infer(state->e, status, node);
    return status;
}

infer_status infer_node(infer_state *const state, ast_node *const node);

infer_status infer(infer_state *const state);
