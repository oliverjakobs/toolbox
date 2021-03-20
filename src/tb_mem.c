#include "tb_mem.h"

#include <string.h>

#define TB_MEM_HDR(block)   ((size_t*)(void*)(block) - 1)
#define TB_MEM_SIZE(block)  TB_MEM_HDR(block)[0]

void* tb_mem_alloc(tb_allocator* allocator, size_t size)
{
    size += sizeof(size_t);
    size_t* hdr = (allocator && allocator->alloc) ? allocator->alloc(size) : malloc(size);

    if (!hdr) return NULL;

    hdr[0] = size;

    return hdr + 1;
}

void* tb_mem_realloc(tb_allocator* allocator, void* block, size_t size)
{
    size_t* hdr = block ? TB_MEM_HDR(block) : NULL;
    size_t old_size = hdr ? hdr[0] : 0;

    size += sizeof(size_t);
    hdr = (allocator && allocator->realloc) ? allocator->realloc(hdr, old_size, size) : realloc(hdr, size);

    if (!hdr) return NULL;

    hdr[0] = size;

    return hdr + 1;
}

void tb_mem_free(tb_allocator* allocator, void* block)
{
    if (!block) return;

    size_t* hdr = TB_MEM_HDR(block);
    if (allocator && allocator->free)   allocator->free(hdr, hdr[0]);
    else                                free(hdr);
}

void* tb_mem_dup(tb_allocator* allocator, const void* src, size_t size)
{
    void* dst = tb_mem_alloc(allocator, size);
    
    return dst ? memcpy(dst, src, size) : NULL;
}
