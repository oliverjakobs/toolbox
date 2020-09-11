#ifndef TB_LIST_H
#define TB_LIST_H

#include <stdlib.h>

typedef unsigned char tb_list_base_type;

typedef enum
{
    TB_LIST_OK = 0,
    TB_LIST_ALLOC_ERROR
} tb_list_error;

typedef struct
{
    tb_list_base_type* data;
    size_t capacity;
    size_t used;

    int (*cmp_func)(const void*,const void*);

    size_t element_size;
} tb_list;

/* Allocates a new list and initializes it */
tb_list_error tb_list_alloc(tb_list* list, size_t initial_size, size_t element_size, int (*cmp)(const void*, const void*));

/* Frees the list and all associated memory */
void tb_list_free(tb_list* list);

/* Resizes the list to new_size */
tb_list_error tb_list_resize(tb_list* list, size_t new_size);

/* Resizes list to free unused capacity */
tb_list_error tb_list_shrink_to_fit(tb_list* list);

/* Clears the array (lists list->used to 0) */
void tb_list_clear(tb_list* list);

/* Inserts an element at the end of the list if there is enough capacity */
void* tb_list_insert(tb_list* list, void* element);

/* Inserts an element at the end of the list and grows the list if necessary */
void* tb_list_insert_and_grow(tb_list* list, void* element, float growth);

/* Removes the element if the list cointains it */
void tb_list_remove(tb_list* list, const void* element);

/* Removes the element at the given index */
void tb_list_remove_at(tb_list* list, size_t index);

/* Searches the list with bsearch and returns the element if found or NULL else */
void* tb_list_find(const tb_list* list, const void* element);

/* Searches the list with bsearch and returns the index of the element if found or list->used else */
size_t tb_list_find_index(const tb_list* list, const void* element);

/* Returns the element at the given index */
void* tb_list_get(const tb_list* list, size_t index); 

/* Returns the first element */
void* tb_list_first(const tb_list* list);

/* Returns the last element */
void* tb_list_last(const tb_list* list);

#endif /* TB_LIST_H */
