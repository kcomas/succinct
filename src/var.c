
#include "var.h"

var *var_copy(const var *const v) {
    switch (v->type) {
        case VAR_PFX(VOID):
        case VAR_PFX(U8):
        case VAR_PFX(U16):
        case VAR_PFX(U32):
        case VAR_PFX(U64):
        case VAR_PFX(I8):
        case VAR_PFX(I16):
        case VAR_PFX(I32):
        case VAR_PFX(I64):
        case VAR_PFX(F32):
        case VAR_PFX(F64):
        case VAR_PFX(CHAR):
        case VAR_PFX(FD):
            return var_init(v->type, false, v->data);
    }
}
