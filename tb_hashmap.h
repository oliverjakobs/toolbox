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

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */
#ifdef TB_HASHMAP_IMPLEMENTATION

#include <string.h>

/* Table sizes must be powers of 2 */
#define TB_HASHMAP_SIZE_MIN               (1 << 5)    /* 32 */
#define TB_HASHMAP_SIZE_DEFAULT           (1 << 8)    /* 256 */
#define TB_HASHMAP_SIZE_MOD(map, val)     ((val) & ((map)->capacity - 1))

/* Limit for probing is 1/2 of table_size */
#define TB_HASHMAP_PROBE_LEN(map)         ((map)->capacity >> 1)
/* Return the next linear probe index */
#define TB_HASHMAP_PROBE_NEXT(map, index) TB_HASHMAP_SIZE_MOD(map, (index) + 1)

/* Check if index b is less than or equal to index a */
#define TB_HASHMAP_INDEX_LESS(map, a, b)  ((a) == (b) || (((b) - (a)) & ((map)->capacity >> 1)) != 0)

/* Enforce a maximum 0.75 load factor. */
static inline size_t tb_hashmap_table_calc_min_size(size_t num_entries) { return num_entries + (num_entries / 3); }

/* Calculate the optimal table size, given the specified max number of elements. */
static size_t tb_hashmap_table_calc_size(size_t num_entries)
{
    size_t table_size = tb_hashmap_table_calc_min_size(num_entries);

    /* Table size is always a power of 2 */
    size_t min_size = TB_HASHMAP_SIZE_MIN;
    while (min_size < table_size) min_size <<= 1;

    return min_size;
}

/* Get a valid hash table index from a key. */
static inline size_t tb_hashmap_calc_index(const tb_hashmap* map, const void* key) { return TB_HASHMAP_SIZE_MOD(map, map->hash(key)); }

/*
 * Return the next populated entry, starting with the specified one.
 * Returns NULL if there are no more valid entries.
 */
static tb_hashmap_entry* tb_hashmap_entry_get_populated(const tb_hashmap* map, tb_hashmap_entry* entry)
{
    for (; entry < &map->table[map->capacity]; ++entry)
        if (entry->key) return entry;
    return NULL;
}

static tb_hashmap_entry* tb_hashmap_alloc_table(tb_hashmap* map, size_t capacity)
{
    if (map->alloc) return map->alloc(map->allocator, capacity, sizeof(tb_hashmap_entry));
    else            return calloc(capacity, sizeof(tb_hashmap_entry));
}

static void tb_hashmap_free_table(tb_hashmap* map, tb_hashmap_entry* table)
{
    if (map->free)  map->free(map->allocator, table);
    else            free(table);
}

tb_hashmap_error tb_hashmap_init(tb_hashmap* map, tb_hashmap_hash hash, tb_hashmap_cmp cmp, size_t initial_capacity)
{
    if (!(map && hash && cmp)) return TB_HASHMAP_ERROR;

    /* Convert init size to valid table size */
    if (!initial_capacity)  map->capacity = TB_HASHMAP_SIZE_DEFAULT;
    else                    map->capacity = tb_hashmap_table_calc_size(initial_capacity);

    map->table = tb_hashmap_alloc_table(map, map->capacity);
    map->used = 0;

    if (!map->table) return TB_HASHMAP_ALLOC_ERROR;

    map->hash = hash;
    map->cmp = cmp;

    return TB_HASHMAP_OK;
}

void tb_hashmap_destroy(tb_hashmap* map)
{
    if (!map) return;

    tb_hashmap_clear(map);

    tb_hashmap_free_table(map, map->table);
    map->capacity = TB_HASHMAP_SIZE_DEFAULT;
    map->used = 0;
}

void tb_hashmap_clear(tb_hashmap* map)
{
    if (!map) return;

    if (map->entry_free)
    {
        for (tb_hashmap_iter* iter = tb_hashmap_iterator(map); iter; iter = tb_hashmap_iter_next(map, iter))
            map->entry_free(map->allocator, (tb_hashmap_entry*)iter);
    }
    map->used = 0;
    memset(map->table, 0, sizeof(tb_hashmap_entry) * map->capacity);
}

