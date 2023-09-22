#ifndef TB_MEM_H
#define TB_MEM_H

#include <stdlib.h>

typedef void* (*tb_mem_malloc_callback)     (size_t size);
typedef void* (*tb_mem_realloc_callback)    (void* block, size_t old_size, size_t new_size);
typedef void  (*tb_mem_free_callback)       (void* block, size_t size);

typedef struct
{
    tb_mem_malloc_callback  malloc;
    tb_mem_realloc_callback realloc;
    tb_mem_free_callback    free;
} tb_allocator;

void* tb_mem_malloc(tb_allocator* allocator, size_t size);
void* tb_mem_calloc(tb_allocator* allocator, size_t count, size_t size);
void* tb_mem_realloc(tb_allocator* allocator, void* block, size_t size);
void  tb_mem_free(tb_allocator* allocator, void* block);
void  tb_mem_constfree(tb_allocator* allocator, const void* block);

void* tb_mem_dup(tb_allocator* allocator, const void* src, size_t size);

#endif /* !TB_MEM_H */

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */
#ifdef TB_MEM_IMPLEMENTATION

#include <string.h>

#define TB_MEM_HDR(block)   ((size_t*)(void*)(block) - 1)
#define TB_MEM_SIZE(block)  TB_MEM_HDR(block)[0]

void* tb_mem_malloc(tb_allocator* allocator, size_t size)
{
    size_t s = size + sizeof(size_t);
    size_t* hdr = (allocator && allocator->malloc) ? allocator->malloc(s) : malloc(s);

    if (hdr) { hdr[0] = s; return hdr + 1; }
    return NULL;
}

void* tb_mem_calloc(tb_allocator* allocator, size_t count, size_t size)
{
    size_t s = (size * count) + sizeof(size_t);
    if (!allocator || !allocator->malloc) return calloc(1, s);

    size_t* hdr = allocator->malloc(s);

    if (!hdr) return NULL;

    memset(hdr, 0, s);
    hdr[0] = s;
    return hdr + 1;
}

void* tb_mem_realloc(tb_allocator* allocator, void* block, size_t size)
{
    size_t* hdr = block ? TB_MEM_HDR(block) : NULL;
    size_t old_size = hdr ? hdr[0] : 0;

    size += sizeof(size_t);
    hdr = (allocator && allocator->realloc) ? allocator->realloc(hdr, old_size, size) : realloc(hdr, size);

    if (hdr) { hdr[0] = size; return hdr + 1; }
    return NULL;
}

void tb_mem_free(tb_allocator* allocator, void* block)
{
    if (!block) return;

    size_t* hdr = TB_MEM_HDR(block);
    if (allocator && allocator->free)   allocator->free(hdr, hdr[0]);
    else                                free(hdr);
}

void tb_mem_constfree(tb_allocator* allocator, const void* block)
{
    tb_mem_free(allocator, (void*)block);
}

void* tb_mem_dup(tb_allocator* allocator, const void* src, size_t size)
{
    void* dst = tb_mem_malloc(allocator, size);
    return dst ? memcpy(dst, src, size) : NULL;
}
#endif /* !TB_MEM_IMPLEMENTATION */

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