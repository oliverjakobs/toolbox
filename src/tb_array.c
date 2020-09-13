#include "tb_array.h"

#include <string.h>

tb_array_error tb_array_alloc(tb_array* arr, size_t initial_size, size_t element_size)
{
    arr->data = malloc(initial_size * element_size);

    if (!arr->data) return TB_ARRAY_ALLOC_ERROR;

    arr->capacity = initial_size;
    arr->used = 0;

    arr->element_size = element_size;

    return TB_ARRAY_OK;
}

void tb_array_free(tb_array* arr)
{ 
    free(arr->data);
    arr->capacity = 0;
    arr->used = 0;
    arr->element_size = 0;
}

tb_array_error tb_array_resize(tb_array* arr, size_t new_size)
{
    tb_array_base_type* data = realloc(arr->data, new_size * arr->element_size);
    
    if (!data) return TB_ARRAY_ALLOC_ERROR;
    
    arr->data = data;
    arr->capacity = new_size;
    return TB_ARRAY_OK;
}

tb_array_error tb_array_shrink_to_fit(tb_array* arr)
{
    return tb_array_resize(arr, arr->used);
}

void tb_array_clear(tb_array* arr)
{
    arr->used = 0;
}

void* tb_array_push(tb_array* arr, void* element)
{
    return tb_array_insert(arr, element, arr->used);
}

void* tb_array_push_and_grow(tb_array* arr, void* element, float growth)
{
    if (arr->used >= arr->capacity)
    {
        size_t size = (arr->capacity > 0) ? arr->capacity : 1;
        tb_array_resize(arr, (size_t)(size * (double)growth));
    }

    return tb_array_push(arr, element);
}

void* tb_array_insert(tb_array* arr, void* element, size_t index)
{
    /* array is to small to insert */
    if (arr->used >= arr->capacity)
        return NULL;

    /* index is out of bounds (inserting would not result in a coherent array) */
    if (index > arr->used)
        return NULL;

    /* move entries back to make space for new element if index is not at the end */
    if (index < arr->used)
    {
        size_t size = (arr->used - index) * arr->element_size;
        size_t dest_offset = (index + 1) * arr->element_size;
        size_t src_offset = index * arr->element_size;

        memcpy(arr->data + dest_offset, arr->data + src_offset, size);
    }

    arr->used++;

    /* copy entry into the array */
    size_t offset = index * arr->element_size;
    return memcpy(arr->data + offset, element, arr->element_size);
}

void tb_array_remove(tb_array* arr, size_t index)
{
    if(index >= arr->used)
        return;
    
    size_t size = (arr->used - (index + 1)) * arr->element_size;
    size_t dest_offset = index * arr->element_size;
    size_t src_offset = (index + 1) * arr->element_size;

    memcpy(arr->data + dest_offset, arr->data + src_offset, size);

    arr->used--;
}

void* tb_array_get(tb_array* arr, size_t index)
{
    if (index >= arr->used) return NULL;

    return arr->data + index * arr->element_size;
}

void* tb_array_first(tb_array* arr)
{
    return tb_array_get(arr, 0);
}

void* tb_array_last(tb_array* arr)
{
    return tb_array_get(arr, arr->used - 1);
}

void tb_array_sort(tb_array* arr, int (*cmp)(const void*, const void*))
{
    qsort(arr->data, arr->used, arr->element_size, cmp);
}

size_t tb_array_search(tb_array* arr, const void* element, int (*cmp)(const void*, const void*))
{
    tb_array_base_type* found = bsearch(element, arr->data, arr->used, arr->element_size, cmp);

    if (!found) return arr->used;

    return (found - arr->data) / arr->element_size;
}
