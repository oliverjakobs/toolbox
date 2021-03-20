#include "tb_hashmap.h"

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

/*
 * Enforce a maximum 0.75 load factor.
 */
static inline size_t tb_hashmap_table_calc_min_size(size_t num_entries)
{
    return num_entries + (num_entries / 3);
}

/*
 * Calculate the optimal table size, given the specified max number of elements.
 */
static size_t tb_hashmap_table_calc_size(size_t num_entries)
{
    size_t table_size = tb_hashmap_table_calc_min_size(num_entries);

    /* Table size is always a power of 2 */
    size_t min_size = TB_HASHMAP_SIZE_MIN;
    while (min_size < table_size)
        min_size <<= 1;

    return min_size;
}

/*
 * Get a valid hash table index from a key.
 */
static inline size_t tb_hashmap_calc_index(const tb_hashmap* map, const void* key)
{
    return TB_HASHMAP_SIZE_MOD(map, map->hash(key));
}

/*
 * Return the next populated entry, starting with the specified one.
 * Returns NULL if there are no more valid entries.
 */
static tb_hashmap_entry* tb_hashmap_entry_get_populated(const tb_hashmap* map, tb_hashmap_entry* entry)
{
    for (; entry < &map->table[map->capacity]; ++entry)
    {
        if (entry->key)
            return entry;
    }
    return NULL;
}

tb_hashmap_error tb_hashmap_alloc(tb_hashmap* map, size_t (*hash)(const void*), int (*cmp)(const void*, const void*), size_t initial_capacity)
{
    return tb_hashmap_allocf(map, hash, cmp, initial_capacity, calloc);
}

tb_hashmap_error tb_hashmap_allocf(tb_hashmap* map, size_t(*hash)(const void*), int(*cmp)(const void*, const void*), size_t initial_capacity, void* (*alloc)(size_t, size_t))
{
    if (!(map && hash && cmp)) return TB_HASHMAP_ERROR;

    /* Convert init size to valid table size */
    if (!initial_capacity)  map->capacity = TB_HASHMAP_SIZE_DEFAULT;
    else                    map->capacity = tb_hashmap_table_calc_size(initial_capacity);

    map->table = alloc(map->capacity, sizeof(tb_hashmap_entry));
    map->used = 0;

    if (!map->table) return TB_HASHMAP_ALLOC_ERROR;

    map->hash = hash;
    map->key_cmp = cmp;

    map->key_alloc = NULL;
    map->key_free = NULL;

    map->value_alloc = NULL;
    map->value_free = NULL;

    return TB_HASHMAP_OK;
}

static void tb_hashmap_free_keys(tb_hashmap* map)
{
    if (!map->key_free)
        return;

    for (tb_hashmap_iter* iter = tb_hashmap_iterator(map); iter; iter = tb_hashmap_iter_next(map, iter))
        map->key_free((void*)tb_hashmap_iter_get_key(iter));
}

static void tb_hashmap_free_values(tb_hashmap* map)
{
    if (!map->value_free)
        return;

    for (tb_hashmap_iter* iter = tb_hashmap_iterator(map); iter; iter = tb_hashmap_iter_next(map, iter))
        map->value_free((void*)tb_hashmap_iter_get_value(iter));
}

void tb_hashmap_free(tb_hashmap* map)
{
    tb_hashmap_freef(map, free);
}

void tb_hashmap_freef(tb_hashmap* map, void (*free_func)(void*))
{
    if (!map) return;

    tb_hashmap_free_keys(map);
    tb_hashmap_free_values(map);
    free_func(map->table);
    map->capacity = TB_HASHMAP_SIZE_DEFAULT;
    map->used = 0;
}

void tb_hashmap_set_key_alloc_funcs(tb_hashmap* map, void* (*alloc)(const void*), void (*free)(void*))
{
    if (!map) return;

    map->key_alloc = alloc;
    map->key_free = free;
}

