#ifndef TB_MAP_H
#define TB_MAP_H

#include <stdint.h>

struct tb_map_iter;
typedef struct tb_map_iter tb_map_iter;

typedef enum
{
    TB_MAP_OK = 0,
    TB_MAP_ALLOC_ERROR,
    TB_MAP_KEY_DUPLICATE,
    TB_MAP_KEY_NOT_FOUND
} tb_map_error;

struct tb_map_entry;
typedef struct tb_map_entry tb_map_entry;

typedef struct
{
    tb_map_entry* root;
    void*   (*alloc_func)(const void*);
    void    (*free_func)(void*);
    int     (*cmp_func)(uint32_t,uint32_t);
} tb_map;

/*
 * Initializes the map and allocates all associated memory.
 */
tb_map_error tb_map_alloc(tb_map* map, int(*cmp_func)(uint32_t,uint32_t));

/*
 * Free the map and all associated memory.
 */
void tb_map_free(tb_map* map);

/*
 * Enable internal memory allocation and management of values.
 */
void tb_map_set_mem_funcs(tb_map* map, void* (*alloc_func)(const void*), void (*free_func)(void*));

/*
 * Remove all entries.
 */
void tb_map_clear(tb_map* map);

/*
 * Insert an new entry into a given map
 * Returns TB_MAP_OK on success and tb_map_error else
 */
tb_map_error tb_map_insert(tb_map* map, uint32_t key, void* value);

/*
 * Remove an entry with the given key from a given map
 * Returns TB_MAP_OK on success and tb_map_error else
 */
tb_map_error tb_map_remove(tb_map* map, uint32_t key);

/*
 * Find an entry in a given map
 * Returns the value of the found entry on success and NULL else
 */
void* tb_map_find(const tb_map* map, uint32_t key);

/*
 * Get a new map iterator.
 * The iterator is an opaque pointer that may be used with map_iter_*() 
 * functions.
 * Map iterators are INVALID after a remove operation is performed.
 * tb_map_iter_remove() allows safe removal during iteration.
 */
tb_map_iter* tb_map_iterator(const tb_map* map);

/*
 * Return an iterator to the next map entry.
 * Returns NULL if there are no more entries.
 */
tb_map_iter* tb_map_iter_next(const tb_map_iter* iter);

/*
 * Remove the map entry pointed to by this iterator and returns an
 * iterator to the next entry.
 * Returns NULL if there are no more entries.
 * 
 * ! NOT IMPLEMENTED YET !
 */
tb_map_iter* tb_map_iter_remove(tb_map* map, const tb_map_iter* iter);

/*
 * Return the key of the entry pointed to by the iterator.
 */
uint32_t tb_map_iter_get_key(const tb_map_iter* iter);

/*
 * Return the value of the entry pointed to by the iterator.
 */
void* tb_map_iter_get_value(const tb_map_iter* iter);

#endif /* !TB_MAP_H */