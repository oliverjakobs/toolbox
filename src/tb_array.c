#include "tb_array.h"

#include <stdio.h>
#include <string.h>

/*
 * if arr == NULL and new_cap == 0 -> new_cap = 1 (first initialization)
 * if arr != NULL and new_cap < old_cap and not shrink -> no need to resize
 * if realloc failed return NULL
 */
void* tb_array__resize(void* arr, size_t elem_size, size_t new_cap, int shrink)
{
    if (arr && new_cap < tb_array__cap(arr) && !shrink) return arr;
    if (!arr && new_cap == 0) new_cap = 1;

    size_t* hdr = realloc(arr ? tb_array__hdr(arr) : NULL, TB_ARRAY_HDR_SIZE + (new_cap * elem_size));
    printf("Realloc %d\n", new_cap);

    if (!hdr) return NULL; /* out of memory */

    hdr[0] = new_cap;
    if (!arr) hdr[1] = 0;

    return hdr + 2;
}

void* tb_array__insert(void* arr, size_t elem_size, void* value, int (*cmp)(const void*,const void*))
{
    /* array needs to grow */
    if (!arr || tb_array__len(arr) >= tb_array__cap(arr))
    {
        arr = tb_array__resize(arr, tb_array__growth(arr), elem_size, 0);
        if (!arr) return NULL;
    }

    /* find location for the new element */
    size_t index = 0;
    if (arr && cmp)
    {
        while (index < tb_array__len(arr) && cmp(((char*)arr) + index, value) < 0)
            ++index;

        /* move entries back to make space for new element if index is not at the end */
        if (index < tb_array__len(arr))
        {
            size_t size = (tb_array__len(arr) - index) * elem_size;
            memcpy(((char*)arr) + (index + 1), ((char*)arr) + index, size);
        }
    }

    tb_array__len(arr)++;
    return memcpy(((char*)arr) + index, value, elem_size);
}

void* tb_array__remove(void* arr, size_t elem_size, size_t index)
{
    if (arr && tb_array__len(arr) > 0)
    {
        size_t size = (tb_array__len(arr) - (index + 1)) * elem_size;
        memcpy(((char*)arr) + index, ((char*)arr) + (index + 1), size);

        tb_array__len(arr)--;
    }
    return arr;
}