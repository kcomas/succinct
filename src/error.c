
#include "error.h"

const char *error_type_string(error_type type) {
    const char *types[] = {
        "OK",
        "ERRNO",
        "PARSER",
        "_END_ERROR"
    };
    return type >= ERROR_PFX(OK) && type < ERROR_PFX(_END_ERROR) ? types[type] : "ERROR_TYPE_NOT_FOUND";
}

extern inline error *error_init(void);

extern inline void error_free(error *e);

extern inline void errno_print_exit(void);

void error_parser(error *const e, uint8_t mode, uint8_t status, const token *const t) {
    if (e->type == ERROR_PFX(OK) || e->type == ERROR_PFX(PARSER)) {
        if (e->type == ERROR_PFX(OK)) {
            e->type = ERROR_PFX(PARSER);
            e->data.parser = calloc(1, sizeof(error_parser_stack) + sizeof(parser_stack) * PARSER_MODE_MAX_STACK_SIZE);
        } else {
            if (e->data.parser->stack_head >= PARSER_MODE_MAX_STACK_SIZE) return;
        }
        e->data.parser->stack[e->data.parser->stack_head++] = (parser_stack) { .mode = mode, .status = status, .t = token_init_copy(t) };
    }
}