void tb_hashmap_set_value_alloc_funcs(tb_hashmap* map, void* (*alloc)(const void*), void (*free)(void*))
{
    if (!map) return;

    map->value_alloc = alloc;
    map->value_free = free;
}

tb_hashmap_error tb_hashmap_rehash(tb_hashmap* map, size_t new_capacity)
{
    if ((new_capacity >= TB_HASHMAP_SIZE_MIN) || ((new_capacity & (new_capacity - 1)) == 0)) 
        return TB_HASHMAP_ERROR;

    tb_hashmap_entry* new_table = calloc(new_capacity, sizeof(tb_hashmap_entry));
    if (!new_table) return TB_HASHMAP_ALLOC_ERROR;

    /* Backup old elements in case of rehash failure */
    size_t old_capacity = map->capacity;
    tb_hashmap_entry* old_table = map->table;

    map->capacity = new_capacity;
    map->table = new_table;

    /* Rehash */
    for (tb_hashmap_entry* entry = old_table; entry < &old_table[old_capacity]; ++entry)
    {
        if (!entry->value) continue; /* Only copy entries with value */

        tb_hashmap_entry* new_entry = tb_hashmap_entry_find(map, entry->key, 1);
        if (!new_entry)
        {
            /*
             * The load factor is too high with the new table size,
             * or a poor hash function was used. 
             */
            map->capacity = old_capacity;
            map->table = old_table;
            free(new_table);
            return TB_HASHMAP_HASH_ERROR;
        }

        /* Shallow copy */
        new_entry->key = entry->key;
        new_entry->value = entry->value;
    }

    free(old_table);
    return TB_HASHMAP_OK;
}

void* tb_hashmap_insert(tb_hashmap* map, const void* key, void* value)
{
    if (!map) return NULL;

    /* Rehash with 2x capacity if load factor is approaching 0.75 */
    if (map->capacity <= tb_hashmap_table_calc_min_size(map->used))
        tb_hashmap_rehash(map, map->capacity << 1);

    tb_hashmap_entry* entry = tb_hashmap_entry_find(map, key, 1);
    if (!entry)
    {
        /*
         * Cannot find an empty slot.  Either out of memory, or using
         * a poor hash function.  Attempt to rehash once to reduce
         * chain length.
         */
        if (tb_hashmap_rehash(map, map->capacity << 1) != TB_HASHMAP_OK)
            return NULL;

        entry = tb_hashmap_entry_find(map, key, 1);
        if (!entry)
            return NULL;
    }
    if (!entry->key) 
    {
        /* Allocate copy of key to simplify memory management */
        if (map->key_alloc)
        {
            entry->key = map->key_alloc(key);
            if (!entry->key)
                return NULL;
        } 
        else 
        {
            entry->key = (void*)key;
        }
        ++map->used;
    }
    else if (entry->value)
    {
        /* Do not overwrite existing value */
        return entry->value;
    }

    if (map->value_alloc)
        entry->value = map->value_alloc(value);
    else
        entry->value = value;
    
    return entry->value ? value : NULL;
}

tb_hashmap_error tb_hashmap_remove(tb_hashmap* map, const void *key)
{
    if (!(map && key)) return TB_HASHMAP_ERROR;

    tb_hashmap_entry* entry = tb_hashmap_entry_find(map, key, 0);
    if (entry)
    {
        /* Clear the entry and make the chain contiguous */
        tb_hashmap_entry_remove(map, entry);
        return TB_HASHMAP_OK;
    }
    return TB_HASHMAP_KEY_NOT_FOUND;
}

void tb_hashmap_clear(tb_hashmap* map)
{
    if (!map) return;

    tb_hashmap_free_keys(map);
    map->used = 0;
    memset(map->table, 0, sizeof(tb_hashmap_entry) * map->capacity);
}

