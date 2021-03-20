#include "tb_array.h"

void* tb_array__resize(void* buf, size_t new_cap, size_t elem_size)
{
    size_t* hdr = realloc(buf ? tb_array__hdr(buf) : NULL, TB_ARRAY_HDR_SIZE + (new_cap * elem_size));

    if (!hdr) return NULL; /* out of memory */

    hdr[0] = new_cap;
    if (!buf) hdr[1] = 0;

    return hdr + 2;
}

void* tb_array__grow(void* buf, size_t increment, size_t elem_size)
{
    if (buf && tb_array__len(buf) + increment < tb_array__cap(buf))
        return buf;

    size_t new_size = tb_array_len(buf) + increment;
    size_t new_cap = tb_array__max((buf ? 2 * tb_array__cap(buf) : 1), new_size);

    return tb_array__resize(buf, new_cap, elem_size);
}

void* tb_array__reserve(void* buf, size_t reserve, size_t elem_size)
{
    if (buf && reserve < tb_array__cap(buf))
        return buf;

    return tb_array__resize(buf, reserve, elem_size);
}