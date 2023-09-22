#ifndef TB_ARRAY_H
#define TB_ARRAY_H

#include <stdlib.h>

#define TB_ARRAY_HDR_ELEM	size_t
#define TB_ARRAY_HDR_SIZE	2 * sizeof(TB_ARRAY_HDR_ELEM)

#define tb_array__hdr(b) ((size_t*)(void*)(b) - 2)
#define tb_array__cap(b) tb_array__hdr(b)[0]
#define tb_array__len(b) tb_array__hdr(b)[1]

#define tb_array_len(b) ((b) ? tb_array__len(b) : 0)
#define tb_array_cap(b) ((b) ? tb_array__cap(b) : 0)

#define tb_array_resize(b, n)   (*((void**)&(b)) = tb_array__resize((b), (n), sizeof(*(b))))
#define tb_array_reserve(b, n)  (*((void**)&(b)) = tb_array__reserve((b), (n), sizeof(*(b))))
#define tb_array_grow(b, n)     (*((void**)&(b)) = tb_array__grow((b), (n), sizeof(*(b))))

#define tb_array_push(b, v) (tb_array_grow((b), 1), (b)[tb_array__len(b)++] = (v))
#define tb_array_free(b)    (tb_array_resize(b, 0))

#define tb_array_pack(b)    (tb_array_resize((b), tb_array_len(b)))
#define tb_array_clear(b)   ((b) ? tb_array__len(b) = 0 : 0)

#define tb_array_last(b)    ((b) + tb_array_len(b))
#define tb_array_sizeof(b)  (tb_array_len(b) * sizeof(*(b)))

void* tb_array__resize(void* buf, size_t new_cap, size_t elem_size);
void* tb_array__reserve(void* buf, size_t reserve, size_t elem_size);
void* tb_array__grow(void* buf, size_t increment, size_t elem_size);

#endif /* !TB_ARRAY_H */

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */
#ifdef TB_ARRAY_IMPLEMENTATION

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
}#endif /* !TB_ARRAY_IMPLEMENTATION */

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