void* tb_hashmap_find(const tb_hashmap* map, const void* key)
{
    if (!map) return NULL;

    tb_hashmap_entry* entry = tb_hashmap_entry_find(map, key, 0);

    if (!entry) return NULL;

    return entry->value;
}

const void* tb_hashmap_get_key_ptr(const tb_hashmap* map, const void* key)
{
    if (!map) return NULL;

    tb_hashmap_entry* entry = tb_hashmap_entry_find(map, key, 0);

    if (!entry) return NULL;

    return entry->key;
}

tb_hashmap_entry* tb_hashmap_entry_find(const tb_hashmap* map, const void* key, int find_empty)
{
    size_t probe_len = TB_HASHMAP_PROBE_LEN(map);
    size_t index = tb_hashmap_calc_index(map, key);

    /* Linear probing */
    for (size_t i = 0; i < probe_len; ++i)
    {
        tb_hashmap_entry* entry = &map->table[index];
        if (!entry->key)
        {
            if (find_empty)
                return entry;

            return NULL;
        }

        if (map->key_cmp(key, entry->key) == 0)
            return entry;

        index = TB_HASHMAP_PROBE_NEXT(map, index);
    }
    return NULL;
}

void tb_hashmap_entry_remove(tb_hashmap* map, tb_hashmap_entry* removed_entry)
{
    size_t removed_index = (removed_entry - map->table);

    /* free memory */
    if (map->key_free)
        map->key_free(removed_entry->key);
    if (map->value_free)
        map->value_free(removed_entry->value);

    --map->used;

    /* Fill the free slot in the chain */
    size_t index = TB_HASHMAP_PROBE_NEXT(map, removed_index);
    for (size_t i = 1; i < map->capacity; ++i)
    {
        tb_hashmap_entry* entry = &map->table[index];
        if (!entry->key)
            break; /* Reached end of chain */

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

tb_hashmap_iter* tb_hashmap_iterator(const tb_hashmap* map)
{
    if (!map) return NULL;

    if (!map->used) return NULL;

    return (tb_hashmap_iter*)tb_hashmap_entry_get_populated(map, map->table);
}

tb_hashmap_iter* tb_hashmap_iter_next(const tb_hashmap* map, const tb_hashmap_iter* iter)
{
    if (!map) return NULL;

    if (!iter) return NULL;

    tb_hashmap_entry* entry = (tb_hashmap_entry*)iter;

    return (tb_hashmap_iter*)tb_hashmap_entry_get_populated(map, entry + 1);
}

tb_hashmap_iter* tb_hashmap_iter_remove(tb_hashmap* map, const tb_hashmap_iter* iter)
{
    if (!map) return NULL;

    if (!iter)
        return NULL;

    tb_hashmap_entry* entry = (tb_hashmap_entry*)iter;

    if (!entry->key)
        return tb_hashmap_iter_next(map, iter); /* Iterator is invalid, so just return the next valid entry */

    tb_hashmap_entry_remove(map, entry);
    return (tb_hashmap_iter*)tb_hashmap_entry_get_populated(map, entry);
}

const void* tb_hashmap_iter_get_key(const tb_hashmap_iter* iter)
{
    if (!iter) return NULL;

    return (const void*)((tb_hashmap_entry*)iter)->key;
}

void* tb_hashmap_iter_get_value(const tb_hashmap_iter* iter)
{
    if (!iter) return NULL;

    return ((tb_hashmap_entry*)iter)->value;
}

int tb_hashmap_str_cmp(const void* a, const void* b)
{
    return strcmp((const char*)a, (const char*)b);
}

void* tb_hashmap_str_alloc(const void* src)
{
    size_t size = strlen(src);
    char* dst = malloc(size + 1);

    if (!dst) return NULL;

    strcpy(dst, src);
    /* make sure string is null-terminated */
    dst[size] = '\0';

    return (void*)dst;
}

void tb_hashmap_str_free(void* block)
{
    free(block);
}

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
