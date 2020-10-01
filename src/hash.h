
#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "string.h"
#include "type.h"

typedef struct _var var;

typedef struct {
    string *key;
    var *value;
} hash_node;

typedef struct {
    size_t size, used;
    hash_node buckets[]; // rehash on collision
} hash;

#define HASH_STATUS_PFX(NAME) HASH_STATUS_##NAME

typedef enum {
    HASH_STATUS_PFX(OK),
    HASH_STATUS_PFX(KEY_NOT_IN_FIXED)
} hash_status;

#define DEFAULT_HASH_SIZE 10

#define MAX_REHASH 5

#define REHASH_SIZE_MULTIPLIER 2

inline hash *hash_init(size_t size) {
    hash *h = calloc(1, sizeof(hash) + sizeof(hash_node) * size);
    h->size = size;
    return h;
}
