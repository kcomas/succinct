
#include "type.h"

const char *var_type_header_string(var_type_header header) {
    static const char *types[] = {
        "_VAR_TYPE_HEADER",
        "UNKNOWN",
        "VOID",
        "U8",
        "U16",
        "U32",
        "U64",
        "I8",
        "I16",
        "I32",
        "I64",
        "F32",
        "F64",
        "CHAR",
        "STRING",
        "DATETIME",
        "VEC",
        "HASH",
        "FN",
        "THREAD",
        "FD",
        "REGEX",
        "_END_VAR_TYPE_HEADER"
    };
    return header > VAR_PFX(_VAR_TYPE_HEADER) && header < VAR_PFX(_END_VAR_TYPE_HEADER) ? types[header] : "VAR_TYPE_HEADER_NOT_FOUND";
}

const char *symbol_table_type_string(symbol_table_type type) {
    static const char *types[] = {
        "_SYMBOL_TYPE",
        "LOCAL",
        "ARG",
        "KEY",
        "_END_SYMBOL_TYPE"
    };
    return type > SYMBOL_PFX(_SYMBOL_TYPE) && type < SYMBOL_PFX(_END_SYMBOL_TYPE) ? types[type] : "SYMBOL_TABLE_TYPE_NOT_FOUND";
}

extern inline symbol_table *symbol_table_init(size_t size);

void symbol_table_free(symbol_table *s) {
    for (size_t i = 0; i < s->size; i++) {
        symbol_table_bucket *b = s->buckets[i];
        while (b != NULL) {
            symbol_table_bucket *tmp = b;
            b = b->next;
            var_type_free(tmp->type);
            free(tmp);
        }
    }
    free(s);
}

static size_t hash_symbol(const token *const t, const string *const s) {
    size_t hash = 5831;
    for (size_t i = t->start_idx; i <= t->end_idx; i++) hash = ((hash << 5) + hash) + s->buffer[i];
    return hash;
}

static symbol_table_bucket *compare_buckets(symbol_table_bucket* b, const token *const t, const string *const s) {
    size_t i = 0;
    size_t len = token_len(t);
    if (len != b->size_len - 1) return NULL; // if they are not the same length no match
    for (; i <= len; i++) if (s->buffer[i + t->start_idx] != b->symbol[i]) break;
    if (i == len) return b;
    return NULL;
}

static symbol_table_bucket *bucket_init(symbol_table_type table_type, size_t symbol_counter, const token *const t, const string *const s) {
    size_t size_len = token_len(t) * sizeof(char) + sizeof(char); // add one for a null terminated string
    symbol_table_bucket *b = calloc(1, sizeof(symbol_table_bucket) + size_len);
    b->table_type = table_type;
    b->symbol_idx = symbol_counter;
    b->size_len = size_len;
    b->type = var_type_init(VAR_PFX(UNKNOWN), (var_type_body) {});
    memcpy(b->symbol, s->buffer + t->start_idx, token_len(t));
    return b;
}

symbol_table_bucket *symbol_table_findsert(symbol_table **table, symbol_table_type table_type, const token *const t, const string *const s) {
    // TODO resize
    // check if the symbol is in table
    size_t hash_idx = hash_symbol(t, s) % (*table)->size;
    if ((*table)->buckets[hash_idx] == NULL) {
        symbol_table_bucket *b = bucket_init(table_type, (*table)->symbol_counter++, t, s);
        (*table)->buckets[hash_idx] = b;
        return b;
    }
    symbol_table_bucket *b = (*table)->buckets[hash_idx];
    while (b != NULL) {
        symbol_table_bucket *tmp = compare_buckets(b, t, s);
        if (tmp != NULL) return tmp;
        b = b->next;
    }
    b->next = bucket_init(table_type, (*table)->symbol_counter++, t, s);
    return b->next;
}

symbol_table_bucket *symbol_table_find(symbol_table *table, const token *const t, const string *const s) {
    size_t hash_idx = hash_symbol(t, s) % table->size;
    if (table->buckets[hash_idx] == NULL) return NULL;
    symbol_table_bucket *b = table->buckets[hash_idx];
    while (b != NULL) {
        symbol_table_bucket *tmp = compare_buckets(b, t, s);
        if (tmp != NULL) return tmp;
        b = b->next;
    }
    return NULL;
}

extern inline var_type *var_type_init(var_type_header header, var_type_body body);

extern inline var_type *var_type_fn_init(size_t symbol_table_size);

void var_type_free(var_type *t) {
    switch (t->header) {
        case VAR_PFX(FN):
            var_type_free(t->body.fn->return_type);
            symbol_table_free(t->body.fn->symbols);
            free(t->body.fn);
            break;
        default:
            break;
    }
    free(t);
}
