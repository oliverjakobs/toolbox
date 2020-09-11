#include "tb_list.h"

#include <string.h>
#include <stdio.h>

tb_list_error tb_list_alloc(tb_list* list, size_t initial_size, size_t element_size, int (*cmp)(const void*, const void*))
{
    list->data = malloc(initial_size * element_size);

    if (!list->data) return TB_LIST_ALLOC_ERROR;

    list->capacity = initial_size;
    list->used = 0;

    list->element_size = element_size;

    list->cmp_func = cmp;

    return TB_LIST_OK;
}

void tb_list_free(tb_list* list)
{
    free(list->data);
    list->capacity = 0;
    list->used = 0;
    list->element_size = 0;

    list->cmp_func = NULL;
}

tb_list_error tb_list_resize(tb_list* list, size_t new_size)
{
    tb_list_base_type* data = realloc(list->data, new_size * list->element_size);

    if (!data) return TB_LIST_ALLOC_ERROR;

    list->data = data;
    list->capacity = new_size;
    return TB_LIST_OK;
}

tb_list_error tb_list_shrink_to_fit(tb_list* list)
{
    return tb_list_resize(list, list->used);
}

void tb_list_clear(tb_list* list)
{
    list->used = 0;
}

void* tb_list_insert(tb_list* list, void* element)
{
    /* list is full */
    if (list->used >= list->capacity)
        return NULL;

    size_t index = 0;

    /* find location for the new element */
    while (index < list->used && list->cmp_func(tb_list_get(list, index), element) < 0)
        ++index;

    /* move entries back to make space for new element if index is not at the end */
    if (index < list->used)
    {
        size_t size = (list->used - index) * list->element_size;
        size_t dest_offset = (index + 1) * list->element_size;
        size_t src_offset = index * list->element_size;

        memcpy(list->data + dest_offset, list->data + src_offset, size);
    }
    
    list->used++;

    /* copy element into the list */
    size_t offset = index * list->element_size;
    return memcpy(list->data + offset, element, list->element_size);
}

void* tb_list_insert_and_grow(tb_list* list, void* element, float growth)
{
    if (list->used >= list->capacity)
    {
        size_t size = (list->capacity > 0) ? list->capacity : 1;
        if (tb_list_resize(list, (size_t)(size * (double)growth)) != TB_LIST_OK)
            return NULL;
    }

    return tb_list_insert(list, element);
}

void tb_list_remove(tb_list* list, const void* element)
{
    tb_list_remove_at(list, tb_list_find_index(list, element));
}

void tb_list_remove_at(tb_list* list, size_t index)
{
    if(index >= list->used) return;
    
    size_t size = (list->used - (index + 1)) * list->element_size;
    size_t dest_offset = index * list->element_size;
    size_t src_offset = (index + 1) * list->element_size;

    memcpy(list->data + dest_offset, list->data + src_offset, size);

    list->used--;
}

void* tb_list_find(const tb_list* list, const void* element)
{
    return bsearch(element, list->data, list->used, list->element_size, list->cmp_func);
}

size_t tb_list_find_index(const tb_list* list, const void* element)
{
    tb_list_base_type* found = tb_list_find(list, element);

    if (!found) return list->used;

    return (found - list->data) / list->element_size;
}

void* tb_list_get(const tb_list* list, size_t index)
{
    if (index >= list->used) return NULL;

    return list->data + index * list->element_size;
}

void* tb_list_first(const tb_list* list)
{
    return tb_list_get(list, 0);
}

void* tb_list_last(const tb_list* list)
{
    return tb_list_get(list, list->used - 1);
}