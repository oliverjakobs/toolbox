#include "tb_array.h"

void* tb_array__resize(void* buf, size_t new_cap, size_t elem_size)
{
    TB_ARRAY_HDR_ELEM* hdr = buf ? tb_array__hdr(buf) : NULL;

    if (hdr && new_cap == 0)
    {
        free(hdr);
        return NULL;
    }

    size_t new_size = TB_ARRAY_HDR_SIZE + (new_cap * elem_size);
    if (new_size <= TB_ARRAY_HDR_SIZE) return buf; /* removes compiler warning (C6386) */

    hdr = realloc(hdr, new_size);

    if (!hdr) return NULL; /* out of memory */

    hdr[0] = new_cap;
    if (!buf) hdr[1] = 0;

    return hdr + 2;
}

void* tb_array__grow(void* buf, size_t increment, size_t elem_size)
{
    if (buf && tb_array__len(buf) + increment < tb_array__cap(buf)) return buf;

    TB_ARRAY_HDR_ELEM new_size = tb_array_len(buf) + increment;
    TB_ARRAY_HDR_ELEM new_cap = buf ? 2 * tb_array__cap(buf) : 1;

    return tb_array__resize(buf, (new_cap < new_size) ? new_size : new_cap, elem_size);
}

void* tb_array__reserve(void* buf, size_t reserve, size_t elem_size)
{
    if (buf && reserve < tb_array__cap(buf)) return buf;
    return tb_array__resize(buf, reserve, elem_size);
}