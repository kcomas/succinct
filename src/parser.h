
#pragma once

#include "file.h"
#include "ast.h"
#include "def.h"
#include "error.h"

#define PARSER_MODE_PFX(NAME) PARSER_MODE_##NAME

typedef enum _parser_mode {
    PARSER_MODE_PFX(NONE),
    PARSER_MODE_PFX(MODULE),
    PARSER_MODE_PFX(VEC_BODY),
    PARSER_MODE_PFX(FN_CALL_ARGS),
    PARSER_MODE_PFX(FN),
    PARSER_MODE_PFX(IF_COND),
    PARSER_MODE_PFX(IF_BODY),
    PARSER_MODE_PFX(_END_MODE)
} parser_mode;

const char *parser_mode_string(parser_mode mode);

#define PARSER_STATUS_PFX(NAME) PARSER_STATUS_##NAME

typedef enum {
    PARSER_STATUS_PFX(SOME),
    PARSER_STATUS_PFX(NONE), // none found
    PARSER_STATUS_PFX(DONE), // no more statements
    // System Error
    PARSER_STATUS_PFX(CANNOT_OPEN_FILE),
    PARSER_STATUS_PFX(CANNOT_READ_FILE),
    PARSER_STATUS_PFX(CANNOT_CLOSE_FILE),
    // Parser Error
    PARSER_STATUS_PFX(MODE_PUSH_FAIL),
    PARSER_STATUS_PFX(MODE_POP_FAIL),
    PARSER_STATUS_PFX(VAR_INSERT_FAIL),
    PARSER_STATUS_PFX(INVALID_VEC),
    PARSER_STATUS_PFX(INVALID_CALL),
    PARSER_STATUS_PFX(INVALID_IF),
    PARSER_STATUS_PFX(INVALID_TOKEN_SEQUENCE),
    PARSER_STATUS_PFX(INVALID_FINAL_VALUE),
    PARSER_STATUS_PFX(_END_PARSER_STATUS)
} parser_status;

const char *parser_status_string(parser_status status);

typedef struct _parser_state {
    size_t mode_head;
    token *next, *peek;
    string *s;
    ast_fn_node *root_fn;
    error *e;
    parser_mode mode[]; // Mode stack
} parser_state;

parser_state *parser_state_init(void);

void parser_state_free(parser_state *state);

inline bool parser_mode_push(parser_state *const state, parser_mode mode) {
    if (state->mode_head > PARSER_MODE_MAX_STACK_SIZE) return false;
    state->mode[state->mode_head++] = mode;
    return true;
}

inline parser_mode parser_mode_pop(parser_state *const state) {
    if (state->mode_head == 0) return PARSER_MODE_PFX(NONE);
    parser_mode m = state->mode[state->mode_head - 1];
    state->mode[state->mode_head--] = PARSER_MODE_PFX(NONE);
    return m;
}

inline parser_mode parser_mode_get(const parser_state *const state) {
    return state->mode_head > 0 ? state->mode[state->mode_head - 1] : PARSER_MODE_PFX(NONE);
}

inline parser_status parser_error(parser_state *const state, parser_status status) {
    error_parser(state->e, parser_mode_pop(state), status, state->next);
    return status;
}

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder *const head);

parser_status parse_stmts(parser_state *const state, ast_fn_node *const cur_fn, ast_node_link *tail);

parser_status parse_module(parser_state *const state, const char *const filename);
