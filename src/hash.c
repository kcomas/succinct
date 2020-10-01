
#include "hash.h"

extern inline hash *hash_init(size_t size);

static size_t hash_string(const char const *s) {
    size_t hash = 5381;
    char c;
    while ((c = *s++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

hash_status hash_upsert(hash *const *h, const char *const key, const var *const v) {

}
