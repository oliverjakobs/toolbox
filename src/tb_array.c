#include "tb_array.h"

#include <stdio.h>
#include <string.h>

void* tb_array__resize(void* buf, size_t elem_size, size_t new_cap, int shrink)
{
    if (!shrink && buf && new_cap < tb_array__cap(buf)) return buf;
    if (!buf && new_cap == 0) new_cap = 1;

    size_t* hdr = realloc(buf ? tb_array__hdr(buf) : NULL, TB_ARRAY_HDR_SIZE + (new_cap * elem_size));
    printf("Realloc %d\n", new_cap);

    if (!hdr) return NULL; /* out of memory */

    hdr[0] = new_cap;
    if (!buf) hdr[1] = 0;

    return hdr + 2;
}

void* tb_array__insert(void* buf, size_t elem_size, void* value, int (*cmp)(const void*,const void*))
{
    /* array needs to grow */
    if (!buf || tb_array__len(buf) >= tb_array__cap(buf))
    {
        buf = tb_array__resize(buf, tb_array__growth(buf), elem_size, 0);
        if (!buf) return NULL;
    }

    /* find location for the new element */
    size_t index = 0;
    if (buf && cmp)
    {
        while (index < tb_array__len(buf) && cmp(((char*)buf) + index, value) < 0)
            ++index;

        /* move entries back to make space for new element if index is not at the end */
        if (index < tb_array__len(buf))
        {
            size_t size = (tb_array__len(buf) - index) * elem_size;
            memcpy(((char*)buf) + (index + 1), ((char*)buf) + index, size);
        }
    }

    tb_array__len(buf)++;
    return memcpy(((char*)buf) + index, value, elem_size);
}

void* tb_array__remove(void* buf, size_t elem_size, size_t index)
{
    if (buf && tb_array__len(buf) > 0)
    {
        size_t size = (tb_array__len(buf) - (index + 1)) * elem_size;
        memcpy(((char*)buf) + index, ((char*)buf) + (index + 1), size);

        tb_array__len(buf)--;
    }
    return buf;
}