/*
 * Find the hashmap entry with the specified key, or an empty slot.
 * Returns NULL if the entire table has been searched without finding a match.
 */
static tb_hashmap_entry* tb_hashmap_find_entry(const tb_hashmap* map, const void* key, int find_empty)
{
    size_t probe_len = TB_HASHMAP_PROBE_LEN(map);
    size_t index = tb_hashmap_calc_index(map, key);

    /* Linear probing */
    for (size_t i = 0; i < probe_len; ++i)
    {
        tb_hashmap_entry* entry = &map->table[index];
        if (!entry->key) return find_empty ? entry : NULL;
        if (map->cmp(key, entry->key) == 0) return entry;

        index = TB_HASHMAP_PROBE_NEXT(map, index);
    }
    return NULL;
}

/*
 * Removes the specified entry and processes the proceeding entries to reduce the load factor and keep the
 * chain continuous. This is a required step for hash maps using linear probing.
 */
static void tb_hashmap_remove_entry(tb_hashmap* map, tb_hashmap_entry* removed_entry)
{
    size_t removed_index = (removed_entry - map->table);

    /* free memory */
    if (map->entry_free) map->entry_free(map->allocator, removed_entry);
    --map->used;

    /* Fill the free slot in the chain */
    size_t index = TB_HASHMAP_PROBE_NEXT(map, removed_index);
    for (size_t i = 1; i < map->capacity; ++i)
    {
        tb_hashmap_entry* entry = &map->table[index];
        if (!entry->key) break; /* Reached end of chain */

        size_t entry_index = tb_hashmap_calc_index(map, entry->key);
        /* Shift in entries with an index <= to the removed slot */
        if (TB_HASHMAP_INDEX_LESS(map, removed_index, entry_index))
        {
            memcpy(removed_entry, entry, sizeof(*removed_entry));
            removed_index = index;
            removed_entry = entry;
        }
        index = TB_HASHMAP_PROBE_NEXT(map, index);
    }
    /* Clear the last removed entry */
    memset(removed_entry, 0, sizeof(*removed_entry));
}

static tb_hashmap_error tb_hashmap_rehash(tb_hashmap* map, size_t new_capacity)
{
    if ((new_capacity >= TB_HASHMAP_SIZE_MIN) || ((new_capacity & (new_capacity - 1)) == 0))
        return TB_HASHMAP_ERROR;

    tb_hashmap_entry* new_table = tb_hashmap_alloc_table(map, new_capacity);
    if (!new_table) return TB_HASHMAP_ALLOC_ERROR;

    /* Backup old elements in case of rehash failure */
    size_t old_capacity = map->capacity;
    tb_hashmap_entry* old_table = map->table;

    map->capacity = new_capacity;
    map->table = new_table;

    /* Rehash */
    for (tb_hashmap_entry* entry = old_table; entry < &old_table[old_capacity]; ++entry)
    {
        if (!entry->val) continue; /* Only copy entries with value */

        tb_hashmap_entry* new_entry = tb_hashmap_find_entry(map, entry->key, 1);
        if (!new_entry)
        {
            /*
             * The load factor is too high with the new table size,
             * or a poor hash function was used.
             */
            map->capacity = old_capacity;
            map->table = old_table;
            tb_hashmap_free_table(map, new_table);
            return TB_HASHMAP_HASH_ERROR;
        }

        /* Shallow copy */
        new_entry->key = entry->key;
        new_entry->val = entry->val;
    }

    tb_hashmap_free_table(map, old_table);
    return TB_HASHMAP_OK;
}

