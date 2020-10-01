
#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef struct _var var;

typedef struct {
    size_t key_len;
    var *value;
    char key[];
} hash_node;

typedef struct {
    bool fixed; // cannot resize all keys optional keys all added at init
    size_t size, used;
    hash_node buckets[]; // rehash on collision
} hash;

#define HASH_STATUS_PFX(NAME) HASH_STATUS_##NAME

typedef enum {
    HASH_STATUS_PFX(OK)
} hash_status;

#define MAX_REHASH 5

inline hash *hash_init(size_t size) {
    hash *h = calloc(1, sizeof(hash) + sizeof(hash_node) * size);
    h->size = size;
    return h;
}

hash_status hash_upsert(hash *const *h, const char *const key, const var *const v);
