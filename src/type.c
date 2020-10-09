
#include "type.h"

extern inline symbol_table *symbol_table_init(size_t size);

void symbol_table_free(symbol_table *s) {
    for (size_t i = 0; i < s->size; i++) {
        symbol_table_bucket *b = s->buckets[i];
        while (b != NULL) {
            symbol_table_bucket *tmp = b;
            b = b->next;
            free(tmp);
        }
    }
    free(s);
}

extern inline var_type *var_type_init(var_type_header header, var_type_body body);

extern inline var_type *var_type_fn_init(size_t symbol_table_size);

void var_type_free(var_type *t) {
    switch (t->header) {
            case VAR_PFX(FN):
                var_type_free(t->body.fn->return_type);
                symbol_table_free(t->body.fn->symbols);
                break;
        default:
            break;
    }
    free(t);
}
