#ifndef TB_ARRAY_H
#define TB_ARRAY_H

#include <stdlib.h>

typedef unsigned char tb_array_base_type;

typedef enum
{
    TB_ARRAY_OK = 0,
    TB_ARRAY_ALLOC_ERROR
} tb_array_error;

typedef struct
{
    tb_array_base_type* data;
    size_t capacity;
    size_t used;

    float growth;

    size_t element_size;
} tb_array;

/* Allocates a new array and initializes it */
tb_array_error tb_array_alloc(tb_array* arr, size_t initial_size, size_t element_size, float growth);

/* Frees the array and all associated memory */
void tb_array_free(tb_array* arr);

/* To prevent the array from growing set growth to zero */
void tb_array_set_growth(tb_array* arr, float growth);

/* Resizes the set to new_size */
tb_array_error tb_array_resize(tb_array* arr, size_t new_size);

/* Resizes array to free unused capacity */
tb_array_error tb_array_shrink_to_fit(tb_array* arr);

/* Clears the array (sets arr->used to 0) */
void tb_array_clear(tb_array* arr);

/* Inserts an element at the end of the array and grows the array if necessary */
void* tb_array_push(tb_array* arr, void* element);

/* Inserts an element at the given index if the index is not out of bounds */
void* tb_array_insert(tb_array* arr, void* element, size_t index);

/* Removes the element at the given index */
void tb_array_remove(tb_array* arr, size_t index);

/* Returns the element at the given index */
void* tb_array_get(tb_array* arr, size_t index); 

/* Returns the first element */
void* tb_array_first(tb_array* arr);

/* Returns the last element */
void* tb_array_last(tb_array* arr);

/* sort array with qsort */
void tb_array_sort(tb_array* arr, int (*cmp)(const void*, const void*));

/* Searches the array with bsearch and returns the index of the element if found or arr->used else */
size_t tb_array_search(tb_array* arr, const void* element, int (*cmp)(const void*, const void*));

#endif /* !TB_ARRAY_H */
