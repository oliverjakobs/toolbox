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

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef TB_ARRAY_IMPLEMENTATION


#include <string.h>

tb_array_error tb_array_alloc(tb_array* arr, size_t initial_size, size_t element_size, float growth)
{
    arr->data = malloc(initial_size * element_size);

    if (!arr->data) return TB_ARRAY_ALLOC_ERROR;

    arr->capacity = initial_size;
    arr->used = 0;

    arr->growth = growth;

    arr->element_size = element_size;

    return TB_ARRAY_OK;
}

void tb_array_free(tb_array* arr)
{ 
    free(arr->data);
    arr->capacity = 0;
    arr->used = 0;
    arr->growth = 0.0f;
    arr->element_size = 0;
}

void tb_array_set_growth(tb_array* arr, float growth)
{
    arr->growth = growth >= 0.0f ? growth : 0.0f; 
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
    if (arr->used >= arr->capacity && arr->growth > 0.0f)
    {
        size_t size = (arr->capacity > 0) ? arr->capacity : 1;
        tb_array_resize(arr, (size_t)(size * arr->growth));
    }

    return tb_array_insert(arr, element, arr->used);
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


#endif /* !TB_ARRAY_IMPLEMENTATION */

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