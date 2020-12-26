
#include "error.h"

const char *error_type_string(error_type type) {
    const char *types[] = {
        "OK",
        "ERRNO",
        "PARSER",
        "INFER",
        "_END_ERROR"
    };
    return type >= ERROR_PFX(OK) && type < ERROR_PFX(_END_ERROR) ? types[type] : "ERROR_TYPE_NOT_FOUND";
}

extern inline error *error_init(void);

void error_free(error *e) {
    switch (e->type) {
        case ERROR_PFX(PARSER):
            for (size_t i = 0; i < e->data.parser->stack_head; i++) token_free(e->data.parser->stack[i].t);
            free(e->data.parser);
            break;
        case ERROR_PFX(INFER):
            break;
        default:
            break;
    }
    free(e);
}

extern inline void errno_print_exit(void);

void error_parser(error *const e, uint8_t mode, uint8_t status, const token *const t) {
    if (e->type == ERROR_PFX(OK) || e->type == ERROR_PFX(PARSER)) {
        if (e->type == ERROR_PFX(OK)) {
            e->type = ERROR_PFX(PARSER);
            e->data.parser = calloc(1, sizeof(error_parser_stack) + sizeof(parser_stack) * PARSER_MODE_MAX_STACK_SIZE);
        } else if (e->data.parser->stack_head >= PARSER_MODE_MAX_STACK_SIZE) {
            return;
        }
        e->data.parser->stack[e->data.parser->stack_head++] = (parser_stack) { .mode = mode, .status = status, .t = token_init_copy(t) };
    }
}

void error_infer(error *const e, uint8_t status, ast_node *const node) {
    if (e->type == ERROR_PFX(OK) || e->type == ERROR_PFX(INFER)) {
        if (e->type == ERROR_PFX(OK)) {
            e->type = ERROR_PFX(INFER);
            e->data.infer = calloc(1, sizeof(error_infer_stack) + sizeof(infer_stack) * ERROR_INFER_MAX_STACK_SIZE);
        } else if (e->data.infer->stack_head >= ERROR_INFER_MAX_STACK_SIZE) {
            return;
        }
        e->data.infer->stack[e->data.infer->stack_head++] = (infer_stack) { .status = status, .node = node };
    }
}
