
#pragma once

#define VAR_PFX(NAME) VAR_##NAME

typedef enum {
    VAR_PFX(VOID),
    VAR_PFX(U8),
    VAR_PFX(U16),
    VAR_PFX(U32),
    VAR_PFX(U64),
    VAR_PFX(I8),
    VAR_PFX(I16),
    VAR_PFX(I32),
    VAR_PFX(I64),
    VAR_PFX(F32),
    VAR_PFX(F64),
    VAR_PFX(CHAR),
    VAR_PFX(STRING),
    VAR_PFX(DATE),
    VAR_PFX(TIME),
    VAR_PFX(DATETIME),
    VAR_PFX(VEC),
    VAR_PFX(HASH),
    VAR_PFX(FN),
    VAR_PFX(THREAD),
    VAR_PFX(FD),
    VAR_PFX(REGEX)
} var_type_header;

typedef union {
    struct {
        size_t len; // 0 for dynamic
        struct _var_type *dynamic, *items[]; // all items have dynamic type
    } vec;
    struct {
        size_t len; // 0 for dynamic
        struct _var_type *dynamic; // all keys have this type
    } hash;
    struct {
        size_t num_args;
        struct _var_type *return_type, *args[];
    } fn;
} var_type_body;

typedef struct {
    var_type_header header;
    var_type_body *body;
} var_type;
