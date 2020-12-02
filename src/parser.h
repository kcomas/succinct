
#pragma once

#include "file.h"
#include "ast.h"

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
    PARSER_STATUS_PFX(INVALID_TOKEN_SEQUENCE),
    PARSER_STATUS_PFX(INVALID_FINAL_VALUE)
} parser_status;

#define PARSER_MODE_PFX(NAME) PARSER_MODE_##NAME

typedef enum {
    PARSER_MODE_PFX(NONE),
    PARSER_MODE_PFX(FN),
    PARSER_MODE_PFX(IF_COND),
    PARSER_MODE_PFX(IF_BODY),
    PARSER_MODE_PFX(FN_ARGS)
} parser_mode;

#ifndef PARSER_MODE_MAX_STACK_SIZE
    #define PARSER_MODE_MAX_STACK_SIZE 20
#endif

typedef struct {
    size_t mode_head;
    token *next, *peek;
    string *s;
    ast_fn_node *root_fn;
    error *e;
    parser_mode mode[]; // Mode stack
} parser_state;

inline parser_state *parser_state_init(void) {
    parser_state *state = calloc(1, sizeof(parser_state) + sizeof(parser_mode) * PARSER_MODE_MAX_STACK_SIZE);
    // string is added before parse
    state->next = token_init();
    state->peek = token_init();
    state->root_fn = ast_fn_node_init(NULL);
    state->e = error_init();
    return state;
}

inline void parser_state_free(parser_state *state) {
    token_free(state->next);
    token_free(state->peek);
    if (state->s) string_free(state->s);
    ast_fn_node_free(state->root_fn, true);
    error_free(state->e);
    free(state);
}

inline bool parser_mode_push(parser_state *const state, parser_mode mode) {
    if (state->mode_head >= PARSER_MODE_MAX_STACK_SIZE) return false;
    state->mode[state->mode_head++] = mode;
    return true;
}

inline bool parser_mode_pop(parser_state *const state) {
    if (state->mode_head == 0) return false;
    state->mode[state->mode_head--] = PARSER_MODE_PFX(NONE);
    return true;
}

inline parser_mode parser_mode_get(parser_state *const state) {
    return state->mode_head > 0 ? state->mode[state->mode_head - 1] : PARSER_MODE_PFX(NONE);
}

parser_status parse_stmt(parser_state *const state, ast_fn_node *const cur_fn, ast_node_holder *const head);

parser_status parse_stmts(parser_state *const state, ast_fn_node *const cur_fn, ast_node_link *tail);

parser_status parse_module(parser_state *const state, const char *const filename);