void* tb_hashmap_insert(tb_hashmap* map, const void* key, void* value)
{
    if (!map) return NULL;

    /* Rehash with 2x capacity if load factor is approaching 0.75 */
    if (map->capacity <= tb_hashmap_table_calc_min_size(map->used))
        tb_hashmap_rehash(map, map->capacity << 1);

    tb_hashmap_entry* entry = tb_hashmap_find_entry(map, key, 1);
    if (!entry)
    {
        /*
         * Cannot find an empty slot. Either out of memory, or using a poor hash function. 
         * Attempt to rehash once to reduce chain length.
         */
        if (tb_hashmap_rehash(map, map->capacity << 1) != TB_HASHMAP_OK) return NULL;

        entry = tb_hashmap_find_entry(map, key, 1);
        if (!entry) return NULL;
    }

    /* Do not overwrite existing value */
    if (entry->val) return NULL;

    if (!map->entry_alloc)
    {
        entry->key = key;
        entry->val = value;
    }
    else if (!map->entry_alloc(map->allocator, entry, key, value))
    {
        /* clean up and return NULL */
        if (map->entry_free) map->entry_free(map->allocator, entry);
        return NULL;
    }

    ++map->used;
    return entry->val;
}

tb_hashmap_error tb_hashmap_remove(tb_hashmap* map, const void *key)
{
    if (!(map && key)) return TB_HASHMAP_ERROR;

    tb_hashmap_entry* entry = tb_hashmap_find_entry(map, key, 0);
    if (entry)
    {
        /* Clear the entry and make the chain contiguous */
        tb_hashmap_remove_entry(map, entry);
        return TB_HASHMAP_OK;
    }
    return TB_HASHMAP_KEY_NOT_FOUND;
}

void* tb_hashmap_find(const tb_hashmap* map, const void* key)
{
    if (!(map && key)) return NULL;

    tb_hashmap_entry* entry = tb_hashmap_find_entry(map, key, 0);

    return entry ? entry->val : NULL;
}

/* -------------------------------| Iterator |----------------------------------------------- */
tb_hashmap_iter* tb_hashmap_iterator(const tb_hashmap* map)
{
    if (!(map && map->used)) return NULL;
    return (tb_hashmap_iter*)tb_hashmap_entry_get_populated(map, map->table);
}

tb_hashmap_iter* tb_hashmap_iter_next(const tb_hashmap* map, const tb_hashmap_iter* iter)
{
    if (!(map && iter)) return NULL;
    return (tb_hashmap_iter*)tb_hashmap_entry_get_populated(map, ((tb_hashmap_entry*)iter) + 1);
}

tb_hashmap_iter* tb_hashmap_iter_remove(tb_hashmap* map, const tb_hashmap_iter* iter)
{
    if (!(map && iter)) return NULL;

    tb_hashmap_entry* entry = (tb_hashmap_entry*)iter;

    /* If the iterator is invalid return the next valid entry */
    if (!entry->key) return tb_hashmap_iter_next(map, iter); 

    tb_hashmap_remove_entry(map, entry);
    return (tb_hashmap_iter*)tb_hashmap_entry_get_populated(map, entry);
}

const void* tb_hashmap_iter_get_key(const tb_hashmap_iter* iter) { return iter ? (const void*)((tb_hashmap_entry*)iter)->key : NULL; }
void*       tb_hashmap_iter_get_val(const tb_hashmap_iter* iter) { return iter ? ((tb_hashmap_entry*)iter)->val : NULL; }

/* -------------------------------| Hash utilities |----------------------------------------- */
size_t tb_hash_string(const char* str)
{
    size_t hash = 0;

    for (const char* str_hash = str; *str_hash; ++str_hash)
    {
        hash += *str_hash;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

uint32_t tb_hash_uint32(uint32_t i)
{
    uint32_t hash = i;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = (hash >> 16) ^ hash;
    return hash;
}

uint64_t tb_hash_uint64(uint64_t i)
{
    uint64_t hash = i;
    hash = (hash ^ (hash >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    hash = (hash ^ (hash >> 27)) * UINT64_C(0x94d049bb133111eb);
    hash = hash ^ (hash >> 31);
    return hash;
}
#endif /* !TB_HASHMAP_IMPLEMENTATION */

/*
MIT License

Copyright (c) 2020 oliverjakobs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/