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

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef TB_LIST_IMPLEMENTATION

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

#endif /* !TB_LIST_IMPLEMENTATION */

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