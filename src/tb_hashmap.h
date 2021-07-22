#ifndef TB_HASHMAP_H
#define TB_HASHMAP_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct tb_hashmap_iter tb_hashmap_iter;
typedef struct tb_hashmap_entry tb_hashmap_entry;

typedef size_t (*tb_hashmap_hash) (const void* key);
typedef int    (*tb_hashmap_cmp)  (const void* left, const void* right);

typedef void* (*tb_hashmap_alloc)(void* allocator, size_t count, size_t size);
typedef void  (*tb_hashmap_free) (void* allocator, void* block);

typedef int  (*tb_hashmap_entry_alloc)(void* allocator, tb_hashmap_entry* entry, const void* key, void* value);
typedef void (*tb_hashmap_entry_free) (void* allocator, tb_hashmap_entry* entry);

typedef enum
{
    TB_HASHMAP_OK = 0,
    TB_HASHMAP_ERROR,
    TB_HASHMAP_ALLOC_ERROR,
    TB_HASHMAP_HASH_ERROR,
    TB_HASHMAP_KEY_NOT_FOUND
} tb_hashmap_error;

struct tb_hashmap_entry
{
    const void* key;
    void* val;
};

typedef struct
{
    tb_hashmap_entry* table;
    size_t capacity;
    size_t used;

    tb_hashmap_hash hash;
    tb_hashmap_cmp  cmp;

    /* memory */
    void* allocator;

    tb_hashmap_alloc alloc;
    tb_hashmap_free  free;

    tb_hashmap_entry_alloc entry_alloc;
    tb_hashmap_entry_free  entry_free;
} tb_hashmap;

/*
 * Initialize an empty hashmap.
 *
 * hash_func should return an even distribution of numbers between 0 and SIZE_MAX varying on the key provided.
 *
 * key_compare_func should return 0 if the keys match, and non-zero otherwise.
 *
 * initial_size is optional, and may be set to the max number of entries expected to be put in the hash table.
 * This is used as a hint to pre-allocate the hash table to the minimum size needed to avoid gratuitous rehashes. 
 * If initial_size is 0, a default size will be used.
 *
 * Returns TB_HASHMAP_OK on success and tb_hashmap_error on failure.
 */
tb_hashmap_error tb_hashmap_init(tb_hashmap* map, tb_hashmap_hash hash, tb_hashmap_cmp cmp, size_t initial_capacity);

/* Free the hashmap and all associated memory. */
void tb_hashmap_destroy(tb_hashmap* map);

/* Remove all entries. */
void tb_hashmap_clear(tb_hashmap* map);

/*
 * Insert an entry to the hashmap.  
 * If an entry with a matching key already exists and has a value pointer associated with it, NULL is returned, 
 * instead of assigning the new value.
 * Returns NULL if memory allocation failed.
 */
void* tb_hashmap_insert(tb_hashmap* map, const void* key, void* value);

/*
 * Remove an entry with the specified key from the map.
 * Returns TB_HASHMAP_KEY_NOT_FOUND if no entry was found, else TB_HASHMAP_OK.
 */
tb_hashmap_error tb_hashmap_remove(tb_hashmap* map, const void *key);

/* Return the value pointer, or NULL if no entry was found. */
void* tb_hashmap_find(const tb_hashmap* map, const void* key);

/*
 * Get a new hashmap iterator.
 * The iterator is an opaque pointer that may be used with hashmap_iter_*() functions.
 * Hashmap iterators are INVALID after a remove operation is performed.
 * hashmap_iter_remove() allows safe removal during iteration.
 */
tb_hashmap_iter* tb_hashmap_iterator(const tb_hashmap* map);

/*
 * Return an iterator to the next hashmap entry.
 * Returns NULL if there are no more entries.
 */
tb_hashmap_iter* tb_hashmap_iter_next(const tb_hashmap* map, const tb_hashmap_iter* iter);

/*
 * Remove the hashmap entry pointed to by this iterator and returns an iterator to the next entry.
 * Returns NULL if there are no more entries.
 */
tb_hashmap_iter* tb_hashmap_iter_remove(tb_hashmap* map, const tb_hashmap_iter* iter);

/* Return the key of the entry pointed to by the iterator. */
const void* tb_hashmap_iter_get_key(const tb_hashmap_iter* iter);

/* Return the value of the entry pointed to by the iterator. */
void* tb_hashmap_iter_get_val(const tb_hashmap_iter* iter);

/* Hash utilities */
size_t tb_hash_string(const char* str);

uint32_t tb_hash_uint32(uint32_t i);
uint64_t tb_hash_uint64(uint64_t i);

#endif /* !TB_HASHMAP_H */
